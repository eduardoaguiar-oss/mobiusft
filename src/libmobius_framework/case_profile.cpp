// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025
// Eduardo Aguiar
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
#include <mobius/core/application.hpp>
#include <mobius/core/decoder/inifile.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/walker.hpp>
#include <mobius/framework/case_profile.hpp>
#include <stdexcept>
#include <set>

namespace mobius::framework
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param id Profile ID
//
// Loads profile from configuration path. If not found, tries data path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
case_profile::case_profile (const std::string &id)
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Search profile file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::application app;

    auto f = mobius::core::io::new_file_by_path (
        app.get_config_path ("profiles/" + id + ".profile")
    );

    if (!f.exists ())
        f = mobius::core::io::new_file_by_path (
            app.get_data_path ("profiles/" + id + ".profile")
        );

    if (!f.exists ())
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("profile '" + id + "' not found")
        );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Read profile data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::decoder::inifile ini (f.new_reader ());
    ini.set_comment_char ('#');

    id_ = id;
    name_ = ini.get_value ("general", "name");
    description_ = ini.get_value ("general", "description");
    processor_scope_ = ini.get_value ("processor", "scope");
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief List profiles
// @return Vector of profile IDs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<std::string>
list_case_profiles ()
{
    std::set<std::string> profiles;

    mobius::core::application app;

    // List profiles from data path
    auto folder =
        mobius::core::io::new_folder_by_path (app.get_data_path ("profiles"));

    auto w = mobius::core::io::walker (folder);

    for (const auto &f : w.get_files_by_pattern ("*.profile"))
    {
        auto name = f.get_name ();
        if (name.size () > 8 && name.substr (name.size () - 8) == ".profile")
            profiles.insert (name.substr (0, name.size () - 8));
    }

    // List profiles from config path
    folder =
        mobius::core::io::new_folder_by_path (app.get_config_path ("profiles"));

    if (!folder.exists ())
        folder.create();

    w = mobius::core::io::walker (folder);

    for (const auto &f : w.get_files_by_pattern ("*.profile"))
    {
        auto name = f.get_name ();
        if (name.size () > 8 && name.substr (name.size () - 8) == ".profile")
            profiles.insert (name.substr (0, name.size () - 8));
    }

    // Return as vector
    return std::vector<std::string> (profiles.begin (), profiles.end ());
}

} // namespace mobius::framework
