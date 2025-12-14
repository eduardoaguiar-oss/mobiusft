// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
// Eduardo Aguiar
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
#include <mobius/core/decoder/json/tokenizer.hpp>
#include <mobius/core/io/bytearray_io.hpp>
#include <cctype>
#include <string>

namespace mobius::core::decoder::json
{

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create tokenizer
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
tokenizer::tokenizer (const mobius::core::io::reader &reader)
    : sourcecode_ (reader)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create tokenizer
// @param bytearray Bytearray object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
tokenizer::tokenizer (const mobius::core::bytearray &bytearray)
    : sourcecode_ (mobius::core::io::new_bytearray_reader (bytearray))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get token
// @return Token type and token text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::pair<tokenizer::token_type, std::string>
tokenizer::get_token ()
{
    while (true)
    {
        auto c = sourcecode_.peek ();

        // End of input
        if (c == 0)
            return {token_type::end, ""};

        // Whitespace
        else if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
        {
            auto text = _get_whitespace_token ();

            if (!ignore_whitespace_)
                return {token_type::whitespace, text};
        }

        // Punctuation
        else if (c == '{')
            return {token_type::left_brace, std::string (1, sourcecode_.get ())};

        else if (c == '}')
            return {token_type::right_brace, std::string (1, sourcecode_.get ())};

        else if (c == '[')
            return {token_type::left_bracket, std::string (1, sourcecode_.get ())};

        else if (c == ']')
            return {token_type::right_bracket, std::string (1, sourcecode_.get ())};

        else if (c == ',')
            return {token_type::comma, std::string (1, sourcecode_.get ())};

        else if (c == ':')
            return {token_type::colon, std::string (1, sourcecode_.get ())};

        // String
        else if (c == '"')
            return {token_type::string, _get_string_token ()};

        // Number
        else if (c == '-' || std::isdigit (c))
            return {token_type::number, _get_number_token ()};

        // Literal: true, false, null
        else if (std::isalpha (c))
        {
            std::string literal = _get_literal_token ();

            if (literal == "true" || literal == "false")
                return {token_type::boolean, literal};

            else if (literal == "null")
                return {token_type::null, literal};

            else if (!ignore_whitespace_)
            {
                // Unknown literal, treat as whitespace
                return {token_type::whitespace, literal};
            }
        }

        // Unknown character, skip
        else
            sourcecode_.get (); // Consume the character
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get string token
// @return String token ("\"abc\"")
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
tokenizer::_get_string_token ()
{
    std::string text;
    auto c = sourcecode_.get (); // consume initial '"'
    text.push_back (c);
    bool escape = false;

    while (true)
    {
        c = sourcecode_.get ();
        if (c == 0)
            break;
        text.push_back (c);
        if (escape)
        {
            escape = false;
            continue;
        }
        if (c == '\\')
        {
            escape = true;
            continue;
        }
        if (c == '"')
            break;
    }
    return text;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number token
// @return Number token ("123", "-123.45e6", etc.)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
tokenizer::_get_number_token ()
{
    std::string text;
    auto c = sourcecode_.peek ();
    if (c == '-')
    {
        text.push_back (sourcecode_.get ());
        c = sourcecode_.peek ();
    }
    while (std::isdigit (c))
    {
        text.push_back (sourcecode_.get ());
        c = sourcecode_.peek ();
    }
    if (c == '.')
    {
        text.push_back (sourcecode_.get ());
        c = sourcecode_.peek ();
        while (std::isdigit (c))
        {
            text.push_back (sourcecode_.get ());
            c = sourcecode_.peek ();
        }
    }
    if (c == 'e' || c == 'E')
    {
        text.push_back (sourcecode_.get ());
        c = sourcecode_.peek ();
        if (c == '+' || c == '-')
        {
            text.push_back (sourcecode_.get ());
            c = sourcecode_.peek ();
        }
        while (std::isdigit (c))
        {
            text.push_back (sourcecode_.get ());
            c = sourcecode_.peek ();
        }
    }
    return text;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get literal token
// @return Literal token ("true", "false", "null")
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
tokenizer::_get_literal_token ()
{
    std::string text;
    auto c = sourcecode_.peek ();
    while (std::isalpha (c))
    {
        text.push_back (sourcecode_.get ());
        c = sourcecode_.peek ();
    }
    return text;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get whitespace token
// @return Whitespace token
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
tokenizer::_get_whitespace_token ()
{
    std::string text;
    auto c = sourcecode_.peek ();
    while (c == ' ' || c == '\t' || c == '\r' || c == '\n')
    {
        text.push_back (sourcecode_.get ());
        c = sourcecode_.peek ();
    }
    return text;
}

} // namespace mobius::core::decoder::json
