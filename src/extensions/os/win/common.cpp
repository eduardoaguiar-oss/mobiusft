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
#include "common.hpp"
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/log.hpp>
#include <format>

namespace mobius::extension::os::win
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get registry data as string
// @param key Hive key
// @param value_name Value name
// @return Data as string (empty string if value doesn't exist or is not a string)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_data_as_string (
    const mobius::core::os::win::registry::hive_key &key,
    const std::string &value_name
)
{
    std::string text;

    if (key)
    {
        auto data = key.get_data_by_name (value_name);

        if (data)
            text = data.get_data_as_string ("utf-16le");
    }

    return text;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get MRUListEx entries
// @param key Hive key containing MRUListEx value and MRU values
// @return Vector of pairs (index, value) sorted by index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<std::pair<std::size_t, mobius::core::os::win::registry::hive_data>>
get_mrulistex (const mobius::core::os::win::registry::hive_key &key)
{
    if (!key)
        return {};

    // Get MRUListEx data
    auto mru_data = key.get_data_by_name ("MRUListEx");
    if (!mru_data)
        return {};

    // Parse MRUListEx data as array of int32_t
    std::vector<
        std::pair<std::size_t, mobius::core::os::win::registry::hive_data>>
        entries;

    mobius::core::decoder::data_decoder decoder (mru_data.get_data ());

    while (decoder)
    {
        auto idx = decoder.get_int32_le ();

        if (idx != -1)
            entries.emplace_back (
                idx, key.get_data_by_name (std::to_string (idx))
            );
    }

    // MRUListEx is in reverse order, so reverse the vector to get correct order
    std::reverse (entries.begin (), entries.end ());

    return entries;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load installed programs from Uninstall hive key
// @param root_key NTUSER.DAT root key
// @param key_path Path to Uninstall key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<installed_program>
get_installed_programs (
    const mobius::core::os::win::registry::hive_key &root_key,
    const std::string &key_path
)
{
    std::vector<installed_program> programs;

    mobius::core::log log (__FILE__, __func__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Try to get Uninstall key by path. If it doesn't exist, return
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto uninstall_key = root_key.get_key_by_path (key_path);

    if (!uninstall_key)
        return {};

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Iterate uninstall key subkeys (each subkey represents an installed program)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &key : uninstall_key)
    {
        installed_program program;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get display name (if not available, use key name)
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        program.display_name = get_data_as_string (key, "DisplayName");

        if (program.display_name.empty ())
            program.display_name = key.get_name ();

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get other data
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        program.comments = get_data_as_string (key, "Comments");
        program.last_modification_time = key.get_last_modification_time ();
        program.name = program.display_name;
        program.help_link = get_data_as_string (key, "HelpLink");
        program.install_location = get_data_as_string (key, "InstallLocation");
        program.install_source = get_data_as_string (key, "InstallSource");
        program.publisher = get_data_as_string (key, "Publisher");
        program.uninstall_string = get_data_as_string (key, "UninstallString");
        program.url_info_about = get_data_as_string (key, "URLInfoAbout");
        program.url_update_info = get_data_as_string (key, "URLUpdateInfo");

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get version (try DisplayVersion first, then Version)
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        using data_type = mobius::core::os::win::registry::hive_data::data_type;
        program.version = get_data_as_string (key, "DisplayVersion");

        if (program.version.empty ())
        {
            auto version_data = key.get_data_by_name ("Version");

            if (version_data)
            {
                if (version_data.get_type () == data_type::reg_sz)
                    program.version =
                        version_data.get_data_as_string ("utf-16le");

                else if (version_data.get_type () == data_type::reg_dword)
                {
                    auto version = version_data.get_data_as_dword ();

                    program.version = std::to_string (version >> 24) + "." +
                                      std::to_string ((version >> 16) & 0xff) +
                                      "." + std::to_string (version & 0xffff);
                }

                else
                {
                    log.development (
                        __LINE__,
                        std::format (
                            "DEV unhandled version type: {:d}. Key path: {}",
                            static_cast<int> (version_data.get_type ()),
                            key_path
                        )
                    );
                }
            }
        }

        if (program.version.empty ())
        {
            auto major_data = key.get_data_by_name ("VersionMajor");
            auto minor_data = key.get_data_by_name ("VersionMinor");

            if (major_data and minor_data)
                program.version = std::format (
                    "{:d}.{:d}", major_data.get_data_as_dword (),
                    minor_data.get_data_as_dword ()
                );
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get install date
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto install_date_data = key.get_data_by_name ("InstallDate");

        if (install_date_data)
        {
            if (install_date_data.get_type () == data_type::reg_dword)
            {
                auto install_date =
                    mobius::core::datetime::new_datetime_from_unix_timestamp (
                        install_date_data.get_data_as_dword ()
                    );
                program.install_date = to_string (install_date);
            }

            else if (install_date_data.get_type () == data_type::reg_sz)
            {
                program.install_date =
                    install_date_data.get_data_as_string ("ascii");

                if (program.install_date.length () == 8)
                    program.install_date = std::format (
                        "{}-{}-{}", program.install_date.substr (0, 4),
                        program.install_date.substr (4, 2),
                        program.install_date.substr (6, 2)
                    );
            }

            else
            {
                log.development (
                    __LINE__,
                    std::format (
                        "DEV unhandled install date type: {:d}. Key path: {}",
                        static_cast<int> (install_date_data.get_type ()),
                        key_path
                    )
                );
            }
        }

        programs.push_back (program);
    }

    return programs;
}

} // namespace mobius::extension::os::win
