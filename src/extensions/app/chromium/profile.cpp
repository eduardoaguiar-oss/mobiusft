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
#include "profile.hpp"
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/core/value_selector.hpp>
#include <tuple>
#include <algorithm>
#include <string>
#include <vector>
#include "file_history.hpp"
#include "file_web_data.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Known Chromium browsers
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
        {"/Opera Software/Opera Stable/", "opera", "Opera"},
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
        {"/Ckaach/", "ckaach", "Ckaach"},
        {"/CocCoc/Browser/User Data/", "coccoc", "Coccoc"},
        {"/Comodo/Dragon/User Data/", "comodo", "Comodo Dragon"},
        {"/CryptoTab Browser/User Data/", "cryptotab", "CryptoTab Browser"},
        {"/Discord/", "discord", "Discord"},
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
        {"/Mighty Browser/User Data/", "mighty", "Mighty Browser"},
        {"/Naver/Whale/User Data/", "whale", "Naver Whale"},
        {"/Opera Software/Opera GX Stable/", "opera-gx", "Opera GX"},
        {"/Orbitum/User Data/", "orbitum", "Orbitum"},
        {"/PlutoTV/", "plutotv", "PlutoTV"},
        {"/Sputnik/Sputnik/User Data/", "sputnik", "Sputnik"},
        {"/Temp/BCLTMP/Chrome/",
         "chrome.bcltmp",
         "Google Chrome from Avast Browser Cleanup"},
        {"/Torch/User Data/", "torch", "Torch"},
        {"/uCozMedia/Uran/User Data/", "uran", "Uran"},
};

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
_get_username (const std::string &path)
{
    auto dirnames = mobius::core::string::split (path, "/");

    if (dirnames.size () > 3 &&
        (dirnames[2] == "Users" || dirnames[2] == "home"))
        return dirnames[3]; // Username is the fourth directory

    return {}; // No username found
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @see
// https://github.com/obsidianforensics/hindsight/blob/main/documentation/Evolution%20of%20Chrome%20Databases%20(v35).pdf
// @see
// https://medium.com/@jsaxena017/web-browser-forensics-part-1-chromium-browser-family-99b807083c25
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set folder
// @param f Folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::set_folder (const mobius::core::io::folder &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    folder_ = f;
    profile_name_ = f.get_name ();
    last_modified_time_ = f.get_modification_time ();
    creation_time_ = f.get_creation_time ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get username from path
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto path = f.get_path ();
    username_ = _get_username (path);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Identify browser based on folder name
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto it = std::find_if (
        chromiumBrowsers.begin (),
        chromiumBrowsers.end (),
        [&path] (const auto &browser)
        { return path.find (std::get<0> (browser)) != std::string::npos; }
    );

    if (it != chromiumBrowsers.end ())
    {
        app_id_ = std::get<1> (*it);
        app_name_ = std::get<2> (*it);
    }
    else
        log.development (
            __LINE__,
            "Unidentified Chromium-based browser. Profile path: " + path
        );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add Preferences file
// @param f Preferences file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_preferences_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto reader = f.new_reader ();
    if (!reader)
        return;

    is_valid_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add History file
// @param f History file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_history_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_history fh (f.new_reader ());

    if (!fh)
    {
        log.info (__LINE__, "File is not a valid 'History' file");
        return;
    }

    log.info (__LINE__, "File " + f.get_path () + " is a valid 'History' file");

    if (!last_modified_time_ ||
        f.get_modification_time () > last_modified_time_)
        last_modified_time_ = f.get_modification_time ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add history entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &entry : fh.get_history_entries ())
    {
        history_entry e (entry);
        e.f = f;

        history_entries_.push_back (e);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add downloads
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &entry : fh.get_downloads ())
    {
        download d (entry);
        d.f = f;

        downloads_.push_back (d);
    }

    is_valid_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add Web Data file
// @param f Web Data file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_web_data_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_web_data web_data (f.new_reader ());

    if (!web_data)
    {
        log.info (__LINE__, "File is not a valid 'Web Data' file");
        return;
    }

    log.info (
        __LINE__,
        "File " + f.get_path () + " is a valid 'Web Data' file"
    );

    if (!last_modified_time_ ||
        f.get_modification_time () > last_modified_time_)
        last_modified_time_ = f.get_modification_time ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add autofill entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &entry : web_data.get_autofill_entries ())
    {
        autofill a;

        a.idx = entry.idx;
        a.name = entry.name;
        a.count = entry.count;
        a.date_created = entry.date_created;
        a.date_last_used = entry.date_last_used;
        a.is_encrypted = entry.is_encrypted;

        if (entry.is_encrypted)
            a.encrypted_value = entry.value; // Store encrypted value
        else
            a.value = entry.value.to_string (); // Store plaintext value

        a.f = f;

        autofill_.push_back (a);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add accounts (and autofill)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &p : web_data.get_autofill_profiles ())
    {
        account acc;

        // Attributes
        acc.id = p.guid;
        acc.emails = p.emails;
        acc.organizations = std::vector<std::string> {p.company_name};
        acc.f = f;

        // Phone numbers
        std::transform (
            p.phones.begin (),
            p.phones.end (),
            std::back_inserter (acc.phone_numbers),
            [] (const auto &phone) { return phone.number; }
        );

        // Addresses
        std::transform (
            p.addresses.begin (),
            p.addresses.end (),
            std::back_inserter (acc.addresses),
            [] (const auto &addr)
            {
                std::string s;
                if (!addr.address_line_1.empty ())
                    s += addr.address_line_1 + ", ";
                if (!addr.address_line_2.empty ())
                    s += addr.address_line_2 + ", ";
                if (!addr.street_address.empty ())
                    s += addr.street_address + ", ";
                if (!addr.city.empty ())
                    s += addr.city + ", ";
                if (!addr.state.empty ())
                    s += addr.state + " ";
                if (!addr.zip_code.empty ())
                    s += addr.zip_code + ", ";
                if (!addr.country.empty ())
                    s += addr.country;
                return s;
            }
        );

        // Metadata
        acc.metadata.set ("origin", p.origin);
        acc.metadata.set ("language_code", p.language_code);
        acc.metadata.set ("date_modified", p.date_modified);
        acc.metadata.set ("date_last_used", p.date_last_used);
        acc.metadata.set ("use_count", p.use_count);
        acc.metadata.set ("is_in_trash", p.is_in_trash);

        accounts_.push_back (acc);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add credit cards
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &card : web_data.get_credit_cards ())
    {
        credit_card c (card);
        c.f = f;

        credit_cards_.push_back (c);
    }

    is_valid_ = true;
}

} // namespace mobius::extension::app::chromium
