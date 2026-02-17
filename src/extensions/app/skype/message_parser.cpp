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
#include <mobius/core/decoder/json/parser.hpp>
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
                richtext_.add_text (text);
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

    else if (tag == "addmember")
        _parse_addmember ();

    else if (tag == "b")
        richtext_.begin_bold ();

    else if (tag == "contacts")
        _parse_contacts ();

    else if (tag == "c_i")
        _parse_c_i ();

    else if (tag == "deletemember")
        _parse_deletemember ();

    else if (tag == "files")
        _parse_files ();

    else if (tag == "flag")
        _parse_flag ();

    else if (tag == "historydisclosedupdate")
        _parse_historydisclosedupdate ();

    else if (tag == "i")
        richtext_.begin_italic ();

    else if (tag == "joiningenabledupdate")
        _parse_joiningenabledupdate ();

    else if (tag == "legacyquote")
        _parse_legacyquote ();

    else if (tag == "partlist")
        _parse_partlist ();

    else if (tag == "quote")
        _parse_quote ();

    else if (tag == "s")
        richtext_.begin_strikethrough ();

    else if (tag == "sms")
        _parse_sms ();

    else if (tag == "ss")
        _parse_ss ();

    else if (tag == "topicupdate")
        _parse_topicupdate ();

    else if (tag == "URIObject")
        _parse_uriobject ();

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
        richtext_.end_bold ();

    else if (tag == "i")
        richtext_.end_italic ();

    else if (tag == "quote")
        richtext_.end_message_bubble ();

    else if (tag == "s")
        richtext_.end_strikethrough ();

    else
        log.development (__LINE__, "Unhandled end tag: </" + tag + ">");
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
    richtext_.add_text (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <a> tag
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
    auto url = mobius::core::string::html_unescape (
        tag.get_attribute<std::string> ("href")
    );

    richtext_.begin_link (url);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <addmember> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_addmember ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <addmember> tag");
        return;
    }

    // Get children tags
    mobius::core::datetime::datetime timestamp;
    std::string initiator;
    std::string target;

    for (const auto &child : tag.get_children ())
    {
        auto child_name = child.get_name ();

        if (child_name == "eventtime")
        {
            auto timestamp_str = child.get_content ();
            timestamp =
                mobius::core::datetime::new_datetime_from_unix_timestamp (
                    std::stoll (timestamp_str) / 1000
                );
        }

        else if (child_name == "initiator")
            initiator = child.get_content ();

        else if (child_name == "target")
            target = child.get_content ();
    }

    // Format system message
    std::string text = std::format ("Member \"{}\"", target);

    if (initiator != "" && initiator != target)
        text += std::format (" added to chat by user \"{}\"", initiator);

    else
        text += " left chat";

    if (timestamp)
        text += std::format (" at {}", to_string (timestamp));

    richtext_.add_system_text (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <contacts> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_contacts ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <contacts> tag");
        return;
    }

    // Get contacts
    std::size_t contact_count = 0;
    std::string contact_list;

    for (const auto &child : tag.get_children ())
    {
        if (child.get_name () == "c")
        {
            auto type = child.get_attribute<std::string> ("t");
            if (type != "s")
                log.development (__LINE__, "Unhandled contact type: " + type);

            auto account_id = child.get_attribute<std::string> ("s");
            auto account_name = child.get_attribute<std::string> ("f");

            if (!contact_list.empty ())
                contact_list += "\n";

            contact_list += account_name.empty ()
                                ? account_id
                                : account_name + " (" + account_id + ")";

            contact_count++;
        }
    }

    // Add system message element
    if (contact_count == 1)
        richtext_.add_system_text (
            std::format ("Contact shared: {}", contact_list)
        );

    else if (contact_count > 1)
        richtext_.add_system_text (
            std::format (
                "Contacts shared ({}):\n{}", contact_count, contact_list
            )
        );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <c_i> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_c_i ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <c_i> tag");
        return;
    }

    auto id = tag.get_attribute<std::string> ("id");

    // Format system message
    richtext_.add_system_text (
        std::format ("<<Clickable Interactive Element (id={})>>", id)
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <deletemember> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_deletemember ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <deletemember> tag");
        return;
    }

    // Get children tags
    mobius::core::datetime::datetime timestamp;
    std::string initiator;
    std::string target;

    for (const auto &child : tag.get_children ())
    {
        auto child_name = child.get_name ();

        if (child_name == "eventtime")
        {
            auto timestamp_str = child.get_content ();
            timestamp =
                mobius::core::datetime::new_datetime_from_unix_timestamp (
                    std::stoll (timestamp_str) / 1000
                );
        }

        else if (child_name == "initiator")
            initiator = child.get_content ();

        else if (child_name == "target")
            target = child.get_content ();
    }

    // Format system message
    std::string text = std::format ("Member \"{}\"", target);

    if (initiator != "" && initiator != target)
        text += std::format (" removed from chat by user \"{}\"", initiator);

    else
        text += " left chat";

    if (timestamp)
        text += std::format (" at {}", to_string (timestamp));

    richtext_.add_system_text (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <files> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_files ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <sms> tag");
        return;
    }

    // Get files
    std::size_t total_size = 0;
    std::size_t file_count = 0;
    std::string file_list;

    for (const auto &child : tag.get_children ())
    {
        if (child.get_name () == "file")
        {
            auto size = stoll (child.get_attribute<std::string> ("size", "0"));
            auto file_name = child.get_content ();

            if (!file_list.empty ())
                file_list += ", ";

            file_list += file_name;

            total_size += size;
            file_count++;
        }
    }

    // Add system message element
    if (file_count == 1)
        richtext_.add_system_text (
            std::format (
                "File sent: {} (Size: {} bytes)", file_list, total_size
            )
        );

    else if (file_count > 1)
        richtext_.add_system_text (
            std::format (
                "Files sent: {} (Total size: {} bytes)", file_list, total_size
            )
        );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <flag> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_flag ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <flag> tag");
        return;
    }

    // Get country
    auto country = tag.get_attribute<std::string> ("country");

    // Add element
    richtext_.add_flag (country);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <historydisclosedupdate> tag
// @see https://docs.microsoft.com/en-us/javascript/api/botbuilder/iconversationupdate?view=botbuilder-ts-3.0#botbuilder-iconversationupdate-historydisclosed
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_historydisclosedupdate ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <historydisclosedupdate> tag");
        return;
    }

    // Get children tags
    mobius::core::datetime::datetime timestamp;
    std::string initiator;
    std::string value;

    for (const auto &child : tag.get_children ())
    {
        auto child_name = child.get_name ();

        if (child_name == "eventtime")
        {
            auto timestamp_str = child.get_content ();
            timestamp =
                mobius::core::datetime::new_datetime_from_unix_timestamp (
                    std::stoll (timestamp_str) / 1000
                );
        }

        else if (child_name == "initiator")
            initiator = child.get_content ();

        else if (child_name == "value")
            value = child.get_content ();
    }

    // Format system message
    std::string text;
    if (value == "true")
        text = "Chat history disclosure enabled";

    else
        text = "Chat history disclosure disabled";

    if (!initiator.empty ())
        text += std::format (" by user \"{}\"", initiator);

    if (timestamp)
        text += std::format (" at {}", to_string (timestamp));

    text += '.';

    richtext_.add_system_text (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <joiningenabledupdate> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_joiningenabledupdate ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <joiningenabledupdate> tag");
        return;
    }

    // Get children tags
    mobius::core::datetime::datetime timestamp;
    std::string initiator;
    std::string value;

    for (const auto &child : tag.get_children ())
    {
        auto child_name = child.get_name ();

        if (child_name == "eventtime")
        {
            auto timestamp_str = child.get_content ();
            timestamp =
                mobius::core::datetime::new_datetime_from_unix_timestamp (
                    std::stoll (timestamp_str) / 1000
                );
        }

        else if (child_name == "initiator")
            initiator = child.get_content ();

        else if (child_name == "value")
            value = child.get_content ();
    }

    // Format system message
    std::string text;
    if (value == "true")
        text = "Joining enabled";

    else
        text = "Joining disabled";

    if (!initiator.empty ())
        text += std::format (" by user \"{}\"", initiator);

    if (timestamp)
        text += std::format (" at {}", to_string (timestamp));

    text += '.';

    richtext_.add_system_text (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <legacyquote> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_legacyquote ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <flag> tag");
        return;
    }

    std::ignore = tag; // ignore data
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <partlist> tag
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
                text += "Participants: ";

            else
                text += ", ";

            text += name.empty () ? identity : name + " (" + identity + ")";
            participant_count++;
        }
    }

    if (participant_count == 0)
        text += "No participants.";

    // Add system message element
    richtext_.add_system_text (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <quote> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_quote ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get last element
    auto e = parser_.get_last ();
    if (e.get_type () !=
            mobius::core::decoder::sgml::parser::element::type::start_tag ||
        e.get_text () != "quote")
    {
        log.warning (__LINE__, "Invalid <quote> tag");
        return;
    }

    // Get attributes
    auto timestamp = e.get_attribute<std::string> ("timestamp");
    auto author_id = e.get_attribute<std::string> ("author");
    auto author_name = e.get_attribute<std::string> ("authorname");

    std::string author = author_id;
    if (!author_name.empty ())
        author += " (" + author_name + ")";

    // Start message bubble
    richtext_.begin_message_bubble (timestamp, author);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <sms> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_sms ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <sms> tag");
        return;
    }

    // Get targets and timestamp
    std::string targets;
    std::string msg = tag.get_attribute<std::string> ("alt");
    mobius::core::datetime::datetime timestamp;

    for (const auto &child : tag.get_children ())
    {
        auto child_name = child.get_name ();

        if (child_name == "sendtimestamp")
        {
            auto timestamp_str = child.get_content ();
            timestamp =
                mobius::core::datetime::new_datetime_from_unix_timestamp (
                    std::stoll (timestamp_str)
                );
        }

        else if (child_name == "targets")
        {
            for (const auto &target : child.get_children ())
            {
                if (target.get_name () == "target")
                {
                    if (!targets.empty ())
                        targets += ", ";
                    targets += target.get_content ();
                }
            }
        }
    }

    // Format system message text
    std::string text = "SMS message sent";

    if (timestamp)
        text += " at " + to_string (timestamp) + " (UTC)";

    if (!targets.empty ())
        text += " to " + targets;

    if (!msg.empty ())
        text += ": " + msg;

    text += ".";

    // Add system message element
    richtext_.add_system_text (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <ss> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_ss ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <ss> tag");
        return;
    }

    // Add emoji segment
    auto ss_type = tag.get_attribute<std::string> ("type");
    richtext_.add_emoji (ss_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <topicupdate> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_topicupdate ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <topicupdate> tag");
        return;
    }

    // Get children tags
    mobius::core::datetime::datetime timestamp;
    std::string initiator;
    std::string value;

    for (const auto &child : tag.get_children ())
    {
        auto child_name = child.get_name ();

        if (child_name == "eventtime")
        {
            auto timestamp_str = child.get_content ();
            timestamp =
                mobius::core::datetime::new_datetime_from_unix_timestamp (
                    std::stoll (timestamp_str) / 1000
                );
        }

        else if (child_name == "initiator")
            initiator = child.get_content ();

        else if (child_name == "value")
            value = child.get_content ();
    }

    // Format system message
    std::string text = std::format ("Topic updated to \"{}\"", value);

    if (!initiator.empty ())
        text += std::format (" by user \"{}\"", initiator);

    if (timestamp)
        text += std::format (" at {}", to_string (timestamp));

    text += '.';

    richtext_.add_system_text (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <URIObject> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_uriobject ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <URIObject> tag");
        return;
    }

    // Get attributes
    auto type = tag.get_attribute<std::string> ("type");
    auto uri = tag.get_attribute<std::string> ("uri");
    std::string content = tag.get_content ();
    std::size_t filesize = 0;
    std::string title;
    std::string description;
    std::string name;

    // Get children
    for (const auto &child : tag.get_children ())
    {
        auto child_name = child.get_name ();

        if (child_name == "FileSize")
            filesize = stoll (child.get_attribute<std::string> ("v", "0"));

        else if (child_name == "Title")
            title = child.get_content ();

        else if (child_name == "Description")
            description = child.get_content ();

        else if (child_name == "OriginalName")
            name = child.get_attribute<std::string> ("v");

        else if (child_name == "meta")
            name = child.get_attribute<std::string> ("originalName");

        else if (child_name == "a")
            ; // ignore link in URIObject

        else
            log.development (
                __LINE__, "Unhandled <URIObject> child tag: " + child_name
            );
    }

    // Create element
    std::string text = std::format ("File \"{}\" shared", name);

    if (!type.empty ())
        text += std::format ("\nType: {}", type);

    if (filesize > 0)
        text += std::format ("\nSize: {} bytes", filesize);

    if (!title.empty ())
        text += std::format ("\nTitle: {}", title);

    if (!description.empty ())
        text += std::format ("\nDescription: {}", description);

    text += std::format ("\nURI: {}", uri);

    if (!content.empty ())
        text += std::format ("\nText: {}", content);

    richtext_.add_system_text (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse Skype message
// @param message Message string
// @return Parsed content
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::richtext
parse_message (const std::string &message)
{
    mobius::core::richtext richtext;
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        message_parser parser (message);
        parser.parse ();

        richtext = parser.get_richtext ();

        if (!richtext)
            richtext.add_text (message);
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }

    return richtext;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse Notice message
// @param message Message string
// @return Parsed content
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::richtext
parse_notice (const std::string &message)
{
    mobius::core::richtext richtext;

    auto parser = mobius::core::decoder::json::parser (message);
    auto l = parser.parse ().to_list ();
    std::string text;

    for (const auto &item : l)
    {
        auto data = item.to_map ();
        auto language = data.get<std::string> ("language");
        auto client_version = data.get<std::string> ("clientVersion");

        // Add text
        if (!text.empty ())
            text += "\n";

        text += "Notice received.";

        if (!language.empty ())
            text += "\n  Language: " + language + ".";

        if (!client_version.empty ())
            text += "\n  Client version: " + client_version + ".";

        // Attachments
        auto attachments =
            data.get_list<mobius::core::pod::map> ("attachments");

        if (!attachments.empty ())
            text += "\n  Attachments:";

        for (const auto &attachment : attachments)
        {
            auto icon_url = attachment.get<std::string> ("iconUrl");
            auto content = attachment.get_map ("content");
            auto title = content.get<std::string> ("title");
            auto modal_title = content.get<std::string> ("modalTitle");
            auto content_text = content.get<std::string> ("text");
            auto main_action_uri = content.get<std::string> ("mainActionUri");

            if (!title.empty ())
                text += "\n    Title: " + title;

            if (!modal_title.empty ())
                text += "\n    Modal Title: " + modal_title;

            if (!content_text.empty ())
                text += "\n    Text: " + content_text;

            if (!main_action_uri.empty ())
                text += "\n    Action URI: " + main_action_uri;

            if (!icon_url.empty ())
                text += "\n    Icon URL: " + icon_url;
        }
    }

    // Create message element
    if (text.empty ())
        text = "Notice received.";

    richtext.add_system_text (text);
    return richtext;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse Popcard message
// @param message Message string
// @return Parsed content
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::richtext
parse_popcard (const std::string &message)
{
    mobius::core::richtext richtext;

    auto parser = mobius::core::decoder::json::parser (message);
    auto l = parser.parse ().to_list ();

    if (!l.empty ())
    {
        auto data = l[0].to_map ();
        auto content = data.get_map ("content");
        auto text = content.get<std::string> ("text");

        richtext.add_system_text ("Popcard received: " + text);
    }

    return richtext;
}

} // namespace mobius::extension::app::skype
