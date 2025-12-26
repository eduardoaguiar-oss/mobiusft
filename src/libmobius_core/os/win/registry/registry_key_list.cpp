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
#include <algorithm>
#include <mobius/core/os/win/registry/registry_key_list.hpp>
#include <mobius/core/string_functions.hpp>

namespace mobius::core::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief add key, sorted by name
// @param key key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry_key_list::add (registry_key key)
{
    auto lambda = [] (registry_key k1, registry_key k2)
    {
        return mobius::core::string::tolower (k1.get_name ()) <
               mobius::core::string::tolower (k2.get_name ());
    };

    keys_.insert (std::upper_bound (keys_.begin (), keys_.end (), key, lambda),
                  key);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief remove key
// @param name key name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry_key_list::remove (const std::string &name)
{
    const std::string lname = mobius::core::string::tolower (name);

    auto lambda = [lname] (registry_key sk)
    { return mobius::core::string::tolower (sk.get_name ()) == lname; };

    keys_.erase (std::remove_if (keys_.begin (), keys_.end (), lambda),
                 keys_.end ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get a key
// @param name key name
// @return registry_key (can be invalid)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_key
registry_key_list::get (const std::string &name) const
{
    const std::string lname = mobius::core::string::tolower (name);

    auto lambda = [lname] (registry_key sk)
    { return mobius::core::string::tolower (sk.get_name ()) == lname; };

    auto iter = std::find_if (keys_.begin (), keys_.end (), lambda);

    if (iter != keys_.end ())
        return *iter;

    return registry_key ();
}

} // namespace mobius::core::os::win::registry
