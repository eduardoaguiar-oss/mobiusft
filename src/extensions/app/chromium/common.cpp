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
#include "common.hpp"
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <iomanip>
#include <limits>
#include <sstream>
#include <iostream>
#include <vector>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Debug flag
// This flag is used to enable or disable debug output in the code.
// When set to true, additional debug information will be printed to the
// console. When set to false, debug output will be suppressed. This can be
// useful for development and troubleshooting purposes.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr bool DEBUG = false;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Known Chromium browsers
// This list is based on the Chromium-based browsers. It includes popular
// browsers and some lesser-known ones. The list is ordered by popularity, with
// the most popular browsers listed first. Note: This list may not be exhaustive
// and can change over time as new browsers are released or existing ones are
// updated.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<std::tuple<std::string, std::string, std::string>>
    chromiumBrowsers = {

        // Most popular Chromium-based browsers first
        {"/Google/Chrome/User Data/", "chrome", "Google Chrome"},
        {"/Microsoft/Edge/User Data/", "edge", "Microsoft Edge"},
        {"/BraveSoftware/Brave-Browser/User Data/", "brave", "Brave"},
        {"/Opera Software/Opera Stable", "opera", "Opera"},
        {"/Vivaldi/User Data/", "vivaldi", "Vivaldi"},
        {"/Yandex/YandexBrowser/User Data/", "yandex", "Yandex Browser"},
        {"/Chromium/User Data/", "chromium", "Chromium"},

        // Other Chromium-based browsers
        {"/1stBrowser/User Data/", "1stbrowser", "1st Browser"},
        {"/7Star/7Star/User Data/", "7star", "7 Star"},
        {"/AliExpress/User Data/", "aliexpress", "AliExpress"},
        {"/Amigo/User Data/", "amigo", "Amigo"},
        {"/AppKiwi/User Data/", "appkiwi", "AppKiwi"},
        {"/Avast Software/Browser/User Data/", "avast", "Avast Browser"},
        {"/AVAST Software/Browser/User Data/", "avast", "Avast Browser"},
        {"/BoBrowser/User Data/", "bobrowser", "BoBrowser"},
        {"/CCleaner Browser/User Data/", "ccleaner", "CCleaner Browser"},
        {"/CentBrowser/User Data/", "centbrowser", "CentBrowser"},
        {"/Chedot/User Data/", "chedot", "Chedot"},
        {"/Ckaach", "ckaach", "Ckaach"},
        {"/CocCoc/Browser/User Data/", "coccoc", "Coccoc"},
        {"/Comodo/Dragon/User Data/", "comodo", "Comodo Dragon"},
        {"/CryptoTab Browser/User Data/", "cryptotab", "CryptoTab Browser"},
        {"/Discord", "discord", "Discord"},
        {"/Elements Browser/User Data/", "elements", "Elements Browser"},
        {"/Epic Privacy Browser/User Data/", "epic", "Epic Privacy Browser"},
        {"/Google/Chrome SxS/User Data/", "chrome.canary", "Chrome Canary"},
        {"/Kiwi/User Data/", "kiwi", "Kiwi Browser"},
        {"/Kodi/userdata/addon_data/plugin.program.browser.launcher/profile/2",
         "kodi.browser",
         "Kodi Browser Launcher"},
        {"/Kodi/userdata/addon_data/plugin.program.chrome.launcher/profile",
         "kodi.chrome",
         "Kodi Chrome Launcher"},
        {"/Kometa/User Data/", "kometa", "Kometa"},
        {"/Lunascape/User Data/", "lunascape", "Lunascape"},
        {"/Maxthon/User Data/", "maxthon", "Maxthon"},
        {"/Microsoft/Edge Beta/User Data/", "edge.beta", "Microsoft Edge Beta"},
        {"/Microsoft/Edge Dev/User Data/", "edge.dev", "Microsoft Edge Dev"},
        {"/Microsoft/Edge SxS/User Data/",
         "edge.canary",
         "Microsoft Edge Canary"},
        {"/Microsoft/Office/", "office", "Microsoft Office"},
        {"/Microsoft/OneDrive/", "onedrive", "Microsoft OneDrive"},
        {"/Mighty Browser/User Data/", "mighty", "Mighty Browser"},
        {"/Naver/Whale/User Data/", "whale", "Naver Whale"},
        {"/Opera Software/Opera GX Stable", "opera-gx", "Opera GX"},
        {"/Orbitum/User Data/", "orbitum", "Orbitum"},
        {"/Packages/Microsoft.MicrosoftOfficeHub_8wekyb3d8bbwe/",
         "officehub",
         "Microsoft Office Hub"},
        {"/Packages/SpotifyAB.SpotifyMusic_zpdnekdrzrea0/",
         "spotify",
         "Spotify"},
        {"/Pale Moon/User Data/", "palemoon", "Pale Moon"},
        {"/PlutoTV/", "plutotv", "PlutoTV"},
        {"/Puffin/User Data/", "puffin", "Puffin"},
        {"/QuteBrowser/User Data/", "qutebrowser", "QuteBrowser"},
        {"/QQBrowser/User Data/", "qqbrowser", "QQ Browser"},
        {"/Rambox/User Data/", "rambox", "Rambox"},
        {"/Razer/Chroma/User Data/", "razer", "Razer Chroma"},
        {"/Rockmelt/User Data/", "rockmelt", "Rockmelt"},
        {"/Sleipnir/User Data/", "sleipnir", "Sleipnir"},
        {"/Slimjet/User Data/", "slimjet", "Slimjet"},
        {"/Sputnik/Sputnik/User Data/", "sputnik", "Sputnik"},
        {"/Temp/BCLTMP/Chrome",
         "chrome.bcltmp",
         "Google Chrome from Avast Browser Cleanup"},
        {"/Torch/User Data/", "torch", "Torch"},
        {"/uCozMedia/Uran/User Data/", "uran", "Uran"},
};

} // namespace

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Generate SQL statement with version-aware column replacements
// @param sql_template The SQL template string with
// ${column,start_version,end_version} placeholders
// @param schema_version The current schema version to check against
// @return Processed SQL statement with appropriate columns based on version
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
generate_sql (const std::string &sql_template, int64_t schema_version)
{
    std::string result = sql_template;
    std::string::size_type pos = 0;

    while ((pos = result.find ("${", pos)) != std::string::npos)
    {
        // Find the closing bracket
        auto end_pos = result.find ("}", pos);
        if (end_pos == std::string::npos)
        {
            pos += 2; // Skip the "${" and continue
            continue;
        }

        // Extract the placeholder content
        std::string placeholder = result.substr (pos + 2, end_pos - pos - 2);

        // Split by comma
        std::vector<std::string> parts;
        std::string::size_type prev = 0;
        std::string::size_type comma_pos = 0;

        while ((comma_pos = placeholder.find (',', prev)) != std::string::npos)
        {
            parts.push_back (placeholder.substr (prev, comma_pos - prev));
            prev = comma_pos + 1;
        }
        parts.push_back (placeholder.substr (prev));

        // Default to empty string if column should be excluded
        std::string replacement = "NULL";

        // Get column name and version ranges
        std::string column_name = parts[0];
        int64_t start_version = -1;
        int64_t end_version = std::numeric_limits<int64_t>::max ();

        // Parse start_version if provided
        if (parts.size () > 1 && !parts[1].empty ())
        {
            try
            {
                start_version = std::stoll (parts[1]);
            }
            catch (...)
            {
                // Keep default if parsing fails
            }
        }

        // Parse end_version if provided
        if (parts.size () > 2 && !parts[2].empty ())
        {
            try
            {
                end_version = std::stoll (parts[2]);
            }
            catch (...)
            {
                // Keep default if parsing fails
            }
        }

        // Check if current schema version is within range
        if (schema_version >= start_version && schema_version <= end_version)
            replacement = column_name;

        // Replace the placeholder with the column name or empty string
        result.replace (pos, end_pos - pos + 1, replacement);

        // Continue searching from current position
        // No need to advance pos since the replacement might be shorter than
        // the placeholder
    }

    if (DEBUG)
        std::cout << "Generated SQL (schema version=" << schema_version
                  << "): " << result << std::endl;

    return result;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert chromium timestamp to date/time
// @param timestamp Numerical value representing the timestamp
// @return Date/time object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
get_datetime (std::uint64_t timestamp)
{
    if (timestamp == 0 || timestamp == 86400000000)
        return {};

    else if (timestamp <= std::numeric_limits<std::uint32_t>::max ())
        return mobius::core::datetime::new_datetime_from_unix_timestamp (
            timestamp
        );

    else if (timestamp < 10000000000000000)
        return mobius::core::datetime::new_datetime_from_unix_timestamp (
            timestamp / 1000000
        );

    else
        return mobius::core::datetime::new_datetime_from_nt_timestamp (
            timestamp * 10
        );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get schema version from database
// @param db Database object
// @return Schema version
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int64_t
get_db_schema_version (mobius::core::database::database db)
{
    std::int64_t schema_version = 0;

    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        auto stmt =
            db.new_statement ("SELECT value FROM meta WHERE key = 'version'");

        if (stmt.fetch_row ())
        {
            schema_version = stmt.get_column_int64 (0);

            if (!schema_version)
            {
                log.warning (
                    __LINE__,
                    "Schema version = 0. Path: " + db.get_path ()
                );
            }
        }
        else
        {
            log.warning (
                __LINE__,
                "Schema version not found in meta table. Path: " +
                    db.get_path ()
            );
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__,
            std::string (e.what ()) + ". Path: " + db.get_path ()
        );
    }

    return schema_version;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get username from path
// @param path Path to profile
// @return Username extracted from path
//
// @note Paths are in the following format: /FSxx/Users/username/... or
// /FSxx/home/username/... where FSxx is the filesystem identifier.
// Example: /FS01/Users/johndoe/AppData/Local/Google/Chrome/User Data/
// In this case, the username is "johndoe".
// If the path does not match the expected format, an empty string is returned.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_username_from_path (const std::string &path)
{
    auto dirnames = mobius::core::string::split (path, "/");

    if (dirnames.size () > 3 &&
        (dirnames[2] == "Users" || dirnames[2] == "home"))
        return dirnames[3]; // Username is the fourth directory

    return {}; // No username found
}

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
    for (const auto &browser : chromiumBrowsers)
    {
        if (path.find (std::get<0> (browser)) != std::string::npos)
            return {std::get<1> (browser), std::get<2> (browser)};
    }

    mobius::core::log log (__FILE__, __FUNCTION__);

    log.development (__LINE__, "Unknown Chromium-based browser. Path: " + path);

    return {"chromium", "Chromium"};
}

} // namespace mobius::extension::app::chromium