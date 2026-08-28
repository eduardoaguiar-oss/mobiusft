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
#include "file_history_db.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>

namespace mobius::extension::app::safari
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_history_db::file_history_db (const mobius::core::io::reader &reader)
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
        // Load data
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::database::database db (tfile.get_path ());
        _load_visited_urls (db);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit ("sampling_file", std::string ("app.safari.history_db"), tfile.new_reader ());
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load visited URLs data from database
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_history_db::_load_visited_urls (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare SQL statement for table history_items
        auto stmt = db.new_statement (
            "SELECT items.autocomplete_triggers, "
            "items.daily_visit_counts, "
            "items.domain_expansion, "
            "items.id, "
            "items.should_recompute_derived_visit_counts, "
            "items.status_code, "
            "items.url, "
            "items.visit_count, "
            "items.visit_count_score, "
            "items.weekly_visit_counts, "
            "visits.attributes, "
            "visits.generation, "
            "visits.history_item, "
            "visits.http_non_get, "
            "visits.id, "
            "visits.load_successful, "
            "visits.origin, "
            "visits.redirect_destination, "
            "visits.redirect_source, "
            "visits.score, "
            "visits.synthesized, "
            "visits.title, "
            "visits.visit_time "
            "FROM history_items AS items, history_visits AS visits "
            "WHERE items.id = visits.history_item"
        );

        // Retrieve records from history_items table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            visited_url obj;

            obj.idx = ++idx;
            obj.autocomplete_triggers = stmt.get_column_bytearray (0);
            obj.daily_visit_counts = stmt.get_column_bytearray (1);
            obj.domain_expansion = stmt.get_column_string (2);
            obj.item_id = stmt.get_column_int64 (3);
            obj.should_recompute_derived_visit_counts = stmt.get_column_int64 (4);
            obj.status_code = stmt.get_column_int64 (5);
            obj.url = stmt.get_column_string (6);
            obj.visit_count = stmt.get_column_int64 (7);
            obj.visit_count_score = stmt.get_column_int64 (8);
            obj.weekly_visit_counts = stmt.get_column_bytearray (9);
            obj.attributes = stmt.get_column_int64 (10);
            obj.generation = stmt.get_column_int64 (11);
            obj.history_item = stmt.get_column_int64 (12);
            obj.http_non_get = stmt.get_column_string (13);
            obj.visit_id = stmt.get_column_int64 (14);
            obj.load_successful = stmt.get_column_bool (15);
            obj.origin = stmt.get_column_int64 (16);
            obj.redirect_destination = stmt.get_column_int64 (17);
            obj.redirect_source = stmt.get_column_int64 (18);
            obj.score = stmt.get_column_int64 (19);
            obj.synthesized = stmt.get_column_string (20);
            obj.title = stmt.get_column_string (21);
            obj.visit_time = mobius::core::datetime::new_datetime_from_cocoa_timestamp (stmt.get_column_int64 (22));

            // Add history_items to the list
            visited_urls_.emplace_back (std::move (obj));
        }

        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::safari
