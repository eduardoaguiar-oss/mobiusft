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
#include "file_history.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <unordered_map>
#include <unordered_set>
#include "common.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// History file tables
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// - downloads: Downloads
//
// - downloads_url_chains: Download URL chains
//
// - keyword_search_terms: Keyword search terms
//      - keyword_id: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - lower_term: 20, 22-23, 28-30, 32-33, 36-41
//      - normalized_term: 43-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - term: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - url_id: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//
// - urls: URLs
//      - favicon_id: 20, 22-23, 28-30, 32-33
//      - hidden: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - id: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - last_visit_time: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - title: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - typed_count: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - url: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - visit_count: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//
// - visited_links: Visited links
//      - frame_url: 67-70
//      - id: 67-70
//      - link_url_id: 67-70
//      - top_level_url: 67-70
//      - visit_count: 67-70

// - visits: Visits
//      - app_id: 69-70
//      - consider_for_ntp_most_visited: 63, 65-70
//      - external_referrer_url: 66-70
//      - from_visit: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - id: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - incremented_omnibox_typed_score: 40-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - is_indexed: 20, 22-23
//      - is_known_to_sync: 59, 61-63, 65-70
//      - opener_visit: 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - originator_cache_guid: 55-56, 58-59, 61-63, 65-70
//      - originator_from_visit: 56, 58-59, 61-63, 65-70
//      - originator_opener_visit: 56, 58-59, 61-63, 65-70
//      - originator_visit_id: 55-56, 58-59, 61-63, 65-70
//      - publicly_routable: 43-45, 48
//      - segment_id: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - transition: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - url: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - visit_duration: 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - visit_time: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-70
//      - visited_link_id: 67-70
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unknown schema versions
// This set contains schema versions that are not recognized or not handled
// by the current implementation. It is used to identify unsupported versions
// of the web data schema in Chromium-based applications.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::unordered_set<std::int64_t> UNKNOWN_SCHEMA_VERSIONS = {
    1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18,
    19, 21, 24, 25, 26, 27, 31, 34, 35, 46, 47, 49, 52, 54, 57, 60, 64,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Last known schema version
// This constant represents the last schema version that is known and handled
// by the current implementation. Any schema version greater than this value
// will be considered unsupported and will trigger a warning in the log.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::int64_t LAST_KNOWN_SCHEMA_VERSION = 70;

} // namespace

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_history::file_history (const mobius::core::io::reader &reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (!reader)
        return;

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Copy reader content to temporary file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::io::tempfile tfile;
        tfile.copy_from (reader);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get schema version
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::database::database db (tfile.get_path ());
        schema_version_ = get_db_schema_version (db);

        if (!schema_version_)
            return;

        if (schema_version_ > LAST_KNOWN_SCHEMA_VERSION ||
            UNKNOWN_SCHEMA_VERSIONS.find (schema_version_) !=
                UNKNOWN_SCHEMA_VERSIONS.end ())
        {
            log.development (
                __LINE__,
                "Unhandled schema version: " + std::to_string (schema_version_)
            );
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load data
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        _load_history (db);
        _load_downloads (db);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Finish decoding
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load downloads
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_history::_load_downloads (mobius::core::database::database &db)
{
    if (schema_version_ < 24)
        _load_downloads_01 (db);

    else
        _load_downloads_24 (db);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load downloads for schema versions 1-23
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_history::_load_downloads_01 (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare SQL statement for table downloads
        auto stmt = db.new_statement (
            "SELECT end_time, "
            "full_path, "
            "id, "
            "opened, "
            "received_bytes, "
            "start_time, "
            "state, "
            "total_bytes, "
            "url "
            "FROM downloads"
        );

        // Retrieve records from downloads table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            download obj;

            obj.idx = idx++;
            obj.end_time = get_datetime (stmt.get_column_int64 (0));
            obj.full_path = stmt.get_column_string (1);
            obj.id = stmt.get_column_int64 (2);
            obj.opened = stmt.get_column_int64 (3);
            obj.received_bytes = stmt.get_column_int64 (4);
            obj.start_time = get_datetime (stmt.get_column_int64 (5));
            obj.state = stmt.get_column_int64 (6);
            obj.total_bytes = stmt.get_column_int64 (7);
            obj.url = stmt.get_column_string (8);

            // Add downloads to the list
            downloads_.emplace_back (std::move (obj));
        }

        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load downloads for schema versions 24 and above
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_history::_load_downloads_24 (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare SQL statement for tables downloads and downloads_url_chains
        auto stmt = db.new_statement_with_pattern (
            "SELECT {downloads:d.by_ext_id}, "
            "{downloads:d.by_ext_name}, "
            "{downloads:d.by_web_app_id}, "
            "d.current_path, "
            "d.danger_type, "
            "{downloads:d.embedder_download_data}, "
            "d.end_time, "
            "{downloads:d.etag}, "
            "{downloads:d.guid}, "
            "{downloads:d.hash}, "
            "{downloads:d.http_method}, "
            "d.id, "
            "d.interrupt_reason, "
            "{downloads:d.last_access_time}, "
            "{downloads:d.last_modified}, "
            "{downloads:d.mime_type}, "
            "d.opened, "
            "{downloads:d.original_mime_type}, "
            "d.received_bytes, "
            "{downloads:d.referrer}, "
            "{downloads:d.site_url}, "
            "d.start_time, "
            "d.state, "
            "{downloads:d.tab_referrer_url}, "
            "{downloads:d.tab_url}, "
            "d.target_path, "
            "d.total_bytes, "
            "{downloads:d.transient}, "
            "c.url "
            "FROM downloads d "
            "LEFT JOIN downloads_url_chains c ON d.id = c.id"
        );

        // Retrieve records from downloads table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            download obj;

            obj.idx = idx++;
            obj.by_ext_id = stmt.get_column_string (0);
            obj.by_ext_name = stmt.get_column_string (1);
            obj.by_web_app_id = stmt.get_column_string (2);
            obj.current_path = stmt.get_column_string (3);
            obj.danger_type = stmt.get_column_int64 (4);
            obj.embedder_download_data = stmt.get_column_bytearray (5).dump ();
            obj.end_time = get_datetime (stmt.get_column_int64 (6));
            obj.etag = stmt.get_column_string (7);
            obj.guid = stmt.get_column_string (8);
            obj.hash = stmt.get_column_bytearray (9).to_hexstring ();
            obj.http_method = stmt.get_column_string (10);
            obj.id = stmt.get_column_int64 (11);
            obj.interrupt_reason = stmt.get_column_int64 (12);
            obj.last_access_time = get_datetime (stmt.get_column_int64 (13));
            obj.last_modified = stmt.get_column_string (14);
            obj.mime_type = stmt.get_column_string (15);
            obj.opened = stmt.get_column_int64 (16);
            obj.original_mime_type = stmt.get_column_string (17);
            obj.received_bytes = stmt.get_column_int64 (18);
            obj.referrer = stmt.get_column_string (19);
            obj.site_url = stmt.get_column_string (20);
            obj.start_time = get_datetime (stmt.get_column_int64 (21));
            obj.state = stmt.get_column_int64 (22);
            obj.tab_referrer_url = stmt.get_column_string (23);
            obj.tab_url = stmt.get_column_string (24);
            obj.target_path = stmt.get_column_string (25);
            obj.total_bytes = stmt.get_column_int64 (26);
            obj.transient = stmt.get_column_int64 (27);

            // Add downloads to the list
            downloads_.emplace_back (std::move (obj));
        }

        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load history
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_history::_load_history (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare SQL statement
        auto stmt = db.new_statement_with_pattern (
            "SELECT {urls:u.activity_time}, "
            "{urls:u.display_count}, "
            "{urls:u.display_time}, "
            "{urls:u.emdd_main}, "
            "{urls:u.emdd_main_ver}, "
            "{urls:u.favicon_id}, "
            "u.hidden, "
            "u.id, "
            "{urls:u.last_display}, "
            "u.last_visit_time, "
            "{urls:u.links_clicked_count}, "
            "{urls:u.open_time}, "
            "u.title, "
            "u.typed_count, "
            "u.url, "
            "u.visit_count, "
            "{visits:v.app_id}, "
            "{visits:v.consider_for_ntp_most_visited}, "
            "{visits:v.external_referrer_url}, "
            "v.from_visit, "
            "v.id, "
            "{visits:v.incremented_omnibox_typed_score}, "
            "{visits:v.is_indexed}, "
            "{visits:v.is_known_to_sync}, "
            "{visits:v.opener_visit}, "
            "{visits:v.originator_cache_guid}, "
            "{visits:v.originator_from_visit}, "
            "{visits:v.originator_opener_visit}, "
            "{visits:v.originator_visit_id}, "
            "{visits:v.publicly_routable}, "
            "v.segment_id, "
            "v.transition, "
            "{visits:v.visit_duration}, "
            "v.visit_time, "
            "{visits:v.visited_link_id} "
            "FROM urls u, visits v "
            "WHERE v.url = u.id "
            "ORDER BY v.visit_time"
        );

        // Retrieve rows from query
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            history_entry obj;

            obj.idx = idx++;
            obj.activity_time = stmt.get_column_int64 (0);
            obj.display_count = stmt.get_column_int64 (1);
            obj.display_time = stmt.get_column_int64 (2);
            obj.emdd_main = stmt.get_column_string (3);
            obj.emdd_main_ver = stmt.get_column_string (4);
            obj.favicon_id = stmt.get_column_int64 (5);
            obj.hidden = stmt.get_column_bool (6);
            obj.id = stmt.get_column_int64 (7);
            obj.last_display = get_datetime (stmt.get_column_int64 (8));
            obj.last_visit_time = get_datetime (stmt.get_column_int64 (9));
            obj.links_clicked_count = stmt.get_column_int64 (10);
            obj.open_time = stmt.get_column_int64 (11);
            obj.title = stmt.get_column_string (12);
            obj.typed_count = stmt.get_column_int64 (13);
            obj.url = stmt.get_column_string (14);
            obj.visit_count = stmt.get_column_int64 (15);
            obj.app_id = stmt.get_column_string (16);
            obj.consider_for_ntp_most_visited = stmt.get_column_string (17);
            obj.external_referrer_url = stmt.get_column_string (18);
            obj.from_visit = stmt.get_column_int64 (19);
            obj.visit_id = stmt.get_column_int64 (20);
            obj.incremented_omnibox_typed_score = stmt.get_column_string (21);
            obj.is_indexed = stmt.get_column_string (22) == "1";
            obj.is_known_to_sync = stmt.get_column_string (23) == "1";
            obj.opener_visit = stmt.get_column_int64 (24);
            obj.originator_cache_guid = stmt.get_column_string (25);
            obj.originator_from_visit = stmt.get_column_int64 (26);
            obj.originator_opener_visit = stmt.get_column_int64 (27);
            obj.originator_visit_id = stmt.get_column_int64 (28);
            obj.publicly_routable = stmt.get_column_string (29);
            obj.segment_id = stmt.get_column_int64 (30);
            obj.transition = stmt.get_column_int64 (31);
            obj.visit_duration = stmt.get_column_int64 (32);
            obj.visit_time = get_datetime (stmt.get_column_int64 (33));
            obj.visited_link_id = stmt.get_column_int64 (34);

            history_entries_.emplace_back (std::move (obj));
        }

        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::chromium
