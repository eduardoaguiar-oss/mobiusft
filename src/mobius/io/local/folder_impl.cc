// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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
#include "folder_impl.h"
#include "file_impl.h"
#include "folder.h"
#include "file.h"
#include "path.h"
#include "uri.h"
#include <mobius/exception.inc>
#include <mobius/exception_posix.inc>
#include <mobius/system/user.h>
#include <mobius/system/group.h>
#include <mobius/collection_impl_base.h>
#include <memory>
#include <stdexcept>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace mobius::io::local
{
namespace
{
using entry_impl = folder_impl_base::entry_impl;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Collection implementation for folder entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class collection_impl_folder : public mobius::collection_impl_base <entry_impl>
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  explicit collection_impl_folder (const std::string&, const std::string&);
  ~collection_impl_folder ();
  bool get (entry_impl&) override;
  void reset () override;

private:
  const std::string path_;
  const std::string given_path_;
  DIR *dir_p_ = nullptr;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize object
// @param path Path to local folder
// @param given_path Path given by user
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
collection_impl_folder::collection_impl_folder (
  const std::string& path,
  const std::string& given_path
)
  : path_ (path),
    given_path_ (given_path)
{
  dir_p_ = opendir (path.c_str ());

  if (!dir_p_)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
collection_impl_folder::~collection_impl_folder ()
{
  if (dir_p_)
    closedir (dir_p_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folder entry
// @param e Entry reference
// @return true/false if entry was found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
collection_impl_folder::get (entry_impl& e)
{
  errno = 0;
  dirent *p_entry = readdir (dir_p_);

  while (p_entry && (!strcmp (p_entry->d_name, ".") || !strcmp (p_entry->d_name, "..")))
    p_entry = readdir (dir_p_);

  if (p_entry)
    {
      const std::string path = path_ + '/' + p_entry->d_name;
      const std::string given_path = given_path_ + '/' + p_entry->d_name;

      if (p_entry->d_type == DT_DIR)
        {
          e.folder_p = std::make_shared <folder_impl> (path);
          e.folder_p->set_path (given_path);
        }

      else  // everything else is file...
        {
          e.file_p = std::make_shared <file_impl> (path);
          e.file_p->set_path (given_path);
	}

      return true;
    }

  else if (errno)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

  return false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset collection
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
collection_impl_folder::reset ()
{
  rewinddir (dir_p_);
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize object
// @param path Folder path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::folder_impl (const std::string& path)
 : path_ (path), given_path_ (path)
{
  if (path_.empty ())
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid path"));

  mobius::io::path p (path_);
  name_ = p.get_filename ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief check if URL folder exists
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
folder_impl::exists () const
{
  if (!is_stat_loaded_)
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
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  return false;  // local folders are never deleted
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if folder is reallocated
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
folder_impl::is_reallocated () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  return false;  // local folders are never reallocated
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if folder is hidden
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
folder_impl::is_hidden () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

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
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

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
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

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
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

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
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  return size_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get folder owner's user id
// @return user ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::user_id_type
folder_impl::get_user_id () const
{
  if (!is_stat_loaded_)
    _load_stat ();

  if (!exists_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  return user_id_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get folder owner's user name
// @return user name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
folder_impl::get_user_name () const
{
  if (!is_stat_loaded_)
    _load_stat ();

  if (!exists_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  return user_name_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get folder group ID
// @return group ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::group_id_type
folder_impl::get_group_id () const
{
  if (!is_stat_loaded_)
    _load_stat ();

  if (!exists_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  return group_id_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get folder owner's group name
// @return group name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
folder_impl::get_group_name () const
{
  if (!is_stat_loaded_)
    _load_stat ();

  if (!exists_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  return group_name_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get file access permissions
// @return permission mask
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::permission_type
folder_impl::get_permissions () const
{
  if (!is_stat_loaded_)
    _load_stat ();

  if (!exists_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  return permissions_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last access timestamp
// @return Last folder access date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
folder_impl::get_access_time () const
{
  if (!is_stat_loaded_)
    _load_stat ();

  if (!exists_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  return access_time_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last metadata modification timestamp
// @return Last folder metadata modification date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
folder_impl::get_modification_time () const
{
  if (!is_stat_loaded_)
    _load_stat ();

  if (!exists_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  return modification_time_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last metadata modification timestamp
// @return Last folder metadata modification date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
folder_impl::get_metadata_time () const
{
  if (!is_stat_loaded_)
    _load_stat ();

  if (!exists_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  return metadata_time_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get creation time
// @return Creation date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
folder_impl::get_creation_time () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  return mobius::datetime::datetime (); // local files don't have creation time
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get deletion time
// @return Deletion date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
folder_impl::get_deletion_time () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  return mobius::datetime::datetime (); // local files don't have deletion time
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get backup time
// @return Backup date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
folder_impl::get_backup_time () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  return mobius::datetime::datetime (); // local files don't have backup time
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get parent folder
// @return Parent folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::folder_type
folder_impl::get_parent () const
{
  mobius::io::path path (path_);

  return std::make_shared <folder_impl> (path.get_dirname ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get children
// @return Collection
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::children_type
folder_impl::get_children () const
{
  return std::make_shared <collection_impl_folder> (path_, given_path_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new file object
// @param name File name
// @return Pointer to file object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::file_type
folder_impl::new_file (const std::string& name) const
{
  auto path = mobius::io::path (path_);
  auto child_path = path.get_child_by_name (name);

  return std::make_shared <file_impl> (child_path.get_value ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new folder object
// @param name Folder name
// @return Pointer to folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::folder_type
folder_impl::new_folder (const std::string& name) const
{
  auto path = mobius::io::path (path_);
  auto child_path = path.get_child_by_name (name);

  return std::make_shared <folder_impl> (child_path.get_value ());
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

  is_stat_loaded_ = false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clear folder content
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::clear ()
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  auto col = get_children ();
  entry_impl e;

  while (col->get (e))
    {
      if (e.folder_p)
        e.folder_p->remove ();

      else
        e.file_p->remove ();
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reload folder info
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::reload ()
{
  is_stat_loaded_ = false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::remove ()
{
  clear ();
  rmdir (path_.c_str ());

  is_stat_loaded_ = false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Rename folder
// @param name New folder name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::rename (const std::string& name)
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  // create new path
  mobius::io::path old_path (path_);
  mobius::io::path new_path = old_path.get_sibling_by_name (name);

  // rename folder
  if (::rename (path_.c_str (), new_path.get_value ().c_str ()) == -1)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

  // update attributes
  path_ = new_path.get_value ();
  name_ = name;
  is_stat_loaded_ = false;  // force reload of attributes
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
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  // if destination folder impl is also local, use rename function
  auto pimpl = std::dynamic_pointer_cast <folder_impl> (impl);

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
  if (is_stat_loaded_)
    return;

  struct stat st;

  if (lstat (path_.c_str (), &st))   // error
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
      access_time_ = mobius::datetime::new_datetime_from_unix_timestamp (st.st_atime);
      modification_time_ = mobius::datetime::new_datetime_from_unix_timestamp (st.st_mtime);
      metadata_time_ = mobius::datetime::new_datetime_from_unix_timestamp (st.st_ctime);

      // get user name
      mobius::system::user user (user_id_);
      if (user)
        user_name_ = user.get_name ();

      // get group name
      mobius::system::group group (group_id_);
      if (group)
        group_name_ = group.get_name ();
    }

  is_stat_loaded_ = true;
}

} // namespace mobius::io::local


