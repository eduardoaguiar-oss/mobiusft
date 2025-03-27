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
#include "writer_impl.h"
#include "imagefile_impl.h"
#include <mobius/exception.inc>
#include <mobius/string_functions.h>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param imagefile_impl imagefile implementation object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
writer_impl::writer_impl (const imagefile_impl& imagefile_impl)
  : segment_size_ (std::int64_t (imagefile_impl.get_attribute ("segment_size"))),
    segment_idx_ (1),
    segments_ (imagefile_impl.get_segment_array ())
{
  _set_stream (0);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set write position
// @param offset offset in bytes
// @param w either beginning, current or end
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
writer_impl::seek (offset_type offset, whence_type w)
{
  // calculate offset from the beginning of data
  offset_type abs_offset;

  if (w == whence_type::beginning)
    abs_offset = offset;

  else if (w == whence_type::current)
    abs_offset = pos_ + offset;

  else if (w == whence_type::end)
    abs_offset = size_ - 1 + offset;

  else
    throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG ("invalid whence_type"));

  // update current pos, if possible
  if (abs_offset >= 0)
    pos_ = abs_offset;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Write bytes
// @param data a bytearray
// @return Number of bytes written
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
writer_impl::size_type
writer_impl::write (const mobius::bytearray& data)
{
  // if write position is ahead of size_, fill gap with zeroes
  if (size_ < pos_)
    {
      constexpr unsigned int BLOCK_SIZE = 65536;    //!< write block size
      size_type count = pos_ - size_;

      mobius::bytearray buffer (BLOCK_SIZE);
      buffer.fill (0);

      while (count >= size_type (BLOCK_SIZE))
        {
          size_type bytes = _write_data (size_, buffer);
          count -= bytes;
          size_ += bytes;
        }

      if (count > 0)
        {
          size_type bytes = _write_data (size_, buffer.slice (0, count - 1));
          size_ += bytes;
        }
    }

  // write data
  size_type bytes = _write_data (pos_, data);
  pos_ += bytes;
  size_ = std::max (size_, pos_);

  // return number of bytes written
  return bytes;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Write data block, splitting into segment files, if necessary
// @param pos write position
// @param data data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
writer_impl::size_type
writer_impl::_write_data (size_type pos, const mobius::bytearray& data)
{
  mobius::bytearray buffer = data;

  while (!buffer.empty ())
    {
      _set_stream (pos);

      // write data up to segment size
      size_type segment_left = segment_size_ - stream_.tell ();

      if (buffer.size () <= segment_left)
        {
          pos += stream_.write (buffer);
          buffer.clear ();
        }

      else
        {
          pos += stream_.write (buffer.slice (0, segment_left - 1));
          buffer = buffer.slice (segment_left, buffer.size () - 1);
        }
    }

  return data.size ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set write stream according to current position
// @param pos Write position
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
writer_impl::_set_stream (size_type pos)
{
  size_type segment_idx = pos / segment_size_;
  size_type stream_pos = pos % segment_size_;

  if (segment_idx != segment_idx_)
    {
      stream_ = segments_.new_writer (segment_idx);
      segment_idx_ = segment_idx;
    }

  stream_.seek (stream_pos);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Flush data to file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
writer_impl::flush ()
{
  stream_.flush ();
}


