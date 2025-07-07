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
//
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

    is_instance_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load history
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_history::_load_history (mobius::core::database::database &db)
{
    // Prepare statement
    mobius::core::database::statement stmt = db.new_statement (
        "SELECT v.id, "
        "u.url, "
        "u.title, "
        "v.visit_time "
        "FROM urls u, visits v "
        "WHERE v.url = u.id "
        "ORDER BY v.visit_time"
    );

    // Retrieve rows from query
    std::uint64_t idx = 0;

    while (stmt.fetch_row ())
    {
        history_entry entry;

        entry.idx = idx++;
        entry.url = stmt.get_column_string (1);
        entry.title = stmt.get_column_string (2);
        entry.visit_time = get_datetime (stmt.get_column_int64 (3));
        entry.visit_id = stmt.get_column_int64 (0);

        history_entries_.emplace_back (std::move (entry));
    }
}

} // namespace mobius::extension::app::chromium
