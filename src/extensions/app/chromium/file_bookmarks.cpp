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
#include "file_bookmarks.hpp"
#include <mobius/core/decoder/json/parser.hpp>
#include <mobius/core/log.hpp>
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
        // Check version
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto map = data.to_map ();

        version_ = map.get<std::int64_t> ("version");
        log.info (__LINE__, "Bookmarks.version = " + std::to_string (version_));

        if (version_ != 1)
            log.development (
                __LINE__, "Unhandled version: " + std::to_string (version_)
            );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Retrieve data
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto root_data = map.get ("roots");
        if (!root_data.is_map ())
        {
            log.warning (__LINE__, "Invalid roots data");
            return;
        }

        for (const auto &item : root_data.to_map ())
        {
            if (item.second.is_map ())
                _load_entry (item.second.to_map ());

            else
                log.development (
                    __LINE__, "Invalid root entry: " + item.first +
                                  " - type: " +
                                  std::to_string (
                                      static_cast<int> (item.second.get_type ())
                                  )
                );
        }

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
// @brief Load entry from JSON dictionary
// @param map Entry map
// @param parent_name Parent folder name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_bookmarks::_load_entry (
    const mobius::core::pod::map &map, const std::string &parent_name
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    auto type = map.get<std::string> ("type");

    if (type == "url")
        _load_url (map, parent_name);

    else if (type == "folder")
        _load_folder (map, parent_name);

    else
        log.development (__LINE__, "Unknown bookmark type: " + type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load folder from JSON dictionary
// @param map Entry map
// @param parent_name Parent folder name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_bookmarks::_load_folder (
    const mobius::core::pod::map &map, const std::string &parent_name
)
{
    // Get folder name
    auto folder_name = map.get<std::string> ("name");

    if (!parent_name.empty ())
        folder_name = parent_name + '.' + folder_name;

    // Load children
    auto children = map.get ("children");

    if (children.is_list ())
    {
        for (const auto &child : children.to_list ())
            _load_entry (child.to_map (), folder_name);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load URL from JSON dictionary
// @param map Entry map
// @param folder_name Folder name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_bookmarks::_load_url (
    const mobius::core::pod::map &map, const std::string &folder_name
)
{
    entry e;

    // Attributes
    e.id = map.get<std::string> ("id");
    e.guid = map.get<std::string> ("guid");
    e.name = map.get<std::string> ("name");
    e.url = map.get<std::string> ("url");
    e.folder_name = folder_name;

    e.creation_time =
        get_datetime_from_string (map.get<std::string> ("date_added"));
    e.last_modified_time =
        get_datetime_from_string (map.get<std::string> ("date_modified"));
    e.last_used_time =
        get_datetime_from_string (map.get<std::string> ("date_last_used"));

    // Meta information
    auto meta_info = map.get ("meta_info");
    if (meta_info.is_map ())
    {
        auto meta_info_map = meta_info.to_map ();

        auto last_visited_desktop = get_datetime_from_string (
            meta_info_map.get<std::string> ("last_visited_desktop")
        );

        if (last_visited_desktop)
            e.last_used_time = get_datetime (last_visited_desktop);
    }

    entries_.push_back (e);
}

} // namespace mobius::extension::app::chromium
