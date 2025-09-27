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
#include <mobius/core/exception_posix.inc>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/folder.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/io/smb/file_impl.hpp>
#include <mobius/core/io/smb/folder_impl.hpp>
#include <mobius/core/io/smb/init.hpp>
#include <mobius/core/io/uri.hpp>
#include <mobius/core/system/group.hpp>
#include <mobius/core/system/user.hpp>
#include <dirent.h>
#include <libsmbclient.h>
#include <memory>
#include <stdexcept>

namespace mobius::core::io::smb
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize object
// @param url Folder URL
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::folder_impl (const std::string &url)
    : url_ (url)
{
    init (); // initialize SMB if necessary

    mobius::core::io::uri uri (url);
    name_ = uri.get_filename ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if folder exists
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
// @return Folder name
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
// @brief Get folder owner ID
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
// @brief Get folder owner name
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
// @brief Get folder group ID
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
// @brief Get folder group name
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
// @brief Get folder access permissions
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
    mobius::core::io::uri uri (url_);
    auto parent = uri.get_parent ();

    return std::make_shared<folder_impl> (parent.get_value ());
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
    if (smbc_mkdir (url_.c_str (), 0755) < 0)
    {
        if (errno == ENOENT)
        {
            auto parent = get_parent ();
            parent->create ();

            if (smbc_mkdir (url_.c_str (), 0755) < 0 && errno != EEXIST)
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

    flag_children_loaded_ = false;
    flag_stat_loaded_ = false;
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
    smbc_rmdir (url_.c_str ());

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

    // create new URI
    mobius::core::io::uri old_uri (url_);
    mobius::core::io::uri new_uri = old_uri.get_sibling_by_name (name);

    // rename file
    if (smbc_rename (url_.c_str (), new_uri.get_value ().c_str ()) < 0)
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

    // update attributes
    url_ = new_uri.get_value ();
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
        if (smbc_rename (url_.c_str (), pimpl->url_.c_str ()) < 0)
            throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

        return true;
    }

    return false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load metadata calling smbc_stat function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::_load_stat () const
{
    if (flag_stat_loaded_)
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
        if ((st.st_mode & S_IFMT) != S_IFDIR)
            throw std::invalid_argument (
                MOBIUS_EXCEPTION_MSG ("entry is not folder")
            );

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
    }

    flag_stat_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load children
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::_load_children () const
{
    if (flag_children_loaded_)
        return;

    // Try to open directory
    int fd = smbc_opendir (url_.c_str ());

    if (fd < 0)
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

    // Read directory entries
    auto *p_entry = smbc_readdir (fd);

    while (p_entry)
    {
        if (strcmp (p_entry->name, ".") && strcmp (p_entry->name, ".."))
        {
            const std::string url = url_ + '/' + p_entry->name;

            mobius::core::io::entry e;

            if (p_entry->smbc_type == SMBC_DIR)
                e = entry (folder (std::make_shared<folder_impl> (url)));

            else // everything else is file...
                e = entry (file (std::make_shared<file_impl> (url)));

            children_.push_back (e);
        }

        p_entry = smbc_readdir (fd);
    }

    // Check for errors
    if (errno)
    {
        smbc_closedir (fd);
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
    }

    // Close directory
    smbc_closedir (fd);

    // Set children loaded flag
    flag_children_loaded_ = true;
}

} // namespace mobius::core::io::smb
