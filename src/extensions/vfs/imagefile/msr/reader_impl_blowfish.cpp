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
#include "reader_impl_blowfish.hpp"
#include "imagefile_impl.hpp"
#include <mobius/core/exception.inc>
#include <mobius/core/io/file.hpp>
#include <stdexcept>

namespace
{
// @brief header size in bytes
constexpr int HEADER_SIZE = 16384;
} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param impl imagefile_impl object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
reader_impl_blowfish::reader_impl_blowfish (const imagefile_impl &impl)
    : size_ (impl.get_size ()),
      cipher_ (mobius::core::crypt::new_cipher_ecb ("blowfish",
                                                    impl.get_encryption_key ()))
{
    auto f = impl.get_file ();
    stream_ = f.new_reader ();
    stream_.seek (HEADER_SIZE);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set read position
// @param offset offset in bytes
// @param w either beginning, current or end
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
reader_impl_blowfish::seek (offset_type offset, whence_type w)
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
        pos_ = abs_offset;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief read bytes from reader
// @param size size in bytes
// @return bytearray containing data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
reader_impl_blowfish::read (size_type size)
{
    mobius::core::bytearray data;

    size = std::min (size_ - pos_, size);
    constexpr int CHUNK_SIZE = 512;

    while (size > 0)
    {
        size_type chunk_idx = pos_ / CHUNK_SIZE;

        if (chunk_idx != chunk_idx_) // retrieve new data chunk, if necessary
        {
            stream_.seek (chunk_idx * CHUNK_SIZE + HEADER_SIZE);
            chunk_data_ = cipher_.decrypt (stream_.read (CHUNK_SIZE));
            chunk_idx_ = chunk_idx;
        }

        size_type slice_start = pos_ % CHUNK_SIZE;
        size_type slice_end =
            std::min (slice_start + size - 1, chunk_data_.size () - 1);
        data += chunk_data_.slice (slice_start, slice_end);
        pos_ += slice_end - slice_start + 1;
        size -= slice_end - slice_start + 1;
    }

    return data;
}
