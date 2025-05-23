#ifndef MOBIUS_CORE_REGISTRY_REGISTRY_DATA_IMPL_BASE_HPP
#define MOBIUS_CORE_REGISTRY_REGISTRY_DATA_IMPL_BASE_HPP

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
#include <mobius/core/os/win/registry/hive_data.hpp>
#include <mobius/core/bytearray.hpp>
#include <cstdint>

namespace mobius::core::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_data implementation base class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class registry_data_impl_base
{
public:
  // datatypes
  using data_type = hive_data::data_type;

  // virtual destructor
  virtual ~registry_data_impl_base () = default;

  // prototypes
  virtual operator bool () const = 0;
  virtual std::uint32_t get_size () const = 0;
  virtual data_type get_type () const = 0;
  virtual mobius::core::bytearray get_data () const = 0;
};

} // namespace mobius::core::os::win::registry

#endif


