#ifndef MOBIUS_REGISTRY_REGISTRY_KEY_IMPL_BASE_H
#define MOBIUS_REGISTRY_REGISTRY_KEY_IMPL_BASE_H

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
#include <string>
#include <vector>

namespace mobius::os::win::registry
{
class registry_key;
class registry_value;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_key implementation base class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class registry_key_impl_base
{
public:
  registry_key_impl_base () = default;
  registry_key_impl_base (const registry_key_impl_base&) = delete;
  registry_key_impl_base (registry_key_impl_base&&) = delete;
  virtual ~registry_key_impl_base () = default;

  // datatypes
  using const_iterator_type = std::vector <registry_key>::const_iterator;

  // prototypes
  virtual operator bool () const = 0;
  virtual bool has_subkeys () const = 0;
  virtual std::string get_name () const = 0;
  virtual void set_name (const std::string&) = 0;
  virtual std::string get_classname () const = 0;
  virtual mobius::datetime::datetime get_last_modification_time () const = 0;
  virtual const_iterator_type begin () const = 0;
  virtual const_iterator_type end () const = 0;
  virtual void add_key (registry_key) = 0;
  virtual void remove_key (const std::string&) = 0;
  virtual void clear_keys () = 0;
  virtual std::vector <registry_value> get_values () const = 0;
};

} // namespace mobius::os::win::registry

#endif


