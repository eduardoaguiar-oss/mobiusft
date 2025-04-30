// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <mobius/core/io/writer_impl_bytearray.hpp>
#include <mobius/core/exception.inc>
#include <algorithm>
#include <stdexcept>

namespace mobius::core::io
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param data Bytearray object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
writer_impl_bytearray::writer_impl_bytearray (mobius::core::bytearray& data)
 : data_ (data)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set write position
// @param offset offset in bytes
// @param w either beginning, current or end
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
writer_impl_bytearray::seek (offset_type offset, whence_type w)
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

  if (abs_offset < 0)
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid offset"));

  pos_ = abs_offset;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Write bytes to writer
// @param data Data
// @return Number of bytes written
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
writer_impl_bytearray::size_type
writer_impl_bytearray::write (const mobius::core::bytearray& data)
{
  if (pos_ + data.size () >= data_.size ())
    data_.resize (pos_ + data.size ());

  std::copy (data.begin (), data.end (), data_.begin () + pos_);
  pos_ += data.size ();

  return data.size ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Flush data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
writer_impl_bytearray::flush ()
{
}

} // namespace mobius::core::io


