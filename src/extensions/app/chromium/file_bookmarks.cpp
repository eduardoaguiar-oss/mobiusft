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
#include "file_bookmarks.hpp"
#include <mobius/core/decoder/json/parser.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/string_functions.hpp>
#include "common.hpp"

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_bookmarks::file_bookmarks (const mobius::core::io::reader &reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (!reader)
        return;

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Try to parse the Bookmarks file as a JSON file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::decoder::json::parser parser (reader);
        auto data = parser.parse ();

        if (!data.is_map ())
            return;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Retrieve data
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto map = data.to_map ();

        version_ = map.get<std::int64_t> ("version");
        log.info (__LINE__, "Bookmarks.version = " + std::to_string (version_));

        if (version_ != 1)
            log.development (
                __LINE__, "Unhandled version: " + std::to_string (version_)
            );

        //_load_accounts (map.get ("account_info"));
        //_load_profile (map.get ("profile"));

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Finish parsing
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        is_instance_ = true;

        // file_for_sampling event requires sampling ID as std::string
        mobius::core::emit (
            "file_for_sampling",
            "app.chromium.bookmarks." +
                mobius::core::string::to_string (version_, 5),
            reader
        );
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode entries
// @param entries Entries dict
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_bookmarks::_load_entries (const mobius::core::pod::map &entries)
{
    mobius::core::log log (__FILE__, __FUNCTION__);
    /*
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
                    a.metadata.set (k, v);

                accounts_.push_back (a);
            }
        }*/
}

} // namespace mobius::extension::app::chromium
