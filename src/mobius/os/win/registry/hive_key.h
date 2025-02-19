#ifndef MOBIUS_OS_WIN_REGISTRY_HIVE_KEY_H
#define MOBIUS_OS_WIN_REGISTRY_HIVE_KEY_H

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
#include <mobius/os/win/registry/hive_value.h>
#include <mobius/datetime/datetime.h>
#include <mobius/io/reader.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace mobius::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief hive_key class
//! \author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class hive_key
{
public:
  // datatypes
  using offset_type = std::uint32_t;
  using const_iterator_type = std::vector <hive_key>::const_iterator;

  // constructors
  hive_key ();
  hive_key (const mobius::io::reader&, offset_type);
  hive_key (hive_key&&) = default;
  hive_key (const hive_key&) = default;

  // operators
  hive_key& operator= (const hive_key&) = default;
  hive_key& operator= (hive_key&&) = default;
  operator bool () const;

  // function prototypes
  bool has_subkeys () const;
  offset_type get_offset () const;
  bool is_readonly () const;
  bool is_symlink () const;
  bool is_root_key () const;
  std::string get_signature () const;
  std::uint16_t get_flags () const;
  mobius::datetime::datetime get_last_modification_time () const;
  std::uint32_t get_access_bits () const;
  std::uint32_t get_subkeys_count () const;
  std::uint32_t get_volatile_subkeys_count () const;
  std::uint32_t get_values_count () const;
  std::string get_name () const;
  std::string get_classname () const;
  std::uint32_t get_subkeys_offset () const;
  std::uint32_t get_volatile_subkeys_offset () const;
  std::uint32_t get_parent_offset () const;
  std::uint32_t get_security_offset () const;
  std::uint32_t get_values_offset () const;
  hive_key get_parent () const;
  const_iterator_type begin () const;
  const_iterator_type end () const;
  std::vector <hive_value> get_values () const;

  hive_key get_key_by_name (const std::string&) const;
  hive_key get_key_by_path (const std::string&) const;
  std::vector <hive_key> get_keys_by_mask (const std::string&) const;

  hive_value get_value_by_name (const std::string&) const;
  std::vector <hive_value> get_values_by_mask (const std::string&) const;

  hive_data get_data_by_name (const std::string&) const;

private:
  //! \brief implementation class forward declaration
  class impl;

  //! \brief implementation pointer
  std::shared_ptr <impl> impl_;
};

} // namespace mobius::os::win::registry

#endif
