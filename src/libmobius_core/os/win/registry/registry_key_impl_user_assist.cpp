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
#include <mobius/core/os/win/registry/registry_key_impl_user_assist.hpp>
#include <mobius/core/crypt/rot13.hpp>

namespace mobius::core::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// @param key delegated key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_key_impl_user_assist::registry_key_impl_user_assist (registry_key key)
  : key_ (key),
    name_ (key.get_name ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load values on demand
// @see http://www.4n6k.com/2013/05/userassist-forensics-timelines.html
// @see https://github.com/libyal/winreg-kb/blob/master/documentation/User%20Assist%20keys.asciidoc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry_key_impl_user_assist::_load_values () const
{
  // return if values are loaded
  if (values_loaded_)
    return;

  // load values
  auto values = key_.get_values ();
  values_.reserve (values.size ());

  for (auto& v: values)
    {
      // decode ROT-13 value name
      auto b = mobius::core::crypt::rot13 (v.get_name ());
      v.set_name (b.to_string ());

      // add value to list
      values_.push_back (v);
    }

  // set values loaded
  values_loaded_ = true;
}

} // namespace mobius::core::os::win::registry


