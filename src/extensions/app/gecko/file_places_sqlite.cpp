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
#include "file_places_sqlite.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
#include <unordered_map>
#include "common.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @see https://developer.mozilla.org/en-US/docs/Mozilla/Tech/Places/Database
// Tested versions: 14.0.1, 45.0.1, 53.0.2, 59.0.2
//
// moz_places: This is the main table of URIs and is managed by the history service
// (see also History service design). Any time a Places component wants to reference
// a URL, whether visited or not, it refers to this table. Each entry has an optional
// reference to the moz_favicon table to identify the favicon of the page. No two
// entries may have the same value in the url column.
//
// moz_historyvisits: One entry in this table is created each time you visit a page.
// It contains the date, referrer, and other information specific to that visit. It
// contains a reference to the moz_places table which contains the URL and other global
// statistics.
//
// moz_bookmarks: This table contains bookmarks, folders, separators and tags, and
// defines the hierarchy. The hierarchy is defined via the parent column, which points
// to the moz_bookmarks record which is the parent. The position column numbers each
// of the peers beneath a given parent starting with 0 and incrementing higher with
// each addition. The fk column provides the id number of the corresponding record
// in moz_places.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace mobius::extension::app::gecko
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_places_sqlite::file_places_sqlite (const mobius::core::io::reader &reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (!reader)
        return;

    try
    {
        // Copy reader content to temporary file
        mobius::core::io::tempfile tfile;
        tfile.copy_from (reader);

        // Load data
        mobius::core::database::database db (tfile.get_path ());
        _load_bookmarks (db);
        _load_visited_urls (db);
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load bookmarks from database
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_places_sqlite::_load_bookmarks (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare SQL statement to retrieve folders
        std::unordered_map<std::int64_t, std::string> folders;
        {
            auto stmt = db.new_statement_with_pattern (
                "SELECT id, title "
                "FROM moz_bookmarks "
                "WHERE type = 2"
            );

            while (stmt.fetch_row ())
            {
                auto id = stmt.get_column_int64 (0);
                auto type = stmt.get_column_string (1);

                folders.emplace (id, type);
            }
        }
        // Prepare SQL statement to retrieve bookmarks
        auto stmt = db.new_statement_with_pattern (
            "SELECT b.dateAdded, "
            "b.fk, "
            "b.folder_type, "
            "{moz_bookmarks:b.guid}, "
            "b.id, "
            "keyword_id, "
            "b.lastModified, "
            "b.parent, "
            "b.position, "
            "{moz_bookmarks:b.syncChangeCounter}, "
            "{moz_bookmarks:b.syncStatus}, "
            "b.title, "
            "b.type, "
            "p.url "
            "FROM moz_bookmarks b, moz_places p "
            "WHERE b.fk = p.id"
        );

        // Retrieve records from moz_bookmarks table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            bookmark obj;

            obj.idx = idx++;
            obj.date_added = get_datetime (stmt.get_column_int64 (0));
            obj.fk = stmt.get_column_int64 (1);
            obj.folder_type = stmt.get_column_string (2);
            obj.guid = stmt.get_column_string (3);
            obj.id = stmt.get_column_int64 (4);
            obj.keyword_id = stmt.get_column_int64 (5);
            obj.last_modified = get_datetime (stmt.get_column_int64 (6));
            obj.parent = stmt.get_column_int64 (7);
            obj.position = stmt.get_column_int64 (8);
            obj.sync_change_counter = stmt.get_column_int64 (9);
            obj.sync_status = stmt.get_column_int64 (10);
            obj.title = stmt.get_column_string (11);
            obj.type = stmt.get_column_int64 (12);
            obj.url = stmt.get_column_string (13);

            // Get parent name
            auto it = folders.find (obj.parent);
            if (it != folders.end ())
                obj.parent_name = it->second;

            // Add bookmark to the list
            bookmarks_.emplace_back (std::move (obj));
        }

        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load visited URLs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_places_sqlite::_load_visited_urls (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare SQL statement to retrieve visited URLs
        auto stmt = db.new_statement_with_pattern (
            "SELECT v.from_visit, "
            "v.id, "
            "v.place_id, "
            "v.session, "
            "{moz_historyvisits:v.source}, "
            "{moz_historyvisits:v.triggering_place_id}, "
            "v.visit_date, "
            "v.visit_type, "
            "{moz_places:p.alt_frecency}, "
            "{moz_places:p.description}, "
            "{moz_places:p.favicon_id}, "
            "{moz_places:p.foreign_count}, "
            "p.frecency, "
            "{moz_places:p.guid}, "
            "p.hidden, "
            "p.id, "
            "p.last_visit_date, "
            "{moz_places:p.origin_id}, "
            "{moz_places:p.preview_image_url}, "
            "{moz_places:p.recalc_alt_frecency}, "
            "{moz_places:p.recalc_frecency}, "
            "p.rev_host, "
            "{moz_places:p.site_name}, "
            "p.title, "
            "p.typed, "
            "p.url, "
            "{moz_places:p.url_hash}, "
            "p.visit_count "
            "FROM moz_historyvisits v, moz_places p "
            "WHERE v.place_id = p.id"
        );

        // Retrieve records from moz_historyvisits table
        while (stmt.fetch_row ())
        {
            visited_url obj;

            obj.from_visit = stmt.get_column_int64 (0);
            obj.visit_id = stmt.get_column_int64 (1);
            obj.place_id = stmt.get_column_int64 (2);
            obj.session = stmt.get_column_int64 (3);
            obj.source = stmt.get_column_int64 (4);
            obj.triggering_place_id = stmt.get_column_int64 (5);
            obj.visit_date = get_datetime (stmt.get_column_int64 (6));
            obj.visit_type = stmt.get_column_int64 (7);
            obj.alt_frecency = stmt.get_column_int64 (8);
            obj.description = stmt.get_column_string (9);
            obj.favicon_id = stmt.get_column_int64 (10);
            obj.foreign_count = stmt.get_column_int64 (11);
            obj.frecency = stmt.get_column_int64 (12);
            obj.guid = stmt.get_column_string (13);
            obj.hidden = stmt.get_column_int64 (14);
            obj.places_id = stmt.get_column_int64 (15);
            obj.last_visit_date = get_datetime (stmt.get_column_int64 (16));
            obj.origin_id = stmt.get_column_int64 (17);
            obj.preview_image_url = stmt.get_column_string (18);
            obj.recalc_alt_frecency = stmt.get_column_int64 (19);
            obj.recalc_frecency = stmt.get_column_int64 (20);
            obj.rev_host = stmt.get_column_string (21);
            obj.site_name = stmt.get_column_string (22);
            obj.title = stmt.get_column_string (23);
            obj.typed = stmt.get_column_int64 (24);
            obj.url = stmt.get_column_string (25);
            obj.url_hash = stmt.get_column_int64 (26);
            obj.visit_count = stmt.get_column_int64 (27);

            // Add visited URL to the list
            visited_urls_.emplace_back (std::move (obj));
        }

        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::gecko