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
#include "file_web_data.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <unordered_map>
#include <unordered_set>
#include "common.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
// @see https://atropos4n6.com/other/chrome-web-data-forensics/
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Web Data file tables
//
// - autofill: autofill entries for forms
// - autofill_dates: autofill entry dates
// - autofill_profiles: autofill profiles
// - autofill_profile_addresses: autofill profile addresses
// - autofill_profile_birthdates
// - autofill_profile_emails: autofill profile emails
// - autofill_profile_names: autofill profile names
// - autofill_profile_phones: autofill profile phones
// - autofill_profile_usernames
// - autofill_profiles_trash: autofill entries in trash
// - credit_cards: credit card entries
// - credit_card_tags
// - credit_card_tags_v2
// - ibans: International Bank Account Numbers
// - masked_credit_cards: masked credit card entries
// - server_card_metadata
// - unmasked_credit_cards: unmasked credit card entries
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
    1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,
    29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  41,  42,  44,
    47,  49,  50,  51,  53,  54,  57,  59,  62,  63,  66,  68,  69,  73,
    75,  79,  85,  89,  93,  94,  95,  101, 102, 103, 105, 106, 114, 115,
    118, 121, 124, 126, 129, 131, 133, 136, 139, 142, 144
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Last known schema version
// This constant represents the last schema version that is known and handled
// by the current implementation. Any schema version greater than this value
// will be considered unsupported and will trigger a warning in the log.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::int64_t LAST_KNOWN_SCHEMA_VERSION = 145;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Server card metadata structure
// This structure represents metadata for server-stored credit cards.
// It contains fields for the card ID, billing address ID, use count, and
// use date. This structure is used to manage server card metadata retrieved
// from the database.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct server_card_metadata
{
    std::string id;
    std::string billing_address_id;
    std::int64_t use_count = 0;
    mobius::core::datetime::datetime use_date;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unmasked credit card structure
// This structure represents an unmasked credit card entry in the web data.
// It contains fields for the card ID, encrypted card number, use count,
// use date, and unmask date. This structure is used to store and manage
// unmasked credit card information retrieved from the database.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct unmasked_credit_card
{
    std::string id;
    mobius::core::bytearray card_number_encrypted;
    std::int64_t use_count = 0;
    mobius::core::datetime::datetime use_date;
    mobius::core::datetime::datetime unmask_date;
};

} // namespace

namespace mobius::extension::app::chromium
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if profile is in trash
// @param db Database object
// @param schema_version Schema version
// @param guid Profile GUID
// @return true if profile is in trash, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static bool
_is_profile_in_trash (
    mobius::core::database::database &db,
    std::int64_t schema_version,
    const std::string &guid
)
{
    if (schema_version < 40 || schema_version > 98)
        return false;

    auto stmt = db.new_statement (
        "SELECT 1 "
        "FROM autofill_profiles_trash "
        "WHERE guid = ?"
    );

    stmt.bind (1, guid);

    return stmt.fetch_row ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get profiles addresses
// @param db Database object
// @param schema_version Schema version
// @param guiid Profile GUID
// @return Vector of autofill profile addresses
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::vector<file_web_data::autofill_profile_address>
_get_profile_addresses (
    mobius::core::database::database &db,
    std::int64_t schema_version,
    const std::string &guid
)
{
    std::vector<file_web_data::autofill_profile_address> addresses;

    if (schema_version < 88 || schema_version > 113)
        return {};

    // Prepare SQL statement for table autofill_profile_addresses
    auto stmt = db.new_statement_with_pattern (
        "SELECT {autofill_profile_addresses.apartment_number}, "
        "{autofill_profile_addresses.apartment_number_status}, "
        "{autofill_profile_addresses.city}, "
        "{autofill_profile_addresses.city_status}, "
        "{autofill_profile_addresses.country_code}, "
        "{autofill_profile_addresses.country_code_status}, "
        "{autofill_profile_addresses.dependent_locality}, "
        "{autofill_profile_addresses.dependent_locality_status}, "
        "dependent_street_name, "
        "dependent_street_name_status, "
        "{autofill_profile_addresses.floor}, "
        "{autofill_profile_addresses.floor_status}, "
        "guid, "
        "house_number, "
        "house_number_status, "
        "premise_name, "
        "premise_name_status, "
        "{autofill_profile_addresses.sorting_code}, "
        "{autofill_profile_addresses.sorting_code_status}, "
        "{autofill_profile_addresses.state}, "
        "{autofill_profile_addresses.state_status}, "
        "street_address, "
        "street_address_status, "
        "street_name, "
        "street_name_status, "
        "subpremise, "
        "subpremise_status, "
        "{autofill_profile_addresses.zip_code}, "
        "{autofill_profile_addresses.zip_code_status} "
        "FROM autofill_profile_addresses "
        "WHERE guid = ?"
    );

    stmt.bind (1, guid);

    // Retrieve records from autofill_profile_addresses table
    while (stmt.fetch_row ())
    {
        file_web_data::autofill_profile_address address;
        address.apartment_number = stmt.get_column_string (0);
        // address.apartment_number_status = stmt.get_column_int64 (1);
        address.city = stmt.get_column_string (2);
        // address.city_status = stmt.get_column_int64 (3);
        address.country_code = stmt.get_column_string (4);
        // address.country_code_status = stmt.get_column_int64 (5);
        address.dependent_locality = stmt.get_column_string (6);
        // address.dependent_locality_status = stmt.get_column_int64 (7);
        address.dependent_street_name = stmt.get_column_string (8);
        // address.dependent_street_name_status = stmt.get_column_int64 (9);
        address.floor = stmt.get_column_string (10);
        // address.floor_status = stmt.get_column_int64 (11);
        // address.guid = stmt.get_column_string (12);
        address.house_number = stmt.get_column_string (13);
        // address.house_number_status = stmt.get_column_int64 (14);
        address.premise_name = stmt.get_column_string (15);
        // address.premise_name_status = stmt.get_column_int64 (16);
        // address.sorting_code = stmt.get_column_string (17);
        // address.sorting_code_status = stmt.get_column_int64 (18);
        address.state = stmt.get_column_string (19);
        // address.state_status = stmt.get_column_int64 (20);
        address.street_address = stmt.get_column_string (21);
        // address.street_address_status = stmt.get_column_int64 (22);
        address.street_name = stmt.get_column_string (23);
        // address.street_name_status = stmt.get_column_int64 (24);
        address.subpremise = stmt.get_column_string (25);
        // address.subpremise_status = stmt.get_column_int64 (26);
        address.zip_code = stmt.get_column_string (27);
        // address.zip_code_status = stmt.get_column_int64 (28);

        // Add address to the profile
        addresses.emplace_back (std::move (address));
    }

    return addresses;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get profile emails
// @param db Database object
// @param schema_version Schema version
// @param guid Profile GUID
// @return Vector of profile emails
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::vector<std::string>
_get_profile_emails (
    mobius::core::database::database &db,
    std::int64_t schema_version,
    const std::string &guid
)
{
    std::vector<std::string> emails;

    if (schema_version < 40 || schema_version > 113)
        return emails;

    // Prepare statement to retrieve emails from autofill_profile_emails table
    mobius::core::database::statement stmt = db.new_statement (
        "SELECT email "
        "FROM autofill_profile_emails "
        "WHERE guid = ?"
    );

    stmt.bind (1, guid);

    // Retrieve records from autofill_profile_emails table
    while (stmt.fetch_row ())
        emails.emplace_back (stmt.get_column_string (0));

    return emails;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get profile names
// @param db Database object
// @param schema_version Schema version
// @param guid Profile GUID
// @return Vector of profile names
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::vector<file_web_data::autofill_profile_name>
_get_profile_names (
    mobius::core::database::database &db,
    std::int64_t schema_version,
    const std::string &guid
)
{
    std::vector<file_web_data::autofill_profile_name> names;

    if (schema_version < 40 || schema_version > 113)
        return names;

    // Prepare SQL statement for table autofill_profile_names
    auto stmt = db.new_statement_with_pattern (
        "SELECT {autofill_profile_names.conjunction_last_name}, "
        "{autofill_profile_names.conjunction_last_name_status}, "
        "{autofill_profile_names.date_of_birth}, "
        "{autofill_profile_names.first_last_name}, "
        "{autofill_profile_names.first_last_name_status}, "
        "first_name, "
        "{autofill_profile_names.first_name_status}, "
        "{autofill_profile_names.full_name}, "
        "{autofill_profile_names.full_name_status}, "
        "{autofill_profile_names.full_name_with_honorific_prefix}, "
        "{autofill_profile_names.full_name_with_honorific_prefix_status}, "
        "{autofill_profile_names.gender}, "
        "guid, "
        "{autofill_profile_names.honorific_prefix}, "
        "{autofill_profile_names.honorific_prefix_status}, "
        "last_name, "
        "{autofill_profile_names.last_name_status}, "
        "middle_name, "
        "{autofill_profile_names.middle_name_status}, "
        "{autofill_profile_names.second_last_name}, "
        "{autofill_profile_names.second_last_name_status} "
        "FROM autofill_profile_names "
        "WHERE guid = ?"
    );

    stmt.bind (1, guid);

    // Retrieve records from autofill_profile_names table
    while (stmt.fetch_row ())
    {
        file_web_data::autofill_profile_name name;

        name.conjunction_last_name = stmt.get_column_string (0);
        // name.conjunction_last_name_status = stmt.get_column_int64 (1);
        // name.date_of_birth = stmt.get_column_string (2);
        name.first_last_name = stmt.get_column_string (3);
        // name.first_last_name_status = stmt.get_column_int64 (4);
        name.first_name = stmt.get_column_string (5);
        // name.first_name_status = stmt.get_column_int64 (6);
        name.full_name = stmt.get_column_bytearray (7);
        // name.full_name_status = stmt.get_column_int64 (8);
        name.full_name_with_honorific_prefix = stmt.get_column_string (9);
        //name.full_name_with_honorific_prefix_status = stmt.get_column_int64 (10);
        // name.gender = stmt.get_column_string (11);
        // name.guid = stmt.get_column_string (12);
        name.honorific_prefix = stmt.get_column_string (13);
        // name.honorific_prefix_status = stmt.get_column_int64 (14);
        name.last_name = stmt.get_column_bytearray (15);
        // name.last_name_status = stmt.get_column_int64 (16);
        name.middle_name = stmt.get_column_string (17);
        // name.middle_name_status = stmt.get_column_int64 (18);
        name.second_last_name = stmt.get_column_string (19);
        // name.second_last_name_status = stmt.get_column_int64 (20);

        // Add name to the profile
        names.emplace_back (std::move (name));
    }

    return names;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get profile phones
// @param db Database object
// @param schema_version Schema version
// @param guid Profile GUID
// @return Vector of profile phones
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::vector<file_web_data::autofill_profile_phone>
_get_profile_phones (
    mobius::core::database::database &db,
    std::int64_t schema_version,
    const std::string &guid
)
{
    std::vector<file_web_data::autofill_profile_phone> phones;

    if (schema_version < 40 || schema_version > 113)
        return phones;

    // Prepare statement to retrieve phones from autofill_profile_phones table
    auto stmt = db.new_statement_with_pattern (
        "SELECT {autofill_profile_phones.type} "
        "number, "
        "FROM autofill_profile_phones "
        "WHERE guid = ?"
    );

    stmt.bind (1, guid);

    // Retrieve records from autofill_profile_phones table
    while (stmt.fetch_row ())
    {
        file_web_data::autofill_profile_phone phone;

        phone.type = stmt.get_column_string (0);
        phone.number = stmt.get_column_string (1);

        // Add phone to the profile
        phones.emplace_back (std::move (phone));
    }

    return phones;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get server card metadata
// @param db Database object
// @param schema_version Schema version
// @return Map of server card metadata
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::unordered_map<std::string, server_card_metadata>
_get_server_card_metadata (
    mobius::core::database::database &db, std::int64_t schema_version
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    std::unordered_map<std::string, server_card_metadata>
        server_card_metadata_map;

    try
    {
        if (schema_version >= 65)
        {
            // Prepare SQL statement for table server_card_metadata
            auto stmt = db.new_statement_with_pattern (
                "SELECT {server_card_metadata.billing_address_id}, "
                "id, "
                "use_count, "
                "use_date "
                "FROM server_card_metadata"
            );

            // Retrieve records from server_card_metadata table
            while (stmt.fetch_row ())
            {
                server_card_metadata obj;

                obj.billing_address_id = stmt.get_column_string (0);
                obj.id = stmt.get_column_string (1);
                obj.use_count = stmt.get_column_int64 (2);
                obj.use_date = get_datetime (stmt.get_column_int64 (3));

                // Add server_card_metadata to the map
                server_card_metadata_map[obj.id] = obj;
            }
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }

    return server_card_metadata_map;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get unmasked credit cards
// @param db Database object
// @param schema_version Schema version
// @return Map of unmasked credit cards
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::unordered_map<std::string, unmasked_credit_card>
_get_unmasked_credit_cards (
    mobius::core::database::database &db, std::int64_t schema_version
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    std::unordered_map<std::string, unmasked_credit_card> unmasked_credit_cards;

    try
    {
        if (schema_version >= 60 && db.has_table ("unmasked_credit_cards"))
        {
            mobius::core::database::statement stmt = db.new_statement (
                "SELECT id, "
                "card_number_encrypted, "
                "${use_count:64-84}, "
                "${use_date:64-84}, "
                "${unmask_date:64-*} "
                "FROM unmasked_credit_cards",
                schema_version
            );

            while (stmt.fetch_row ())
            {
                unmasked_credit_card card;

                card.id = stmt.get_column_string (0);
                card.card_number_encrypted = stmt.get_column_bytearray (1);
                card.use_count = stmt.get_column_int64 (2);
                card.use_date = get_datetime (stmt.get_column_int64 (3));
                card.unmask_date = get_datetime (stmt.get_column_int64 (4));

                // Add card to the list
                unmasked_credit_cards[card.id] = card;
            }
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }

    return unmasked_credit_cards;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_web_data::file_web_data (const mobius::core::io::reader &reader)
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
        _load_autofill_entries (db);
        _load_autofill_profiles (db);
        _load_credit_cards (db);
        _load_masked_credit_cards (db);
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load autofill entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_web_data::_load_autofill_entries (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare statement
        mobius::core::database::statement stmt;

        if (schema_version_ < 53)
            stmt = db.new_statement (
                "SELECT a.name, "
                "a.value, "
                "a.count, "
                "d.date_created, "
                "NULL "
                "FROM autofill a "
                "LEFT JOIN autofill_dates d ON a.pair_id = d.pair_id"
            );

        else
            stmt = db.new_statement (
                "SELECT name, "
                "value, "
                "count, "
                "date_created, "
                "date_last_used "
                "FROM autofill"
            );

        // Retrieve records from autofill table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            autofill_entry entry;

            entry.idx = idx++;
            entry.name = stmt.get_column_string (0);
            entry.value = stmt.get_column_bytearray (1);
            entry.count = stmt.get_column_int (2);
            entry.date_created = get_datetime (stmt.get_column_int64 (3));
            entry.date_last_used = get_datetime (stmt.get_column_int64 (4));
            entry.is_encrypted = entry.value.startswith ("v10") ||
                                 entry.value.startswith ("v20");

            // Add entry to the list
            autofill_entries_.emplace_back (std::move (entry));
        }

        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load autofill profiles
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_web_data::_load_autofill_profiles (mobius::core::database::database &db)
{
    // Check schema version
    if (schema_version_ < 40 || schema_version_ > 113)
        return;

    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare SQL statement for table autofill_profiles
        auto stmt = db.new_statement_with_pattern (
            "SELECT {autofill_profiles.address_line_1}, "
            "{autofill_profiles.address_line_2}, "
            "city, "
            "company_name, "
            "{autofill_profiles.country}, "
            "country_code, "
            "date_modified, "
            "{autofill_profiles.dependent_locality}, "
            "{autofill_profiles.disallow_settings_visible_updates}, "
            "guid, "
            "{autofill_profiles.is_client_validity_states_updated}, "
            "{autofill_profiles.label}, "
            "{autofill_profiles.language_code}, "
            "{autofill_profiles.origin}, "
            "{autofill_profiles.sorting_code}, "
            "state, "
            "{autofill_profiles.street_address}, "
            "{autofill_profiles.use_count}, "
            "{autofill_profiles.use_date}, "
            "{autofill_profiles.validity_bitfield}, "
            "zipcode "
            "FROM autofill_profiles"
        );

        // Retrieve records from autofill_profiles table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            autofill_profile profile;

            // Set profile fields
            profile.idx = idx++;
            profile.company_name = stmt.get_column_string (3);
            profile.date_modified = get_datetime (stmt.get_column_int64 (6));
            // profile.disallow_settings_visible_updates = stmt.get_column_int64 (8);
            profile.guid = stmt.get_column_string (9);
            // profile.is_client_validity_states_updated = stmt.get_column_string (10);
            // profile.label = stmt.get_column_string (11);
            profile.language_code = stmt.get_column_string (12);
            profile.origin = stmt.get_column_string (13);
            // profile.sorting_code = stmt.get_column_bytearray (14);
            profile.use_count = stmt.get_column_int64 (17);
            profile.date_last_used = get_datetime (stmt.get_column_int64 (18));
            // profile.validity_bitfield = stmt.get_column_string (19);

            profile.is_in_trash =
                _is_profile_in_trash (db, schema_version_, profile.guid);
            profile.addresses =
                _get_profile_addresses (db, schema_version_, profile.guid);
            profile.emails =
                _get_profile_emails (db, schema_version_, profile.guid);
            profile.names =
                _get_profile_names (db, schema_version_, profile.guid);
            profile.phones =
                _get_profile_phones (db, schema_version_, profile.guid);

            // Create address, if available
            auto address_line_1 = stmt.get_column_string (0);
            auto address_line_2 = stmt.get_column_string (1);
            auto city = stmt.get_column_string (2);
            auto country = stmt.get_column_string (4);
            auto country_code = stmt.get_column_string (5);
            auto dependent_locality = stmt.get_column_string (7);
            auto state = stmt.get_column_string (15);
            auto street_address = stmt.get_column_string (16);
            auto zip_code = stmt.get_column_string (20);

            if (!address_line_1.empty () || !address_line_2.empty () ||
                !street_address.empty () || !dependent_locality.empty () ||
                !city.empty () || !state.empty () || !zip_code.empty () ||
                !country_code.empty () || !country.empty ())
            {
                autofill_profile_address address;

                address.address_line_1 = std::move (address_line_1);
                address.address_line_2 = std::move (address_line_2);
                address.street_address = std::move (street_address);
                address.dependent_locality = std::move (dependent_locality);
                address.city = std::move (city);
                address.state = std::move (state);
                address.zip_code = std::move (zip_code);
                address.country_code = std::move (country_code);
                address.country = std::move (country);

                // Add address to the profile
                profile.addresses.emplace_back (std::move (address));
            }

            // Add profile to the list
            autofill_profiles_.emplace_back (std::move (profile));
        }

        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load credit cards
// @param db Database object
// @see autofill/core/browser/webdata/payments/payments_autofill_table.h
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_web_data::_load_credit_cards (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare SQL statement for table credit_cards
        auto stmt = db.new_statement_with_pattern (
            "SELECT {credit_cards.billing_address_id}, "
            "card_number_encrypted, "
            "date_modified, "
            "expiration_month, "
            "expiration_year, "
            "guid, "
            "name_on_card, "
            "{credit_cards.nickname}, "
            "{credit_cards.origin}, "
            "{credit_cards.use_count}, "
            "{credit_cards.use_date} "
            "FROM credit_cards"
        );

        // Retrieve records from credit_cards table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            credit_card obj;

            obj.idx = idx++;
            obj.billing_address_id = stmt.get_column_string (0);
            obj.card_number_encrypted = stmt.get_column_bytearray (1);
            obj.date_modified = get_datetime (stmt.get_column_int64 (2));
            obj.expiration_month = stmt.get_column_bytearray (3);
            obj.expiration_year = stmt.get_column_int64 (4);
            obj.guid = stmt.get_column_string (5);
            obj.nickname = stmt.get_column_string (7);
            obj.origin = stmt.get_column_string (8);
            obj.use_count = stmt.get_column_int64 (9);
            obj.use_date = get_datetime (stmt.get_column_int64 (10));

            // Set name on card
            auto name_on_card = stmt.get_column_bytearray (6);

            if (is_encrypted (name_on_card))
                obj.name_on_card_encrypted = name_on_card;

            else
                obj.name_on_card = name_on_card.to_string ();

            // Add credit_cards to the list
            credit_cards_.emplace_back (std::move (obj));
        }

        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load masked credit cards
// @param db Database object
// @see autofill/core/browser/webdata/payments/payments_autofill_table.h
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_web_data::_load_masked_credit_cards (mobius::core::database::database &db)
{
    if (schema_version_ < 60)
        return;

    mobius::core::log log (__FILE__, __FUNCTION__);

    auto server_card_metadata = _get_server_card_metadata (db, schema_version_);

    auto unmasked_credit_cards =
        _get_unmasked_credit_cards (db, schema_version_);

    try
    {
        // Prepare SQL statement for table masked_credit_cards
        auto stmt = db.new_statement_with_pattern (
            "SELECT {masked_credit_cards.bank_name}, "
            "{masked_credit_cards.billing_address_id}, "
            "{masked_credit_cards.card_art_url}, "
            "{masked_credit_cards.card_benefit_source}, "
            "{masked_credit_cards.card_creation_source}, "
            "{masked_credit_cards.card_info_retrieval_enrollment_state}, "
            "{masked_credit_cards.card_issuer}, "
            "{masked_credit_cards.card_issuer_id}, "
            "exp_month, "
            "exp_year, "
            "id, "
            "{masked_credit_cards.instrument_id}, "
            "last_four, "
            "name_on_card, "
            "{masked_credit_cards.network}, "
            "{masked_credit_cards.nickname}, "
            "{masked_credit_cards.product_description}, "
            "{masked_credit_cards.product_terms_url}, "
            "{masked_credit_cards.status}, "
            "{masked_credit_cards.type}, "
            "{masked_credit_cards.virtual_card_enrollment_state}, "
            "{masked_credit_cards.virtual_card_enrollment_type} "
            "FROM masked_credit_cards"
        );

        // Retrieve records from masked_credit_cards and unmasked_credit_cards tables
        auto idx = 0;

        while (stmt.fetch_row ())
        {
            credit_card card;

            // Fill data from masked_credit_cards table
            card.idx = idx++;
            card.bank_name = stmt.get_column_string (0);
            card.billing_address_id = stmt.get_column_string (1);
            card.card_art_url = stmt.get_column_string (2);
            card.card_benefit_source = stmt.get_column_int64 (3);
            // card.card_creation_source = stmt.get_column_int64 (4);
            card.card_info_retrieval_enrollment_state =
                stmt.get_column_int64 (5);
            card.card_issuer = stmt.get_column_int64 (6);
            card.card_issuer_id = stmt.get_column_string (7);
            card.expiration_month = stmt.get_column_int64 (8);
            card.expiration_year = stmt.get_column_int64 (9);
            card.id = stmt.get_column_string (10);
            card.instrument_id = stmt.get_column_int64 (11);
            card.last_four = stmt.get_column_string (12);
            card.name_on_card = stmt.get_column_string (13);
            card.network = stmt.get_column_string (14);
            card.nickname = stmt.get_column_string (15);
            card.product_description = stmt.get_column_string (16);
            card.product_terms_url = stmt.get_column_string (17);
            card.status = stmt.get_column_string (18);
            card.type = stmt.get_column_int64 (19);
            card.virtual_card_enrollment_state = stmt.get_column_int64 (20);
            card.virtual_card_enrollment_type = stmt.get_column_int64 (21);

            if (!card.last_four.empty ())
                card.card_number =
                    std::string ("**** **** **** ") + card.last_four;

            // name_on_card
            auto name_on_card = stmt.get_column_bytearray (12);
            if (is_encrypted (name_on_card))
                card.name_on_card_encrypted = name_on_card;

            else
                card.name_on_card = name_on_card.to_string ();

            // If server card metadata exists, copy its data
            auto iter_server_card = server_card_metadata.find (card.id);
            if (iter_server_card != server_card_metadata.end ())
            {
                card.billing_address_id =
                    iter_server_card->second.billing_address_id;
                card.use_count = iter_server_card->second.use_count;
                card.use_date = iter_server_card->second.use_date;
            }

            // If unmasked credit card exists, copy its data
            auto iter_unmasked = unmasked_credit_cards.find (card.id);
            if (iter_unmasked != unmasked_credit_cards.end ())
            {
                card.unmask_date = iter_unmasked->second.unmask_date;
                card.use_count = iter_unmasked->second.use_count;
                card.use_date = iter_unmasked->second.use_date;
                card.card_number_encrypted =
                    iter_unmasked->second.card_number_encrypted;
            }

            // Add card to the list
            credit_cards_.emplace_back (std::move (card));
        }

        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::chromium
