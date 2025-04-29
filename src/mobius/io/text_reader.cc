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
#include "text_reader.h"
#include <mobius/core/charset.hpp>
#include <mobius/string_functions.h>

namespace mobius::io
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param encoding Charset encoding
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
text_reader::text_reader (
  const mobius::io::reader& reader,
  const std::string& encoding
)
 : reader_ (reader),
   encoding_ (mobius::string::toupper (encoding))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Read data from text_reader, returning valid utf-8 sequences
// @param size Size in bytes
// @return String
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
text_reader::read (reader::size_type size)
{
  std::string data = _read_chars (size);

  // check if data ends with partially read char
  if ((size > 0) && (data.size () == size) && (data[size-1] & 0x80))
    {
      std::uint64_t pos = size - 1;
      std::uint64_t count = 0;

      while (pos > 0 && (data[pos] & 0xc0) == 0x80)
        --pos, ++count;

      if ((data[pos] & 0xf8) == 0xf0 && count <= 3)
        data += _read_chars (3 - count);

      else if ((data[pos] & 0xf0) == 0xe0 && count <= 2)
        data += _read_chars (2 - count);

      else if ((data[pos] & 0xe0) == 0xc0 && count <= 1)
        data += _read_chars (1 - count);
    }

  return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Read characters from reader
// @param size Size in bytes
// @return String
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
text_reader::_read_chars (reader::size_type size)
{
  // shortcut for "conversion" from utf8 to utf8
  if (encoding_ == "UTF-8" || encoding_ == "UTF8")
    return reader_.read (size).to_string ();

  // read until str_ has at least size chars or until the end of stream
  bool eof = false;

  while (str_.size () < size && !eof)
    {
      auto data = reader_.read (65536);

      if (data.empty ())      // no more data available
        eof = true;

      else
        {
          // convert data encoding
          data_ += data;
          auto p = mobius::core::conv_charset_to_utf8_partial (data_, encoding_);

          // update buffers
          str_ += p.first;
          data_ = p.second;
        }
    }

  // update chars buffer
  std::string ret = str_.substr (0, size);

  if (eof)
    str_.clear ();

  else
    str_ = std::string (str_.begin () + size, str_.end ());

  return ret;
}

} // namespace mobius::io


