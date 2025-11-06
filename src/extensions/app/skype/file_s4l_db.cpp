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
#include "file_s4l_db.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/decoder/json/parser.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <limits>
#include <unordered_set>
#include "common.hpp"

#include <iostream>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// s4l-*.db file tables
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// - internaldata
//      - nsp_data: 84, 88
//      - nsp_pk: 84, 88
//
// - metadata: DB schema version
//      - name: 84, 88
//      - value: 84, 88
//
// - profilecachev8: Contacts
//      - nsp_data: 84, 88
//      - nsp_pk: 84, 88
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief DEBUG flag
static constexpr bool DEBUG = false;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Last known schema version
static constexpr std::int64_t LAST_KNOWN_SCHEMA_VERSION = 88;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unknown schema versions
// This set contains schema versions that are not recognized or not handled
// by the current implementation.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::unordered_set<std::int64_t> UNKNOWN_SCHEMA_VERSIONS = {
    1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18,
    19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
    55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
    73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 85, 86, 87
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get database schema version
// @param db Database object
// @return Schema version or 0 if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::int64_t
get_db_schema_version (mobius::core::database::database &db)
{
    std::int64_t schema_version = 0;

    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        auto stmt = db.new_statement (
            "SELECT value FROM metadata WHERE name  = 'schemaVersion'"
        );

        if (stmt.fetch_row ())
        {
            schema_version = stmt.get_column_int64 (0);

            if (!schema_version)
            {
                log.warning (
                    __LINE__, "Schema version = 0. Path: " + db.get_path ()
                );
            }
        }
        else
        {
            log.warning (
                __LINE__, "Schema version not found in metadata table. Path: " +
                              db.get_path ()
            );
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + ". Path: " + db.get_path ()
        );
    }

    return schema_version;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert MRI to skype name
// @param mri MRI string
// @return Skype name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
mri_to_skype_name (const std::string &mri)
{
    auto pos = mri.find (':');

    if (pos == std::string::npos)
        return {};

    return mri.substr (pos + 1);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get value from internalData table
// @param db Database object
// @param nsp_pk Key to search for
// @return Value, if any
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T>
static T
get_internaldata_value (
    mobius::core::database::database &db, const std::string &nsp_pk
)
{
    T value;
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        auto stmt = db.new_statement (
            "SELECT nsp_data FROM internaldata WHERE nsp_pk = ?"
        );

        stmt.bind (1, nsp_pk);

        if (stmt.fetch_row ())
        {
            auto parser = mobius::core::decoder::json::parser (
                stmt.get_column_bytearray (0)
            );
            auto nsp_data = mobius::core::pod::map (parser.parse ());
            value = nsp_data.get<T> ("value");
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + ". Path: " + db.get_path ()
        );
    }

    return value;
}

} // namespace

namespace mobius::extension::app::skype
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_s4l_db::file_s4l_db (const mobius::core::io::reader &reader)
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
        _load_contacts (db);
        _load_account (db);

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
// @brief Load account data
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_s4l_db::_load_account (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load Cpriv_prefs_v2 data from internalData table
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto value = get_internaldata_value<mobius::core::pod::map> (
            db, "Cpriv_prefs_v2"
        );

        if (!value)
        {
            log.development (
                __LINE__,
                "Cpriv_prefs_v2 entry not found in internaldata table. Path: " +
                    db.get_path ()
            );
            return;
        }

        acc_.skype_name = value.get<std::string> ("skypeName");
        acc_.primary_member_name = value.get<std::string> ("primaryMemberName");
        acc_.timezone = value.get<std::string> ("timezone");
        acc_.locale = value.get<std::string> ("locale");

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load Cpriv_myuserstore data from internalData table
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        value = get_internaldata_value<mobius::core::pod::map> (
            db, "Cpriv_myuserstore"
        );

        if (value)
        {
            acc_.msaid_from_signin = value.get<std::string> ("msaIdFromSignIn");
            acc_.msa_cid = value.get<std::string> ("msaCid");
            acc_.msa_cid_hex = value.get<std::string> ("msaCidHex");
            acc_.msa_id = value.get<std::string> ("msaId");
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load Cpriv_ecsParamCache data from internalData table
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        value = get_internaldata_value<mobius::core::pod::map> (
            db, "Cpriv_ecsParamCache"
        );

        if (value)
            acc_.app_version = value.get<std::string> ("appVersion");

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load Cpriv_deviceid data from internalData table
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        acc_.device_id =
            get_internaldata_value<std::string> (db, "Cpriv_deviceid");

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Fill account data with contact data
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto iter = contacts_.find (acc_.skype_name);

        if (iter != contacts_.end ())
        {
            const auto &c = iter->second;
            acc_.full_name = c.full_name;
            acc_.birthdate = c.birthdate;
            acc_.country = c.country;
            acc_.province = c.province;
            acc_.city = c.city;
            acc_.mood_text = c.mood_text;
            acc_.thumbnail_url = c.thumbnail_url;
        }
        else
        {
            log.development (
                __LINE__, "Contact data for account skype name '" +
                              acc_.skype_name +
                              "' not found. Path: " + db.get_path ()
            );
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + ". Path: " + db.get_path ()
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load contacts data
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_s4l_db::_load_contacts (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load contacts data from profilecachev8 table
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto stmt = db.new_statement ("SELECT nsp_data FROM profilecachev8");

        while (stmt.fetch_row ())
        {
            auto parser = mobius::core::decoder::json::parser (
                stmt.get_column_bytearray (0)
            );
            auto nsp_data = mobius::core::pod::map (parser.parse ());

            contact c;
            c.mri = nsp_data.get<std::string> ("mri");
            c.skype_name = mri_to_skype_name (c.mri);
            c.birthdate = nsp_data.get<std::string> ("birthday");
            c.gender = nsp_data.get<std::int64_t> ("gender");
            c.country = nsp_data.get<std::string> ("country");
            c.province = nsp_data.get<std::string> ("province");
            c.city = nsp_data.get<std::string> ("city");
            c.mood_text = nsp_data.get<std::string> ("mood");
            c.thumbnail_url = nsp_data.get<std::string> ("thumbUrl");
            c.fetched_time =
                get_datetime (nsp_data.get<std::int64_t> ("fetchedDate"));

            c.full_name = nsp_data.get<std::string> ("fullName");
            if (c.full_name.empty ())
                c.full_name = nsp_data.get<std::string> ("displayNameOverride");

            auto emails = nsp_data.get ("emails");
            if (emails.is_list ())
                c.emails = emails.to_list<std::string> ();

            auto phone_numbers = nsp_data.get ("phones");
            if (phone_numbers.is_list ())
            {
                for (const auto &p_data :
                     phone_numbers.to_list<mobius::core::pod::map> ())
                    c.phone_numbers.push_back (
                        mobius::core::string::strip (
                            p_data.get<std::string> ("number")
                        )
                    );
            }

            contacts_.emplace (c.skype_name, c);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + ". Path: " + db.get_path ()
        );
    }
}

} // namespace mobius::extension::app::skype
