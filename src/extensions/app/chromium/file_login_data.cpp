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
//
// - logins: This table contains the login credentials and related information
//       for websites. It includes fields such as action_url, username_element,
//       password_element, and date_created, among others. Each record in this
//       table represents a saved login credential.
//
// - meta: This table contains metadata about the database, including the
//       schema version. It has fields such as key and value, where the key
//       "version" indicates the schema version of the database.
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
    2, 4, 6, 8, 9, 10, 11, 14, 15, 20, 23, 30, 36, 37, 38,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Last known schema version
// This constant represents the last schema version that is known and handled
// by the current implementation. Any schema version greater than this value
// will be considered unsupported and will trigger a warning in the log.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::int64_t LAST_KNOWN_SCHEMA_VERSION = 43;

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
        // Prepare SQL statement for table logins
        auto stmt = db.new_statement_with_pattern (
           "SELECT action_url, "
                  "{logins.actor_login_approved}, "
                  "{logins.avatar_url}, "
                  "blacklisted_by_user, "
                  "date_created, "
                  "{logins.date_last_filled}, "
                  "{logins.date_last_used}, "
                  "{logins.date_password_modified}, "
                  "{logins.date_received}, "
                  "{logins.date_synced}, "
                  "{logins.display_name}, "
                  "{logins.federation_url}, "
                  "{logins.form_data}, "
                  "{logins.generation_upload_status}, "
                  "{logins.icon_url}, "
                  "{logins.id}, "
                  "{logins.is_zero_click}, "
                  "{logins.keychain_identifier}, "
                  "{logins.moving_blocked_for}, "
                  "origin_url, "
                  "password_element, "
                  "{logins.password_type}, "
                  "password_value, "
                  "{logins.possible_username_pairs}, "
                  "{logins.possible_usernames}, "
                  "{logins.preferred}, "
                  "scheme, "
                  "{logins.sender_email}, "
                  "{logins.sender_name}, "
                  "{logins.sender_profile_image_url}, "
                  "{logins.sharing_notification_displayed}, "
                  "signon_realm, "
                  "{logins.skip_zero_click}, "
                  "{logins.ssl_valid}, "
                  "submit_element, "
                  "{logins.times_used}, "
                  "{logins.use_additional_auth}, "
                  "username_element, "
                  "username_value "
             "FROM logins"
        );

        // Retrieve records from logins table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            login obj;

            obj.idx = idx++;
            obj.action_url = stmt.get_column_string (0);
            obj.actor_login_approved = stmt.get_column_int64 (1);
            obj.avatar_url = stmt.get_column_string (2);
            obj.blacklisted_by_user = stmt.get_column_int64 (3);
            obj.date_created = get_datetime (stmt.get_column_int64 (4));
            obj.date_last_filled = get_datetime (stmt.get_column_int64 (5));
            obj.date_last_used = get_datetime (stmt.get_column_int64 (6));
            obj.date_password_modified = get_datetime (stmt.get_column_int64 (7));
            obj.date_received = get_datetime (stmt.get_column_int64 (8));
            obj.date_synced = get_datetime (stmt.get_column_int64 (9));
            obj.display_name = stmt.get_column_string (10);
            obj.federation_url = stmt.get_column_string (11);
            // obj.form_data = stmt.get_column_bytearray (12);
            obj.generation_upload_status = stmt.get_column_int64 (13);
            obj.icon_url = stmt.get_column_string (14);
            obj.id = stmt.get_column_int64 (15);
            obj.is_zero_click = stmt.get_column_int64 (16);
            obj.keychain_identifier = stmt.get_column_bytearray (17);
            // obj.moving_blocked_for = stmt.get_column_bytearray (18);
            obj.origin_url = stmt.get_column_string (19);
            obj.password_element = stmt.get_column_string (20);
            obj.password_type = stmt.get_column_int64 (21);
            obj.password_value = stmt.get_column_bytearray (22);
            // obj.possible_username_pairs = stmt.get_column_bytearray (23);
            // obj.possible_usernames = stmt.get_column_bytearray (24);
            obj.preferred = stmt.get_column_int64 (25);
            obj.scheme = stmt.get_column_int64 (26);
            obj.sender_email = stmt.get_column_string (27);
            obj.sender_name = stmt.get_column_string (28);
            obj.sender_profile_image_url = stmt.get_column_string (29);
            obj.sharing_notification_displayed = stmt.get_column_int64 (30);
            obj.signon_realm = stmt.get_column_string (31);
            obj.skip_zero_click = stmt.get_column_int64 (32);
            obj.ssl_valid = stmt.get_column_int64 (33);
            obj.submit_element = stmt.get_column_string (34);
            obj.times_used = stmt.get_column_int64 (35);
            obj.use_additional_auth = stmt.get_column_int64 (36);
            obj.username_element = stmt.get_column_string (37);
            obj.username_value = stmt.get_column_string (38);

            // Add logins to the list
            logins_.emplace_back (std::move (obj));
        }

        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::chromium
