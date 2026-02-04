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
#include "file_cookies_sqlite.hpp"
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
file_cookies_sqlite::file_cookies_sqlite (
    const mobius::core::io::reader &reader
)
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
        _load_cookies (db);
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load cookies
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_cookies_sqlite::_load_cookies (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare SQL statement for table moz_cookies
        auto stmt = db.new_statement_with_pattern (
            "SELECT {moz_cookies.appId}, "
            "{moz_cookies.baseDomain}, "
            "{moz_cookies.creationTime}, "
            "expiry, "
            "host, "
            "id, "
            "{moz_cookies.inBrowserElement}, "
            "isHttpOnly, "
            "{moz_cookies.isPartitionedAttributeSet}, "
            "isSecure, "
            "lastAccessed, "
            "name, "
            "{moz_cookies.originAttributes}, "
            "path, "
            "{moz_cookies.rawSameSite}, "
            "{moz_cookies.sameSite}, "
            "{moz_cookies.schemeMap}, "
            "value "
            "FROM moz_cookies"
        );

        // Retrieve records from moz_cookies table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            cookie obj;

            obj.idx = idx++;
            obj.app_id = stmt.get_column_int64 (0);
            obj.base_domain = stmt.get_column_string (1);
            obj.creation_time = get_datetime (stmt.get_column_int64 (2));
            obj.expiry = get_datetime (stmt.get_column_int64 (3));
            obj.host = stmt.get_column_string (4);
            obj.id = stmt.get_column_int64 (5);
            obj.in_browser_element = stmt.get_column_int64 (6);
            obj.is_http_only = stmt.get_column_bool (7);
            obj.is_partitioned_attribute_set = stmt.get_column_bool (8);
            obj.is_secure = stmt.get_column_bool (9);
            obj.last_accessed = get_datetime (stmt.get_column_int64 (10));
            obj.name = stmt.get_column_string (11);
            obj.origin_attributes = stmt.get_column_string (12);
            obj.path = stmt.get_column_string (13);
            obj.raw_same_site = stmt.get_column_int64 (14);
            obj.same_site = stmt.get_column_int64 (15);
            obj.scheme_map = stmt.get_column_int64 (16);
            obj.value = stmt.get_column_string (17);

            // Add cookie to the list
            cookies_.emplace_back (std::move (obj));
        }

        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::gecko