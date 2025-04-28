#ifndef MOBIUS_OS_WIN_REGISTRY_HIVE_FILE_H
#define MOBIUS_OS_WIN_REGISTRY_HIVE_FILE_H

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
#include <mobius/os/win/registry/hive_key.h>
#include <mobius/datetime/datetime.h>
#include <mobius/io/reader.h>
#include <mobius/core/metadata.hpp>
#include <cstdint>
#include <memory>
#include <string>

namespace mobius::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Window's registry (hive) file class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class hive_file
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  explicit hive_file (const mobius::io::reader&);
  hive_file (hive_file&&) noexcept = default;
  hive_file (const hive_file&) noexcept = default;

  // operators
  hive_file& operator= (const hive_file&) = default;
  hive_file& operator= (hive_file&&) = default;

  // function prototypes
  std::uint32_t eval_header_checksum ();
  bool is_instance () const;
  std::string get_signature () const;
  std::uint32_t get_sequence_1 () const;
  std::uint32_t get_sequence_2 () const;
  mobius::datetime::datetime get_last_modification_time () const;
  std::uint32_t get_major_version () const;
  std::uint32_t get_minor_version () const;
  std::uint32_t get_file_type () const;
  std::uint32_t get_file_format () const;
  std::uint32_t get_hbin_data_size () const;
  std::uint32_t get_disk_sector_size () const;
  std::string get_filename () const;
  std::uint32_t get_header_checksum () const;
  std::string get_rm_guid () const;
  std::string get_log_guid () const;
  std::uint32_t get_flags () const;
  std::string get_tm_guid () const;
  std::string get_guid_signature () const;
  mobius::datetime::datetime get_last_reorganization_time () const;
  hive_key get_root_key () const;
  mobius::core::metadata get_metadata () const;

private:
  // @brief implementation class forward declaration
  class impl;

  // @brief implementation pointer
  std::shared_ptr <impl> impl_;
};

} // namespace mobius::os::win::registry

#endif


