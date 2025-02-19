#ifndef MOBIUS_REGISTRY_REGISTRY_H
#define MOBIUS_REGISTRY_REGISTRY_H

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
#include <mobius/os/win/registry/registry_file.h>
#include <mobius/os/win/registry/registry_key.h>
#include <mobius/os/win/registry/registry_value.h>
#include <mobius/os/win/registry/registry_data.h>
#include <mobius/io/reader.h>
#include <mobius/bytearray.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace mobius
{
namespace os
{
namespace win
{
namespace registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief registry class
//! \author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class registry
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  registry ();
  registry (registry&&) noexcept = default;
  registry (const registry&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  registry& operator= (const registry&) = default;
  registry& operator= (registry&&) = default;
  operator bool () const;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // function prototypes (files)
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  registry_file add_file_by_path (const std::string&, const std::string&, const std::string&);
  registry_file add_file_by_url (const std::string&, const std::string&, const std::string&);
  registry_file add_file_by_reader (const std::string&, const std::string&, mobius::io::reader);
  void remove_file (std::uint32_t);
  std::vector <registry_file> get_files () const;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // function prototypes (keys)
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::vector <registry_key> get_keys () const;
  registry_key get_key_by_path (const std::string&) const;
  std::vector <registry_key> get_key_by_mask (const std::string&) const;

  registry_value get_value_by_path (const std::string&) const;
  std::vector <registry_value> get_value_by_mask (const std::string&) const;

  registry_data get_data_by_path (const std::string&) const;
  std::vector <registry_data> get_data_by_mask (const std::string&) const;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // function prototypes (data)
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::bytearray get_syskey () const;

private:
  //! \brief implementation class forward declaration
  class impl;

  //! \brief implementation pointer
  std::shared_ptr <impl> impl_;
};

} // namespace registry
} // namespace win
} // namespace os
} // namespace mobius

#endif
