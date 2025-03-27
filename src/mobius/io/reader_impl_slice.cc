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
#include "reader_impl_slice.h"
#include <mobius/exception.inc>
#include <stdexcept>

namespace mobius::io
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param pos Initial position
// @param end Final position
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
reader_impl_slice::reader_impl_slice (
  mobius::io::reader reader,
  offset_type pos,
  offset_type end
)
  : reader_ (reader)
{
  if (pos < 0)
    pos += reader.get_size ();

  if (end < 0)
    end += reader.get_size ();

  if (pos < 0)
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid start position"));

  if (end < pos)
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid end position"));

  start_ = pos;
  end_ = end;
  reader_.seek (start_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set read position
// @param offset Offset in bytes
// @param w Either beginning, current or end
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
reader_impl_slice::seek (offset_type offset, whence_type w)
{
  // calculate offset from the beginning of data
  offset_type abs_offset;

  if (w == whence_type::beginning)
    abs_offset = start_ + offset;

  else if (w == whence_type::current)
    abs_offset = reader_.tell () + offset;

  else if (w == whence_type::end)
    abs_offset = end_ + offset;

  else
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid whence_type"));

  // update current pos, if possible
  if (abs_offset < static_cast <offset_type> (start_))
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid offset"));

  else if (abs_offset > static_cast <offset_type> (end_))
    reader_.seek (end_ + 1);

  else
    reader_.seek (abs_offset);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Read bytes from reader
// @param size Size in bytes
// @return Data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
reader_impl_slice::read (size_type size)
{
  if (reader_.tell () > end_ || reader_.tell () < start_)
    return {};

  if (size + reader_.tell () > end_)
    size = end_ - reader_.tell () + 1;

  return reader_.read (size);
}

} // namespace mobius::io


