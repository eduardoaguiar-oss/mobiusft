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
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <tuple>
#include <utility>
#include <algorithm>
#include <set>
#include <vector>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Known Emule based applications (dirname, app_id, app_name)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<std::tuple<std::string, std::string, std::string>> EMULE_APPS = {
    {"emule", "emule", "eMule"},
    {"dreamule", "dreamule", "DreaMule"},
    {"amule", "amule", "aMule"},
};

} // namespace

namespace mobius::extension::app::emule
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get application ID and name from path
// @param path Path to profile
// @return Pair containing application ID and name
// @note This function checks the path against known Chromium-based browsers
// and returns the corresponding application ID and name.
// If the path does not match any known browser, it returns "chromium" as
// the application ID and "Chromium" as the application name.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::pair<std::string, std::string>
get_app_from_path (const std::string &path)
{
    std::set<std::string> lower_path_parts;
    auto path_parts = mobius::core::string::split (path, "/");

    std::transform (
        path_parts.begin (), path_parts.end (),
        std::inserter (lower_path_parts, lower_path_parts.end ()),
        [] (const std::string &part)
        { return mobius::core::string::tolower (part); }
    );

    for (const auto &app : EMULE_APPS)
    {
        if (lower_path_parts.find (std::get<0> (app)) != lower_path_parts.end ())
            return {std::get<1> (app), std::get<2> (app)};
    }

    mobius::core::log log (__FILE__, __FUNCTION__);

    log.development (
        __LINE__, "Unknown eMule-based application. Path: " + path
    );

    return {"emule", "eMule"};
}

} // namespace mobius::extension::app::emule