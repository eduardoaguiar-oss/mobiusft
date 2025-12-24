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
#include <mobius/core/decoder/sgml/parser.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/string_functions.hpp>
#include <stdexcept>

namespace mobius::core::decoder::sgml
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse tag with attributes
// @param type Tag type (empty_tag or start_tag)
// @param text Text
// @return Start tag element
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static parser::element
_parse_tag_with_attributes (parser::element::type type, const std::string &text)
{
    std::size_t pos = 1;
    std::size_t len = (type == parser::element::type::start_tag)
                          ? text.size () - 1
                          : text.size () - 2;

    std::string tag_name;
    std::string attr_name;
    std::string attr_value;
    mobius::core::pod::map attributes;
    char attr_value_char = 0;

    int state = 0;

    while (pos < len)
    {
        char c = text[pos];

        // state: beginning
        if (state == 0)
        {

            if (isupper (c) || islower (c))
            {
                tag_name.push_back (c);
                state = 1;
            }

            else
                throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid tag"));
        }

        // state: tag_name
        else if (state == 1)
        {
            if (islower (c) || isupper (c) || isdigit (c) || c == '.' ||
                c == '-' || c == '_')
                tag_name.push_back (c);

            else if (isspace (c))
                state = 2;

            else
                throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid tag"));
        }

        // state: attr start
        else if (state == 2)
        {
            attr_name.clear ();
            attr_value.clear ();

            if (islower (c) || isupper (c) || c == '_')
            {
                attr_name.push_back (c);
                state = 3;
            }

            else if (!isspace (c))
                throw std::runtime_error (
                    MOBIUS_EXCEPTION_MSG ("invalid attribute")
                );
        }

        // state: in attr_name
        else if (state == 3)
        {
            if (islower (c) || isupper (c) || isdigit (c) || c == '-' ||
                c == ':' || c == '.' || c == '_')
                attr_name.push_back (c);

            else if (c == '=')
                state = 5;

            else if (isspace (c))
                state = 4;

            else
                throw std::runtime_error (
                    MOBIUS_EXCEPTION_MSG ("invalid attribute")
                );
        }

        // state: after attr_name
        else if (state == 4)
        {
            if (c == '=')
                state = 5;

            else if (!isspace (c))
                state = 2;
        }

        // state: after '='
        else if (state == 5)
        {
            if (c == '\'' || c == '"')
            {
                attr_value_char = c;
                state = 6;
            }

            else if (!isspace (c))
            {
                attr_value.push_back (c);
                state = 7;
            }
        }

        // state: in "" or ''
        else if (state == 6)
        {
            if (c == attr_value_char)
            {
                attributes.set (attr_name, attr_value);
                state = 2;
            }

            else
                attr_value.push_back (c);
        }

        // state: in attr_value without " or '
        else if (state == 7)
        {
            if (isspace (c))
            {
                attributes.set (attr_name, attr_value);
                state = 2;
            }

            else if (c == '\'' || c == '"' || c == '=')
                throw std::runtime_error (
                    MOBIUS_EXCEPTION_MSG ("invalid attribute value")
                );

            else
                attr_value.push_back (c);
        }

        pos++;
    }

    if (tag_name.empty ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("unname tag"));

    return parser::element (type, tag_name, attributes);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse start_tag element
// @param text Text
// @return Start tag element
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static parser::element
_parse_start_tag (const std::string &text)
{
    parser::element e;

    // comment
    if (mobius::core::string::startswith (text, "<!--"))
    {
        if (!mobius::core::string::endswith (text, "-->"))
            throw std::runtime_error (
                MOBIUS_EXCEPTION_MSG ("unterminated comment")
            );

        e = parser::element (
            parser::element::type::comment, text.substr (4, text.size () - 7)
        );
    }

    // declaration
    else if (mobius::core::string::startswith (text, "<!"))
    {
        e = parser::element (
            parser::element::type::declaration,
            text.substr (2, text.size () - 3)
        );
    }

    // proper start tag <tag[ value1 value2]>
    else
    {
        e = _parse_tag_with_attributes (parser::element::type::start_tag, text);
    }

    return e;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create parser
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
parser::parser (const mobius::core::io::reader &reader)
    : tokenizer_ (reader)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get element
// @return SGML element
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
parser::element
parser::get ()
{
    element e;

    auto p = tokenizer_.get_token ();
    auto type = p.first;
    auto text = p.second;

    using token_type = mobius::core::decoder::sgml::tokenizer::token_type;

    switch (type)
    {
        case token_type::text:
            e = element (element::type::text, text);
            break;

        case token_type::start_tag:
            e = _parse_start_tag (text);
            break;

        case token_type::end_tag:
            e = element (
                element::type::end_tag, text.substr (2, text.size () - 3)
            );
            break;

        case token_type::empty_tag:
            e = _parse_tag_with_attributes (element::type::empty_tag, text);
            break;

        case token_type::entity:
            e = element (
                element::type::entity, text.substr (1, text.size () - 2)
            );
            break;

        case token_type::end:
            break;
    }

    last_ = e;

    return e;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Tag constructor
// @param parser SGML parser
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
parser::tag::tag (parser &parser)
{
    // Get start tag
    auto e = parser.get_last ();

    if (e.get_type () != element::type::start_tag &&
        e.get_type () != element::type::empty_tag)
        return;     // invalid tag

    // Get tag name and attributes
    name_ = e.get_text ();
    attributes_ = e.get_attributes ();

    if (e.get_type () == element::type::empty_tag)
        return;

    // Get children tags and content
    e = parser.get ();

    while (e.get_type () != element::type::end &&
           (e.get_type () != element::type::end_tag || e.get_text () != name_))
    {
        if (e.get_type () == element::type::text)
            content_ = mobius::core::string::strip (content_ + e.get_text ());

        else if (e.get_type () == element::type::start_tag)
        {
            tag child_tag (parser);
            children_.push_back (child_tag);
        }

        e = parser.get ();
    }
}

} // namespace mobius::core::decoder::sgml
