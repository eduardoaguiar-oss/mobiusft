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
#include <mobius/core/exception.inc>
#include <mobius/core/vfs/tsk/exception.hpp>
#include <mobius/core/vfs/tsk/fs_file.hpp>
#include <mobius/core/vfs/tsk/stream_impl.hpp>
#include <stdexcept>
#include <tsk/libtsk.h>
#include <algorithm>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file type from name type
// @param name_type Name type
// @return File type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::vfs::tsk::fs_file::fs_file_type
get_file_type_from_name_type (TSK_FS_NAME_TYPE_ENUM name_type)
{
    switch (name_type)
    {
        case TSK_FS_NAME_TYPE_FIFO:
            return mobius::core::vfs::tsk::fs_file::fs_file_type::fifo;
            break;

        case TSK_FS_NAME_TYPE_CHR:
            return mobius::core::vfs::tsk::fs_file::fs_file_type::char_device;
            break;

        case TSK_FS_NAME_TYPE_DIR:
        case TSK_FS_NAME_TYPE_VIRT_DIR:
            return mobius::core::vfs::tsk::fs_file::fs_file_type::folder;
            break;

        case TSK_FS_NAME_TYPE_BLK:
            return mobius::core::vfs::tsk::fs_file::fs_file_type::block_device;
            break;

        case TSK_FS_NAME_TYPE_REG:
        case TSK_FS_NAME_TYPE_VIRT:
            return mobius::core::vfs::tsk::fs_file::fs_file_type::regular;
            break;

        case TSK_FS_NAME_TYPE_LNK:
            return mobius::core::vfs::tsk::fs_file::fs_file_type::symlink;
            break;

        case TSK_FS_NAME_TYPE_SOCK:
            return mobius::core::vfs::tsk::fs_file::fs_file_type::socket;
            break;

        default:
            return mobius::core::vfs::tsk::fs_file::fs_file_type::none;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file type from meta type
// @param meta_type Meta type
// @return File type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::vfs::tsk::fs_file::fs_file_type
get_file_type_from_meta_type (TSK_FS_META_TYPE_ENUM meta_type)
{
    switch (meta_type)
    {
        case TSK_FS_META_TYPE_FIFO:
            return mobius::core::vfs::tsk::fs_file::fs_file_type::fifo;
            break;

        case TSK_FS_META_TYPE_CHR:
            return mobius::core::vfs::tsk::fs_file::fs_file_type::char_device;
            break;

        case TSK_FS_META_TYPE_DIR:
        case TSK_FS_META_TYPE_VIRT_DIR:
            return mobius::core::vfs::tsk::fs_file::fs_file_type::folder;
            break;

        case TSK_FS_META_TYPE_BLK:
            return mobius::core::vfs::tsk::fs_file::fs_file_type::block_device;
            break;

        case TSK_FS_META_TYPE_REG:
        case TSK_FS_META_TYPE_VIRT:
            return mobius::core::vfs::tsk::fs_file::fs_file_type::regular;
            break;

        case TSK_FS_META_TYPE_LNK:
            return mobius::core::vfs::tsk::fs_file::fs_file_type::symlink;
            break;

        case TSK_FS_META_TYPE_SOCK:
            return mobius::core::vfs::tsk::fs_file::fs_file_type::socket;
            break;

        default:
            return mobius::core::vfs::tsk::fs_file::fs_file_type::none;
    }
}

} // namespace

namespace mobius::core::vfs::tsk
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Implementation class for fs_file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class fs_file::impl
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors and destructor
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl () = default;
    explicit impl (TSK_FS_FILE *);
    impl (const impl &) = delete;
    impl (impl &&) noexcept = delete;
    ~impl ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl &operator= (const impl &) = delete;
    impl &operator= (impl &&) noexcept = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_hidden () const;
    void set_path (const std::string &);
    std::uint64_t get_inode () const;
    fs_file_type get_type () const;
    fs_file get_parent () const;
    std::vector<fs_file> get_children () const;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if fs_file is valid
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    constexpr
    operator bool () const noexcept
    {
        return bool (p_);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get internal pointer
    // @return Pointer
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    constexpr TSK_FS_FILE *
    get_pointer () const noexcept
    {
        return p_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if fs_file exists
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    constexpr bool
    exists () const noexcept
    {
        return bool (p_);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if file is deleted
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_deleted () const
    {
        _load_fs_name ();
        return is_deleted_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if file is reallocated
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_reallocated () const
    {
        _load_fs_meta ();
        return is_reallocated_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get file name
    // @return File name
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_name () const
    {
        _load_fs_name ();
        return name_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get short file name
    // @return Short file name
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_short_name () const
    {
        _load_fs_name ();
        return short_name_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get path
    // @return Path
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_path () const
    {
        if (!exists ())
            throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid fs_file"));

        return path_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get file size
    // @return Size
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_size () const
    {
        _load_fs_meta ();
        return size_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get User ID
    // @return User ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    int
    get_user_id () const
    {
        _load_fs_meta ();
        return user_id_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get Group ID
    // @return Group ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    int
    get_group_id () const
    {
        _load_fs_meta ();
        return group_id_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get file access permissions
    // @return Permission mask
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    int
    get_permissions () const
    {
        _load_fs_meta ();
        return permissions_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get creation time
    // @return Creation date/time
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_creation_time () const
    {
        _load_fs_meta ();
        return creation_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get last access timestamp
    // @return Last file access date/time
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_access_time () const
    {
        _load_fs_meta ();
        return access_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get last metadata modification timestamp
    // @return Last file metadata modification date/time
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_modification_time () const
    {
        _load_fs_meta ();
        return modification_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get last metadata modification timestamp
    // @return Last file metadata modification date/time
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_metadata_time () const
    {
        _load_fs_meta ();
        return metadata_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get deletion time
    // @return Deletion date/time
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_deletion_time () const
    {
        _load_fs_meta ();
        return deletion_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get backup time
    // @return Backup date/time
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_backup_time () const
    {
        _load_fs_meta ();
        return backup_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get file streams
    // @return Collection of file streams
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto
    get_streams () const
    {
        _load_streams ();
        return streams_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Reload data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    reload ()
    {
        fs_name_loaded_ = false;
        fs_meta_loaded_ = false;
    }

  private:
    // @brief libtsk file structure pointer
    TSK_FS_FILE *p_ = nullptr;

    // @brief libtsk dir structure pointer (for folders)
    mutable TSK_FS_DIR *dir_p_ = nullptr;

    // @brief i-node
    mutable std::uint64_t inode_ = 0;

    // @brief name
    mutable std::string name_;

    // @brief short name
    mutable std::string short_name_;

    // @brief path
    mutable std::string path_;

    // @brief deleted flag
    mutable bool is_deleted_ = false;

    // @brief file type
    mutable fs_file_type type_ = fs_file_type::none;

    // @brief size in bytes
    mutable std::uint64_t size_ = 0;

    // @brief user id
    mutable int user_id_ = -1;

    // @brief group id
    mutable int group_id_ = -1;

    // @brief RWX permissions
    mutable int permissions_ = 0;

    // @brief creation date/time
    mutable mobius::core::datetime::datetime creation_time_;

    // @brief last modification date/time
    mutable mobius::core::datetime::datetime modification_time_;

    // @brief last access date/time
    mutable mobius::core::datetime::datetime access_time_;

    // @brief last metadata modification date/time
    mutable mobius::core::datetime::datetime metadata_time_;

    // @brief deletion date/time
    mutable mobius::core::datetime::datetime deletion_time_;

    // @brief last backup date/time
    mutable mobius::core::datetime::datetime backup_time_;

    // @brief Streams
    mutable std::vector<const TSK_FS_ATTR *> streams_;

    // @brief reallocated flag
    mutable bool is_reallocated_ = false;

    // @brief fs_name loaded flag
    mutable bool fs_name_loaded_ = false;

    // @brief fs_meta loaded flag
    mutable bool fs_meta_loaded_ = false;

    // @brief Streams loaded flag
    mutable bool streams_loaded_ = false;

  private:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_fs_name () const;
    void _load_fs_meta () const;
    void _load_streams () const;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param p Pointer to file structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
fs_file::impl::impl (TSK_FS_FILE *p)
    : p_ (p)
{
    if (!p)
        throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid fs_file"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
fs_file::impl::~impl ()
{
    if (dir_p_)
        tsk_fs_dir_close (dir_p_);

    if (p_)
        tsk_fs_file_close (p_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set path
// @param path Path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
fs_file::impl::set_path (const std::string &path)
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid fs_file"));

    path_ = path;

    if (name_.empty ())
    {
        auto pos = path.find_last_of ('/');

        if (pos != std::string::npos && pos < path.size () - 1)
            name_ = path.substr (pos + 1);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file inode
// @return Inode
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
fs_file::impl::get_inode () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid fs_file"));

    if (p_->name)
        _load_fs_name ();

    else
        _load_fs_meta ();

    return inode_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file type
// @return Size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
fs_file::fs_file_type
fs_file::impl::get_type () const
{
    _load_fs_name ();

    // _load_fs_meta is expensive, so only call it if type is not set by
    // _load_fs_name or if file is deleted, since deleted files may have
    // type set by _load_fs_name but it may be inaccurate
    if (type_ == fs_file_type::none || is_deleted_)
        _load_fs_meta ();

    return type_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get parent
// @return parent entry, if found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
fs_file
fs_file::impl::get_parent () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid fs_file"));

    fs_file parent;

    if (p_->name && p_->name->meta_addr != p_->name->par_addr)
    {
        TSK_FS_FILE *file_p =
            tsk_fs_file_open_meta (p_->fs_info, nullptr, p_->name->par_addr);

        if (!file_p)
            throw std::runtime_error (TSK_EXCEPTION_MSG);

        // build parent object
        parent = fs_file (file_p);

        auto path = get_path ();
        auto pos = path.find_last_of ('/');

        if (pos != std::string::npos && pos > 0)
            parent.set_path (path.substr (0, pos));
    }

    return parent;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get children
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<fs_file>
fs_file::impl::get_children () const
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Check if fs_file is a folder
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    _load_fs_meta ();

    if (!p_->meta || !p_->meta->addr ||
        (p_->meta->type != TSK_FS_META_TYPE_DIR &&
         p_->meta->type != TSK_FS_META_TYPE_VIRT_DIR))
        return {};

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Try to open directory, if necessary
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (!dir_p_)
    {
        dir_p_ = tsk_fs_dir_open_meta (p_->fs_info, p_->meta->addr);

        if (!dir_p_)
            throw std::runtime_error (TSK_EXCEPTION_MSG);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Read directory entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto count = tsk_fs_dir_getsize (dir_p_);
    auto path = get_path ();

    std::vector<fs_file> children;
    children.reserve (count);

    for (std::size_t i = 0; i < count; i++)
    {
        TSK_FS_FILE *fp = tsk_fs_dir_get (dir_p_, i);
        if (!fp)
            throw std::runtime_error (TSK_EXCEPTION_MSG);

        // if entry is not '.' and '..', create and return true
        fs_file f (fp);

        if (f.get_inode () != p_->meta->addr && f.get_name () != "." &&
            f.get_name () != "..")
        {
            f.set_path (path + '/' + f.get_name ());
            children.push_back (f);
        }
    }

    return children;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load fs_name on demand
//
// According to libtsk manual: A file will typically have a name and metadata.
// This structure holds that type of information. When deleted files are being
// processed, this structure may have the name defined but not metadata because
// it no longer exists. Or, if you are calling meta_walk and are not processing
// at the name level, then the name will not be defined. always check these to
// make sure they are not null before they are read.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
fs_file::impl::_load_fs_name () const
{
    if (!p_)
        throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid fs_file"));

    if (fs_name_loaded_ || !p_->name)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // set data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    inode_ = p_->name->meta_addr;
    is_deleted_ = bool (p_->name->flags & TSK_FS_NAME_FLAG_UNALLOC);

    if (p_->name->name)
        name_ = p_->name->name;

    if (p_->name->shrt_name)
        short_name_ = p_->name->shrt_name;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // set file type
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (type_ == fs_file_type::none)
        type_ = get_file_type_from_name_type (p_->name->type);

    fs_name_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load fs_meta on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
fs_file::impl::_load_fs_meta () const
{
    if (!p_)
        throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid fs_file"));

    if (fs_meta_loaded_)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Retrieve meta structure if needed
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (!p_->meta && p_->name && p_->name->meta_addr)
    {
        int rc =
            p_->fs_info->file_add_meta (p_->fs_info, p_, p_->name->meta_addr);

        if (rc)
            throw std::runtime_error (TSK_EXCEPTION_MSG);
    }

    fs_meta_loaded_ = true;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Set metadata
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (!p_->meta)
        return;

    size_ = p_->meta->size;
    user_id_ = p_->meta->uid;
    group_id_ = p_->meta->gid;
    permissions_ = p_->meta->mode;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Set timestamps
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    access_time_ = mobius::core::datetime::new_datetime_from_unix_timestamp (
        p_->meta->atime
    );
    modification_time_ =
        mobius::core::datetime::new_datetime_from_unix_timestamp (
            p_->meta->mtime
        );
    metadata_time_ = mobius::core::datetime::new_datetime_from_unix_timestamp (
        p_->meta->ctime
    );
    creation_time_ = mobius::core::datetime::new_datetime_from_unix_timestamp (
        p_->meta->crtime
    );

    if (p_->fs_info->ftype & TSK_FS_TYPE_EXT_DETECT)
        deletion_time_ =
            mobius::core::datetime::new_datetime_from_unix_timestamp (
                p_->meta->time2.ext2.dtime
            );

    if (p_->fs_info->ftype & TSK_FS_TYPE_HFS_DETECT)
        backup_time_ =
            mobius::core::datetime::new_datetime_from_unix_timestamp (
                p_->meta->time2.hfs.bkup_time
            );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // If file has name, check if it is reallocated
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (p_->name)
        is_reallocated_ = (p_->name->flags & TSK_FS_NAME_FLAG_UNALLOC) &&
                          (p_->meta->flags & TSK_FS_META_FLAG_ALLOC);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Set i-node
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (!inode_)
        inode_ = p_->meta->addr;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Set type
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (type_ == fs_file_type::none)
        type_ = get_file_type_from_meta_type (p_->meta->type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load streams on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
fs_file::impl::_load_streams () const
{
    if (streams_loaded_)
        return;

    _load_fs_meta ();

    std::size_t count = tsk_fs_file_attr_getsize (p_);
    streams_.resize (count);

    for (std::size_t i = 0; i < count; i++)
    {
        streams_[i] = tsk_fs_file_attr_get_idx (p_, i);

        if (!streams_[i])
            throw std::runtime_error (TSK_EXCEPTION_MSG);
    }

    streams_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize default object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
fs_file::fs_file ()
    : impl_ (std::make_shared<impl> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize object
// @param p Pointer to file structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
fs_file::fs_file (TSK_FS_FILE *p)
    : impl_ (std::make_shared<impl> (p))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if fs_file is valid
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
fs_file::
operator bool () const noexcept
{
    return impl_->operator bool ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get internal pointer
// @return Pointer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
TSK_FS_FILE *
fs_file::get_pointer () const
{
    return impl_->get_pointer ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file exists
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
fs_file::exists () const
{
    return impl_->exists ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file name
// @return File name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
fs_file::get_name () const
{
    return impl_->get_name ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get short file name
// @return Short file name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
fs_file::get_short_name () const
{
    return impl_->get_short_name ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get path
// @return Path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
fs_file::get_path () const
{
    return impl_->get_path ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set path
// @param path Path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
fs_file::set_path (const std::string &path)
{
    impl_->set_path (path);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file is deleted
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
fs_file::is_deleted () const
{
    return impl_->is_deleted ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file is reallocated
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
fs_file::is_reallocated () const
{
    return impl_->is_reallocated ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file inode
// @return Inode
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
fs_file::get_inode () const
{
    return impl_->get_inode ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file size
// @return Size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
fs_file::get_size () const
{
    return impl_->get_size ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file type
// @return Size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
fs_file::fs_file_type
fs_file::get_type () const
{
    return impl_->get_type ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get User ID
// @return User ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
fs_file::get_user_id () const
{
    return impl_->get_user_id ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get Group ID
// @return Group ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
fs_file::get_group_id () const
{
    return impl_->get_group_id ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file access permissions
// @return Permission mask
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
fs_file::get_permissions () const
{
    return impl_->get_permissions ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get creation time
// @return Creation date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
fs_file::get_creation_time () const
{
    return impl_->get_creation_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last access timestamp
// @return Last file access date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
fs_file::get_access_time () const
{
    return impl_->get_access_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last metadata modification timestamp
// @return Last file metadata modification date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
fs_file::get_modification_time () const
{
    return impl_->get_modification_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last metadata modification timestamp
// @return Last file metadata modification date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
fs_file::get_metadata_time () const
{
    return impl_->get_metadata_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get deletion time
// @return Deletion date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
fs_file::get_deletion_time () const
{
    return impl_->get_deletion_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get backup time
// @return Backup date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
fs_file::get_backup_time () const
{
    return impl_->get_backup_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reload data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
fs_file::reload ()
{
    impl_->reload ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get parent
// @return parent entry, if found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
fs_file
fs_file::get_parent () const
{
    return impl_->get_parent ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get children
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<fs_file>
fs_file::get_children () const
{
    return impl_->get_children ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get streams
// @return Streams
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<fs_file::stream_type>
fs_file::get_streams () const
{
    auto stream_pointers = impl_->get_streams ();

    std::vector<stream_type> streams (stream_pointers.size ());

    std::transform (
        stream_pointers.begin (), stream_pointers.end (), streams.begin (),
        [this] (const TSK_FS_ATTR *attr_p)
        { return std::make_shared<stream_impl> (*this, attr_p); }
    );

    return streams;
}

} // namespace mobius::core::vfs::tsk
