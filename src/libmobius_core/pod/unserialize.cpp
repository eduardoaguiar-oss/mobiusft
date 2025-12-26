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
#include <cstdint>
#include <mobius/core/bytearray.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/pod/map.hpp>
#include <stdexcept>
#include <vector>

namespace mobius::core::pod
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Serialization version
static constexpr std::uint32_t VERSION = 1;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unserialize data item
// @param decoder Decoder object
// @return Data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data
_unserialize_data (mobius::core::decoder::data_decoder &decoder)
{
    mobius::core::pod::data data;
    auto type = decoder.get_uint8 ();

    switch (type)
    {
    case 'N':
        break; // null data

    case 't':
        data = true;
        break;

    case 'f':
        data = false;
        break;

    case 'I':
        data = decoder.get_int64_le ();
        break;

    case 'F':
    {
        auto siz = decoder.get_uint16_le ();
        auto text = decoder.get_string_by_size (siz);
        data = strtold (text.c_str (), nullptr);
    }
    break;

    case 'D':
    {
        auto siz = decoder.get_uint16_le ();
        auto text = decoder.get_string_by_size (siz);

        if (text.empty ())
            data = mobius::core::datetime::datetime ();

        else
            data = mobius::core::datetime::new_datetime_from_iso_string (text);
    }
    break;

    case 'S':
    {
        auto size = decoder.get_uint64_le ();
        data = decoder.get_string_by_size (size);
    }
    break;

    case 'B':
    {
        auto size = decoder.get_uint64_le ();
        data = decoder.get_bytearray_by_size (size);
    }
    break;

    case 'L':
    {
        auto size = decoder.get_uint64_le ();
        std::vector<mobius::core::pod::data> v;

        for (std::uint64_t i = 0; i < size; i++)
            v.push_back (_unserialize_data (decoder));

        data = v;
    }
    break;

    case 'M':
    {
        auto size = decoder.get_uint64_le ();
        mobius::core::pod::map map;

        for (std::uint64_t i = 0; i < size; i++)
        {
            auto siz = decoder.get_uint32_le ();
            auto key = decoder.get_string_by_size (siz);
            auto value = _unserialize_data (decoder);
            map.set (key, value);
        }

        data = map;
    }
    break;

    default:
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("unknown data type"));
    }

    return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unserialize data object
// @param decoder Decoder object
// @return Data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data
_unserialize (mobius::core::decoder::data_decoder &decoder)
{
    // check serialization version
    auto version = decoder.get_uint32_le ();

    if (version < 1 || version > VERSION)
        throw std::out_of_range (MOBIUS_EXCEPTION_MSG ("version out of range"));

    // decode data
    return _unserialize_data (decoder);
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unserialize data object
// @param b Encoded data
// @return Data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data
unserialize (const mobius::core::bytearray &b)
{
    mobius::core::decoder::data_decoder decoder (b);
    return _unserialize (decoder);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unserialize data object from reader
// @param reader Reader object
// @return Data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data
unserialize (mobius::core::io::reader reader)
{
    mobius::core::decoder::data_decoder decoder (reader);
    return _unserialize (decoder);
}

} // namespace mobius::core::pod
