#ifndef MOBIUS_CORE_REGISTRY_HIVE_VALUE_HPP
#define MOBIUS_CORE_REGISTRY_HIVE_VALUE_HPP

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
#include <mobius/core/os/win/registry/hive_data.hpp>
#include <mobius/core/io/reader.hpp>
#include <cstdint>
#include <memory>
#include <string>

namespace mobius::core::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief hive value class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class hive_value
{
public:
  // datatypes
  using offset_type = std::uint32_t;

  // constructors
  hive_value ();
  hive_value (mobius::core::io::reader, offset_type);
  hive_value (hive_value&&) = default;
  hive_value (const hive_value&) = default;

  // operators
  hive_value& operator= (const hive_value&) = default;
  hive_value& operator= (hive_value&&) = default;
  operator bool () const;

  // accessor prototypes
  offset_type get_offset () const;
  std::string get_signature () const;
  std::uint16_t get_flags () const;
  std::string get_name () const;
  hive_data get_data () const;

private:
  // @brief implementation class forward declaration
  class impl;

  // @brief implementation pointer
  std::shared_ptr <impl> impl_;
};

} // namespace mobius::core::os::win::registry

#endif


