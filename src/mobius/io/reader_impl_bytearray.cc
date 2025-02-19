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
#include <mobius/io/reader_impl_bytearray.h>
#include <mobius/exception.inc>
#include <stdexcept>

namespace mobius
{
namespace io
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Constructor
//! \param data bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
reader_impl_bytearray::reader_impl_bytearray (const mobius::bytearray& data)
  : data_ (data)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Set read position
//! \param offset Offset in bytes
//! \param w Either beginning, current or end
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
reader_impl_bytearray::seek (offset_type offset, whence_type w)
{
  // calculate offset from the beginning of data
  offset_type abs_offset;

  if (w == whence_type::beginning)
    abs_offset = offset;

  else if (w == whence_type::current)
    abs_offset = pos_ + offset;

  else if (w == whence_type::end)
    abs_offset = data_.size () - 1 + offset;

  else
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid whence_type"));

  // update current pos, if possible
  if (abs_offset < 0)
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid offset"));

  else if (size_type (abs_offset) <= data_.size ())
    pos_ = abs_offset;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Read bytes from reader
//! \param size Size in bytes
//! \return Data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
reader_impl_bytearray::read (size_type size)
{
  offset_type end = std::min (data_.size (), pos_ + size) - 1;
  mobius::bytearray data;

  if (offset_type (pos_) <= end)
    {
      data = data_.slice (pos_, end);
      pos_ = end + 1;
    }

  return data;
}

} // namespace io
} // namespace mobius
