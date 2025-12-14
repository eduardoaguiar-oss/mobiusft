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
#include <climits>
#include <mobius/core/exception.inc>
#include <mobius/core/exception_posix.inc>
#include <mobius/core/io/entry.hpp>
#include <mobius/core/io/local/file_impl.hpp>
#include <mobius/core/io/local/folder_impl.hpp>
#include <mobius/core/io/local/reader_impl.hpp>
#include <mobius/core/io/local/writer_impl.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/io/uri.hpp>
#include <mobius/core/system/group.hpp>
#include <mobius/core/system/user.hpp>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>

namespace mobius::core::io::local
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize object
// @param path File path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_impl::file_impl (const std::string &path)
    : path_ (path),
      given_path_ (path)
{
    if (path_.empty ())
        throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid path"));

    mobius::core::io::path p (path_);
    name_ = p.get_filename ();
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

    return false; // local files are never deleted
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

    return false; // local files are never reallocated
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

    return std::string (); // local files don't have short names
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
// @brief get file owner's user id
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
// @brief get file owner's user name
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
// @brief get file group ID
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
// @brief get file owner's group name
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
// @brief get file access permissions
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

    return mobius::core::datetime::datetime (); // local files don't have
                                                // creation time
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

    return mobius::core::datetime::datetime (); // local files don't have
                                                // deletion time
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

    return mobius::core::datetime::datetime (); // local files don't have backup
                                                // time
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get parent folder
// @return Parent folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_impl::folder_type
file_impl::get_parent () const
{
    mobius::core::io::path path (path_);

    return std::make_shared<folder_impl> (path.get_dirname ());
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

    if (::remove (path_.c_str ()) == -1)
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

    // create new path
    mobius::core::io::path old_path (path_);
    mobius::core::io::path new_path = old_path.get_sibling_by_name (filename);

    // rename file
    if (::rename (path_.c_str (), new_path.get_value ().c_str ()) == -1)
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

    // update attributes
    path_ = new_path.get_value ();
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

    // if destination file impl is also local, use rename function
    auto pimpl = std::dynamic_pointer_cast<file_impl> (impl);

    if (pimpl)
    {
        if (::rename (path_.c_str (), pimpl->path_.c_str ()) == -1)
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

    return std::make_shared<reader_impl> (path_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create a writer for file
// @param overwite true/false to wipe file content
// @return Pointer to a new created writer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_impl::writer_type
file_impl::new_writer (bool overwrite) const
{
    return std::make_shared<writer_impl> (path_, overwrite);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load metadata calling POSIX stat function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_impl::_load_stat () const
{
    if (is_stat_loaded_)
        return;

    struct stat st;

    if (lstat (path_.c_str (), &st)) // error
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

        // get user name
        mobius::core::system::user user (user_id_);
        if (user)
            user_name_ = user.get_name ();

        // get group name
        mobius::core::system::group group (group_id_);
        if (group)
            group_name_ = group.get_name ();

        // handle file according to type
        auto t = st.st_mode & S_IFMT;

        if (t == S_IFBLK)
            type_ = type::block_device;

        else if (t == S_IFCHR)
            type_ = type::char_device;

        else if (t == S_IFIFO)
            type_ = type::fifo;

        else if (t == S_IFLNK)
        {
            type_ = type::symlink;

            /*char buffer[PATH_MAX];
            auto rc = readlink (path_.c_str (), buffer, PATH_MAX);

            if (rc != -1)
              metadata_.set ("linkpath", std::string (buffer, rc));*/
        }

        else if (t == S_IFREG)
            type_ = type::regular;

        else if (t == S_IFSOCK)
            type_ = type::socket;

        else
            type_ = type::none;
    }

    is_stat_loaded_ = true;
}

} // namespace mobius::core::io::local
