// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
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
#include <set>
#include "common.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
//   @see https://bebinary4n6.blogspot.com/2019/07/
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
        _load_contacts (db); // contacts are needed by other evidence types
        _load_account (db);
        _load_calls (db);

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
// @brief Load calls data
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_s4l_db::_load_calls (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load calls data from calllogs table
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto stmt = db.new_statement ("SELECT nsp_pk, nsp_data FROM calllogs");

        while (stmt.fetch_row ())
        {
            call c;
            c.nsp_pk = stmt.get_column_string (0);

            auto parser = mobius::core::decoder::json::parser (
                stmt.get_column_bytearray (1)
            );
            auto nsp_data = mobius::core::pod::map (parser.parse ());

            c.call_id = nsp_data.get<std::string> ("callId");
            c.call_direction = nsp_data.get<std::string> ("callDirection");
            c.call_type = nsp_data.get<std::string> ("callType");
            c.call_state = nsp_data.get<std::string> ("callState");
            c.connect_time =
                mobius::core::datetime::new_datetime_from_iso_string (
                    nsp_data.get<std::string> ("connectTime")
                );
            c.end_time = mobius::core::datetime::new_datetime_from_iso_string (
                nsp_data.get<std::string> ("endTime")
            );
            c.message_id = nsp_data.get<std::string> ("messageId");
            c.message_cuid = nsp_data.get<std::string> ("messageCuid");
            c.originator = nsp_data.get<std::string> ("originator");
            auto participants = nsp_data.get ("participants");
            c.start_time =
                mobius::core::datetime::new_datetime_from_iso_string (
                    nsp_data.get<std::string> ("startTime")
                );
            c.session_type = nsp_data.get<std::string> ("sessionType");
            c.target = nsp_data.get<std::string> ("target");
            c.thread_id = nsp_data.get<std::string> ("threadId");

            // Originator Participant
            auto originator_participant =
                nsp_data.get<mobius::core::pod::map> ("originatorParticipant");

            if (originator_participant)
            {
                c.originator_participant.full_name =
                    originator_participant.get<std::string> ("displayName");
                c.originator_participant.mri =
                    originator_participant.get<std::string> ("id");
                c.originator_participant.skype_name =
                    get_skype_name_from_mri (c.originator_participant.mri);
                c.originator_participant.type =
                    originator_participant.get<std::string> ("type");
            }

            // Target Participant
            auto target_participant =
                nsp_data.get<mobius::core::pod::map> ("targetParticipant");

            if (target_participant)
            {
                c.target_participant.full_name =
                    target_participant.get<std::string> ("displayName");
                c.target_participant.mri =
                    target_participant.get<std::string> ("id");
                c.target_participant.skype_name =
                    get_skype_name_from_mri (c.target_participant.mri);
                c.target_participant.type =
                    target_participant.get<std::string> ("type");

                if (c.target_participant.full_name.empty ())
                {
                    // Fallback to contacts data
                    auto iter =
                        contacts_.find (c.target_participant.skype_name);

                    if (iter != contacts_.end ())
                        c.target_participant.full_name = iter->second.full_name;
                }
            }

            // Participants
            auto participant_list = nsp_data.get ("participantList");

            for (const auto &p_data :
                 participant_list.to_list<mobius::core::pod::map> ())
            {
                call_participant p;

                p.full_name = p_data.get<std::string> ("displayName");
                p.mri = p_data.get<std::string> ("id");
                p.skype_name = get_skype_name_from_mri (p.mri);
                p.type = p_data.get<std::string> ("type");

                if (p.full_name.empty ())
                {
                    // Fallback to contacts data
                    auto iter = contacts_.find (p.skype_name);

                    if (iter != contacts_.end ())
                        p.full_name = iter->second.full_name;
                }

                c.participants.push_back (p);
            }

            // Unhandled fields
            auto call_attributes = nsp_data.get ("callAttributes");
            if (!call_attributes.is_null ())
                log.development (
                    __LINE__, "Call attributes: " + call_attributes.to_string ()
                );

            auto forwarded_info = nsp_data.get ("forwardedInfo");
            if (!forwarded_info.is_null ())
                log.development (
                    __LINE__, "Forwarded Info: " + forwarded_info.to_string ()
                );

            auto transfer_info = nsp_data.get ("transferInfo");
            if (!transfer_info.is_null ())
                log.development (
                    __LINE__, "Transfer Info: " + transfer_info.to_string ()
                );

            calls_.emplace_back (std::move (c));
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
        // Load phone numbers from profilecachev8_phoneNumbersIndex table
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        std::unordered_multimap<std::string, std::string> phone_numbers_index;

        // Prepare SQL statement for table profilecachev8_phoneNumbersIndex
        auto phone_stmt = db.new_statement (
            "SELECT nsp_key,"
            "nsp_refpk "
            "FROM profilecachev8_phoneNumbersIndex"
        );

        while (phone_stmt.fetch_row ())
        {
            // Remote leading 'C' from phone number
            std::string phone_number =
                phone_stmt.get_column_string (0).substr (1);
            std::string skype_name = phone_stmt.get_column_string (1);

            phone_numbers_index.emplace (skype_name, phone_number);
        }

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
            c.skype_name = get_skype_name_from_mri (c.mri);
            c.birthdate = nsp_data.get<std::string> ("birthday");
            c.gender = nsp_data.get<std::int64_t> ("gender");
            c.country = nsp_data.get<std::string> ("country");
            c.province = nsp_data.get<std::string> ("province");
            c.city = nsp_data.get<std::string> ("city");
            c.mood_text = nsp_data.get<std::string> ("mood");
            c.thumbnail_url = nsp_data.get<std::string> ("thumbUrl");
            c.fetched_time =
                mobius::core::datetime::new_datetime_from_unix_timestamp (
                    nsp_data.get<std::int64_t> ("fetchedDate") / 1000
                );

            c.full_name = nsp_data.get<std::string> ("fullName");
            if (c.full_name.empty ())
                c.full_name = nsp_data.get<std::string> ("displayNameOverride");

            auto emails = nsp_data.get ("emails");
            if (emails.is_list ())
                c.emails = emails.to_list<std::string> ();

            // Get phone numbers
            std::set<std::string> phone_numbers_set;
            auto phone_numbers = nsp_data.get ("phones");
            if (phone_numbers.is_list ())
            {
                for (const auto &p_data :
                     phone_numbers.to_list<mobius::core::pod::map> ())
                    phone_numbers_set.insert (
                        mobius::core::string::strip (
                            p_data.get<std::string> ("number")
                        )
                    );
            }

            // Add phone numbers from index table
            auto range = phone_numbers_index.equal_range (c.skype_name);
            std::transform (
                range.first, range.second,
                std::inserter (phone_numbers_set, phone_numbers_set.end ()),
                [] (const auto &pair) { return pair.second; }
            );

            // Copy unique phone numbers to contact
            std::copy (
                phone_numbers_set.begin (), phone_numbers_set.end (),
                std::back_inserter (c.phone_numbers)
            );

            // Add contact to map
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
