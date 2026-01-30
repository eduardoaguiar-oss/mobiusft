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
#include "file_places_sqlite.hpp"

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
        file_places_sqlite fp (f.new_reader ());

        if (!fp)
            return;

        log.info (__LINE__, "File decoded [places.sqlite]: " + f.get_path ());

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add bookmarks
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &entry : fp.get_bookmarks ())
        {
            bookmark b;
            b.creation_time = entry.date_added;
            b.folder = entry.parent_name;
            b.name = entry.title;
            b.url = entry.url;
            b.f = f;

            // Metadata
            b.metadata.set ("date_added", entry.date_added);
            b.metadata.set ("fk", entry.fk);
            b.metadata.set ("folder_type", entry.folder_type);
            b.metadata.set ("guid", entry.guid);
            b.metadata.set ("id", entry.id);
            b.metadata.set ("keyword_id", entry.keyword_id);
            b.metadata.set ("last_modified", entry.last_modified);
            b.metadata.set ("parent", entry.parent);
            b.metadata.set ("position", entry.position);
            b.metadata.set ("sync_change_counter", entry.sync_change_counter);
            b.metadata.set ("sync_status", entry.sync_status);
            b.metadata.set ("type", entry.type);

            bookmarks_.push_back (b);
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add visited URLs
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &entry : fp.get_visited_urls ())
        {
            visited_url vu;
            vu.timestamp = entry.visit_date;
            vu.title = entry.title;
            vu.url = entry.url;
            vu.f = f;

            // Metadata
            vu.metadata.set ("from_visit", entry.from_visit);
            vu.metadata.set ("visit_id", entry.visit_id);
            vu.metadata.set ("place_id", entry.place_id);
            vu.metadata.set ("session", entry.session);
            vu.metadata.set ("source", entry.source);
            vu.metadata.set ("triggering_place_id", entry.triggering_place_id);
            vu.metadata.set ("visit_type", entry.visit_type);
            vu.metadata.set ("alt_frecency", entry.alt_frecency);
            vu.metadata.set ("description", entry.description);
            vu.metadata.set ("favicon_id", entry.favicon_id);
            vu.metadata.set ("foreign_count", entry.foreign_count);
            vu.metadata.set ("frecency", entry.frecency);
            vu.metadata.set ("guid", entry.guid);
            vu.metadata.set ("hidden", entry.hidden);
            vu.metadata.set ("places_id", entry.places_id);
            vu.metadata.set ("last_visit_date", entry.last_visit_date);
            vu.metadata.set ("origin_id", entry.origin_id);
            vu.metadata.set ("preview_image_url", entry.preview_image_url);
            vu.metadata.set ("recalc_alt_frecency", entry.recalc_alt_frecency);
            vu.metadata.set ("recalc_frecency", entry.recalc_frecency);
            vu.metadata.set ("rev_host", entry.rev_host);
            vu.metadata.set ("site_name", entry.site_name);
            vu.metadata.set ("title", entry.title);
            vu.metadata.set ("typed", entry.typed);
            vu.metadata.set ("url_hash", entry.url_hash);
            vu.metadata.set ("visit_count", entry.visit_count);

            visited_urls_.push_back (vu);
        }

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
