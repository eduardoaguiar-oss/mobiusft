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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Web Data file tables
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// - autofill: autofill entries
//      - count: 40, 43, 45, 48, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78,
//      80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119, 122, 125,
//      127, 130, 132, 134-135
//      - date_created: 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84,
//      86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119, 122, 125, 127,
//      130, 132, 134-135
//      - date_last_used: 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84,
//      86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119, 122, 125, 127,
//      130, 132, 134-135
//      - guid: 83-84, 87-88, 90
//      - name: 40, 43, 45, 48, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78,
//      80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119, 122, 125,
//      127, 130, 132, 134-135
//      - pair_id: 40, 43, 45, 48
//      - value: 40, 43, 45, 48, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78,
//      80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119, 122, 125,
//      127, 130, 132, 134-135
//      - value_lower: 40, 43, 45, 48, 55-56, 58, 60-61, 64-65, 67, 70-72, 74,
//      76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119, 122,
//      125, 127, 130, 132, 134-135
//
// - autofill_dates: autofill entry dates
//      - date_created: 40, 43, 45, 48
//      - pair_id: 40, 43, 45, 48
//
// - autofill_profiles: autofill profiles
//      guid: 40??-113??
//      company_name: 40??-113??
//      address_line_1: 40??-48??
//      address_line_2: 40??-48??
//      street_address: 55??-113??
//      dependent_locality: 55??-113??
//      city: 40??-113??
//      state: 40??-113??
//      zipcode: 40??-113??
//      country_code: 40??-113??
//      country: 40??-48??
//      date_modified: 40??-113??
//      origin: 55??-113??
//      language_code: 56-113??
//      use_count: 61-135
//      use_date: 61-135
//      DELETED: 116??
//
// - autofill_profile_addresses: autofill profile addresses
//      guid: 88-113??
//      street_address: 88-113??
//      street_name: 88-113??
//      dependent_street_name: 88-113??
//      house_number: 88-113??
//      subpremise: 88-113??
//      premise_name: 88-113??
//      street_address_status: 88-113??
//      street_name_status: 88-113??
//      dependent_street_name_status: 88-113??
//      house_number_status: 88-113??
//      subpremise_status: 88-113??
//      premise_name_status: 88-113??
//      dependent_locality: 90-113??
//      city: 90-113??
//      state: 90-113??
//      zip_code: 90-113??
//      country_code: 90-113??
//      dependent_locality_status: 90-113??
//      city_status: 90-113??
//      state_status: 90-113??
//      zip_code_status: 90-113??
//      country_code_status: 90-113??
//      apartment_number: 91-113??
//      floor: 91-113??
//      apartment_number_status: 91-113??
//      floor_status: 91-113??
//      DELETED: 116??
//
// - autofill_profile_emails: autofill profile emails
//      guid: 40-113??
//      email: 40-113??
//      DELETED: 116??
//
// - autofill_profile_names: autofill profile names
//      guid: 40-58??
//      first_name: 40-113??
//      middle_name: 40-113??
//      last_name: 40-113??
//      full_name: 58??-113??
//      honorific_prefix: 88-113??
//      first_last_name: 88-113??
//      conjunction_last_name: 88-113??
//      second_last_name: 88-113??
//      honorific_prefix_status: 88-113??
//      first_name_status: 88-113??
//      middle_name_status: 88-113??
//      last_name_status: 88-113??
//      first_last_name_status: 88-113??
//      conjunction_last_name_status: 88-113??
//      second_last_name_status: 88-113??
//      full_name_status: 88-113??
//      full_name_with_honorific_prefix: 92-113??
//      full_name_with_honorific_prefix_status: 92-113??
//      DELETED: 116??
//
// - autofill_profile_phones: autofill profile phones
//      guid: 40-113??
//      type: 40-48??
//      number: 40-113??
//      DELETED: 116??
//
// - autofill_profiles_trash: autofill entries in trash
//      guid: 40-98??
//      DELETED: 100??
//
// - credit_cards: credit card entries
//      guid: 40??-135
//      name_on_card: 40??-135
//      expiration_month: 40??-135
//      expiration_year: 40??-135
//      card_number_encrypted: 40??-135
//      date_modified: 40??-135
//      origin: 55??-135
//      use_count: 61-135
//      use_date: 61-135
//      billing_address_id: 67??-135
//      nickname: 87-135
//
// - masked_credit_cards: masked credit card entries
//      id: 60??-135
//      status: 60??-97
//      name_on_card??: 60-135
//      type: 60??-71,74??-82
//      last_four: 60??-135
//      exp_month: 60??-135
//      exp_year: 60??-135
//      billing_address_id: 67??-70
//      network: 72-135
//      bank_name: 74??-135
//      card_issuer: 86??-135
//      nickname: 84??-135
//
// - unmasked_credit_cards: unmasked credit card entries
//    - card_number_encrypted: 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88,
//    90-92, 96-98, 100, 104, 107-113, 116-117, 119, 122, 130, 132, 134-135
//    - id: 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100,
//    104, 107-113, 116-117, 119, 122, 130, 132, 134-135
//    - unmask_date: 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98,
//    100, 104, 107-113, 116-117, 119, 122, 130, 132, 134-135
//    - use_count: 64-65, 67, 70-72, 74, 76-78, 80-84
//    - use_date: 64-65, 67, 70-72, 74, 76-78, 80-84
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "file_web_data.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <unordered_set>
#include "common.hpp"

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
    46,  47,  49,  50,  51,  52,  53,  54,  57,  59,  62,  63,  66,  68,
    69,  73,  75,  79,  85,  89,  93,  94,  95,  99,  101, 102, 103, 105,
    106, 114, 115, 118, 120, 121, 123, 124, 126, 128, 129, 131, 133,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Last known schema version
// This constant represents the last schema version that is known and handled
// by the current implementation. Any schema version greater than this value
// will be considered unsupported and will trigger a warning in the log.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::int64_t LAST_KNOWN_SCHEMA_VERSION = 135;

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

    mobius::core::database::statement stmt = db.new_statement (generate_sql (
        "SELECT street_address, "
        "street_name, "
        "dependent_street_name, "
        "house_number, "
        "subpremise, "
        "premise_name, "
        "street_address_status, "
        "street_name_status, "
        "dependent_street_name_status, "
        "house_number_status, "
        "subpremise_status, "
        "premise_name_status, "
        "${dependent_locality,90}, "
        "${city,90}, "
        "${state,90}, "
        "${zip_code,90}, "
        "${country_code,90}, "
        "${dependent_locality_status,90}, "
        "${city_status,90}, "
        "${state_status,90}, "
        "${zip_code_status,90}, "
        "${country_code_status,90}, "
        "${apartment_number,91}, "
        "${floor,91}, "
        "${apartment_number_status,91}, "
        "${floor_status,91} "
        "FROM autofill_profile_addresses "
        "WHERE guid = ?",
        schema_version
    ));

    stmt.bind (1, guid);

    // Retrieve records from autofill_profile_addresses table
    while (stmt.fetch_row ())
    {
        file_web_data::autofill_profile_address address;
        address.street_address = stmt.get_column_string (0);
        address.street_name = stmt.get_column_string (1);
        address.dependent_street_name = stmt.get_column_string (2);
        address.house_number = stmt.get_column_string (3);
        address.subpremise = stmt.get_column_string (4);
        address.premise_name = stmt.get_column_string (5);
        address.dependent_locality = stmt.get_column_string (12);
        address.city = stmt.get_column_string (13);
        address.state = stmt.get_column_string (14);
        address.zip_code = stmt.get_column_string (15);
        address.country_code = stmt.get_column_string (16);
        address.apartment_number = stmt.get_column_string (22);
        address.floor = stmt.get_column_string (23);

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
    mobius::core::database::statement stmt = db.new_statement (generate_sql (
        "SELECT email "
        "FROM autofill_profile_emails "
        "WHERE guid = ?",
        schema_version
    ));

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

    // Prepare statement to retrieve names from autofill_profile_names table
    mobius::core::database::statement stmt = db.new_statement (generate_sql (
        "SELECT first_name, "
        "middle_name, "
        "last_name, "
        "${full_name,58}, "
        "${honorific_prefix,88}, "
        "${first_last_name,88}, "
        "${conjunction_last_name,88}, "
        "${second_last_name,88}, "
        "${full_name_with_honorific_prefix,92} "
        "FROM autofill_profile_names "
        "WHERE guid = ?",
        schema_version
    ));

    stmt.bind (1, guid);

    // Retrieve records from autofill_profile_names table
    while (stmt.fetch_row ())
    {
        file_web_data::autofill_profile_name name;

        name.first_name = stmt.get_column_string (0);
        name.middle_name = stmt.get_column_string (1);
        name.last_name = stmt.get_column_string (2);
        name.full_name = stmt.get_column_string (3);
        name.honorific_prefix = stmt.get_column_string (4);
        name.first_last_name = stmt.get_column_string (5);
        name.conjunction_last_name = stmt.get_column_string (6);
        name.second_last_name = stmt.get_column_string (7);
        name.full_name_with_honorific_prefix = stmt.get_column_string (8);

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
    mobius::core::database::statement stmt = db.new_statement (generate_sql (
        "SELECT ${type,40,48}, "
        "number "
        "FROM autofill_profile_phones "
        "WHERE guid = ?",
        schema_version
    ));

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

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Copy reader content to temporary file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::io::tempfile tfile;
    tfile.copy_from (reader);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get schema version
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::database::database db (tfile.get_path ());

    auto stmt = db.new_statement (
        "SELECT value "
        "FROM meta "
        "WHERE key = 'version'"
    );

    if (stmt.fetch_row ())
        schema_version_ = stmt.get_column_int64 (0);

    else
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

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Load data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    _load_autofill_entries (db);
    _load_autofill_profiles (db);
    _load_credit_cards (db);
    _load_masked_credit_cards (db);

    is_instance_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load autofill entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_web_data::_load_autofill_entries (mobius::core::database::database &db)
{
    // Prepare statement
    mobius::core::database::statement stmt;

    if (schema_version_ < 55)
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
        entry.is_encrypted =
            entry.value.startswith ("v10") || entry.value.startswith ("v20");

        // Add entry to the list
        autofill_entries_.emplace_back (std::move (entry));
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

    // Prepare SQL statement for table autofill_profiles
    mobius::core::database::statement stmt = db.new_statement (generate_sql (
        "SELECT guid, "
        "company_name, "
        "${address_line_1,40,48}, "
        "${address_line_2,40,48}, "
        "${street_address,55}, "
        "${dependent_locality,55}, "
        "city, "
        "state, "
        "zipcode, "
        "country_code, "
        "${country,40,48}, "
        "${date_modified,40}, "
        "${origin,55}, "
        "${language_code,56}, "
        "${use_count,61}, "
        "${use_date,61} "
        "FROM autofill_profiles p",
        schema_version_
    ));

    // Retrieve records from autofill_profiles table
    std::uint64_t idx = 0;

    while (stmt.fetch_row ())
    {
        autofill_profile profile;

        // Set profile fields
        profile.idx = idx++;
        profile.guid = stmt.get_column_string (0);
        profile.company_name = stmt.get_column_string (1);
        profile.date_modified = get_datetime (stmt.get_column_int64 (11));
        profile.origin = stmt.get_column_string (12);
        profile.language_code = stmt.get_column_string (13);
        profile.use_count = stmt.get_column_int64 (14);
        profile.date_last_used = get_datetime (stmt.get_column_int64 (15));

        profile.is_in_trash =
            _is_profile_in_trash (db, schema_version_, profile.guid);
        profile.addresses =
            _get_profile_addresses (db, schema_version_, profile.guid);
        profile.emails =
            _get_profile_emails (db, schema_version_, profile.guid);
        profile.names = _get_profile_names (db, schema_version_, profile.guid);
        profile.phones =
            _get_profile_phones (db, schema_version_, profile.guid);

        // Create address, if available
        auto address_line_1 = stmt.get_column_string (2);
        auto address_line_2 = stmt.get_column_string (3);
        auto street_address = stmt.get_column_string (4);
        auto dependent_locality = stmt.get_column_string (5);
        auto city = stmt.get_column_string (6);
        auto state = stmt.get_column_string (7);
        auto zip_code = stmt.get_column_string (8);
        auto country_code = stmt.get_column_string (9);
        auto country = stmt.get_column_string (10);

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
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load credit cards
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_web_data::_load_credit_cards (mobius::core::database::database &db)
{
    // Prepare SQL statement for table credit_cards
    mobius::core::database::statement stmt = db.new_statement (generate_sql (
        "SELECT guid, "
        "name_on_card, "
        "expiration_month, "
        "expiration_year, "
        "card_number_encrypted, "
        "date_modified, "
        "${origin,55}, "
        "${use_count,61}, "
        "${use_date,61}, "
        "${nickname,87} "
        "FROM credit_cards",
        schema_version_
    ));

    // Retrieve records from credit_cards table
    std::uint64_t idx = 0;

    while (stmt.fetch_row ())
    {
        credit_card card;

        card.idx = idx++;
        card.name_on_card = stmt.get_column_string (1);
        card.expiration_month = stmt.get_column_int64 (2);
        card.expiration_year = stmt.get_column_int64 (3);
        card.card_number_encrypted = stmt.get_column_bytearray (4);
        card.date_modified = get_datetime (stmt.get_column_int64 (5));
        card.origin = stmt.get_column_string (6);
        card.use_count = stmt.get_column_int64 (7);
        card.use_date = get_datetime (stmt.get_column_int64 (8));
        card.nickname = stmt.get_column_string (9);

        card.metadata.set ("guid", stmt.get_column_string (0));
        card.is_encrypted = card.card_number_encrypted.startswith ("v10") ||
                            card.card_number_encrypted.startswith ("v20");

        // Add card to the list
        credit_cards_.emplace_back (std::move (card));
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load masked credit cards
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_web_data::_load_masked_credit_cards (mobius::core::database::database &db)
{
    if (schema_version_ < 60)
        return;

    // Prepare SQL statement for tables masked_credit_cards and
    // unmasked_credit_cards
    auto stmt = db.new_statement (generate_sql (
        "SELECT m.id, "
        "${m.status,60,97}, "
        "${m.name_on_card,60}, "
        "${m.type,60,71}, "
        "m.last_four, "
        "${m.exp_month,60}, "
        "${m.exp_year,60}, "
        "${m.network,72}, "
        "${m.bank_name,74}, "
        "${m.card_issuer,86}, "
        "${m.nickname,84}, "
        "${u.card_number_encrypted,60}, "
        "${u.use_count,64,84}, "
        "${u.use_date,64,84}, "
        "${u.unmask_date,64} "
        "FROM masked_credit_cards m "
        "LEFT JOIN unmasked_credit_cards u ON m.id = u.id ",
        schema_version_
    ));

    // Retrieve records from masked_credit_cards and unmasked_credit_cards
    // tables
    auto idx = 0;

    while (stmt.fetch_row ())
    {
        credit_card card;

        card.idx = idx++;
        card.name_on_card = stmt.get_column_string (2);
        card.type = stmt.get_column_string (3);
        card.expiration_month = stmt.get_column_int64 (5);
        card.expiration_year = stmt.get_column_int64 (6);
        card.network = stmt.get_column_string (7);
        card.bank_name = stmt.get_column_string (8);
        card.card_issuer = stmt.get_column_string (9);
        card.nickname = stmt.get_column_string (10);
        card.card_number_encrypted = stmt.get_column_bytearray (11);
        card.use_count = stmt.get_column_int64 (12);
        card.use_date = get_datetime (stmt.get_column_int64 (13));
        card.unmask_date = get_datetime (stmt.get_column_int64 (14));

        auto last_four = stmt.get_column_string (4);

        if (!last_four.empty ())
            card.card_number = std::string ("**** **** **** ") + last_four;

        card.metadata.set ("last_four", last_four);
        card.metadata.set ("id", stmt.get_column_string (0));
        card.metadata.set ("status", stmt.get_column_string (1));
        card.is_encrypted = card.card_number_encrypted.startswith ("v10") ||
                            card.card_number_encrypted.startswith ("v20");

        // Add card to the list
        credit_cards_.emplace_back (std::move (card));
    }
}

} // namespace mobius::extension::app::chromium
