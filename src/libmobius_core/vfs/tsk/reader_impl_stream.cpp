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
#include <mobius/core/exception.inc>
#include <mobius/core/vfs/tsk/exception.hpp>
#include <mobius/core/vfs/tsk/reader_impl_stream.hpp>
#include <stdexcept>
#include <tsk/libtsk.h>

namespace mobius::core::vfs::tsk
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param fs_file fs_file object
// @param fs_attr_p TSK_FS_ATTR pointer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
reader_impl_stream::reader_impl_stream (const fs_file &fs_file,
                                        const TSK_FS_ATTR *fs_attr_p)
    : fs_file_ (fs_file),
      fs_attr_ (fs_attr_p),
      size_ (fs_attr_p->size)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set reading position
// @param offsetOoffset in bytes
// @param w Either beginning, current or end
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
reader_impl_stream::seek (offset_type offset, whence_type w)
{
    // calculate offset from the beginning of data
    offset_type abs_offset;

    if (w == whence_type::beginning)
        abs_offset = offset;

    else if (w == whence_type::current)
        abs_offset = pos_ + offset;

    else if (w == whence_type::end)
        abs_offset = size_ + offset;

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
// @brief Read bytes from stream
// @param size Size in bytes
// @return Bytearray containing data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
reader_impl_stream::read (size_type size)
{
    if (pos_ + size > size_)
        size = size_ - pos_;

    mobius::core::bytearray data (size);

    if (size > 0)
    {
        ssize_t count = tsk_fs_file_read_type (
            fs_attr_->fs_file, fs_attr_->type, fs_attr_->id, pos_,
            reinterpret_cast<char *> (data.data ()), size,
            TSK_FS_FILE_READ_FLAG_NONE);

        if (count < 0)
            throw std::runtime_error (TSK_EXCEPTION_MSG);

        if (size_type (count) < size)
            data.resize (count);

        pos_ += count;
    }

    return data;
}

} // namespace mobius::core::vfs::tsk
