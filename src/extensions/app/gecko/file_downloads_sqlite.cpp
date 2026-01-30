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
#include "file_downloads_sqlite.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
#include "common.hpp"

namespace mobius::extension::app::gecko
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @see http://doxygen.db48x.net/mozilla/html/interfacensIDownloadManager.html
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_downloads_sqlite::file_downloads_sqlite (
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
file_downloads_sqlite::_load_downloads (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare SQL statement for table moz_downloads
        auto stmt = db.new_statement_with_pattern (
            "SELECT {moz_downloads.autoResume}, "
            "{moz_downloads.currBytes}, "
            "{moz_downloads.endTime}, "
            "{moz_downloads.entityID}, "
            "{moz_downloads.guid}, "
            "{moz_downloads.id}, "
            "{moz_downloads.maxBytes}, "
            "{moz_downloads.mimeType}, "
            "{moz_downloads.name}, "
            "{moz_downloads.preferredAction}, "
            "{moz_downloads.preferredApplication}, "
            "{moz_downloads.referrer}, "
            "{moz_downloads.source}, "
            "{moz_downloads.startTime}, "
            "{moz_downloads.state}, "
            "{moz_downloads.target}, "
            "{moz_downloads.tempPath} "
            "FROM moz_downloads"
        );

        // Retrieve records from moz_downloads table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            download obj;

            obj.idx = idx++;
            obj.auto_resume = stmt.get_column_int64 (0);
            obj.curr_bytes = stmt.get_column_int64 (1);
            obj.end_time = get_datetime (stmt.get_column_int64 (2));
            obj.entity_id = stmt.get_column_string (3);
            obj.guid = stmt.get_column_string (4);
            obj.id = stmt.get_column_int64 (5);
            obj.max_bytes = stmt.get_column_int64 (6);
            obj.mime_type = stmt.get_column_string (7);
            obj.name = stmt.get_column_string (8);
            obj.preferred_action = stmt.get_column_int64 (9);
            obj.preferred_application = stmt.get_column_string (10);
            obj.referrer = stmt.get_column_string (11);
            obj.source = stmt.get_column_string (12);
            obj.start_time = get_datetime (stmt.get_column_int64 (13));
            obj.state = stmt.get_column_int64 (14);
            obj.target = stmt.get_column_string (15);
            obj.temp_path = stmt.get_column_string (16);

            // Add moz_downloads to the list
            downloads_.emplace_back (std::move (obj));
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::gecko