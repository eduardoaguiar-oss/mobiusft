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
#include <tuple>
#include <string>
#include <vector>
#include "common.hpp"
#include "file_cookies_sqlite.hpp"
#include "file_downloads_sqlite.hpp"
#include "file_formhistory_sqlite.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Known Gecko browsers
// This list is based on the Gecko-based browsers. It includes popular
// browsers and some lesser-known ones. The list is ordered by popularity, with
// the most popular browsers listed first. Note: This list may not be exhaustive
// and can change over time as new browsers are released or existing ones are
// updated.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<std::tuple<std::string, std::string, std::string>> GECKO_BROWSERS = {
    {"/Mozilla/Firefox/", "firefox", "Mozilla Firefox"},
    {"/Waterfox/", "waterfox", "Waterfox"},
    {"/SeaMonkey/", "seamonkey", "SeaMonkey"},
    {"/Pale Moon/", "palemoon", "Pale Moon"},
    {"/K-Meleon/", "kmeleon", "K-Meleon"},
    {"/Basilisk/", "basilisk", "Basilisk"},
    {"/Comodo/ICEDragon/", "icedragon", "Comodo ICEDragon"},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Download states mapping
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::map<std::int64_t, std::string> DOWNLOAD_STATES = {
    {0, ""},
    {1, "Finished"},
    {2, "Failed"},
    {3, "Cancelled"},
    {4, "Paused"},
    {5, "Queued"},
    {6, "Blocked Parental"},
    {7, "Scanning"},
    {8, "Virus Detected"},
    {9, "Blocked Policy"},
};

} // namespace

namespace mobius::extension::app::gecko
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
    // Set profile ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto parent_name =
        f.get_parent () ? f.get_parent ().get_name () : std::string {};

    if (parent_name == "Profiles")
    {
        auto pos = parent_name.find ('.');

        if (pos != std::string::npos)
            profile_id_ = parent_name.substr (0, pos);

        else
            profile_id_ = parent_name;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get application ID and name according to path
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto path = f.get_path ();

    for (const auto &browser : GECKO_BROWSERS)
    {
        if (path.find (std::get<0> (browser)) != std::string::npos)
        {
            app_id_ = std::get<1> (browser);
            app_name_ = std::get<2> (browser);
            break;
        }
    }

    if (app_id_.empty () || app_name_.empty ())
    {
        mobius::core::log log (__FILE__, __FUNCTION__);
        log.development (
            __LINE__, "Unknown Gecko-based browser. Path: " + path
        );

        app_id_ = "gecko";
        app_name_ = "Gecko";
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Emit sampling_folder event
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::emit (
        "sampling_folder", std::string ("app.gecko.profiles"), f
    );
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
// @brief Add cookies.sqlite file
// @param f cookies.sqlite file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_cookies_sqlite (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_cookies_sqlite fc (f.new_reader ());

        if (!fc)
            return;

        log.info (__LINE__, "File decoded [cookies.sqlite]: " + f.get_path ());

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add cookies
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &entry : fc.get_cookies ())
        {
            cookie c;
            c.domain = entry.host;
            c.name = entry.name;
            c.value = entry.value;
            c.creation_time = entry.creation_time;
            c.expiration_time = entry.expiry;
            c.last_access_time = entry.last_accessed;

            if (c.last_access_time == c.creation_time)
                c.last_update_time = entry.last_accessed;

            c.metadata.set ("app_id", entry.app_id);
            c.metadata.set ("base_domain", entry.base_domain);
            c.metadata.set ("host", entry.host);
            c.metadata.set ("id", entry.id);
            c.metadata.set ("in_browser_element", entry.in_browser_element);
            c.metadata.set ("is_http_only", entry.is_http_only);
            c.metadata.set ("is_secure", entry.is_secure);
            c.metadata.set ("origin_attributes", entry.origin_attributes);
            c.metadata.set ("path", entry.path);
            c.metadata.set ("raw_same_site", entry.raw_same_site);
            c.metadata.set ("same_site", entry.same_site);
            c.f = f;

            cookies_.push_back (c);
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.gecko.cookies_sqlite"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add downloads.sqlite file
// @param f downloads.sqlite file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_downloads_sqlite (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_downloads_sqlite fd (f.new_reader ());

        if (!fd)
            return;

        log.info (
            __LINE__, "File decoded [downloads.sqlite]: " + f.get_path ()
        );

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add downloads
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &entry : fd.get_downloads ())
        {
            download d;
            d.timestamp = entry.start_time;
            d.filename = entry.name;
            d.path = entry.target;

            d.metadata.set ("auto_resume", entry.auto_resume);
            d.metadata.set ("curr_bytes", entry.curr_bytes);
            d.metadata.set ("end_time", entry.end_time);
            d.metadata.set ("entity_id", entry.entity_id);
            d.metadata.set ("guid", entry.guid);
            d.metadata.set ("id", entry.id);
            d.metadata.set ("max_bytes", entry.max_bytes);
            d.metadata.set ("mime_type", entry.mime_type);
            d.metadata.set ("preferred_action", entry.preferred_action);
            d.metadata.set (
                "preferred_application", entry.preferred_application
            );
            d.metadata.set ("referrer", entry.referrer);
            d.metadata.set ("source", entry.source);
            d.metadata.set (
                "state", mobius::framework::get_domain_text (
                             DOWNLOAD_STATES, entry.state
                         )
            );
            d.metadata.set ("start_time", entry.start_time);
            d.metadata.set ("target", entry.target);
            d.metadata.set ("temp_path", entry.temp_path);
            d.f = f;

            downloads_.push_back (d);
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.gecko.downloads_sqlite"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add formhistory.sqlite file
// @param f formhistory.sqlite file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_formhistory_sqlite (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_formhistory_sqlite ff (f.new_reader ());

        if (!ff)
            return;

        log.info (
            __LINE__, "File decoded [formhistory.sqlite]: " + f.get_path ()
        );

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add form history entries
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &entry : ff.get_form_history ())
        {
            form_history fh;
            fh.fieldname = entry.fieldname;
            fh.value = entry.value;
            fh.first_used = entry.first_used;
            fh.last_used = entry.last_used;

            fh.metadata.set ("guid", entry.guid);
            fh.metadata.set ("id", entry.id);
            fh.metadata.set ("idx", entry.idx);
            fh.metadata.set ("times_used", entry.times_used);
            fh.f = f;

            form_history_.push_back (fh);
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.gecko.formhistory_sqlite"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add places.sqlite file
// @param f places.sqlite file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_places_sqlite (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // file_places_sqlite fp (f.new_reader ());

        /*if (!fp)
        {
            log.info (__LINE__, "File is not a valid 'places.sqlite' file");
            return;
        }*/

        log.info (__LINE__, "File decoded [places.sqlite]: " + f.get_path ());

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.gecko.places_sqlite"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add signons.sqlite file
// @param f signons.sqlite file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_signons_sqlite (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // file_signons_sqlite fs (f.new_reader ());

        /*if (!fs)
        {
            log.info (__LINE__, "File is not a valid 'signons.sqlite' file");
            return;
        }*/

        log.info (__LINE__, "File decoded [signons.sqlite]: " + f.get_path ());

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.gecko.signons_sqlite"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

} // namespace mobius::extension::app::gecko
