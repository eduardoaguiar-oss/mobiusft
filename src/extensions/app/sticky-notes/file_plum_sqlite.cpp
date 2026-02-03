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
#include "file_plum_sqlite.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert Plum.sqlite timestamp to datetime
// @param timestamp Plum.sqlite file timestamp
// @return datetime object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
get_datetime (std::int64_t timestamp)
{
    mobius::core::datetime::datetime dt;

    if (timestamp > 0)
        dt = mobius::core::datetime::new_datetime_from_dot_net_timestamp (
            timestamp
        );

    return dt;
}

} // namespace

namespace mobius::extension::app::sticky_notes
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_plum_sqlite::file_plum_sqlite (const mobius::core::io::reader &reader)
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
        _load_notes (db);

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
// @brief Load notes
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_plum_sqlite::_load_notes (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare SQL statement for table Note
        auto stmt = db.new_statement_with_pattern (
            "SELECT {Note.ChangeKey}, "
            "{Note.CreatedAt}, "
            "{Note.CreationNoteIdAnchor}, "
            "{Note.DeletedAt}, "
            "{Note.Id}, "
            "{Note.IsAlwaysOnTop}, "
            "{Note.IsFutureNote}, "
            "{Note.IsOpen}, "
            "{Note.IsRemoteDataInvalid}, "
            "{Note.LastServerVersion}, "
            "{Note.ParentId}, "
            "{Note.PendingInsightsScan}, "
            "{Note.RemoteId}, "
            "{Note.RemoteSchemaVersion}, "
            "{Note.Text}, "
            "{Note.Theme}, "
            "{Note.Type}, "
            "{Note.UpdatedAt}, "
            "{Note.WindowPosition} "
            "FROM Note"
        );

        // Retrieve records from Note table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            note obj;

            obj.idx = idx++;
            obj.change_key = stmt.get_column_string (0);
            obj.created_at = get_datetime (stmt.get_column_int64 (1));
            obj.creation_note_id_anchor = stmt.get_column_string (2);
            obj.deleted_at = get_datetime (stmt.get_column_int64 (3));
            obj.id = stmt.get_column_string (4);
            obj.is_always_on_top = stmt.get_column_int64 (5);
            obj.is_future_note = stmt.get_column_int64 (6);
            obj.is_open = stmt.get_column_int64 (7);
            obj.is_remote_data_invalid = stmt.get_column_int64 (8);
            obj.last_server_version = stmt.get_column_string (9);
            obj.parent_id = stmt.get_column_string (10);
            obj.pending_insights_scan = stmt.get_column_int64 (11);
            obj.remote_id = stmt.get_column_string (12);
            obj.remote_schema_version = stmt.get_column_int64 (13);
            obj.text = stmt.get_column_string (14);
            obj.theme = stmt.get_column_string (15);
            obj.type = stmt.get_column_string (16);
            obj.updated_at = get_datetime (stmt.get_column_int64 (17));
            obj.window_position = stmt.get_column_string (18);

            // Add note to the list
            notes_.emplace_back (std::move (obj));
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::sticky_notes