#ifndef MOBIUS_EXTENSION_OS_WIN_COMMON_HPP
#define MOBIUS_EXTENSION_OS_WIN_COMMON_HPP

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
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/os/win/registry/hive_key.hpp>
#include <string>
#include <vector>
#include <utility>

namespace mobius::extension::os::win
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Installed program structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct installed_program
{
    std::string comments;
    std::string display_name;
    std::string help_link;
    std::string install_date;
    std::string install_location;
    std::string install_source;
    mobius::core::datetime::datetime last_modification_time;
    std::string name;
    std::string publisher;
    std::string uninstall_string;
    std::string url_info_about;
    std::string url_update_info;
    std::string version;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string get_data_as_string (
    const mobius::core::os::win::registry::hive_key &, const std::string &
);
std::string get_data_as_string (
    const mobius::core::os::win::registry::hive_data &, const std::string &
);
std::vector<installed_program> get_installed_programs (
    const mobius::core::os::win::registry::hive_key &, const std::string &
);
std::vector<std::pair<std::size_t, mobius::core::os::win::registry::hive_data>>
get_mrulistex (const mobius::core::os::win::registry::hive_key &);

} // namespace mobius::extension::os::win

#endif // MOBIUS_EXTENSION_OS_WIN_COMMON_HPP