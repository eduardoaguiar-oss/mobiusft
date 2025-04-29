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
#include <mobius/exception.inc>
#include <mobius/io/path.h>
#include <stdexcept>
#include <mobius/core/vfs/tsk/fs_file.hpp>
#include <mobius/core/vfs/tsk/stream_impl.hpp>
#include <mobius/core/vfs/tsk/exception.hpp>
#include <tsk/libtsk.h>

namespace mobius::core::vfs::tsk
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize object
// @param p Pointer to file structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
fs_file::fs_file (TSK_FS_FILE *p)
{
  if (!p)
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  p_ = std::shared_ptr <TSK_FS_FILE> (p, tsk_fs_file_close);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file exists
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
fs_file::exists () const
{
  return bool (p_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file name
// @return File name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
fs_file::get_name () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  _load_fs_name ();
  return name_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get short file name
// @return Short file name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
fs_file::get_short_name () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  _load_fs_name ();
  return short_name_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get path
// @return Path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
fs_file::get_path () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  return path_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set path
// @param path Path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
fs_file::set_path (const std::string& path)
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  path_ = path;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file is deleted
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
fs_file::is_deleted () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  _load_fs_name ();
  return is_deleted_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file is reallocated
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
fs_file::is_reallocated () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  _load_fs_meta ();
  return is_reallocated_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file inode
// @return Inode
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
fs_file::get_inode () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  if (p_->name)
    _load_fs_name ();

  else
    _load_fs_meta ();

  return inode_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file size
// @return Size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
fs_file::get_size () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  _load_fs_meta ();
  return size_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file type
// @return Size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
fs_file::fs_file_type
fs_file::get_type () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  if (p_->name)
    _load_fs_name ();

  else
    _load_fs_meta ();

  return type_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get User ID
// @return User ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
fs_file::get_user_id () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  _load_fs_meta ();
  return user_id_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get Group ID
// @return Group ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
fs_file::get_group_id () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  _load_fs_meta ();
  return group_id_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file access permissions
// @return Permission mask
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
fs_file::get_permissions () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  _load_fs_meta ();
  return permissions_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get creation time
// @return Creation date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
fs_file::get_creation_time () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  _load_fs_meta ();
  return creation_time_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last access timestamp
// @return Last file access date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
fs_file::get_access_time () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  _load_fs_meta ();
  return access_time_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last metadata modification timestamp
// @return Last file metadata modification date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
fs_file::get_modification_time () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  _load_fs_meta ();
  return modification_time_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last metadata modification timestamp
// @return Last file metadata modification date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
fs_file::get_metadata_time () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  _load_fs_meta ();
  return metadata_time_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get deletion time
// @return Deletion date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
fs_file::get_deletion_time () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  _load_fs_meta ();
  return deletion_time_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get backup time
// @return Backup date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
fs_file::get_backup_time () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  _load_fs_meta ();
  return backup_time_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reload data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
fs_file::reload ()
{
  fs_name_loaded_ = false;
  fs_meta_loaded_ = false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get parent
// @return parent entry, if found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
fs_file
fs_file::get_parent () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  fs_file parent;

  if (p_->name && p_->name->meta_addr != p_->name->par_addr)
    {
      TSK_FS_FILE *file_p = tsk_fs_file_open_meta (
                              p_->fs_info,
                              nullptr,
                              p_->name->par_addr
                            );

      if (!file_p)
        throw std::runtime_error (TSK_EXCEPTION_MSG);

      // build parent object
      parent = fs_file (file_p);
    }

  return parent;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get streams
// @return Streams
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <fs_file::stream_type>
fs_file::get_streams () const
{
  if (!exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("pointer is null"));

  _load_streams ();
  return streams_;
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
fs_file::_load_fs_name () const
{
  if (fs_name_loaded_ || !p_->name)
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // set data
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  inode_ = p_->name->meta_addr;
  is_deleted_ = bool (p_->name->flags & TSK_FS_NAME_FLAG_UNALLOC);

  if (p_->name->name)
    name_ = p_->name->name;

  if (p_->name->shrt_name)
    short_name_ = p_->name->shrt_name;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // set file type
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  switch (p_->name->type)
    {
      case TSK_FS_NAME_TYPE_FIFO:
        type_ = fs_file_type::fifo; break;

      case TSK_FS_NAME_TYPE_CHR:
        type_ = fs_file_type::char_device; break;

      case TSK_FS_NAME_TYPE_DIR: [[fallthrough]]
      case TSK_FS_NAME_TYPE_VIRT_DIR:
        type_ = fs_file_type::folder; break;

      case TSK_FS_NAME_TYPE_BLK:
        type_ = fs_file_type::block_device; break;

      case TSK_FS_NAME_TYPE_REG: [[fallthrough]]
      case TSK_FS_NAME_TYPE_VIRT:
        type_ = fs_file_type::regular; break;

      case TSK_FS_NAME_TYPE_LNK:
        type_ = fs_file_type::symlink; break;

      case TSK_FS_NAME_TYPE_SOCK:
        type_ = fs_file_type::socket; break;

      default:
        type_ = fs_file_type::none;
    }

  fs_name_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load fs_meta on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
fs_file::_load_fs_meta () const
{
  if (fs_meta_loaded_)
    return;

  // retrieve meta structure if needed
  if (!p_->meta && p_->name && p_->name->meta_addr)
    {
      int rc = p_->fs_info->file_add_meta (p_->fs_info, p_.get (), p_->name->meta_addr);

      if (rc)
        throw std::runtime_error (TSK_EXCEPTION_MSG);
    }

  // set metadata
  if (p_->meta)
    {
      size_ = p_->meta->size;
      user_id_ = p_->meta->uid;
      group_id_ = p_->meta->gid;
      permissions_ = p_->meta->mode;

      // set timestamps
      access_time_ = mobius::datetime::new_datetime_from_unix_timestamp (p_->meta->atime);
      modification_time_ = mobius::datetime::new_datetime_from_unix_timestamp (p_->meta->mtime);
      metadata_time_ = mobius::datetime::new_datetime_from_unix_timestamp (p_->meta->ctime);
      creation_time_ = mobius::datetime::new_datetime_from_unix_timestamp (p_->meta->crtime);

      if (p_->fs_info->ftype & TSK_FS_TYPE_EXT_DETECT)
        deletion_time_ = mobius::datetime::new_datetime_from_unix_timestamp (p_->meta->time2.ext2.dtime);

      if (p_->fs_info->ftype & TSK_FS_TYPE_HFS_DETECT)
        backup_time_ = mobius::datetime::new_datetime_from_unix_timestamp (p_->meta->time2.hfs.bkup_time);

      // if file has name, check if it is reallocated
      if (p_->name)
        is_reallocated_ = (p_->name->flags & TSK_FS_NAME_FLAG_UNALLOC) &&
                          (p_->meta->flags & TSK_FS_META_FLAG_ALLOC);

      // otherwise, set i-node and type
      else
        {
          inode_ = p_->meta->addr;

          switch (p_->meta->type)
            {
              case TSK_FS_META_TYPE_FIFO:
                type_ = fs_file_type::fifo; break;

              case TSK_FS_META_TYPE_CHR:
                type_ = fs_file_type::char_device; break;

              case TSK_FS_META_TYPE_DIR: [[fallthrough]]
              case TSK_FS_META_TYPE_VIRT_DIR:
                type_ = fs_file_type::folder; break;

              case TSK_FS_META_TYPE_BLK:
                type_ = fs_file_type::block_device; break;

              case TSK_FS_META_TYPE_REG: [[fallthrough]]
              case TSK_FS_META_TYPE_VIRT:
                type_ = fs_file_type::regular; break;

              case TSK_FS_META_TYPE_LNK:
                type_ = fs_file_type::symlink; break;

              case TSK_FS_META_TYPE_SOCK:
                type_ = fs_file_type::socket; break;

              default:
                type_ = fs_file_type::none;
            }
        }
    }

  fs_meta_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load streams on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
fs_file::_load_streams () const
{
  if (streams_loaded_)
    return;

  _load_fs_meta ();

  std::size_t count = tsk_fs_file_attr_getsize (p_.get ());
  streams_.reserve (count);

  for (std::size_t i = 0; i < count; i++)
    {
      const TSK_FS_ATTR *fs_attr_p = tsk_fs_file_attr_get_idx (p_.get (), i);

      if (!fs_attr_p)
        throw std::runtime_error (TSK_EXCEPTION_MSG);

      auto stream = std::make_shared <stream_impl> (*this, fs_attr_p);
      streams_.push_back (stream);
    }

  streams_loaded_ = true;
}

} // namespace mobius::core::vfs::tsk


