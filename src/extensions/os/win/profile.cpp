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
#include "profile.hpp"
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/framework/utils.hpp>
#include <format>
#include <unordered_map>
#include <string>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
const std::unordered_map<std::string, std::string> SEARCH_ASSIST_FIELDS = {
    {"5001", "internet"},
    {"5603", "filename"},
    {"5604", "file-content"},
    {"5647", "computers"}
};

} // namespace

namespace mobius::extension::os::win
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set folder
// @param f Folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::_set_folder (const mobius::core::io::folder &f)
{
    if (folder_ || !f)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Set data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    folder_ = f;
    last_modified_time_ = f.get_modification_time ();
    creation_time_ = f.get_creation_time ();
    username_ = mobius::framework::get_username_from_path (f.get_path ());

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Emit sampling_folder event
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::emit ("sampling_folder", std::string ("os.win.profiles"), f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update last modified time based on file
// @param f File
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::_update_mtime (const mobius::core::io::file &f)
{
    if (!f)
        return;

    if (!last_modified_time_ ||
        f.get_modification_time () > last_modified_time_)
        last_modified_time_ = f.get_modification_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add NTUSER.DAT file
// @param file NTUSER.DAT file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_ntuser_dat_file (const mobius::core::io::file &f)
{
    // Check if it is a valid NTUSER.DAT file
    if (!f || f.get_size () == 0)
        return;

    if (ntuser_dat_file_ && !ntuser_dat_file_.is_deleted ())
        return;

    // Decode hive file
    mobius::core::os::win::registry::hive_file hive_file (f.new_reader ());

    if (!hive_file.is_instance ())
        return;

    // Clear old artifacts
    installed_programs_.clear ();
    autofill_entries_.clear ();

    // Load artifacts
    _load_installed_programs (hive_file);
    _load_search_assist_entries (hive_file);
    _load_wordwheel_queries (hive_file);

    // Set folder based on parent of NTUSER.DAT file
    _set_folder (f.get_parent ());
    _update_mtime (f);

    // Set active flag. Active path = /FSxx/Users/USERNAME/NTUSER.DAT
    if (f && !f.is_deleted ())
    {
        auto path_segments = mobius::core::string::split (f.get_path (), "/");
        is_active_ =
            (path_segments.size () == 5 && path_segments[2] == "Users");
    }

    // Set profile flags
    is_deleted_ = f.is_deleted ();
    is_valid_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load installed programs from NTUSER.DAT hive file
// @param hive_file NTUSER.DAT hive file
// @see Forensic Analysis of the Windows Registry, by Lih Wern Wong, p.8
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::_load_installed_programs (
    const mobius::core::os::win::registry::hive_file &hive_file
)
{
    mobius::core::log log (__FILE__, __func__);

    try
    {
        auto root_key = hive_file.get_root_key ();
        if (!root_key)
            return;

        auto programs = mobius::extension::os::win::get_installed_programs (
            root_key, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
        );

        std::copy (
            programs.begin (), programs.end (),
            std::back_inserter (installed_programs_)
        );

        programs = mobius::extension::os::win::get_installed_programs (
            root_key, "Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersio"
                      "n\\Uninstall"
        );

        std::copy (
            programs.begin (), programs.end (),
            std::back_inserter (installed_programs_)
        );
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load search assist entries from NTUSER.DAT hive file
// @param hive_file NTUSER.DAT hive file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::_load_search_assist_entries (
    const mobius::core::os::win::registry::hive_file &hive_file
)
{
    mobius::core::log log (__FILE__, __func__);

    try
    {
        auto root_key = hive_file.get_root_key ();
        if (!root_key)
            return;

        for (const auto &key : root_key.get_keys_by_mask (
                 "Software\\Microsoft\\Windows\\CurrentVersion\\Search\\SearchA"
                 "ssistant\\ACMru\\*"
             ))
        {
            auto iter = SEARCH_ASSIST_FIELDS.find (key.get_name ());

            if (iter == SEARCH_ASSIST_FIELDS.end ())
            {
                log.development (
                    __LINE__,
                    std::format (
                        "unhandled search assist key: {}", key.get_name ()
                    )
                );
            }

            else
            {
                for (const auto &value : key.get_values ())
                {
                    auto data = value.get_data ();

                    if (data)
                    {
                        autofill entry;
                        entry.field_name = iter->second;
                        entry.value = data.get_data_as_string ("utf-16le");
                        entry.f = ntuser_dat_file_;

                        entry.metadata.set (
                            "registry_key_path",
                            std::format (
                                "HKCU\\Software\\Microsoft\\Windows\\CurrentVer"
                                "sion\\Search\\SearchAssistant\\ACMru\\{}",
                                key.get_name ()
                            )
                        );
                        entry.metadata.set (
                            "registry_value_name", value.get_name ()
                        );
                        autofill_entries_.push_back (entry);
                    }
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load WordWheel autocomplete entries from NTUSER.DAT hive file
// @param hive_file NTUSER.DAT hive file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::_load_wordwheel_queries (
    const mobius::core::os::win::registry::hive_file &hive_file
)
{
    mobius::core::log log (__FILE__, __func__);

    try
    {
        // Get root key
        auto root_key = hive_file.get_root_key ();
        if (!root_key)
            return;

        // Get WordWheelQuery key, if any
        auto ww_key = root_key.get_key_by_path (
            "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\WordWheelQ"
            "uery"
        );
        if (!ww_key)
            return;

        // Iterate through MRUListEx values (each value represents a search query)
        for (const auto &[idx, data] : get_mrulistex (ww_key))
        {
            autofill entry;
            entry.field_name = "search_query";
            entry.value = data.get_data_as_string ("utf-16le");
            entry.f = ntuser_dat_file_;

            entry.metadata.set (
                "registry_key_path", "Software\\Microsoft\\Windows\\Current"
                                     "Version\\Explorer\\WordWheelQuery"
            );
            entry.metadata.set ("mru_index", idx);

            autofill_entries_.push_back (entry);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::os::win
