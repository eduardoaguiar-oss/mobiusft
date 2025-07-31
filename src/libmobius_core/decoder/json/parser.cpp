// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025
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
#include <mobius/core/decoder/json/parser.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/io/bytearray_io.hpp>
#include <mobius/core/string_functions.hpp>
#include <stdexcept>
#include <iostream>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Debug flag. Set to true for debugging output
static constexpr bool DEBUG = false;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert number value to mobius::core::pod::data
// @param value Number value as string
// @return mobius::core::pod::data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::pod::data
_convert_number_to_data (const std::string &value)
{
    try
    {
        if (value.find_first_of (".") != std::string::npos)
            return mobius::core::pod::data (std::stod (value));

        return mobius::core::pod::data (
            static_cast<std::int64_t> (std::stoll (value))
        );
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG (
            "Failed to convert number: " + value + " - " + e.what ()
        ));
    }
}

} // namespace

namespace mobius::core::decoder::json
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create parser
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
parser::parser (const mobius::core::io::reader &reader)
    : tokenizer_ (reader)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create parser
// @param bytearray Bytearray object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
parser::parser (const mobius::core::bytearray &bytearray)
    : tokenizer_ (mobius::core::io::new_bytearray_reader (bytearray))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get element
// @return JSON root element
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
parser::get ()
{
    return _decode_value ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode JSON value
// @return Decoded value as mobius::core::pod::data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
parser::_decode_value ()
{
    mobius::core::pod::data data;

    auto [type, value] = tokenizer_.get_token ();

    if (DEBUG)
        std::cout << "Token type: " << static_cast<int> (type) << ", value: '"
                  << value << "'" << std::endl;

    switch (type)
    {
    case tokenizer::token_type::string:
        data = mobius::core::pod::data (value.substr (1, value.size () - 2));
        break;

    case tokenizer::token_type::number:
        data = _convert_number_to_data (value);
        break;

    case tokenizer::token_type::boolean:
        data = mobius::core::pod::data (value == "true");
        break;

    case tokenizer::token_type::null:
        break;

    case tokenizer::token_type::left_brace: // {
        data = _decode_map ();
        break;

    case tokenizer::token_type::left_bracket: // [
        data = _decode_array ();
        break;

    case tokenizer::token_type::end:
        break;

    default:
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG (
            "Unexpected token [type: " +
            std::to_string (static_cast<int> (type)) + "] with value: '" +
            value + "'"
        ));
    }

    return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode JSON map
// @return Decoded map as mobius::core::pod::map object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::map
parser::_decode_map ()
{
    mobius::core::pod::map map;

    std::string current_key;

    while (true)
    {
        if (DEBUG)
            std::cout << "Map: " << map.to_string () << std::endl;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get key
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        std::string current_key;

        auto [type, value] = tokenizer_.get_token ();

        if (DEBUG)
            std::cout << "Token type: " << static_cast<int> (type)
                      << ", Value: '" << value << "'" << std::endl;

        if (type == tokenizer::token_type::string)
            current_key = value.substr (1, value.size () - 2);

        else if (type == tokenizer::token_type::end ||
                 type == tokenizer::token_type::right_brace)
            return map;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Expect colon
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        std::tie (type, value) = tokenizer_.get_token ();

        if (type == tokenizer::token_type::end ||
            type == tokenizer::token_type::right_brace)
            return map;

        else if (type != tokenizer::token_type::colon)
            throw std::runtime_error (
                MOBIUS_EXCEPTION_MSG ("Unexpected colon token")
            );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get value
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::pod::data value_data = _decode_value ();

        if (DEBUG)
            std::cout << "Decoded value: " << value_data.to_string ()
                      << std::endl;

        map.set (current_key, value_data);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Expect comma or end
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        std::tie (type, value) = tokenizer_.get_token ();

        if (type == tokenizer::token_type::end ||
            type == tokenizer::token_type::right_brace)
            return map;

        else if (type != tokenizer::token_type::comma)
            throw std::runtime_error (
                MOBIUS_EXCEPTION_MSG ("Unexpected token after value")
            );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode JSON array
// @return Decoded array as mobius::core::pod::data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
parser::_decode_array ()
{
    std::vector<mobius::core::pod::data> array;

    auto [type, value] = tokenizer_.get_token ();

    while (type != tokenizer::token_type::end &&
           type != tokenizer::token_type::right_bracket)
    {
        array.push_back (_decode_value ());

        std::tie (type, value) = tokenizer_.get_token ();
    }

    return mobius::core::pod::data (array);
}

} // namespace mobius::core::decoder::json
