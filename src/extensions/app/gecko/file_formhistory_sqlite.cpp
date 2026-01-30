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
#include "file_formhistory_sqlite.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
#include "common.hpp"

namespace mobius::extension::app::gecko
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_formhistory_sqlite::file_formhistory_sqlite (
    const mobius::core::io::reader &reader
)
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
        _load_form_history (db);

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
// @brief Load form_history entries from database
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_formhistory_sqlite::_load_form_history (
    mobius::core::database::database &db
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare SQL statement for table moz_formhistory
        auto stmt = db.new_statement_with_pattern (
            "SELECT {moz_formhistory.fieldname}, "
            "{moz_formhistory.firstUsed}, "
            "{moz_formhistory.guid}, "
            "{moz_formhistory.id}, "
            "{moz_formhistory.lastUsed}, "
            "{moz_formhistory.timesUsed}, "
            "{moz_formhistory.value} "
            "FROM moz_formhistory"
        );

        // Retrieve records from moz_formhistory table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            form_history obj;

            obj.idx = idx++;
            obj.fieldname = stmt.get_column_string (0);
            obj.first_used = get_datetime (stmt.get_column_int64 (1));
            obj.guid = stmt.get_column_string (2);
            obj.id = stmt.get_column_int64 (3);
            obj.last_used = get_datetime (stmt.get_column_int64 (4));
            obj.times_used = stmt.get_column_int64 (5);
            obj.value = stmt.get_column_string (6);

            // Add moz_formhistory to the list
            form_history_.emplace_back (std::move (obj));
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::gecko