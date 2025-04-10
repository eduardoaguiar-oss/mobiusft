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
#include "reader_impl.hpp"
#include "imagefile_impl.hpp"
#include <mobius/exception.inc>
#include <mobius/zlib_functions.h>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param impl imagefile implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
reader_impl::reader_impl (const imagefile_impl& impl)
  : size_ (impl.get_size ()),
    chunk_size_ (impl.get_chunk_size ()),
    segments_ (impl.get_segment_array ()),
    chunk_offset_table_ (impl.get_chunk_offset_table ()),
    chunk_idx_ (impl.get_chunk_count ()),
    last_chunk_idx_ (impl.get_chunk_count () - 1)
{
  segments_.scan ();
  segment_idx_ = segments_.get_size ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set read position
// @param offset offset in bytes
// @param w either beginning, current or end
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
reader_impl::seek (offset_type offset, whence_type w)
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
  if (abs_offset < 0)
    throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG ("invalid offset"));

  else if (size_type (abs_offset) <= size_)
    pos_ = abs_offset;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Read bytes from reader
// @param size size in bytes
// @return bytearray containing data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
reader_impl::read (size_type size)
{
  size = std::min (size_ - pos_, size);
  mobius::bytearray data;

  while (size > 0)
    {
      _retrieve_current_chunk ();

      size_type slice_start = pos_ % chunk_size_;
      size_type slice_end = std::min (slice_start + size - 1, chunk_data_.size () - 1);

      if (slice_end < slice_start)
        return data;

      mobius::bytearray tmp = chunk_data_.slice (slice_start, slice_end);
      data += tmp;
      pos_ += tmp.size ();
      size -= tmp.size ();
    }

  return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Retrieve current data chunk
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
reader_impl::_retrieve_current_chunk ()
{
  size_type chunk_idx = pos_ / chunk_size_;

  // if chunk is loaded, return
  if (chunk_idx == chunk_idx_)
    return;

  // find segment info for pos_ offset (binary search)
  int hi = chunk_offset_table_.size ();
  int lo = 0;
  int mid;
  bool found = false;

  while (lo < hi && !found)
    {
      mid = (lo + hi) / 2;

      if (chunk_offset_table_[mid].start > pos_)
        hi = mid;

      else if (chunk_offset_table_[mid].end < pos_)
        lo = mid;

      else
        found = true;
    }

  // if segment info not found, return
  if (!found)
    return;

  // set stream
  auto segment_idx = mid;

  if (segment_idx != segment_idx_)
    {
      stream_ = segments_.new_reader (segment_idx);
      segment_idx_ = segment_idx;
    }

  // get chunk data offset
  const auto& offset_table = chunk_offset_table_[mid];
  size_type table_idx = (pos_ - offset_table.start) / chunk_size_;
  auto offset = offset_table.offsets[table_idx];

  constexpr std::uint64_t compressed_bit = std::uint64_t (1) << 63;
  bool compressed = offset & compressed_bit;
  offset = offset & 0x7fffffffffffffff;

  // read chunk data
  stream_.seek (offset);

  if (compressed)
    chunk_data_ = zlib_decompress (stream_.read (chunk_size_ + 4));

  else
    chunk_data_ = stream_.read (chunk_size_);

  if (chunk_data_.size () != chunk_size_ && chunk_idx != last_chunk_idx_)
    throw std::runtime_error (mobius::MOBIUS_EXCEPTION_MSG ("invalid chunk"));

  // set new current chunk index
  chunk_idx_ = chunk_idx;
}


