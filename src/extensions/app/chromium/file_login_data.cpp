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
#include "file_login_data.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <unordered_map>
#include <unordered_set>
#include "common.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
// @see https://atropos4n6.com/windows/chrome-login-data-forensics/
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Login Data file tables
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// - logins
//      - action_url: 1, 3, 5, 7, 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - avatar_url: 7, 12-13
//      - blacklisted_by_user: 1, 3, 5, 7, 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - date_created: 1, 3, 5, 7, 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - date_last_filled: 42
//      - date_last_used: 25-29, 31-35, 40-42
//      - date_password_modified: 31-35, 40-42
//      - date_received: 40-42
//      - date_synced: 7, 12-13, 16-19, 21-22, 24-29
//      - display_name: 7, 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - federation_url: 7, 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - form_data: 5, 7, 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - generation_upload_status: 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - icon_url: 16-19, 21-22, 24-29, 31-35, 40-42
//      - id: 21-22, 24-29, 31-35, 40-42
//      - is_zero_click: 7
//      - keychain_identifier: 40-42
//      - moving_blocked_for: 27-29, 31-35, 40-42
//      - origin_url: 1, 3, 5, 7, 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - password_element: 1, 3, 5, 7, 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - password_type: 3, 5, 7, 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - password_value: 1, 3, 5, 7, 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - possible_username_pairs: 19, 21-22, 24-29, 31-35, 40-42
//      - possible_usernames: 3, 5, 7, 12-13, 16-18
//      - preferred: 1, 3, 5, 7, 12-13, 16-19, 21-22, 24-27
//      - scheme: 1, 3, 5, 7, 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - sender_email: 40-42
//      - sender_name: 40-42
//      - sender_profile_image_url: 41-42
//      - sharing_notification_displayed: 40-42
//      - signon_realm: 1, 3, 5, 7, 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - skip_zero_click: 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - ssl_valid: 1, 3, 5, 7, 12-13, 16-17
//      - submit_element: 1, 3, 5, 7, 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - times_used: 3, 5, 7, 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - use_additional_auth: 5, 7
//      - username_element: 1, 3, 5, 7, 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
//      - username_value: 1, 3, 5, 7, 12-13, 16-19, 21-22, 24-29, 31-35, 40-42
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
    2, 4, 6, 8, 9, 10, 11, 14, 15, 20, 23, 30, 36, 37, 38, 39,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Last known schema version
// This constant represents the last schema version that is known and handled
// by the current implementation. Any schema version greater than this value
// will be considered unsupported and will trigger a warning in the log.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::int64_t LAST_KNOWN_SCHEMA_VERSION = 42;

} // namespace

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_login_data::file_login_data (const mobius::core::io::reader &reader)
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
        _load_logins (db);

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
// @brief Load logins
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_login_data::_load_logins (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare statement
        mobius::core::database::statement stmt;

        stmt = db.new_statement (
            "SELECT action_url, "
            "${avatar_url:7-13}, "
            "blacklisted_by_user, "
            "date_created, "
            "${date_last_filled:42}, "
            "${date_last_used:25}, "
            "${date_password_modified:31}, "
            "${date_received:40}, "
            "${date_synced:7-29}, "
            "${display_name:7}, "
            "${federation_url:7}, "
            "${generation_upload_status:12}, "
            "${icon_url:16}, "
            "${id:21}, "
            "${is_zero_click:7-7}, "
            "${keychain_identifier:40}, "
            "origin_url, "
            "password_element, "
            "${password_type:3}, "
            "password_value, "
            "${preferred:1-27}, "
            "scheme, "
            "${sender_email:40}, "
            "${sender_name:40}, "
            "${sender_profile_image_url:41}, "
            "${sharing_notification_displayed:40}, "
            "signon_realm, "
            "${skip_zero_click:12}, "
            "${ssl_valid:1-17}, "
            "submit_element, "
            "${times_used:3}, "
            "${use_additional_auth:5-7}, "
            "username_element, "
            "username_value "
            "FROM logins",
            schema_version_
        );

        // Retrieve rows from query
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            login l;

            // Set attributes
            l.idx = idx++;
            l.schema_version = schema_version_;
            l.action_url = stmt.get_column_string (0);
            l.avatar_url = stmt.get_column_string (1);
            l.blacklisted_by_user = stmt.get_column_bool (2);
            l.date_created = get_datetime (stmt.get_column_int64 (3));
            l.date_last_filled = get_datetime (stmt.get_column_int64 (4));
            l.date_last_used = get_datetime (stmt.get_column_int64 (5));
            l.date_password_modified = get_datetime (stmt.get_column_int64 (6));
            l.date_received = get_datetime (stmt.get_column_int64 (7));
            l.date_synced = get_datetime (stmt.get_column_int64 (8));
            l.display_name = stmt.get_column_string (9);
            l.federation_url = stmt.get_column_string (10);
            l.generation_upload_status = stmt.get_column_int64 (11);
            l.icon_url = stmt.get_column_string (12);
            l.id = stmt.get_column_string (13);
            l.is_zero_click = stmt.get_column_bool (14);
            l.keychain_identifier = stmt.get_column_string (15);
            l.origin_url = stmt.get_column_string (16);
            l.password_element = stmt.get_column_string (17);
            l.password_type = stmt.get_column_int64 (18);
            l.password_value = stmt.get_column_bytearray (19);
            l.preferred = stmt.get_column_bool (20);
            l.scheme = stmt.get_column_string (21);
            l.sender_email = stmt.get_column_string (22);
            l.sender_name = stmt.get_column_string (23);
            l.sender_profile_image_url = stmt.get_column_string (24);
            l.sharing_notification_displayed = stmt.get_column_bool (25);
            l.signon_realm = stmt.get_column_string (26);
            l.skip_zero_click = stmt.get_column_bool (27);
            l.ssl_valid = stmt.get_column_bool (28);
            l.submit_element = stmt.get_column_string (29);
            l.times_used = stmt.get_column_int64 (30);
            l.use_additional_auth = stmt.get_column_bool (31);
            l.username_element = stmt.get_column_string (32);
            l.username_value = stmt.get_column_string (33);

            // Add to logins vector
            logins_.emplace_back (std::move (l));
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::chromium
