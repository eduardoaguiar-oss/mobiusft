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
#include "evidence_processor_impl.hpp"
#include <mobius/core/datasource/datasource_vfs.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/io/uri.hpp>
#include <mobius/core/io/walker.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/framework/evidence_flag.hpp>
#include <mobius/framework/model/evidence.hpp>
#include <mobius/framework/utils.hpp>
#include "file_cookie.hpp"
#include "file_msiecf.hpp"

#include <iostream>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string APP_ID = "internet-explorer";
static const std::string APP_NAME = "Internet Explorer";

} // namespace

namespace mobius::extension::app::internet_explorer
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param item Item object
// @param profile Profile object
// @param mediator Mediator object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
evidence_processor_impl::evidence_processor_impl (
    const mobius::framework::model::item &item,
    const mobius::framework::evidence_processor::profile &,
    const mobius::framework::evidence_processor::mediator &mediator
)
    : item_ (item),
      mediator_ (mediator)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan all subfolders of a folder
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::on_folder_entered (const mobius::core::io::folder &folder)
{
    _scan_folder (folder);
    _scan_cookies_folder (folder);
    _scan_favorites_folder (folder);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Called when processing is complete
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::on_complete ()
{
    _save_cookies ();
    _save_visited_urls ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for Internet Explorer artifacts
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::_scan_folder (const mobius::core::io::folder &folder)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Scan folder
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto w = mobius::core::io::walker (folder);

    for (const auto &[name, f] : w.get_files_with_names ())
    {
        try
        {
            if (name == "index.dat")
                _decode_index_dat_file (f);

            else if (name == "webcachev01.dat" || name == "container.dat")
                _decode_webcachev01_dat_file (f);
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")");
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for Internet Explorer cookies
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::_scan_cookies_folder (const mobius::core::io::folder &folder)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (folder.get_name () != "Cookies")
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Scan folder
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto w = mobius::core::io::walker (folder);

    for (const auto &[name, f] : w.get_files_with_names ())
    {
        try
        {
            if (name.ends_with (".txt") || name.ends_with (".cookie"))
                _decode_cookie_file (f);
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")");
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for Internet Explorer favorites
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::_scan_favorites_folder (const mobius::core::io::folder &folder)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    //if (folder.get_name () != "Favorites")
    //    return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Scan folder
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto w = mobius::core::io::walker (folder);

    for (const auto &[name, f] : w.get_files_with_names ())
    {
        try
        {
            if (name.ends_with (".url"))
std::cout << "Decoding favorite file: " << f.get_path () << std::endl;
                //_decode_favorite_file (f);
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")");
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode Internet Explorer favorite file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*void
evidence_processor_impl::_decode_favorite_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        file_msiecf fm (f.new_reader ());

        if (!fm)
            return;

        log.info (__LINE__, "File decoded [favorite]: " + f.get_path ());

        auto username = mobius::framework::get_username_from_path (f.get_path ());

        for (const auto &url : fm.get_urls ())
        {
            if (url.location_type == "favorite")
            {
                visited_url vu;
                vu.timestamp = url.access_time;
                vu.title = url.page_title;
                vu.url = url.location_value;
                vu.username = username;

                if (vu.username.empty ())
                    vu.username = url.location_username;

                vu.metadata.set ("access_time", url.access_time);
                vu.metadata.set ("data_offset", url.data_offset);
                vu.metadata.set ("data_size", url.data_size);
                vu.metadata.set ("expiration_time", url.expiration_time);

                vu.f = f;

                visited_urls_.emplace_back (std::move (vu));
            }
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")");
    }
}*/

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add index.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::_decode_index_dat_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Try to parse the index.dat file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_msiecf fm (f.new_reader ());

        if (!fm)
            return;

        log.info (__LINE__, "File decoded [index.dat]: " + f.get_path ());

        auto username = mobius::framework::get_username_from_path (f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Retrieve Visited URLs
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        bool has_iedownload = false;

        for (const auto &url : fm.get_urls ())
        {
            if (url.location_type == "visited" || url.location_type == "mshist")
            {
                visited_url vu;
                vu.timestamp = url.access_time;
                vu.title = url.page_title;
                vu.url = url.location_value;
                vu.username = username;

                if (vu.username.empty ())
                    vu.username = url.location_username;

                vu.metadata.set ("access_time", url.access_time);
                vu.metadata.set ("data_offset", url.data_offset);
                vu.metadata.set ("data_size", url.data_size);
                vu.metadata.set ("expiration_time", url.expiration_time);
                vu.metadata.set ("filename", url.filename);
                vu.metadata.set ("filename_offset", url.filename_offset);
                vu.metadata.set ("hits", url.hits);
                vu.metadata.set ("is_deleted", url.is_deleted);
                vu.metadata.set ("is_reallocated", url.is_reallocated);
                vu.metadata.set ("last_sync_time", url.last_sync_time);
                vu.metadata.set ("local_time", url.local_time);
                vu.metadata.set ("location", url.location);
                vu.metadata.set ("location_offset", url.location_offset);
                vu.metadata.set ("location_type", url.location_type);
                vu.metadata.set ("location_username", url.location_username);
                vu.metadata.set ("location_value", url.location_value);
                vu.metadata.set ("modification_time", url.modification_time);
                vu.metadata.set ("page_title", url.page_title);
                vu.metadata.set ("query_string", url.query_string);
                vu.metadata.set (
                    "record_calculated_hash",
                    "0x" + mobius::core::string::to_hex (url.calculated_hash, 8)
                );
                vu.metadata.set ("record_hash", "0x" + mobius::core::string::to_hex (url.record_hash, 8));
                vu.metadata.set ("record_hash_table_idx", url.record_hash_table_idx);
                vu.metadata.set ("record_hash_entry_idx", url.record_hash_entry_idx);
                vu.metadata.set ("record_offset", url.record_offset);
                vu.metadata.set (
                    "record_original_hash",
                    "0x" + mobius::core::string::to_hex (url.record_original_hash, 8)
                );
                vu.metadata.set ("record_type", url.record_type);

                // Add sorted tags to metadata
                std::vector<std::pair<uint8_t, mobius::core::pod::data>> sorted_tags (
                    url.tags.begin (),
                    url.tags.end ()
                );

                std::sort (
                    sorted_tags.begin (),
                    sorted_tags.end (),
                    [] (const auto &a, const auto &b) { return a.first < b.first; }
                );

                for (const auto &[k, v] : sorted_tags)
                    vu.metadata.set ("tag_" + mobius::core::string::to_hex (k, 2), v.to_string ());

                // Set the file object for the visited URL entry
                vu.f = f;

                visited_urls_.push_back (vu);
            }

            else if (url.location_type == "iedownload")
                has_iedownload = true;
        }

        if (has_iedownload)
            log.development (__LINE__, "IEDownload location type detected. Path: " + f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit ("sampling_file", std::string ("app.internet-explorer.index_dat"), f.new_reader ());
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")");
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add WebCacheV01.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::_decode_webcachev01_dat_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Test signature for now. We do not parse WebCacheV01.dat file yet
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto reader = f.new_reader ();
        reader.skip (4);
        auto signature = reader.read (4);

        if (signature != "\x89\xab\xcd\xef")
            return;

        log.info (__LINE__, "File detected [WebCacheV01.dat]: " + f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit ("sampling_file", std::string ("app.internet-explorer.webcachev01_dat"), f.new_reader ());
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")");
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode cookie file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::_decode_cookie_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Try to parse the cookie file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_cookie fc (f.new_reader ());

        if (!fc)
            return;

        log.info (__LINE__, "File decoded [cookie]: " + f.get_path ());

        auto username = mobius::framework::get_username_from_path (f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Retrieve cookies
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &c : fc.get_cookies ())
        {
            cookie cookie;

            cookie.domain = c.domain;
            cookie.name = c.name;
            cookie.value = c.value;
            cookie.creation_time = c.creation_time;
            cookie.expiration_time = c.expiration_time;
            cookie.username = username;

            cookie.metadata.set ("is_secure", c.is_secure);
            cookie.metadata.set ("is_session", c.is_session);
            cookie.metadata.set ("is_third_party", c.is_third_party);
            cookie.metadata.set ("is_restricted", c.is_restricted);
            cookie.metadata.set ("is_ie6", c.is_ie6);
            cookie.metadata.set ("is_legacy", c.is_legacy);
            cookie.metadata.set ("is_httponly", c.is_httponly);

            cookie.f = f;

            cookies_.emplace_back (std::move (cookie));
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit ("sampling_file", std::string ("app.internet-explorer.cookie"), f.new_reader ());
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")");
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save cookies
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::_save_cookies ()
{
    for (const auto &c : cookies_)
    {
        auto e = item_.new_evidence ("cookie");
        e.set_attribute ("app_id", APP_ID);
        e.set_attribute ("app_name", APP_NAME);
        e.set_attribute ("username", c.username);
        e.set_attribute ("name", c.name);
        e.set_attribute ("value", c.value);
        e.set_attribute ("domain", c.domain);
        e.set_attribute ("creation_time", c.creation_time);
        e.set_attribute ("expiration_time", c.expiration_time);
        e.set_attribute ("is_deleted", c.f.is_deleted ());
        e.set_attribute ("metadata", c.metadata);

        e.set_tag ("app.browser");
        e.add_source (c.f);

        // Tell mediator about the new evidence
        mediator_.on_evidence_created (e);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save visited URLs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::_save_visited_urls ()
{
    for (const auto &vu : visited_urls_)
    {
        auto e = item_.new_evidence ("visited-url");

        e.set_attribute ("username", vu.username);
        e.set_attribute ("url", vu.url);
        e.set_attribute ("title", vu.title);
        e.set_attribute ("timestamp", vu.timestamp);
        e.set_attribute ("app_name", APP_NAME);
        e.set_attribute ("metadata", vu.metadata);

        e.set_tag ("app.browser");
        e.add_source (vu.f);

        // Tell mediator about the new evidence
        mediator_.on_evidence_created (e);
    }
}

} // namespace mobius::extension::app::internet_explorer
