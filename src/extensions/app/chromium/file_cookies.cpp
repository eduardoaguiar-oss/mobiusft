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
#include "file_cookies.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/string_functions.hpp>
#include <unordered_set>
#include "common.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Cookies file tables
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// - cookies
//      - browser_provenance: 12-16, 18, 21, 23-24
//      - creation_utc: 4-5, 7-19, 21, 23-24
//      - encrypted_value: 7-19, 21, 23-24
//      - expires_utc: 4-5, 7-19, 21, 23-24
//      - firstpartyonly: 8-10
//      - has_cross_site_ancestor: 23-24
//      - has_expires: 5, 7-19, 21, 23-24
//      - host_key: 4-5, 7-19, 21, 23-24
//      - httponly: 4-5, 7-9
//      - is_edgelegacycookie: 12-16, 18, 21, 23-24
//      - is_httponly: 10-19, 21, 23-24
//      - is_persistent: 10-19, 21, 23-24
//      - is_same_party: 13-19
//      - is_secure: 10-19, 21, 23-24
//      - last_access_utc: 4-5, 7-19, 21, 23-24
//      - last_update_utc: 18-19, 21, 23-24
//      - name: 4-5, 7-19, 21, 23-24
//      - path: 4-5, 7-19, 21, 23-24
//      - persistent: 5, 7-9
//      - priority: 7-19, 21, 23-24
//      - samesite: 11-19, 21, 23-24
//      - secure: 4-5, 7-9
//      - source_port: 13-19, 21, 23-24
//      - source_scheme: 12-19, 21, 23-24
//      - source_type: 23-24
//      - top_frame_site_key: 15-19, 21, 23-24
//      - value: 4-5, 7-19, 21, 23-24
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
    1, 2, 3, 6, 20, 22,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Last known schema version
// This constant represents the last schema version that is known and handled
// by the current implementation. Any schema version greater than this value
// will be considered unsupported and will trigger a warning in the log.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::int64_t LAST_KNOWN_SCHEMA_VERSION = 24;

} // namespace

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_cookies::file_cookies (const mobius::core::io::reader &reader)
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
        _load_cookies (db);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Finish decoding
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        is_instance_ = true;

        mobius::core::emit (
            "file_for_sampling",
            "app.chromium.cookies." +
                mobius::core::string::to_string (schema_version_, 5),
            reader
        );
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
file_cookies::_load_cookies (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare statement
        mobius::core::database::statement stmt;

        stmt = db.new_statement (generate_sql (
            "SELECT creation_utc, "
            "${encrypted_value,7}, "
            "expires_utc, "
            "${has_cross_site_ancestor,23}, "
            "${has_expires,5}, "
            "host_key, "
            "${httponly,4,9}, "
            "${is_httponly,10}, "
            "${is_persistent,10}, "
            "${is_same_party,13,19}, "
            "${is_secure,10}, "
            "last_access_utc, "
            "${last_update_utc,18}, "
            "name, "
            "path, "
            "${persistent,5,9}, "
            "${priority,7}, "
            "${samesite,11}, "
            "${secure,4,9}, "
            "${source_port,13}, "
            "${source_scheme,12}, "
            "${source_type,23}, "
            "${top_frame_site_key,15}, "
            "${value,4} "
            "FROM cookies ",
            schema_version_
        ));

        // Retrieve rows from query
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            cookie c;

            // Set attributes
            c.idx = idx++;
            c.schema_version = schema_version_;
            c.creation_utc = get_datetime (stmt.get_column_int64 (0));
            c.encrypted_value = stmt.get_column_bytearray (1);
            c.expires_utc = get_datetime (stmt.get_column_int64 (2));
            c.has_cross_site_ancestor = stmt.get_column_bool (3);
            c.has_expires = stmt.get_column_bool (4);
            c.host_key =
                mobius::core::string::lstrip (stmt.get_column_string (5), ".");
            c.httponly = stmt.get_column_bool (6);
            c.is_httponly = stmt.get_column_bool (7);
            c.is_persistent = stmt.get_column_bool (8);
            c.is_same_party = stmt.get_column_bool (9);
            c.is_secure = stmt.get_column_bool (10);
            c.last_access_utc = get_datetime (stmt.get_column_int64 (11));
            c.last_update_utc = get_datetime (stmt.get_column_int64 (12));
            c.name = stmt.get_column_string (13);
            c.path = stmt.get_column_string (14);
            c.persistent = stmt.get_column_bool (15);
            c.priority = stmt.get_column_int (16);
            c.samesite = stmt.get_column_bool (17);
            c.secure = stmt.get_column_bool (18);
            c.source_port = stmt.get_column_int (19);
            c.source_scheme = stmt.get_column_string (20);
            c.source_type = stmt.get_column_int (21);
            c.top_frame_site_key = stmt.get_column_string (22);
            c.value = stmt.get_column_bytearray (23);

            // Set last_update_utc if not set
            if (!c.last_update_utc && c.creation_utc == c.last_access_utc)
                c.last_update_utc = c.creation_utc;

            // Add to cookies vector
            cookies_.emplace_back (std::move (c));
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::chromium
