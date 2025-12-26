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
#include <mobius/core/exception_posix.inc>
#include <mobius/core/io/entry.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/folder.hpp>
#include <mobius/core/io/local/file_impl.hpp>
#include <mobius/core/io/local/folder_impl.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/io/uri.hpp>
#include <mobius/core/system/group.hpp>
#include <mobius/core/system/user.hpp>
#include <dirent.h>
#include <memory>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace mobius::core::io::local
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize object
// @param path Folder path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::folder_impl (const std::string &path)
    : path_ (path),
      given_path_ (path)
{
    if (path_.empty ())
        throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid path"));

    mobius::core::io::path p (path_);
    name_ = p.get_filename ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief check if URL folder exists
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
folder_impl::exists () const
{
    if (!flag_stat_loaded_)
        _load_stat ();

    return exists_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if folder is deleted
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
folder_impl::is_deleted () const
{
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return false; // local folders are never deleted
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if folder is reallocated
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
folder_impl::is_reallocated () const
{
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return false; // local folders are never reallocated
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if folder is hidden
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
folder_impl::is_hidden () const
{
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return !name_.empty () && name_[0] == '.';
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folder name
// @return File name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
folder_impl::get_name () const
{
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return name_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get short folder name
// @return Short folder name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
folder_impl::get_short_name () const
{
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return std::string (); // local files don't have short names
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folder inode
// @return Inode
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::inode_type
folder_impl::get_inode () const
{
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return inode_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folder size
// @return Size in bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::size_type
folder_impl::get_size () const
{
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return size_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get folder owner's user id
// @return user ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::user_id_type
folder_impl::get_user_id () const
{
    if (!flag_stat_loaded_)
        _load_stat ();

    if (!exists_)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return user_id_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get folder owner's user name
// @return user name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
folder_impl::get_user_name () const
{
    if (!flag_stat_loaded_)
        _load_stat ();

    if (!exists_)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return user_name_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get folder group ID
// @return group ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::group_id_type
folder_impl::get_group_id () const
{
    if (!flag_stat_loaded_)
        _load_stat ();

    if (!exists_)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return group_id_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get folder owner's group name
// @return group name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
folder_impl::get_group_name () const
{
    if (!flag_stat_loaded_)
        _load_stat ();

    if (!exists_)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return group_name_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get file access permissions
// @return permission mask
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::permission_type
folder_impl::get_permissions () const
{
    if (!flag_stat_loaded_)
        _load_stat ();

    if (!exists_)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return permissions_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last access timestamp
// @return Last folder access date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
folder_impl::get_access_time () const
{
    if (!flag_stat_loaded_)
        _load_stat ();

    if (!exists_)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return access_time_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last metadata modification timestamp
// @return Last folder metadata modification date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
folder_impl::get_modification_time () const
{
    if (!flag_stat_loaded_)
        _load_stat ();

    if (!exists_)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return modification_time_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last metadata modification timestamp
// @return Last folder metadata modification date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
folder_impl::get_metadata_time () const
{
    if (!flag_stat_loaded_)
        _load_stat ();

    if (!exists_)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return metadata_time_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get creation time
// @return Creation date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
folder_impl::get_creation_time () const
{
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return mobius::core::datetime::datetime (); // local files don't have
                                                // creation time
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get deletion time
// @return Deletion date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
folder_impl::get_deletion_time () const
{
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return mobius::core::datetime::datetime (); // local files don't have
                                                // deletion time
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get backup time
// @return Backup date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
folder_impl::get_backup_time () const
{
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return mobius::core::datetime::datetime (); // local files don't have backup
                                                // time
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get parent folder
// @return Parent folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::folder_type
folder_impl::get_parent () const
{
    mobius::core::io::path path (path_);

    return std::make_shared<folder_impl> (path.get_dirname ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get children
// @return Collection
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<entry>
folder_impl::get_children () const
{
    _load_children ();

    return children_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::create ()
{
    if (mkdir (path_.c_str (), 0755) == -1)
    {
        if (errno == ENOENT)
        {
            auto parent = get_parent ();
            parent->create ();

            if (mkdir (path_.c_str (), 0755) == -1 && errno != EEXIST)
                throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
        }

        else if (errno != EEXIST)
            throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
    }

    flag_stat_loaded_ = false;
    flag_children_loaded_ = false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clear folder content
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::clear ()
{
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    for (auto entry : get_children ())
    {
        if (entry.is_folder ())
            entry.get_folder ().remove ();

        else
            entry.get_file ().remove ();
    }

    flag_stat_loaded_ = false;
    flag_children_loaded_ = false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reload folder info
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::reload ()
{
    flag_stat_loaded_ = false;
    flag_children_loaded_ = false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::remove ()
{
    clear ();
    rmdir (path_.c_str ());

    flag_stat_loaded_ = false;
    flag_children_loaded_ = false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Rename folder
// @param name New folder name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::rename (const std::string &name)
{
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    // create new path
    mobius::core::io::path old_path (path_);
    mobius::core::io::path new_path = old_path.get_sibling_by_name (name);

    // rename folder
    if (::rename (path_.c_str (), new_path.get_value ().c_str ()) == -1)
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

    // update attributes
    path_ = new_path.get_value ();
    name_ = name;

    flag_stat_loaded_ = false; // force reload of attributes
    flag_children_loaded_ = false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Move folder
// @param folder File implementation pointer
// @return <b>true</b> if folder has been moved, <b>false</b> otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
folder_impl::move (folder_type impl)
{
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    // if destination folder impl is also local, use rename function
    auto pimpl = std::dynamic_pointer_cast<folder_impl> (impl);

    if (pimpl)
    {
        if (::rename (path_.c_str (), pimpl->path_.c_str ()) == -1)
            throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

        return true;
    }

    return false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load metadata calling POSIX stat function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::_load_stat () const
{
    if (flag_stat_loaded_)
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
                st.st_atime
            );
        modification_time_ =
            mobius::core::datetime::new_datetime_from_unix_timestamp (
                st.st_mtime
            );
        metadata_time_ =
            mobius::core::datetime::new_datetime_from_unix_timestamp (
                st.st_ctime
            );

        // get user name
        mobius::core::system::user user (user_id_);
        if (user)
            user_name_ = user.get_name ();

        // get group name
        mobius::core::system::group group (group_id_);
        if (group)
            group_name_ = group.get_name ();
    }

    flag_stat_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load children calling POSIX readdir function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::_load_children () const
{
    // Return if children are already loaded
    if (flag_children_loaded_)
        return;

    // Check if folder exists
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    // Open directory
    DIR *dir_p = opendir (path_.c_str ());

    if (!dir_p)
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

    // Read directory entries
    errno = 0;
    dirent *p_entry = readdir (dir_p);

    while (p_entry)
    {
        if (strcmp (p_entry->d_name, ".") && strcmp (p_entry->d_name, ".."))
        {
            const std::string path = path_ + '/' + p_entry->d_name;
            const std::string given_path = given_path_ + '/' + p_entry->d_name;

            mobius::core::io::entry e;

            if (p_entry->d_type == DT_DIR)
                e = entry (folder (std::make_shared<folder_impl> (path)));

            else // everything else is file...
                e = entry (file (std::make_shared<file_impl> (path)));

            e.set_path (given_path);

            children_.push_back (e);
        }

        p_entry = readdir (dir_p);
    }

    // Check for errors
    if (errno)
    {
        closedir (dir_p);
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
    }

    // Close directory
    closedir (dir_p);

    // set are children loaded flag
    flag_children_loaded_ = true;
}

} // namespace mobius::core::io::local
