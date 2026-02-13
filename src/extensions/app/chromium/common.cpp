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

// @brief DPAPI prefix used to identify encrypted data
const mobius::core::bytearray DPAPI_PREFIX = {0x01, 0x00, 0x00, 0x00, 0xd0,
                                              0x8c, 0x9d, 0xdf, 0x01, 0x15,
                                              0xd1, 0x11, 0x8c, 0x7a, 0x00,
                                              0xc0, 0x4f, 0xc2, 0x97, 0xeb};

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
        {"/google/chrome/user data/", "chrome", "Google Chrome"},
        {"/microsoft/edge/user data/", "edge", "Microsoft Edge"},
        {"/bravesoftware/brave-browser/user data/", "brave", "Brave"},
        {"/opera software/opera stable", "opera", "Opera"},
        {"/vivaldi/user data/", "vivaldi", "Vivaldi"},
        {"/yandex/yandexbrowser/user data/", "yandex", "Yandex Browser"},
        {"/chromium/user data/", "chromium", "Chromium"},

        // Other Chromium-based browsers
        {"/1stbrowser/user data/", "1stbrowser", "1st Browser"},
        {"/7star/7star/user data/", "7star", "7 Star"},
        {"/aliexpress/user data/", "aliexpress", "AliExpress"},
        {"/amigo/user data/", "amigo", "Amigo"},
        {"/appkiwi/user data/", "appkiwi", "AppKiwi"},
        {"/avast software/browser/user data/", "avast", "Avast Browser"},
        {"/bobrowser/user data/", "bobrowser", "BoBrowser"},
        {"/ccleaner browser/user data/", "ccleaner", "CCleaner Browser"},
        {"/centbrowser/user data/", "centbrowser", "CentBrowser"},
        {"/chedot/user data/", "chedot", "Chedot"},
        {"/chromium/user data/", "chromium", "Chromium"},
        {"/ckaach", "ckaach", "Ckaach"},
        {"/coccoc/browser/user data/", "coccoc", "Coccoc"},
        {"/comodo/dragon/user data/", "comodo", "Comodo Dragon"},
        {"/cryptotab browser/user data/", "cryptotab", "CryptoTab Browser"},
        {"/discord", "discord", "Discord"},
        {"/elements browser/user data/", "elements", "Elements Browser"},
        {"/epic privacy browser/user data/", "epic", "Epic Privacy Browser"},
        {"/google/chrome sxs/user data/", "chrome.canary", "Chrome Canary"},
        {"/kiwi/user data/", "kiwi", "Kiwi Browser"},
        {"/kodi/userdata/addon_data/plugin.program.browser.launcher/profile/2",
         "kodi.browser", "Kodi Browser Launcher"},
        {"/kodi/userdata/addon_data/plugin.program.chrome.launcher/profile",
         "kodi.chrome", "Kodi Chrome Launcher"},
        {"/kometa/user data/", "kometa", "Kometa"},
        {"/lunascape/user data/", "lunascape", "Lunascape"},
        {"/maxthon/user data/", "maxthon", "Maxthon"},
        {"/microsoft/edge beta/user data/", "edge.beta", "Microsoft Edge Beta"},
        {"/microsoft/edge dev/user data/", "edge.dev", "Microsoft Edge Dev"},
        {"/microsoft/edge sxs/user data/", "edge.canary",
         "Microsoft Edge Canary"},
        {"/microsoft/office/", "office", "Microsoft Office"},
        {"/microsoft/onedrive/", "onedrive", "Microsoft OneDrive"},
        {"/mighty browser/user data/", "mighty", "Mighty Browser"},
        {"/naver/whale/user data/", "whale", "Naver Whale"},
        {"/opera software/opera gx stable", "opera-gx", "Opera GX"},
        {"/orbitum/user data/", "orbitum", "Orbitum"},
        {"/packages/microsoft.copilot_8wekyb3d8bbwe/", "copilot",
         "Microsoft Copilot"},
        {"/packages/microsoft.microsoftofficehub_8wekyb3d8bbwe", "officehub",
         "Microsoft Office Hub"},
        {"/packages/microsoft.skypeapp_kzf8qxf38zg5c", "skype", "Skype App"},
        {"/packages/microsoft.windowscommunicationsapps_8wekyb3d8bbwe", "mail",
         "Windows Mail and Calendar"},
        {"/packages/spotifyab.spotifymusic_zpdnekdrzrea0", "spotify",
         "Spotify"},
        {"/packages/msteams_8wekyb3d8bbwe", "teams", "Microsoft Teams"},
        {"/pale moon/user data/", "palemoon", "Pale Moon"},
        {"/plutotv", "plutotv", "PlutoTV"},
        {"/puffin/user data/", "puffin", "Puffin"},
        {"/qutebrowser/user data/", "qutebrowser", "QuteBrowser"},
        {"/qqbrowser/user data/", "qqbrowser", "QQ Browser"},
        {"/rambox/user data/", "rambox", "Rambox"},
        {"/razer/chroma/user data/", "razer", "Razer Chroma"},
        {"/roaming/whatsapp", "whatsapp", "WhatsApp"},
        {"/rockmelt/user data/", "rockmelt", "Rockmelt"},
        {"/sleipnir/user data/", "sleipnir", "Sleipnir"},
        {"/slimjet/user data/", "slimjet", "Slimjet"},
        {"/sputnik/sputnik/user data/", "sputnik", "Sputnik"},
        {"/temp/avastbcltmp/chrome", "avast.bcltmp", "Avast Browser Cleanup"},
        {"/temp/bcltmp/chrome", "chrome.bcltmp",
         "Google Chrome from Avast Browser Cleanup"},
        {"/torch/user data/", "torch", "Torch"},
        {"/ucbrowser/user data/", "ucbrowser", "UC Browser"},
        {"/ucozmedia/uran/user data/", "uran", "Uran"},
        {"/zoom", "zoom", "Zoom"},
};

} // namespace

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert duration to string format
// @param duration Duration in microseconds
// @return Formatted string representing the duration in days, hours, minutes,
// and seconds
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
duration_to_string (std::uint64_t duration)
{
    std::uint64_t seconds = duration / 1000000;
    std::uint64_t microseconds = duration % 1000000;
    std::uint64_t hours = seconds / 3600;
    std::uint64_t minutes = (seconds % 3600) / 60;
    seconds = seconds % 60;

    std::stringstream ss;
    ss << std::setfill ('0') << std::setw (2) << hours << ":"
       << std::setfill ('0') << std::setw (2) << minutes << ":"
       << std::setfill ('0') << std::setw (2) << seconds;

    if (microseconds > 0)
        ss << "." << std::setfill ('0') << std::setw (6) << microseconds;

    return ss.str ();
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
// @brief Convert chromium timestamp string to date/time
// @param timestamp_str String representing the timestamp
// @return Date/time object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
get_datetime_from_string (const std::string &timestamp_str)
{
    if (timestamp_str.empty ())
        return {};

    try
    {
        auto timestamp = std::stoull (timestamp_str);
        return get_datetime (timestamp);
    }
    catch (...)
    {
        return {};
    }
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
                    __LINE__, "Schema version = 0. Path: " + db.get_path ()
                );
            }
        }
        else
        {
            log.warning (
                __LINE__, "Schema version not found in meta table. Path: " +
                              db.get_path ()
            );
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + ". Path: " + db.get_path ()
        );
    }

    return schema_version;
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
    const std::string lower_path = mobius::core::string::tolower (path);

    for (const auto &browser : chromiumBrowsers)
    {
        if (lower_path.find (std::get<0> (browser)) != std::string::npos)
            return {std::get<1> (browser), std::get<2> (browser)};
    }

    mobius::core::log log (__FILE__, __FUNCTION__);

    log.development (__LINE__, "Unknown Chromium-based browser. Path: " + path);

    return {"chromium", "Chromium"};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if a bytearray represents encrypted data
// @param data Bytearray to check
// @return True if the bytearray represents encrypted data, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_encrypted (const mobius::core::bytearray &data)
{
    return data.startswith ("v10") || data.startswith ("v20") ||
           data.startswith (DPAPI_PREFIX);
}

} // namespace mobius::extension::app::chromium