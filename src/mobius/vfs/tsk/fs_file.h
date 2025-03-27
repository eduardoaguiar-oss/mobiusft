#ifndef MOBIUS_VFS_TSK_FS_FILE_H
#define MOBIUS_VFS_TSK_FS_FILE_H

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
#include <mobius/datetime/datetime.h>
#include <mobius/io/stream_impl_base.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief libtsk exception message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct TSK_FS_FILE;

namespace mobius::vfs::tsk
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief TSK_FS_FILE C++ class
// @author Eduardo Aguiar
// Shared owner of TSK_FS_FILE pointer, calling tsk_fs_file_close when
// pointer is finally deallocated.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class fs_file
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief File subtype
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  enum class fs_file_type {
          none,
          block_device,
          char_device,
          fifo,
          symlink,
          regular,
          socket,
          folder
  };

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Datatypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  using stream_type = std::shared_ptr <mobius::io::stream_impl_base>;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  fs_file () = default;
  explicit fs_file (TSK_FS_FILE *);
  fs_file (const fs_file&) = default;
  fs_file (fs_file&&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  fs_file& operator= (const fs_file&) = default;
  fs_file& operator= (fs_file&&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  bool exists () const;
  bool is_deleted () const;
  bool is_reallocated () const;
  bool is_hidden () const;
  std::string get_name () const;
  std::string get_short_name () const;
  std::string get_path () const;
  void set_path (const std::string&);
  std::uint64_t get_inode () const;
  std::uint64_t get_size () const;
  fs_file_type get_type () const;
  int get_user_id () const;
  int get_group_id () const;
  int get_permissions () const;
  mobius::datetime::datetime get_creation_time () const;
  mobius::datetime::datetime get_access_time () const;
  mobius::datetime::datetime get_modification_time () const;
  mobius::datetime::datetime get_metadata_time () const;
  mobius::datetime::datetime get_deletion_time () const;
  mobius::datetime::datetime get_backup_time () const;
  void reload ();
  fs_file get_parent () const;
  std::vector <stream_type> get_streams () const;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Check if fs_file is valid
  // @return true/false
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  operator bool () const noexcept
  {
    return bool (p_);
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get internal pointer
  // @return Pointer
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  TSK_FS_FILE *
  get_pointer () const
  {
    return p_.get ();
  }

private:
  // @brief libtsk file structure pointer
  std::shared_ptr <TSK_FS_FILE> p_;

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
  mutable fs_file_type type_ = {};

  // @brief size in bytes
  mutable std::uint64_t size_ = 0;

  // @brief user id
  mutable int user_id_ = -1;

  // @brief group id
  mutable int group_id_ = -1;

  // @brief RWX permissions
  mutable int permissions_ = 0;

  // @brief creation date/time
  mutable mobius::datetime::datetime creation_time_;

  // @brief last modification date/time
  mutable mobius::datetime::datetime modification_time_;

  // @brief last access date/time
  mutable mobius::datetime::datetime access_time_;

  // @brief last metadata modification date/time
  mutable mobius::datetime::datetime metadata_time_;

  // @brief deletion date/time
  mutable mobius::datetime::datetime deletion_time_;

  // @brief last backup date/time
  mutable mobius::datetime::datetime backup_time_;

  // @brief Streams
  mutable std::vector <stream_type> streams_;

  // @brief reallocated flag
  mutable bool is_reallocated_ = false;

  // @brief fs_name loaded flag
  mutable bool fs_name_loaded_ = false;

  // @brief fs_meta loaded flag
  mutable bool fs_meta_loaded_ = false;

  // @brief Streams loaded flag
  mutable bool streams_loaded_ = false;

private:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Helper functions
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void _load_fs_name () const;
  void _load_fs_meta () const;
  void _load_streams () const;
};

} // namespace mobius::vfs::tsk

#endif


