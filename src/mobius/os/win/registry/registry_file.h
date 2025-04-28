#ifndef MOBIUS_OS_WIN_REGISTRY_REGISTRY_FILE_H
#define MOBIUS_OS_WIN_REGISTRY_REGISTRY_FILE_H

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
#include <mobius/core/metadata.hpp>
#include <mobius/os/win/registry/registry_key.h>
#include <mobius/datetime/datetime.h>
#include <mobius/io/reader.h>
#include <cstdint>
#include <memory>
#include <string>

namespace mobius::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_file class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class registry_file
{
public:
  // constructors
  registry_file (std::uint32_t, const std::string&, const std::string&, mobius::io::reader);
  registry_file (registry_file&&) noexcept = default;
  registry_file (const registry_file&) noexcept = default;

  // operators
  registry_file& operator= (const registry_file&) = default;
  registry_file& operator= (registry_file&&) = default;

  // function prototypes
  std::uint32_t get_uid () const;
  std::string get_role () const;
  std::string get_path () const;
  mobius::datetime::datetime get_last_modification_time () const;
  registry_key get_root_key () const;
  mobius::core::metadata get_metadata () const;

private:
  // @brief implementation class forward declaration
  class impl;

  // @brief implementation pointer
  std::shared_ptr <impl> impl_;
};

} // namespace mobius::os::win::registry

#endif


