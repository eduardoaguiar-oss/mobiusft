#ifndef MOBIUS_CORE_IO_SMB_FILE_IMPL_HPP
#define MOBIUS_CORE_IO_SMB_FILE_IMPL_HPP

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
#include <mobius/core/io/file_impl_base.hpp>
#include <string>

namespace mobius::core::io::smb
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief SMB file implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_impl : public file_impl_base
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  file_impl (const std::string&);
  file_impl (const file_impl&) = delete;
  file_impl (file_impl&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  file_impl& operator=(const file_impl&) = delete;
  file_impl& operator=(file_impl&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  bool exists () const override;
  bool is_deleted () const override;
  bool is_reallocated () const override;
  bool is_hidden () const override;
  std::string get_name () const override;
  std::string get_short_name () const override;
  inode_type get_inode () const override;
  size_type get_size () const override;
  type get_type () const override;
  user_id_type get_user_id () const override;
  std::string get_user_name () const override;
  group_id_type get_group_id () const override;
  std::string get_group_name () const override;
  permission_type get_permissions () const override;
  mobius::core::datetime::datetime get_access_time () const override;
  mobius::core::datetime::datetime get_modification_time () const override;
  mobius::core::datetime::datetime get_metadata_time () const override;
  mobius::core::datetime::datetime get_creation_time () const override;
  mobius::core::datetime::datetime get_deletion_time () const override;
  mobius::core::datetime::datetime get_backup_time () const override;
  folder_type get_parent () const override;
  void reload () override;
  void remove () override;
  void rename (const std::string&) override;
  bool move (file_type) override;
  reader_type new_reader () const override;
  writer_type new_writer (bool=true) const override;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Check if object is valid
  // @return true/false
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  operator bool () const noexcept override
  {
    return true;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get path
  // @return Path
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_path () const override
  {
    return {};
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Set path
  // @param path Path
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_path (const std::string&) override
  {
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get streams
  // @return Streams
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::vector <stream_type>
  get_streams () const override
  {
    return {}; // SMB files have no streams
  }

private:
  // @brief URL
  std::string url_;

  // @brief File name
  std::string name_;

  // @brief File exists flag
  mutable bool exists_;

  // @brief I-node
  mutable inode_type inode_;

  // @brief Size in bytes
  mutable size_type size_;

  // @brief File type
  mutable type type_;

  // @brief User id
  mutable user_id_type user_id_;

  // @brief User name
  mutable std::string user_name_;

  // @brief Group id
  mutable group_id_type group_id_;

  // @brief Group name
  mutable std::string group_name_;

  // @brief RWX permissions
  mutable permission_type permissions_;

  // @brief Last access date/time
  mutable mobius::core::datetime::datetime access_time_;

  // @brief Last modification date/time
  mutable mobius::core::datetime::datetime modification_time_;

  // @brief Last metadata modification date/time
  mutable mobius::core::datetime::datetime metadata_time_;

  // @brief Is stat loaded flag
  mutable bool is_stat_loaded_ = false;

private:
  void _load_stat () const;
};

} // namespace mobius::core::io::smb

#endif


