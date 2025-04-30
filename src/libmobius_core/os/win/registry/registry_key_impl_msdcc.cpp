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
#include <mobius/core/os/win/registry/registry_key_impl_msdcc.hpp>
#include <mobius/core/os/win/registry/registry_data_impl_msdcc.hpp>
#include <mobius/core/os/win/registry/registry_value.hpp>
#include <mobius/core/string_functions.hpp>

namespace mobius::core::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// @param key \\HKLM\\SECURITY\\Cache key
// @param nlkm NL$KM value from LSA Secrets
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_key_impl_msdcc::registry_key_impl_msdcc (registry_key key, const mobius::core::bytearray& nlkm)
  : key_ (key),
    name_ (key.get_name ()),
    nlkm_ (nlkm)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load values on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry_key_impl_msdcc::_load_values () const
{
  // return if values are loaded
  if (values_loaded_)
    return;

  // load values from Cache key
  for (auto k_value : key_.get_values ())
    {
      if (mobius::core::string::fnmatch ("NL$[0-9]*", k_value.get_name ()))
        {
          auto v_data = k_value.get_data ().get_data ();

          registry_data data (
            std::make_shared <registry_data_impl_msdcc> (nlkm_, v_data)
          );

          registry_value value (k_value.get_name (), data);
          values_.push_back (value);
        }

      else
        values_.push_back (k_value);
    }

  // set values loaded
  values_loaded_ = true;
}

} // namespace mobius::core::os::win::registry


