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
#include <mobius/zlib_functions.h>
#include <mobius/exception.inc>
#include <stdexcept>

#define ZLIB_CONST
#include <zlib.h>
#define MOBIUS_EXCEPTION_ZLIB exception_msg (__FILE__, __func__, __LINE__, zError (ret))

static constexpr int CHUNK_SIZE = 512 * 1024;

namespace mobius
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief compress zlib stream
// @param bytearray
// @return compressed bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
zlib_compress (const mobius::bytearray& array, int level)
{
  // initialize z_stream
  z_stream stream;
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  stream.avail_in = array.size ();
  stream.next_in = array.data ();

  // init deflate
  int ret;
  ret = deflateInit (&stream, level);
  if (ret != Z_OK)
    throw std::runtime_error (MOBIUS_EXCEPTION_ZLIB);

  // deflate array
  mobius::bytearray out;
  std::uint8_t buffer[CHUNK_SIZE];

  do
    {
      stream.avail_out = CHUNK_SIZE;
      stream.next_out = buffer;
      ret = deflate (&stream, Z_FINISH);

      if (ret == Z_STREAM_ERROR)
        {
          deflateEnd (&stream);
          throw std::runtime_error (MOBIUS_EXCEPTION_ZLIB);
        }

      out += mobius::bytearray (buffer, CHUNK_SIZE - stream.avail_out);
    }
  while (stream.avail_out == 0);

  // end deflate
  deflateEnd (&stream);

  // return bytearray
  return out;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief decompress zlib stream
// @param compressed bytearray
// @return decompressed bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
zlib_decompress (const mobius::bytearray& array)
{
  // initialize z_stream
  z_stream stream;
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  stream.avail_in = array.size ();
  stream.next_in = array.data ();

  // init inflate
  int ret = inflateInit (&stream);

  if (ret != Z_OK)
    throw std::runtime_error (MOBIUS_EXCEPTION_ZLIB);

  // inflate array
  mobius::bytearray out;
  std::uint8_t buffer[CHUNK_SIZE];

  do
    {
      stream.avail_out = CHUNK_SIZE;
      stream.next_out = buffer;
      ret = inflate (&stream, Z_NO_FLUSH);

      if (ret != Z_STREAM_END && ret != Z_OK)
        {
          inflateEnd (&stream);
          throw std::runtime_error (MOBIUS_EXCEPTION_ZLIB);
        }

      out += mobius::bytearray (buffer, CHUNK_SIZE - stream.avail_out);
    }
  while (stream.avail_out == 0);

  // end inflate
  inflateEnd (&stream);

  // return bytearray
  return out;
}

} // namespace mobius


