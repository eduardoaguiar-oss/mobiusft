#ifndef MOBIUS_IO_FILE_IMPL_BASE_H
#define MOBIUS_IO_FILE_IMPL_BASE_H

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
#include <memory>
#include <string>
#include <vector>
#include <sys/types.h>

namespace mobius::io
{
class folder_impl_base;
class reader_impl_base;
class writer_impl_base;
class stream_impl_base;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief File implementation base class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_impl_base
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors and destructor
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  file_impl_base () = default;
  file_impl_base (const file_impl_base&) = delete;
  file_impl_base (file_impl_base&&) = delete;
  virtual ~file_impl_base () = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Assignment operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  file_impl_base& operator= (const file_impl_base&) = delete;
  file_impl_base& operator= (file_impl_base&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Datatypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  using reader_type = std::shared_ptr <reader_impl_base>;
  using writer_type = std::shared_ptr <writer_impl_base>;
  using folder_type = std::shared_ptr <folder_impl_base>;
  using file_type = std::shared_ptr <file_impl_base>;
  using stream_type = std::shared_ptr <stream_impl_base>;
  using size_type = std::uint64_t;	//< file size in bytes
  using inode_type = std::uint64_t;	//< inode

  using user_id_type = uid_t;		//< user ID
  using group_id_type = gid_t;		//< group ID
  using permission_type = mode_t;	//< access permission mask

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief File subtype
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  enum class type {
          none,
          block_device,
          char_device,
          fifo,
          symlink,
          regular,
          socket
  };

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Abstract methods
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  virtual operator bool () const noexcept = 0;
  virtual bool exists () const = 0;
  virtual bool is_deleted () const = 0;
  virtual bool is_reallocated () const = 0;
  virtual bool is_hidden () const = 0;
  virtual std::string get_name () const = 0;
  virtual std::string get_short_name () const = 0;
  virtual std::string get_path () const = 0;
  virtual void set_path (const std::string&) = 0;
  virtual inode_type get_inode () const = 0;
  virtual size_type get_size () const = 0;
  virtual type get_type () const = 0;
  virtual user_id_type get_user_id () const = 0;
  virtual std::string get_user_name () const = 0;
  virtual group_id_type get_group_id () const = 0;
  virtual std::string get_group_name () const = 0;
  virtual permission_type get_permissions () const = 0;
  virtual mobius::datetime::datetime get_access_time () const = 0;
  virtual mobius::datetime::datetime get_modification_time () const = 0;
  virtual mobius::datetime::datetime get_metadata_time () const = 0;
  virtual mobius::datetime::datetime get_creation_time () const = 0;
  virtual mobius::datetime::datetime get_deletion_time () const = 0;
  virtual mobius::datetime::datetime get_backup_time () const = 0;
  virtual folder_type get_parent () const = 0;
  virtual void reload () = 0;
  virtual void remove () = 0;
  virtual void rename (const std::string&) = 0;
  virtual bool move (file_type) = 0;
  virtual reader_type new_reader () const = 0;
  virtual writer_type new_writer (bool) const = 0;
  virtual std::vector <stream_type> get_streams () const = 0;
};

} // namespace mobius::io

#endif


