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
#include "file_history.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
#include <unordered_map>
#include <unordered_set>
#include "common.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// History file tables
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// - downloads: Downloads
//      - by_ext_id: 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63,
//      65-69
//      - by_ext_name: 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63,
//      65-69
//      - by_web_app_id: 65-69
//      - current_path: 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63,
//      65-69
//      - danger_type: 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63,
//      65-69
//      - embedder_download_data: 53, 55-56, 58-59, 61-63, 65-69
//      - end_time: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59,
//      61-63, 65-69
//      - etag: 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-69
//      - full_path: 20, 22-23
//      - guid: 30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-69
//      - hash: 30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-69
//      - http_method: 30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63,
//      65-69
//      - id: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59,
//      61-63, 65-69
//      - interrupt_reason: 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59,
//      61-63, 65-69
//      - last_access_time: 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-69
//      - last_modified: 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59,
//      61-63, 65-69
//      - mime_type: 29-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63,
//      65-69
//      - opened: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59,
//      61-63, 65-69
//      - original_mime_type: 29-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59,
//      61-63, 65-69
//      - received_bytes: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56,
//      58-59, 61-63, 65-69
//      - referrer: 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63,
//      65-69
//      - site_url: 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-69
//      - start_time: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56,
//      58-59, 61-63, 65-69
//      - state: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59,
//      61-63, 65-69
//      - tab_referrer_url: 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63,
//      65-69
//      - tab_url: 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-69
//      - target_path: 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63,
//      65-69
//      - total_bytes: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56,
//      58-59, 61-63, 65-69
//      - transient: 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-69
//      - url: 20, 22-23
//
// - downloads_url_chains: Download URL chains
//      - chain_index: 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63,
//      65-69
//      - id: 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-69
//      - url: 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-69
//
// - keyword_search_terms: Keyword search terms
//      - keyword_id: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56,
//      58-59, 61-63, 65-69
//      - lower_term: 20, 22-23, 28-30, 32-33, 36-42
//      - normalized_term: 42-45, 48, 50-51, 53, 55-56, 58-59, 61-63, 65-69
//      - term: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59,
//      61-63, 65-69
//      - url_id: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59,
//      61-63, 65-69
//
// - urls: URLs
//      - activity_time: 42-43, 45, 48, 51, 62, 67
//      - display_count: 42-43, 45, 48, 51, 62, 67
//      - display_time: 42-43, 45, 48, 51, 62, 67
//      - favicon_id: 20, 22-23, 28-30, 32-33
//      - hidden: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59,
//      61-63, 65-69
//      - id: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59,
//      61-63, 65-69
//      - last_display: 42-43, 45, 48, 51, 62, 67
//      - last_visit_time: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56,
//      58-59, 61-63, 65-69
//      - links_clicked_count: 42-43, 45, 48, 51, 62, 67
//      - open_time: 42-43, 45, 48, 51, 62, 67
//      - title: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59,
//      61-63, 65-69
//      - typed_count: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56,
//      58-59, 61-63, 65-69
//      - url: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59,
//      61-63, 65-69
//      - visit_count: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56,
//      58-59, 61-63, 65-69
//
// - visited_links: Visited links
//      - frame_url: 67-69
//      - id: 67-69
//      - link_url_id: 67-69
//      - top_level_url: 67-69
//      - visit_count: 67-69
//
// - visits: Visits
//      - app_id: 69
//      - consider_for_ntp_most_visited: 63, 65-69
//      - external_referrer_url: 66-69
//      - from_visit: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56,
//      58-59, 61-63, 65-69
//      - id: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59,
//      61-63, 65-69
//      - incremented_omnibox_typed_score: 40-45, 48, 50-51, 53, 55-56, 58-59,
//      61-63, 65-69
//      - is_indexed: 20, 22-23, 29, 32, 39, 41
//      - is_known_to_sync: 59, 61-63, 65-69
//      - opener_visit: 50-51, 53, 55-56, 58-59, 61-63, 65-69
//      - originator_cache_guid: 55-56, 58-59, 61-63, 65-69
//      - originator_from_visit: 56, 58-59, 61-63, 65-69
//      - originator_opener_visit: 56, 58-59, 61-63, 65-69
//      - originator_visit_id: 55-56, 58-59, 61-63, 65-69
//      - publicly_routable: 43-45, 48
//      - segment_id: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56,
//      58-59, 61-63, 65-69
//      - transition: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56,
//      58-59, 61-63, 65-69
//      - url: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56, 58-59,
//      61-63, 65-69
//      - visit_duration: 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56,
//      58-59, 61-63, 65-69
//      - visit_time: 20, 22-23, 28-30, 32-33, 36-45, 48, 50-51, 53, 55-56,
//      58-59, 61-63, 65-69
//      - visited_link_id: 67-69
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
static constexpr std::int64_t LAST_KNOWN_SCHEMA_VERSION = 69;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Download states
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::unordered_map<std::int32_t, std::string>
    DOWNLOAD_STATE_STRINGS = {
        {-1, "invalid"},
        {0, "in_progress"},
        {1, "complete"},
        {2, "cancelled"},
        {3, "bug_140687"},
        {4, "interrupted"},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Download danger types
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::unordered_map<std::int32_t, std::string>
    DOWNLOAD_DANGER_TYPE_STRINGS = {
        {-1, "invalid"},
        {0, "not_dangerous"},
        {1, "dangerous_file"},
        {2, "dangerous_url"},
        {3, "dangerous_content"},
        {4, "maybe_dangerous_content"},
        {5, "uncommon_content"},
        {6, "user_validated"},
        {7, "dangerous_host"},
        {8, "potentially_unwanted"},
        {9, "allowlisted_by_policy"},
        {10, "async_scanning"},
        {11, "blocked_password_protected"},
        {12, "blocked_too_large"},
        {13, "sensitive_content_warning"},
        {14, "sensitive_content_block"},
        {15, "deep_scanned_safe"},
        {16, "deep_scanned_opened_dangerous"},
        {17, "prompt_for_scanning"},
        {18, "blocked_unsupported_filetype"},
        {19, "dangerous_account_compromise"},
        {20, "deep_scanned_failed"},
};

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

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Copy reader content to temporary file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::io::tempfile tfile;
    tfile.copy_from (reader);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get schema version
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::database::database db (tfile.get_path ());

    auto stmt = db.new_statement (
        "SELECT value "
        "FROM meta "
        "WHERE key = 'version'"
    );

    if (stmt.fetch_row ())
        schema_version_ = stmt.get_column_int64 (0);

    else
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

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Load data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    _load_history (db);
    _load_downloads (db);

    is_instance_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load downloads
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_history::_load_downloads (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Prepare statement
    mobius::core::database::statement stmt;

    if (schema_version_ < 24)
        stmt = db.new_statement (
            "SELECT "
            "NULL AS by_ext_id, "
            "NULL AS by_ext_name, "
            "NULL AS by_web_app_id, "
            "NULL AS current_path, "
            "NULL AS danger_type, "
            "NULL AS embedder_download_data, "
            "end_time, "
            "NULL AS etag, "
            "full_path, "
            "NULL AS guid, "
            "NULL AS hash, "
            "NULL AS http_method, "
            "id, "
            "NULL AS interrupt_reason, "
            "NULL AS last_access_time, "
            "NULL AS last_modified, "
            "NULL AS mime_type, "
            "opened, "
            "NULL AS original_mime_type, "
            "received_bytes, "
            "NULL AS referrer, "
            "NULL AS site_url, "
            "start_time, "
            "state, "
            "NULL AS tab_referrer_url, "
            "NULL AS tab_url, "
            "NULL AS target_path, "
            "total_bytes, "
            "NULL AS transient, "
            "url "
            "FROM downloads"
        );

    else
        stmt = db.new_statement (generate_sql (
            "SELECT "
            "d.by_ext_id, "
            "d.by_ext_name, "
            "${d.by_web_app_id,65}, "
            "d.current_path, "
            "d.danger_type, "
            "${d.embedder_download_data,53}, "
            "d.end_time, "
            "d.etag, "
            "NULL AS full_path, "
            "${d.guid,30}, "
            "${d.hash,30}, "
            "${d.http_method,30}, "
            "d.id, "
            "d.interrupt_reason, "
            "${d.last_access_time,36}, "
            "d.last_modified, "
            "${d.mime_type,29}, "
            "d.opened, "
            "${d.original_mime_type,29}, "
            "d.received_bytes, "
            "d.referrer, "
            "${d.site_url,32}, "
            "d.start_time, "
            "d.state, "
            "${d.tab_referrer_url,32}, "
            "${d.tab_url,32}, "
            "d.target_path, "
            "d.total_bytes, "
            "${d.transient,36}, "
            "c.url "
            "FROM downloads d "
            "LEFT JOIN downloads_url_chains c ON d.id = c.id",
            schema_version_
        ));

    // Retrieve rows from query
    std::uint64_t idx = 0;

    while (stmt.fetch_row ())
    {
        download entry;

        entry.idx = idx++;
        entry.by_ext_id = stmt.get_column_int64 (0);
        entry.by_ext_name = stmt.get_column_string (1);
        entry.by_web_app_id = stmt.get_column_int64 (2);
        entry.current_path = stmt.get_column_string (3);

        auto embedder_download_data = stmt.get_column_bytearray (5);
        entry.embedder_download_data = embedder_download_data.dump ();

        entry.end_time = get_datetime (stmt.get_column_int64 (6));
        entry.etag = stmt.get_column_string (7);
        entry.full_path = stmt.get_column_string (8);
        entry.guid = stmt.get_column_string (9);

        entry.http_method = stmt.get_column_string (11);
        entry.id = stmt.get_column_int64 (12);
        entry.interrupt_reason = stmt.get_column_int64 (13);
        entry.last_access_time = get_datetime (stmt.get_column_int64 (14));
        entry.last_modified = stmt.get_column_string (15);
        entry.mime_type = stmt.get_column_string (16);
        entry.opened = stmt.get_column_bool (17);
        entry.original_mime_type = stmt.get_column_string (18);
        entry.received_bytes = stmt.get_column_int64 (19);
        entry.referrer = stmt.get_column_string (20);
        entry.site_url = stmt.get_column_string (21);
        entry.start_time = get_datetime (stmt.get_column_int64 (22));

        entry.tab_referrer_url = stmt.get_column_string (24);
        entry.tab_url = stmt.get_column_string (25);
        entry.target_path = stmt.get_column_string (26);
        entry.total_bytes = stmt.get_column_int64 (27);
        entry.transient = stmt.get_column_bool (28);
        entry.url = stmt.get_column_string (29);

        // Handle danger type
        auto danger_type_iter =
            DOWNLOAD_DANGER_TYPE_STRINGS.find (stmt.get_column_int64 (4));

        if (danger_type_iter != DOWNLOAD_DANGER_TYPE_STRINGS.end ())
            entry.danger_type = danger_type_iter->second;

        // Handle hash
        auto h = stmt.get_column_bytearray (10);
        if (h)
        {
            entry.hash = h.to_hexstring ();
            log.development (__LINE__, "Download hash found: " + entry.hash);
        }

        // Handle download state
        auto state_iter =
            DOWNLOAD_STATE_STRINGS.find (stmt.get_column_int64 (23));
        if (state_iter != DOWNLOAD_STATE_STRINGS.end ())
            entry.state = state_iter->second;

        // Add entry to the list
        downloads_.emplace_back (std::move (entry));
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load history
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_history::_load_history (mobius::core::database::database &db)
{
    // Prepare statement
    mobius::core::database::statement stmt = db.new_statement (generate_sql (
        "SELECT "
        "${u.favicon_id,20,33}, "
        "u.hidden, "
        "u.id, "
        "u.last_visit_time, "
        "u.title, "
        "u.typed_count, "
        "u.url, "
        "u.visit_count, "
        "${v.app_id,69}, "
        "${v.consider_for_ntp_most_visited,63}, "
        "${v.external_referrer_url,66}, "
        "v.from_visit, "
        "v.id, "
        "${v.incremented_omnibox_typed_score,40}, "
        "${v.is_indexed,20,32}, "
        "${v.is_known_to_sync,59}, "
        "${v.opener_visit,50}, "
        "${v.originator_cache_guid,55}, "
        "${v.originator_from_visit,56}, "
        "${v.originator_opener_visit,56}, "
        "${v.originator_visit_id,55}, "
        "${v.publicly_routable,43,48}, "
        "v.segment_id, "
        "v.transition, "
        "v.url, "
        "v.visit_duration, "
        "v.visit_time, "
        "${v.visited_link_id,67} "
        "FROM urls u, visits v "
        "WHERE v.url = u.id "
        "ORDER BY v.visit_time",
        schema_version_
    ));

    // Retrieve rows from query
    std::uint64_t idx = 0;

    while (stmt.fetch_row ())
    {
        history_entry entry;

        entry.idx = idx++;
        entry.schema_version = schema_version_;
        entry.favicon_id = stmt.get_column_int64 (0);
        entry.hidden = stmt.get_column_bool (1);
        entry.id = stmt.get_column_int64 (2);
        entry.last_visit_time = get_datetime (stmt.get_column_int64 (3));
        entry.title = stmt.get_column_string (4);
        entry.typed_count = stmt.get_column_int64 (5);
        entry.url = stmt.get_column_string (6);
        entry.visit_count = stmt.get_column_int64 (7);
        entry.app_id = stmt.get_column_int64 (8);
        entry.consider_for_ntp_most_visited = stmt.get_column_bool (9);
        entry.external_referrer_url = stmt.get_column_string (10);
        entry.from_visit = stmt.get_column_int64 (11);
        entry.visit_id = stmt.get_column_int64 (12);
        entry.incremented_omnibox_typed_score = stmt.get_column_bool (13);
        entry.is_indexed = stmt.get_column_bool (14);
        entry.is_known_to_sync = stmt.get_column_bool (15);
        entry.opener_visit = stmt.get_column_int64 (16);
        entry.originator_cache_guid = stmt.get_column_string (17);
        entry.originator_from_visit = stmt.get_column_int64 (18);
        entry.originator_opener_visit = stmt.get_column_int64 (19);
        entry.originator_visit_id = stmt.get_column_int64 (20);
        entry.publicly_routable = stmt.get_column_bool (21);
        entry.segment_id = stmt.get_column_int64 (22);
        entry.transition = stmt.get_column_int64 (23);
        entry.visit_url = stmt.get_column_int64 (24);
        entry.visit_duration = stmt.get_column_int64 (25);
        entry.visit_time = get_datetime (stmt.get_column_int64 (26));
        entry.visited_link_id = stmt.get_column_int64 (27);

        history_entries_.emplace_back (std::move (entry));
    }
}

} // namespace mobius::extension::app::chromium
