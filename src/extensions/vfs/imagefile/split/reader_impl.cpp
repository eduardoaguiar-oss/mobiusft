// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025
// Eduardo Aguiar
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
#include <mobius/core/exception.inc>
#include <mobius/core/string_functions.hpp>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param imagefile_impl imagefile_impl object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
reader_impl::reader_impl (const imagefile_impl &imagefile_impl)
    : size_ (imagefile_impl.get_size ()),
      segments_ (imagefile_impl.get_segment_array ())
{
    segment_idx_ = std::int64_t (imagefile_impl.get_attribute ("segments"));

    if (segment_idx_ == 0)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("segment files not found"));

    segment_size_ =
        std::int64_t (imagefile_impl.get_attribute ("segment_size"));

    _set_stream ();
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
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("invalid whence_type"));

    // update current pos, if possible
    if (abs_offset < 0)
        throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid offset"));

    else if (size_type (abs_offset) <= size_)
    {
        pos_ = abs_offset;
        _set_stream ();
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Read bytes from reader
// @param size size in bytes
// @return bytearray containing data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
reader_impl::read (size_type size)
{
    if (!stream_)
        _set_stream ();

    mobius::core::bytearray data = stream_.read (size);
    pos_ += data.size ();
    size -= data.size ();

    while (size > 0 && pos_ < size_)
    {
        _set_stream ();
        mobius::core::bytearray tmp = stream_.read (size);
        pos_ += tmp.size ();
        size -= tmp.size ();
        data += tmp;
    }

    return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set read stream according to current position
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
reader_impl::_set_stream ()
{
    if (pos_ >= size_)
        return;

    size_type segment_idx = pos_ / segment_size_;
    size_type pos = pos_ % segment_size_;

    if (segment_idx != segment_idx_)
    {
        stream_ = segments_.new_reader (segment_idx);
        segment_idx_ = segment_idx;
    }

    stream_.seek (pos);
}
