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
#include <mobius/core/os/win/registry/registry_key.hpp>
#include <mobius/core/os/win/registry/registry_key_impl_msdcc.hpp>

namespace mobius::core::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set MS Domain Cached Credential key
// @see http://support.microsoft.com/kb/913485
// @see https://github.com/moyix/creddump/blob/master/cachedump.py (creddump7)
// @param root registry root key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry_set_msdcc_key (registry_key root)
{
    auto security_key = root.get_key_by_path ("\\HKEY_LOCAL_MACHINE\\SECURITY");

    if (security_key)
    {
        auto nlkm = security_key.get_data_by_path (
            "Policy\\Secrets\\NL$KM\\Currval\\(default)");
        auto key = security_key.get_key_by_name ("Cache");

        if (key && nlkm)
            security_key.add_key (
                registry_key (std::make_shared<registry_key_impl_msdcc> (
                    key, nlkm.get_data ())));
    }
}

} // namespace mobius::core::os::win::registry
