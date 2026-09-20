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
#include "file_cookies.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
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
//      - creation_utc: 4-19, 21, 23-24
//      - encrypted_value: 7-19, 21, 23-24
//      - expires_utc: 4-19, 21, 23-24
//      - firstpartyonly: 8-10
//      - has_cross_site_ancestor: 23-24
//      - has_expires: 5-19, 21, 23-24
//      - host_key: 4-19, 21, 23-24
//      - httponly: 4-9
//      - is_httponly: 10-19, 21, 23-24
//      - is_persistent: 10-19, 21, 23-24
//      - is_same_party: 13-19
//      - is_secure: 10-19, 21, 23-24
//      - last_access_utc: 4-19, 21, 23-24
//      - last_update_utc: 18-19, 21, 23-24
//      - name: 4-19, 21, 23-24
//      - path: 4-19, 21, 23-24
//      - persistent: 5-9
//      - priority: 6-19, 21, 23-24
//      - samesite: 11-19, 21, 23-24
//      - secure: 4-9
//      - source_port: 13-19, 21, 23-24
//      - source_scheme: 12-19, 21, 23-24
//      - source_type: 23-24
//      - top_frame_site_key: 15-19, 21, 23-24
//      - value: 4-19, 21, 23-24
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
    1,
    2,
    3,
    20,
    22,
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
            UNKNOWN_SCHEMA_VERSIONS.find (schema_version_) != UNKNOWN_SCHEMA_VERSIONS.end ())
            log.development (__LINE__, "Unhandled schema version: " + std::to_string (schema_version_));

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load data
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        _load_cookies (db);

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
// @brief Load cookies
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_cookies::_load_cookies (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare SQL statement for table cookies
        auto stmt = db.new_statement_with_pattern (
           "SELECT {cookies.browser_provenance}, "
                  "creation_utc, "
                  "{cookies.encrypted_value}, "
                  "expires_utc, "
                  "{cookies.firstpartyonly}, "
                  "{cookies.has_cross_site_ancestor}, "
                  "{cookies.has_expires}, "
                  "host_key, "
                  "{cookies.httponly}, "
                  "{cookies.is_edgelegacycookie}, "
                  "{cookies.is_httponly}, "
                  "{cookies.is_persistent}, "
                  "{cookies.is_same_party}, "
                  "{cookies.is_secure}, "
                  "last_access_utc, "
                  "{cookies.last_update_utc}, "
                  "name, "
                  "path, "
                  "{cookies.persistent}, "
                  "{cookies.priority}, "
                  "{cookies.samesite}, "
                  "{cookies.secure}, "
                  "{cookies.source_port}, "
                  "{cookies.source_scheme}, "
                  "{cookies.source_type}, "
                  "{cookies.top_frame_site_key}, "
                  "value "
             "FROM cookies"
        );

        // Retrieve rows from query
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            cookie c;

            // Set attributes
            c.idx = idx++;
            c.schema_version = schema_version_;
            c.browser_provenance = stmt.get_column_int64 (0);
            c.creation_utc = get_datetime (stmt.get_column_int64 (1));
            c.encrypted_value = stmt.get_column_bytearray (2);
            c.expires_utc = get_datetime (stmt.get_column_int64 (3));
            c.first_party_only = stmt.get_column_bool (4);
            c.has_cross_site_ancestor = stmt.get_column_bool (5);
            c.has_expires = stmt.get_column_bool (6);
            c.host_key = mobius::core::string::lstrip (stmt.get_column_string (7), ".");
            c.is_edge_legacy_cookie = stmt.get_column_bool (9);
            c.is_httponly = stmt.get_column_bool (8) || stmt.get_column_bool (10);
            c.is_persistent = stmt.get_column_bool (11) || stmt.get_column_bool (18);
            c.is_same_party = stmt.get_column_bool (12) || stmt.get_column_bool (20);
            c.is_secure = stmt.get_column_bool (13) || stmt.get_column_bool (21);
            c.last_access_utc = get_datetime (stmt.get_column_int64 (14));
            c.last_update_utc = get_datetime (stmt.get_column_int64 (15));
            c.name = stmt.get_column_string (16);
            c.path = stmt.get_column_string (17);
            c.priority = stmt.get_column_int (19);
            c.source_port = stmt.get_column_int (22);
            c.source_scheme = stmt.get_column_string (23);
            c.source_type = stmt.get_column_int (24);
            c.top_frame_site_key = stmt.get_column_string (25);
            c.value = stmt.get_column_bytearray (26);

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
