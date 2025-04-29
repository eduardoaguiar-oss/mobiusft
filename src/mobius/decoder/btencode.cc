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
#include "btencode.h"
#include <mobius/io/bytearray_io.h>
#include <mobius/io/sequential_reader_adaptor.h>
#include <mobius/core/pod/map.hpp>
#include <mobius/string_functions.h>
#include <mobius/exception.inc>
#include <stdexcept>
#include <vector>

namespace mobius::decoder
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::pod::map _decode_dict (mobius::io::sequential_reader_adaptor&);
static std::vector <mobius::core::pod::data> _decode_list (mobius::io::sequential_reader_adaptor&);
static mobius::core::pod::data _decode_string (mobius::io::sequential_reader_adaptor&);
static mobius::core::pod::data _decode_integer (mobius::io::sequential_reader_adaptor&);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode btencode data, according to type
// @param adaptor Reader adaptor object
// @return Pod object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::pod::data
_decode_data (mobius::io::sequential_reader_adaptor& adaptor)
{
  mobius::core::pod::data data;

  auto b = adaptor.peek ();

  if (b == 'd')
    data = _decode_dict (adaptor);

  else if (b == 'l')
    data = _decode_list (adaptor);

  else if (b == 'i')
    data = _decode_integer (adaptor);

  else if (isdigit (b))
    data = _decode_string (adaptor);

  else
    throw std::runtime_error (
       MOBIUS_EXCEPTION_MSG (std::string ("invalid byte (" + mobius::string::to_hex (b, 2) + " at pos " + std::to_string (adaptor.tell ())))
    );

  return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode dict object
// @param adaptor sequential_reader_adaptor object
// @return Dict object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::pod::map
_decode_dict (mobius::io::sequential_reader_adaptor& adaptor)
{
  mobius::core::pod::map m;

  adaptor.skip (1);	// 'd'

  while (adaptor.peek () != 'e')
    {
      auto key_value = mobius::bytearray (_decode_data (adaptor));
      auto key = key_value.to_string ();
      auto value = _decode_data (adaptor);
      m.set (key, value);
    }

  adaptor.skip (1);  // 'e'

  return m;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode list object
// @param adaptor sequential_reader_adaptor object
// @return Vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::vector <mobius::core::pod::data>
_decode_list (mobius::io::sequential_reader_adaptor& adaptor)
{
  std::vector <mobius::core::pod::data> v;

  adaptor.skip (1);	// 'l'

  while (adaptor.peek () != 'e')
    {
      auto value = _decode_data (adaptor);
      v.push_back (value);
    }

  adaptor.skip (1);  // 'e'

  return v;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode string object
// @param adaptor sequential_reader_adaptor object
// @return Dict object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::pod::data
_decode_string (mobius::io::sequential_reader_adaptor& adaptor)
{
  mobius::core::pod::data data;

  // get string size
  std::string s_siz;
  auto b = adaptor.get ();

  while (b != ':')
    {
      s_siz += b;
      b = adaptor.get ();
    }

  // get string data
  auto size = std::strtoull (s_siz.c_str (), nullptr, 10);
  auto value = adaptor.get (size);

  return mobius::core::pod::data (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode integer object
// @param adaptor sequential_reader_adaptor object
// @return Integer as pod::data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::pod::data
_decode_integer (mobius::io::sequential_reader_adaptor& adaptor)
{
  mobius::core::pod::data data;

  adaptor.skip (1);	// 'i'
  std::string s_value;

  auto b = adaptor.get ();

  while (b != 'e')
    {
      s_value += b;
      b = adaptor.get ();
    }

  std::uint64_t value = std::strtoull (s_value.c_str (), nullptr, 10);
  return mobius::core::pod::data (value);
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode btencode data
// @param reader Reader object
// @return Pod object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
btencode (const mobius::io::reader& reader)
{
  mobius::io::sequential_reader_adaptor adaptor (reader);
  return _decode_data (adaptor);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode btencode data
// @param data Data
// @return Pod object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
btencode (const mobius::bytearray& data)
{
  return btencode (mobius::io::new_bytearray_reader (data));
}

} // namespace mobius::decoder


