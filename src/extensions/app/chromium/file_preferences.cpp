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
#include "file_preferences.hpp"
#include <mobius/core/decoder/json/parser.hpp>
#include <mobius/core/log.hpp>
#include "common.hpp"

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_preferences::file_preferences (const mobius::core::io::reader &reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (!reader)
        return;

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Try to parse the Preferences file as a JSON file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::decoder::json::parser parser (reader);
        auto data = parser.parse ();

        if (!data.is_map ())
            return;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Retrieve data
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto map = data.to_map ();

        _load_accounts (map.get ("account_info"));
        _load_profile (map.get ("profile"));

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Finish parsing
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode account map
// @param account_info Account info json node
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_preferences::_load_accounts (const mobius::core::pod::data &account_info)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (account_info.is_null ())
        return;

    if (!account_info.is_list ())
    {
        log.warning (__LINE__, "Account list is not a valid list");
        return;
    }

    for (const auto &item : account_info.to_list ())
    {
        if (item.is_map ())
        {
            auto map = item.to_map ();

            account a;

            a.idx = accounts_.size ();
            a.id = map.pop<std::string> ("account_id");
            a.name = map.pop<std::string> ("given_name");
            a.full_name = map.pop<std::string> ("full_name");
            a.email = map.pop<std::string> ("email");
            a.locale = map.pop<std::string> ("locale");
            a.picture_url = map.pop<std::string> ("picture_url");

            for (const auto &[k, v] : map)
            {
                if (!v.is_map () && !v.is_list ())
                    a.metadata.set (k, v);
            }

            accounts_.push_back (a);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load profile data
// @param profile_info Profile info json node
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_preferences::_load_profile (const mobius::core::pod::data &profile_info)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (profile_info.is_null ())
        return;

    if (!profile_info.is_map ())
    {
        log.warning (__LINE__, "Profile info is not a valid map");
        return;
    }

    auto map = profile_info.to_map ();

    profile_.name = map.pop<std::string> ("name");
    profile_.created_by_version = map.pop<std::string> ("created_by_version");

    profile_.creation_time =
        get_datetime_from_string (map.pop<std::string> ("creation_time"));

    profile_.last_engagement_time = get_datetime_from_string (
        map.pop<std::string> ("last_engagement_time")
    );
}

} // namespace mobius::extension::app::chromium
