// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008-2026 Eduardo Aguiar
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
#include <mobius/core/vfs/tsk/adaptor.hpp>
#include <mobius/core/vfs/tsk/exception.hpp>
#include <mobius/core/vfs/tsk/folder_impl.hpp>
#include <cstring>
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
// @brief Read data from reader
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
// @brief Close TSK_IMG_INFO_MOBIUS
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
mobius_close (TSK_IMG_INFO *img)
{
    TSK_IMG_INFO_MOBIUS *p = (TSK_IMG_INFO_MOBIUS *) img;
    delete p;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show img status
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
mobius_imgstat (TSK_IMG_INFO *, FILE *)
{
}

} // namespace

namespace mobius::core::vfs::tsk
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Adaptor implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class adaptor::impl
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors and destructor
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl (const mobius::core::io::reader &, std::uint64_t);
    impl (
        const mobius::core::io::reader &,
        std::uint64_t,
        std::uint64_t,
        std::uint64_t
    );
    impl (const impl &) = delete;
    impl (impl &&) noexcept = delete;
    ~impl ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Assignment operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl &operator= (const impl &) = delete;
    impl &operator= (impl &&) noexcept = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::io::folder get_root_folder () const;

  private:
    // @brief Reader object
    mobius::core::io::reader reader_;

    // @brief Offset in bytes from the beginning of the stream
    std::uint64_t offset_ = 0;

    // @brief Volume Superblock offset in bytes from the beginning of the stream
    std::uint64_t volume_offset_ = 0;

    // @brief Volume Superblock block number
    std::uint64_t volume_block_number_ = 0;

    // @brief TSK image, filesystem and pool info structures
    mutable TSK_IMG_INFO *img_info_ = nullptr;
    mutable TSK_FS_INFO *fs_info_ = nullptr;
    mutable const TSK_POOL_INFO *pool_info_ = nullptr;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _create_tsk () const;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param offset Offset in bytes from the beginning of the stream
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
adaptor::impl::impl (
    const mobius::core::io::reader &reader, std::uint64_t offset
)
    : reader_ (reader),
      offset_ (offset)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param offset Offset in bytes from the beginning of the stream
// @param volume_offset Volume Superblock offset in bytes from the beginning of the stream
// @param volume_block_number Volume Superblock block number
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
adaptor::impl::impl (
    const mobius::core::io::reader &reader,
    std::uint64_t offset,
    std::uint64_t volume_offset,
    std::uint64_t volume_block_number
)
    : reader_ (reader),
      offset_ (offset),
      volume_offset_ (volume_offset),
      volume_block_number_ (volume_block_number)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
adaptor::impl::~impl ()
{
    if (fs_info_)
    {
        tsk_fs_close (fs_info_);
        fs_info_ = nullptr;
    }

    if (pool_info_)
    {
        tsk_pool_close (pool_info_);
        pool_info_ = nullptr;
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
adaptor::impl::get_root_folder () const
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
// @brief Create TSK image and filesystem objects
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
adaptor::impl::_create_tsk () const
{
    // create image info structure
    TSK_IMG_INFO_MOBIUS *p = new TSK_IMG_INFO_MOBIUS ();
    p->reader = reader_;

    constexpr int SECTOR_SIZE = 512; // unused

    img_info_ = tsk_img_open_external (
        p, p->reader.get_size (), SECTOR_SIZE, mobius_read, mobius_close,
        mobius_imgstat
    );

    if (!img_info_)
        throw std::runtime_error (TSK_EXCEPTION_MSG);

    // Create APFS pool if volume block number is specified
    if (volume_block_number_ > 0)
    {
        // Open APFS pool
        pool_info_ =
            tsk_pool_open_img_sing (img_info_, offset_, TSK_POOL_TYPE_APFS);

        if (!pool_info_)
            throw std::runtime_error (TSK_EXCEPTION_MSG);

        // Open APFS volume
        fs_info_ = tsk_fs_open_img (
            pool_info_->get_img_info (pool_info_, volume_block_number_),
            volume_offset_, TSK_FS_TYPE_APFS
        );

        if (!fs_info_)
            throw std::runtime_error (TSK_EXCEPTION_MSG);
    }

    // If VSB offset is not specified, create regular filesystem info structure
    else
    {
        fs_info_ = tsk_fs_open_img (img_info_, offset_, TSK_FS_TYPE_DETECT);

        if (!fs_info_)
            throw std::runtime_error (TSK_EXCEPTION_MSG);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param offset Offset in bytes from the beginning of the stream
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
adaptor::adaptor (const mobius::core::io::reader &reader, std::uint64_t offset)
    : impl_ (std::make_shared<impl> (reader, offset))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param offset Offset in bytes from the beginning of the stream
// @param volume_offset Volume Superblock offset in bytes from the beginning of the stream
// @param volume_block_number Volume Superblock block number
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
adaptor::adaptor (
    const mobius::core::io::reader &reader,
    std::uint64_t offset,
    std::uint64_t volume_offset,
    std::uint64_t volume_block_number
)
    : impl_ (
          std::make_shared<impl> (
              reader, offset, volume_offset, volume_block_number
          )
      )
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get filesystem root folder
// @return Root folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::folder
adaptor::get_root_folder () const
{
    return impl_->get_root_folder ();
}

} // namespace mobius::core::vfs::tsk
