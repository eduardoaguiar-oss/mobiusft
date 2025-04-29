// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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
#include <mobius/core/pod/data.hpp>
#include <mobius/core/pod/map.hpp>
#include <mobius/bytearray.h>
#include <mobius/core/encoder/data_encoder.hpp>
#include <cstdint>
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
// @brief Serialize data item
// @param encoder Encoder object
// @param d Data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_serialize_data (mobius::core::encoder::data_encoder& encoder, const data& data)
{
  switch (data.get_type ())
    {
    case data::type::null:
      encoder.encode_uint8 ('N');
      break;

    case data::type::boolean:
      encoder.encode_uint8 (bool (data) ? 't' : 'f');
      break;

    case data::type::integer:
      encoder.encode_uint8 ('I');
      encoder.encode_int64_le (std::int64_t (data));
      break;

    case data::type::floatn:
      {
        auto text = std::to_string (static_cast <long double> (data));
        encoder.encode_uint8 ('F');
        encoder.encode_uint16_le (text.size ());
        encoder.encode_string_by_size (text, text.size ());
      }
      break;

    case data::type::datetime:
      {
        auto text = to_string (mobius::datetime::datetime (data));
        encoder.encode_uint8 ('D');
        encoder.encode_uint16_le (text.size ());
        encoder.encode_string_by_size (text, text.size ());
      }
      break;

    case data::type::string:
      {
        auto text = std::string (data);
        encoder.encode_uint8 ('S');
        encoder.encode_uint64_le (text.size ());
        encoder.encode_string_by_size (text, text.size ());
      }
      break;

    case data::type::bytearray:
      {
        auto b = mobius::bytearray (data);
        encoder.encode_uint8 ('B');
        encoder.encode_uint64_le (b.size ());
        encoder.encode_bytearray (b);
      }
      break;

    case data::type::list:
      {
        auto v = std::vector <mobius::core::pod::data> (data);

        encoder.encode_uint8 ('L');
        encoder.encode_uint64_le (v.size ());

        for (const auto& i : v)
          _serialize_data (encoder, i);
      }
      break;

    case data::type::map:
      {
        mobius::core::pod::map map (data);

        encoder.encode_uint8 ('M');
        encoder.encode_uint64_le (map.get_size ());

        for (const auto& p : map)
          {
            encoder.encode_uint32_le (p.first.size ());
            encoder.encode_string_by_size (p.first, p.first.size ());
            _serialize_data (encoder, p.second);
          }
      }
      break;

    default:
      throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("unknown data type"));
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Serialize data object
// @param data Data object
// @return Encoded data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_serialize (mobius::core::encoder::data_encoder& encoder, const data& data)
{
  encoder.encode_uint32_le (VERSION);
  _serialize_data (encoder, data);
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Serialize data object
// @param data Data object
// @return Encoded data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
serialize (const data& data)
{
  mobius::bytearray out;
  mobius::core::encoder::data_encoder encoder (out);

  encoder.encode_uint32_le (VERSION);
  _serialize_data (encoder, data);

  return out;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Serialize data object
// @param data Data object
// @return Encoded data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
serialize (mobius::io::writer writer, const data& data)
{
  mobius::core::encoder::data_encoder encoder (writer);
  _serialize (encoder, data);
}

} // namespace mobius::core::pod


