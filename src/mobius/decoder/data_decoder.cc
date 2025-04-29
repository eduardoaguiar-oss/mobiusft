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
#include "data_decoder.h"
#include <mobius/datetime/datetime.h>
#include <mobius/datetime/timedelta.h>
#include <mobius/io/bytearray_io.h>
#include <mobius/charset.h>
#include <mobius/exception.inc>
#include <string>
#include <cstdio>
#include <sstream>
#include <stdexcept>

namespace mobius::decoder
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Read <i>size</i> bytes from reader
// @param reader Reader object
// @param size Size in bytes
// @return Bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static inline mobius::bytearray
read (mobius::io::reader& reader, std::size_t size)
{
  auto data = reader.read (size);

  if (data.size () < size)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("cannot read enough bytes"));

  return data;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param in reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data_decoder::data_decoder (const mobius::io::reader& in)
  : in_ (in)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param data bytearray object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data_decoder::data_decoder (const mobius::bytearray& data)
  : in_ (mobius::io::new_bytearray_reader (data))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Operator bool
// @return true If there is data to read, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data_decoder::operator bool () const
{
  return !in_.eof ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Skip n bytes
// @param size size in bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
data_decoder::skip (size_type size)
{
  in_.skip (size);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Jump to position pos
// @param pos position from the start of data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
data_decoder::seek (size_type pos)
{
  in_.seek (pos);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get current position
// @return position from the start of data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data_decoder::size_type
data_decoder::tell () const
{
  return in_.tell ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get data size
// @return data size in bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data_decoder::size_type
data_decoder::get_size () const
{
  return in_.get_size ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode an int8
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int8_t
data_decoder::get_int8 ()
{
  return static_cast <std::int8_t> (get_uint8 ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode an int16 (le)
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int16_t
data_decoder::get_int16_le ()
{
  return static_cast <std::int16_t> (get_uint16_le ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode an int16 (be)
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int16_t
data_decoder::get_int16_be ()
{
  return static_cast <std::int16_t> (get_uint16_be ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode an int32 (le)
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int32_t
data_decoder::get_int32_le ()
{
  return std::int32_t (get_uint32_le ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode an int32 (be)
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int32_t
data_decoder::get_int32_be ()
{
  return std::int32_t (get_uint32_be ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode an int64 (le)
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int64_t
data_decoder::get_int64_le ()
{
  return std::int64_t (get_uint64_le ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode an int64 (be)
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int64_t
data_decoder::get_int64_be ()
{
  return std::int64_t (get_uint64_be ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode an uint8
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uint8_t
data_decoder::get_uint8 ()
{
  const bytearray data = read (in_, 1);
  return data[0];
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode an uint16 (le)
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uint16_t
data_decoder::get_uint16_le ()
{
  const bytearray data = read (in_, 2);
  const uint8_t *p = data.begin ();

  return std::uint16_t (*p) | (std::uint16_t (*(p + 1)) << 8);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode an uint16 (be)
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uint16_t
data_decoder::get_uint16_be ()
{
  const bytearray data = read (in_, 2);
  const uint8_t *p = data.begin ();

  return (std::uint16_t (*p) << 8) | std::uint16_t (*(p + 1));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode an uint32 (le)
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uint32_t
data_decoder::get_uint32_le ()
{
  const bytearray data = read (in_, 4);
  const uint8_t *p = data.begin ();

  return std::uint32_t (*p) |
         (std::uint32_t (*(p + 1)) << 8) |
         (std::uint32_t (*(p + 2)) << 16) |
         (std::uint32_t (*(p + 3)) << 24);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode an uint32 (be)
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uint32_t
data_decoder::get_uint32_be ()
{
  const bytearray data = read (in_, 4);
  const uint8_t *p = data.begin ();

  return std::uint32_t (*(p + 3)) |
         (std::uint32_t (*(p + 2)) << 8) |
         (std::uint32_t (*(p + 1)) << 16) |
         (std::uint32_t (*p) << 24);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode an uint64 (le)
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uint64_t
data_decoder::get_uint64_le ()
{
  const bytearray data = read (in_, 8);
  const uint8_t *p = data.begin ();

  return std::uint64_t (*p) |
         (std::uint64_t (*(p + 1)) << 8) |
         (std::uint64_t (*(p + 2)) << 16) |
         (std::uint64_t (*(p + 3)) << 24) |
         (std::uint64_t (*(p + 4)) << 32) |
         (std::uint64_t (*(p + 5)) << 40) |
         (std::uint64_t (*(p + 6)) << 48) |
         (std::uint64_t (*(p + 7)) << 56);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode an uint64 (be)
// @return value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uint64_t
data_decoder::get_uint64_be ()
{
  const bytearray data = read (in_, 8);
  const uint8_t *p = data.begin ();

  return std::uint64_t (*(p + 7)) |
         (std::uint64_t (*(p + 6)) << 8) |
         (std::uint64_t (*(p + 5)) << 16) |
         (std::uint64_t (*(p + 4)) << 24) |
         (std::uint64_t (*(p + 3)) << 32) |
         (std::uint64_t (*(p + 2)) << 40) |
         (std::uint64_t (*(p + 1)) << 48) |
         (std::uint64_t (*p) << 56);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode HFS timestamp
// @return datetime object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
data_decoder::get_hfs_datetime ()
{
  mobius::datetime::datetime dt;
  auto timestamp = get_uint32_be ();

  if (timestamp)
    dt = mobius::datetime::datetime (1904, 1, 1, 0, 0, 0) + mobius::datetime::timedelta (0, 0, timestamp, 0);

  return dt;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode ISO9660 timestamp
// @return datetime object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
data_decoder::get_iso9660_datetime ()
{
  auto data = read (in_, 17);

  // parse date/time from string
  int y, m, d;
  int hh, mm, ss;
  sscanf (reinterpret_cast<const char *> (data.data ()),
          "%04d%02d%02d%02d%02d%02d",
          &y, &m, &d, &hh, &mm, &ss);

  // create datetime object
  mobius::datetime::datetime dt;

  if (y || m || d || hh || mm || ss)
    {
      dt = mobius::datetime::datetime (y, m, d, hh, mm, ss);

      // offset from UTC in 15min intervals
      int offset = static_cast<char> (data[16]);

      if (offset)
        {
          mobius::datetime::timedelta delta (0, 0, offset * 15 * 60, 0);
          dt = dt + delta;  //dt += delta;
        }
    }

  return dt;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode NT timestamp
// @return datetime object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
data_decoder::get_nt_datetime ()
{
  std::uint64_t timestamp = get_uint64_le ();

  return mobius::datetime::new_datetime_from_nt_timestamp (timestamp);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode UNIX timestamp
// @return datetime object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
data_decoder::get_unix_datetime ()
{
  mobius::datetime::datetime dt;
  auto timestamp = get_uint32_le ();

  if (timestamp)
    dt = mobius::datetime::new_datetime_from_unix_timestamp (timestamp);

  return dt;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode FAT timestamp
// @return datetime object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
data_decoder::get_fat_datetime ()
{
  auto d = get_uint16_le ();
  auto t = get_uint16_le ();

  return mobius::datetime::new_datetime_from_fat_time (d, t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode bytearray by size
// @param size size in bytes
// @return bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
data_decoder::get_bytearray_by_size (std::size_t size)
{
  return read (in_, size);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode bytearray as hex string
// @param size size in bytes
// @return string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
data_decoder::get_hex_string_by_size (std::size_t size)
{
  return read (in_, size).to_hexstring ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode string by size
// @param size size in bytes
// @return string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
data_decoder::get_string_by_size (std::size_t size, const std::string& encoding)
{
  bytearray data = read (in_, size);
  std::string result;

  if (encoding == "ASCII" || encoding == "UTF-8")
    result = std::string (data.begin (), data.end ());

  else
    result = conv_charset_to_utf8 (data, encoding);

  auto pos = result.find ('\0');

  if (pos != std::string::npos)
    result.erase (pos);

  return result;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode C '\0' terminated string
// @param encoding Encoding of original data
// @return string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
data_decoder::get_c_string (const std::string& encoding)
{
  bytearray data;

  // set ending block and read size according to encoding
  bytearray ending;
  std::uint64_t read_size;


  if (encoding == "UTF-16LE" ||
      encoding == "UTF-16" ||
      encoding == "UTF-16BE")
    {
      read_size = 2;
      ending = bytearray ({0, 0});
    }
  else
    {
      read_size = 1;
      ending = bytearray ({0});
    }

  // read data until ending is found
  bytearray tmp = read (in_, read_size);

  while (tmp != ending)
    {
      data += tmp;
      tmp = read (in_, read_size);
    }

  // convert charset, if necessary
  std::string result;

  if (encoding == "ASCII" || encoding == "UTF-8")
    result = std::string (data.begin (), data.end ());

  else
    result = conv_charset_to_utf8 (data, encoding);

  return result;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode GUID
// @return GUID as formatted string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
data_decoder::get_guid ()
{
  uint32_t guid1 = get_uint32_le ();
  uint16_t guid2 = get_uint16_le ();
  uint16_t guid3 = get_uint16_le ();
  uint16_t guid4 = get_uint16_be ();
  uint16_t guid5 = get_uint16_be ();
  uint32_t guid6 = get_uint32_be ();

  char buffer[64];
  sprintf (buffer, "%08X-%04X-%04X-%04X-%04X%08X",
           guid1, guid2, guid3, guid4, guid5, guid6);

  return buffer;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode UUID
// @return UUID as formatted string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
data_decoder::get_uuid ()
{
  std::string uuid = get_bytearray_by_size (16).to_hexstring ();

  return uuid.substr (0, 8) + '-' +
         uuid.substr (8, 4) + '-' +
         uuid.substr (12, 4) + '-' +
         uuid.substr (16, 4) + '-' +
         uuid.substr (20);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode SID
// @return SID as formatted string
// @see https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-dtyp/f992ad60-0fe4-4b87-9fed-beb478836861
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
data_decoder::get_sid ()
{
  uint8_t revision = get_uint8 ();
  uint8_t subauth_count = get_uint8 ();		// number of sub authorities

  uint64_t auth = std::uint64_t (get_uint16_be ()) << 32;	// big endian
  auth |= get_uint32_be ();

  std::string sid = "S-" + std::to_string (revision) + '-' + std::to_string (auth);

  for (std::uint8_t i = 0;i < subauth_count;i++)
    {
      uint32_t subauth = get_uint32_le ();
      sid += '-';
      sid += std::to_string (subauth);
    }

  return sid;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode IPv4
// @return IPv4 as formatted string
//! \deprecated Use either get_ipv4_le or get_ipv4_be
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
data_decoder::get_ipv4 ()
{
  return get_ipv4_be ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode IPv4 (big endian)
// @return IPv4 as formatted string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
data_decoder::get_ipv4_be ()
{
  bytearray data = read (in_, 4);

  return std::to_string (data[0]) + '.' +
         std::to_string (data[1]) + '.' +
         std::to_string (data[2]) + '.' +
         std::to_string (data[3]);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode IPv4 (little endian)
// @return IPv4 as formatted string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
data_decoder::get_ipv4_le ()
{
  bytearray data = read (in_, 4);

  return std::to_string (data[3]) + '.' +
         std::to_string (data[2]) + '.' +
         std::to_string (data[1]) + '.' +
         std::to_string (data[0]);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode IPv6
// @return IPv6 as formatted string
// @see RFC 4291 - section 2.2.2
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
data_decoder::get_ipv6 ()
{
  std::stringstream stream;
  int state = 0;

  for (int i = 0;i < 8;i++)
    {
      auto segment = get_uint16_be ();

      switch (state)
        {

        case 0:               // start
          if (segment)
            {
              stream << std::uppercase << std::hex << segment << std::dec;
              state = 1;
            }

          else
            {
              stream << "::";
              state = 2;
            }
          break;

        case 1:               // nth segment
          if (segment)
            stream << ':' << std::uppercase << std::hex << segment << std::dec;

          else
            {
              stream << "::";
              state = 2;
            }
          break;

        case 2:               // "::" deployed
          if (segment)
            {
              stream << std::uppercase << std::hex << segment << std::dec;
              ++state;
            }
          break;

        case 3:	              // end segments
          stream << ':' << std::uppercase << std::hex << segment << std::dec;
          break;
        }
    }

  return stream.str ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode IPv6
// @return IPv6 as formatted string
// @see RFC 4291 - section 2.2.3
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
data_decoder::get_ipv4_mapped_ipv6 ()
{
  std::stringstream stream;
  int state = 0;

  for (int i = 0;i < 6;i++)
    {
      auto segment = get_uint16_be ();

      switch (state)
        {

        case 0:               // start
          if (segment)
            {
              stream << std::uppercase << std::hex << segment << std::dec;
              state = 1;
            }

          else
            {
              stream << "::";
              state = 2;
            }
          break;

        case 1:               // nth segment
          if (segment)
            stream << ':' << std::uppercase << std::hex << segment << std::dec;

          else
            {
              stream << "::";
              state = 2;
            }
          break;

        case 2:               // "::" deployed
          if (segment)
            {
              stream << std::uppercase << std::hex << segment << std::dec;
              ++state;
            }
          break;

        case 3:	              // end segments
          stream << ':' << std::uppercase << std::hex << segment << std::dec;
          break;
        }
    }

  if (stream.str () == "::FFFF")      // ipv4 mapped IPV6
    stream = std::stringstream ();

  else if (state != 2)
    stream << ":";

  stream << static_cast <int> (get_uint8 ())
         << '.' << static_cast <int> (get_uint8 ())
         << '.' << static_cast <int> (get_uint8 ())
         << '.' << static_cast <int> (get_uint8 ());

  return stream.str ();
}

} // namespace mobius::decoder


