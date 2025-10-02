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
#include <cstring>
#include <mobius/core/vfs/tsk/adaptor.hpp>
#include <mobius/core/vfs/tsk/exception.hpp>
#include <mobius/core/vfs/tsk/folder_impl.hpp>
#include <stdexcept>
#include <tsk/libtsk.h>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief img_info for mobius::core::io::reader
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct TSK_IMG_INFO_MOBIUS
{
    TSK_IMG_INFO img_info;
    mobius::core::io::reader reader;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief read data from reader
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static ssize_t
mobius_read (TSK_IMG_INFO *img, TSK_OFF_T off, char *buf, size_t len)
{
    TSK_IMG_INFO_MOBIUS *p = (TSK_IMG_INFO_MOBIUS *) img;
    ssize_t ret = -1;

    try
    {
        p->reader.seek (off);
        auto data = p->reader.read (len);
        memcpy (buf, data.data (), data.size ());
        ret = data.size ();
    }
    catch (const std::exception &e)
    {
        tsk_error_set_errstr2 ("%s", e.what ());
        ret = -1;
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief close TSK_IMG_INFO_MOBIUS
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
mobius_close (TSK_IMG_INFO *img)
{
    TSK_IMG_INFO_MOBIUS *p = (TSK_IMG_INFO_MOBIUS *) img;
    delete p;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief show img status
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
mobius_imgstat (TSK_IMG_INFO *, FILE *)
{
}

} // namespace

namespace mobius::core::vfs::tsk
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// @param reader generic reader
// @param offset offset in bytes from the beginning of the stream
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
adaptor::adaptor (const mobius::core::io::reader reader, std::uint64_t offset)
    : reader_ (reader),
      offset_ (offset)
{
    // create image info structure
    TSK_IMG_INFO_MOBIUS *p = new TSK_IMG_INFO_MOBIUS ();
    p->reader = reader;

    constexpr int SECTOR_SIZE = 512; // unused

    img_info_ =
        tsk_img_open_external (p, p->reader.get_size (), SECTOR_SIZE,
                               mobius_read, mobius_close, mobius_imgstat);

    if (!img_info_)
        throw std::runtime_error (TSK_EXCEPTION_MSG);

    // create filesystem info structure
    fs_info_ = tsk_fs_open_img (img_info_, offset, TSK_FS_TYPE_DETECT);

    if (!fs_info_)
        throw std::runtime_error (TSK_EXCEPTION_MSG);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
adaptor::~adaptor ()
{
    if (fs_info_)
    {
        tsk_fs_close (fs_info_);
        fs_info_ = nullptr;
    }

    if (img_info_)
    {
        tsk_img_close (img_info_);
        img_info_ = nullptr;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get filesystem root folder
// @return Root folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::folder
adaptor::get_root_folder () const
{
    if (!fs_info_)
        _create_tsk ();

    TSK_FS_FILE *p = tsk_fs_file_open_meta (fs_info_, 0, fs_info_->root_inum);

    if (!p)
        throw std::runtime_error (TSK_EXCEPTION_MSG);

    fs_file fs_file (p);
    return mobius::core::io::folder (std::make_shared<folder_impl> (fs_file));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create TSK image and filesystem objects
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
adaptor::_create_tsk () const
{
    // create image info structure
    TSK_IMG_INFO_MOBIUS *p = new TSK_IMG_INFO_MOBIUS ();
    p->reader = reader_;

    constexpr int SECTOR_SIZE = 512; // unused

    img_info_ =
        tsk_img_open_external (p, p->reader.get_size (), SECTOR_SIZE,
                               mobius_read, mobius_close, mobius_imgstat);

    if (!img_info_)
        throw std::runtime_error (TSK_EXCEPTION_MSG);

    // create filesystem info structure
    fs_info_ = tsk_fs_open_img (img_info_, offset_, TSK_FS_TYPE_DETECT);

    if (!fs_info_)
        throw std::runtime_error (TSK_EXCEPTION_MSG);
}

} // namespace mobius::core::vfs::tsk
