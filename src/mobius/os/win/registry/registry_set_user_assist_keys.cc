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
#include "registry_key.h"
#include "registry_key_impl_user_assist.h"

namespace mobius::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set User Assist "Count" keys
// @see http://www.4n6k.com/2013/05/userassist-forensics-timelines.html
// @see https://windowsexplored.com/2012/02/06/a-quick-glance-at-the-userassist-key-in-windows/
// @see https://github.com/libyal/winreg-kb/blob/master/documentation/User%20Assist%20keys.asciidoc
// @param root registry root key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry_set_user_assist_keys (registry_key root)
{
  for (auto guid_key : root.get_key_by_mask ("HKEY_USERS\\*\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist\\*"))
    {
      auto count_key = guid_key.get_key_by_name ("Count");

      if (count_key)
        {
          auto ua_key = registry_key (std::make_shared <registry_key_impl_user_assist> (count_key));
          guid_key.add_key (ua_key);
        }
    }
}

} // namespace mobius::os::win::registry


