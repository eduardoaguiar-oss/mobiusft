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
#include <libsmbclient.h>
#include <mobius/core/exception.inc>
#include <mobius/core/exception_posix.inc>
#include <mobius/core/io/smb/file_impl.hpp>
#include <mobius/core/io/smb/folder_impl.hpp>
#include <mobius/core/io/smb/init.hpp>
#include <mobius/core/io/smb/reader_impl.hpp>
#include <mobius/core/io/smb/writer_impl.hpp>
#include <mobius/core/io/uri.hpp>
#include <stdexcept>

namespace mobius::core::io::smb
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize object
// @param url File URL
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_impl::file_impl (const std::string &url)
    : url_ (url)
{
    init (); // initialize SMB if necessary

    mobius::core::io::uri uri (url);
    name_ = uri.get_filename ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file name
// @return File name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
file_impl::get_name () const
{
    return name_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if URL file exists
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
file_impl::exists () const
{
    if (!is_stat_loaded_)
        _load_stat ();

    return exists_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file is deleted
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
file_impl::is_deleted () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return false; // smb files are never deleted
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file is reallocated
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
file_impl::is_reallocated () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return false; // smb files are never reallocated
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file is hidden
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
file_impl::is_hidden () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return !name_.empty () && name_[0] == '.';
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get short file name
// @return Short file name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
file_impl::get_short_name () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return std::string (); // smb files don't have short names
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file inode
// @return Inode
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_impl::inode_type
file_impl::get_inode () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return inode_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file size
// @return Size in bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_impl::size_type
file_impl::get_size () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return size_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file type
// @return File type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_impl::type
file_impl::get_type () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return type_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file owner's user id
// @return user ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_impl::user_id_type
file_impl::get_user_id () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return user_id_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file owner's user name
// @return user name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
file_impl::get_user_name () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return user_name_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file group ID
// @return group ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_impl::group_id_type
file_impl::get_group_id () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return group_id_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file owner's group name
// @return group name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
file_impl::get_group_name () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return group_name_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file access permissions
// @return permission mask
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_impl::permission_type
file_impl::get_permissions () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return permissions_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last access timestamp
// @return Last file access date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
file_impl::get_access_time () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return access_time_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last metadata modification timestamp
// @return Last file metadata modification date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
file_impl::get_modification_time () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return modification_time_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last metadata modification timestamp
// @return Last file metadata modification date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
file_impl::get_metadata_time () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return metadata_time_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get creation time
// @return Creation date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
file_impl::get_creation_time () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return mobius::core::datetime::datetime (); // smb files don't have creation
                                                // time
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get deletion time
// @return Deletion date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
file_impl::get_deletion_time () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return mobius::core::datetime::datetime (); // smb files don't have deletion
                                                // time
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get backup time
// @return Backup date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
file_impl::get_backup_time () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return mobius::core::datetime::datetime (); // smb files don't have backup
                                                // time
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get parent folder
// @return Parent folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_impl::folder_type
file_impl::get_parent () const
{
    mobius::core::io::uri uri (url_);
    auto parent = uri.get_parent ();

    return std::make_shared<folder_impl> (parent.get_value ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reload file info
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_impl::reload ()
{
    is_stat_loaded_ = false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_impl::remove ()
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    if (smbc_unlink (url_.c_str ()) < 0)
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

    is_stat_loaded_ = false; // force reload of attributes
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Rename file
// @param filename New filename
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_impl::rename (const std::string &filename)
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    // create new URI
    mobius::core::io::uri old_uri (url_);
    mobius::core::io::uri new_uri = old_uri.get_sibling_by_name (filename);

    // rename file
    if (smbc_rename (url_.c_str (), new_uri.get_value ().c_str ()) < 0)
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

    // update attributes
    url_ = new_uri.get_value ();
    name_ = filename;

    is_stat_loaded_ = false; // force reload of attributes
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Move file
// @param f File implementation pointer
// @return <b>true</b> if file has been moved, <b>false</b> otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
file_impl::move (file_type impl)
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    // if destination file impl is also smb, use rename function
    auto pimpl = std::dynamic_pointer_cast<file_impl> (impl);

    if (pimpl)
    {
        if (smbc_rename (url_.c_str (), pimpl->url_.c_str ()) < 0)
            throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

        return true;
    }

    return false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create a reader for file
// @return Pointer to a new created reader
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_impl::reader_type
file_impl::new_reader () const
{
    if (!exists ())
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("file does not exist"));

    return std::make_shared<reader_impl> (url_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create a writer for file
// @param overwite true/false to wipe file content
// @return Pointer to a new created writer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_impl::writer_type
file_impl::new_writer (bool overwrite) const
{
    return std::make_shared<writer_impl> (url_, overwrite);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load metadata calling smbc_stat function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_impl::_load_stat () const
{
    if (is_stat_loaded_)
        return;

    struct stat st;

    if (smbc_stat (url_.c_str (), &st) < 0) // error
    {
        exists_ = false;

        if (errno != ENOENT && errno != ENOTDIR)
            throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
    }

    else
    {
        exists_ = true;
        inode_ = st.st_ino;
        size_ = st.st_size;
        user_id_ = st.st_uid;
        group_id_ = st.st_gid;
        permissions_ = st.st_mode & 0777;
        access_time_ =
            mobius::core::datetime::new_datetime_from_unix_timestamp (
                st.st_atime);
        modification_time_ =
            mobius::core::datetime::new_datetime_from_unix_timestamp (
                st.st_mtime);
        metadata_time_ =
            mobius::core::datetime::new_datetime_from_unix_timestamp (
                st.st_ctime);

        switch (st.st_mode & S_IFMT)
        {
        case S_IFBLK:
            type_ = type::block_device;
            break;
        case S_IFCHR:
            type_ = type::char_device;
            break;
        case S_IFIFO:
            type_ = type::fifo;
            break;
        case S_IFLNK:
            type_ = type::symlink;
            break;
        case S_IFREG:
            type_ = type::regular;
            break;
        case S_IFSOCK:
            type_ = type::socket;
            break;
        default:
            type_ = type::none;
            break;
        }
    }

    is_stat_loaded_ = true;
}

} // namespace mobius::core::io::smb
