// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008-2026 Eduardo Aguiar
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "message_parser.hpp"
#include <mobius/core/io/bytearray_io.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <format>

namespace mobius::extension::app::skype
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param message Message string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
message_parser::message_parser (const std::string &message)
    : parser_ (mobius::core::io::new_bytearray_reader (message))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add element to content list
// @param element Element to add
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::add_element (const mobius::core::pod::map &element)
{
    const auto element_type = element.get<std::string> ("type");
    const auto element_text =
        mobius::core::string::strip (element.get<std::string> ("text"));

    // Check if text or system message is empty
    if ((element_type == "text" || element_type == "system") &&
        element_text.empty ())
        return;

    // Try to merge text or system message with previous element
    if (content_.size () > 0)
    {
        auto p_element = content_.back ();
        const auto p_type = p_element.get<std::string> ("type");
        const auto p_text = p_element.get<std::string> ("text");

        if (p_type == "text" && element_type == "text")
            p_element.set ("text", p_text + element_text);

        else if (p_type == "system" &&
                 (element_type == "system" || element_type == "text"))
            p_element.set ("text", p_text + ". " + element_text);

        else
            content_.push_back (element);
    }

    // Add new element
    else
        content_.push_back (element);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::parse ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    using element_type = mobius::core::decoder::sgml::parser::element::type;
    auto e = parser_.get ();

    while (e.get_type () != element_type::end)
    {
        auto text = e.get_text ();

        switch (e.get_type ())
        {
            case element_type::text:
                _add_text_element (text);
                break;

            case element_type::start_tag:
                _parse_start_tag (text);
                break;

            case element_type::end_tag:
                _parse_end_tag (text);
                break;

            case element_type::empty_tag:
                _parse_empty_tag (text);
                break;

            case element_type::entity:
                _parse_entity (text);
                break;

            default:;
        };

        e = parser_.get ();
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse start tag
// @param tag Tag name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_start_tag (const std::string &tag)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (tag == "a")
        _parse_a ();

    else if (tag == "b")
        add_element (mobius::core::pod::map {{"type", "start/b"}});

    else if (tag == "i")
        add_element (mobius::core::pod::map {{"type", "start/i"}});

    else if (tag == "partlist")
        _parse_partlist ();

    else if (tag == "quote")
        _parse_quote ();
        
    else if (tag == "s")
        add_element (mobius::core::pod::map {{"type", "start/s"}});

    else
        log.development (__LINE__, "Unhandled start tag: <" + tag + ">");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse end tag
// @param tag Tag name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_end_tag (const std::string &tag)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    mobius::core::pod::map element;

    if (tag == "b")
        element = mobius::core::pod::map {{"type", "end/b"}};

    else if (tag == "i")
        element = mobius::core::pod::map {{"type", "end/i"}};

    else if (tag == "quote")
        element = mobius::core::pod::map {{"type", "end/quote"}};

    else if (tag == "s")
        element = mobius::core::pod::map {{"type", "end/s"}};

    else
        log.development (__LINE__, "Unhandled end tag close </" + tag + ">");

    if (element)
        add_element (element);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse empty tag
// @param tag Tag name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_empty_tag (const std::string &tag)
{
    mobius::core::log log (__FILE__, __FUNCTION__);
    log.development (__LINE__, "Unhandled empty tag: " + tag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse entity and add to content
// @param entity
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_entity (const std::string &entity)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    std::string text;

    // Handle predefined entities
    if (entity == "lt")
        text = "<";

    else if (entity == "gt")
        text = ">";

    else if (entity == "amp")
        text = "&";

    else if (entity == "apos")
        text = "'";

    else if (entity == "quot")
        text = "\"";

    // Unhandled entity
    else
    {
        log.development (__LINE__, "Unhandled entity: " + entity);
        return;
    }

    // Add text element
    _add_text_element (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse a tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_a ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <a> tag");
        return;
    }

    // Add href element
    auto href = tag.get_attribute<std::string> ("href");

    add_element (mobius::core::pod::map {{"type", "href"}, {"url", href}});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse partlist tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_partlist ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <partlist> tag");
        return;
    }

    // Get participants
    std::size_t participant_count = 0;
    std::string text;

    for (const auto &child : tag.get_children ())
    {
        if (child.get_name () == "part")
        {
            auto identity = child.get_attribute<std::string> ("identity");

            auto name_tag = child.get_child ("name");
            auto name = name_tag ? name_tag.get_content () : std::string ();

            if (participant_count == 0)
                text += " Participants: ";

            else
                text += ", ";

            text += name.empty () ? identity : name + " (" + identity + ")";
            participant_count++;
        }
    }

    if (participant_count == 0)
        text += " No participants.";

    // Add system message element
    add_system_element (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse quote tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_quote ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <quote> tag");
        return;
    }

    // Get attributes
    auto timestamp = tag.get_attribute<std::string> ("timestamp");
    auto author_id = tag.get_attribute<std::string> ("author");
    auto author_name = tag.get_attribute<std::string> ("authorname");

    std::string author = author_id;
    if (!author_name.empty ())
        author += " (" + author_name + ")";

    auto element = mobius::core::pod::map {
        {"type", "start/quote"},
        {"author", author},
    };

    if (!timestamp.empty ())
        element.set (
            "timestamp",
            mobius::core::datetime::new_datetime_from_unix_timestamp (
                std::stoul (timestamp)
            )
        );

    add_element (element);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse Skype message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<mobius::core::pod::map>
parse_message (const std::string &message)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        message_parser parser (message);
        parser.parse ();

        auto content = parser.get_content ();

        if (content.empty ())
            content = {
                mobius::core::pod::map {{"type", "text"}, {"text", message}}
            };

        return content;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
        return {};
    }
}

} // namespace mobius::extension::app::skype
