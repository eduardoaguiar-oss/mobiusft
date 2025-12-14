// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
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
#include "reader_impl_dynamic.hpp"
#include "imagefile_impl.hpp"
#include <mobius/core/exception.inc>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param impl imagefile_impl object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
reader_impl_dynamic::reader_impl_dynamic (const imagefile_impl &impl)
    : size_ (impl.get_size ()),
      sector_size_ (impl.get_sector_size ()),
      block_size_ (impl.get_block_size ()),
      block_allocation_table_ (impl.get_block_allocation_table ())
{
    auto sector_size = impl.get_sector_size ();

    sectors_per_block_ = block_size_ / sector_size;
    bitmap_size_ = sectors_per_block_ / 8;

    std::uint32_t remaining = bitmap_size_ % sector_size;

    if (remaining)
        bitmap_size_ = bitmap_size_ + sector_size - remaining;

    auto f = impl.get_file ();
    reader_ = f.new_reader ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set read position
// @param offset offset in bytes
// @param w either beginning, current or end
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
reader_impl_dynamic::seek (offset_type offset, whence_type w)
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
// @brief Read bytes from reader
// @param size size in bytes
// @return Data
//
// Block is composed of Bitmap array + Data. The bitmap array seems to be
// filled with 1's bits, so there is no need to read the bit corresponding to
// the sector we want to read.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
reader_impl_dynamic::read (size_type size)
{
    mobius::core::bytearray data;
    size = std::min (size_ - pos_, size);

    while (size > 0)
    {
        std::uint32_t block_idx = pos_ / block_size_;
        std::uint32_t block_pos = pos_ % block_size_; // data pos inside block
        std::uint32_t sector_offset = block_allocation_table_[block_idx];
        std::uint32_t data_size = std::min (size, block_size_ - block_pos);

        if (sector_offset == 0xffffffff)
            data += mobius::core::bytearray (data_size);

        else
        {
            std::uint64_t offset =
                std::uint64_t (sector_offset) * sector_size_ + bitmap_size_ +
                block_pos;
            reader_.seek (offset);
            data += reader_.read (data_size);
        }

        size -= data_size;
        pos_ += data_size;
    }

    return data;
}
