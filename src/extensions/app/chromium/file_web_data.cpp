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
#include "file_web_data.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Web Data file tables
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// VERSIONS UNKNOWN: 1-39, 41, 42, 44, 46, 47, 49-54, 57, 59, 62, 63, 66, 68,
// 69, 73, 75, 79, 85, 89, 93-95, 99, 101-103, 105, 106, 114, 115, 118, 120-133,
// 135-
//
// - autofill: autofill entries
//      name: 40??-134
//      value: 40??-134
//      pair_id: 40??-48??
//      count: 40??-134
//      date_created: 55??-134
//      date_last_used: 55??-134

// - autofill_dates: autofill entry dates
//      pair_id: 40??-48??
//      date_created: 40??-48??
//      DELETED: 55??
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
//      full_name: 60??-113??
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
//      first_name: 40-58??
//      middle_name: 40-58??
//      last_name: 40-58??
//      full_name: 58??
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
//      guid: 40??-134
//      name_on_card: 40??-134
//      expiration_month: 40??-134
//      expiration_year: 40??-134
//      card_number_encrypted: 40??-134
//      date_modified: 40??-134
//      origin: 55??-134
//      use_count: 61-134
//      use_date: 61-134
//      billing_address_id: 67??-134
//      nickname: 87-134
//
// - masked_credit_cards: masked credit card entries
//      id: 60??-134
//      status: 60??-97
//      name_on_card??: 60-134
//      type: 60??-71,74??-82
//      last_four: 60??-134
//      exp_month: 60??-134
//      exp_year: 60??-134
//      billing_address_id: 67??-70
//      network: 72-134
//      bank_name: 74??-134
//      card_issuer: 86??-134
//      nickname: 84??-134

// - unmasked_credit_cards: unmasked credit card entries
//      id: 60??-134
//      card_number_encrypted: 60??-134
//      use_count: 64??-84??
//      use_date: 64??-84??
//      unmask_date: 64??-134
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Generate SQL statement with version-aware column replacements
// @param sql_template The SQL template string with
// ${column,start_version,end_version} placeholders
// @param schema_version The current schema version to check against
// @return Processed SQL statement with appropriate columns based on version
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
_generate_sql (const std::string &sql_template, int64_t schema_version)
{
    std::string result = sql_template;
    std::string::size_type pos = 0;

    while ((pos = result.find ("${", pos)) != std::string::npos)
    {
        // Find the closing bracket
        auto end_pos = result.find ("}", pos);
        if (end_pos == std::string::npos)
        {
            pos += 2; // Skip the "${" and continue
            continue;
        }

        // Extract the placeholder content
        std::string placeholder = result.substr (pos + 2, end_pos - pos - 2);

        // Split by comma
        std::vector<std::string> parts;
        std::string::size_type prev = 0;
        std::string::size_type comma_pos = 0;

        while ((comma_pos = placeholder.find (',', prev)) != std::string::npos)
        {
            parts.push_back (placeholder.substr (prev, comma_pos - prev));
            prev = comma_pos + 1;
        }
        parts.push_back (placeholder.substr (prev));

        // Default to empty string if column should be excluded
        std::string replacement = "NULL";

        // Get column name and version ranges
        std::string column_name = parts[0];
        int64_t start_version = -1;
        int64_t end_version = std::numeric_limits<int64_t>::max ();

        // Parse start_version if provided
        if (parts.size () > 1 && !parts[1].empty ())
        {
            try
            {
                start_version = std::stoll (parts[1]);
            }
            catch (...)
            {
                // Keep default if parsing fails
            }
        }

        // Parse end_version if provided
        if (parts.size () > 2 && !parts[2].empty ())
        {
            try
            {
                end_version = std::stoll (parts[2]);
            }
            catch (...)
            {
                // Keep default if parsing fails
            }
        }

        // Check if current schema version is within range
        if (schema_version >= start_version && schema_version <= end_version)
            replacement = column_name;

        // Replace the placeholder with the column name or empty string
        result.replace (pos, end_pos - pos + 1, replacement);

        // Continue searching from current position
        // No need to advance pos since the replacement might be shorter than
        // the placeholder
    }

    return result;
}

} // namespace

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_web_data::file_web_data (const mobius::core::io::reader &reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (!reader)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Copy reader content to temporary file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::io::tempfile tfile;
    tfile.copy_from (reader);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get schema version
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Load data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    _load_autofill_entries (db);
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
            "SELECT a.name, a.value, a.count, d.date_created, NULL "
            "FROM autofill a "
            "LEFT JOIN autofill_dates d ON a.pair_id = d.pair_id"
        );

    else
        stmt = db.new_statement (
            "SELECT name, value, count, date_created, date_last_used "
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
        entry.date_created =
            mobius::core::datetime::new_datetime_from_unix_timestamp (
                stmt.get_column_int64 (3)
            );
        entry.date_last_used =
            mobius::core::datetime::new_datetime_from_unix_timestamp (
                stmt.get_column_int64 (4)
            );
        entry.is_encrypted =
            entry.value.startswith ("v10") || entry.value.startswith ("v20");

        // Add entry to the list
        autofill_entries_.emplace_back (std::move (entry));
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
    mobius::core::database::statement stmt = db.new_statement (_generate_sql (
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
        card.origin = stmt.get_column_string (6);
        card.use_count = stmt.get_column_int64 (7);
        card.nickname = stmt.get_column_string (9);

        auto timestamp = stmt.get_column_int64 (5);
        card.date_modified =
            mobius::core::datetime::new_datetime_from_unix_timestamp (
                timestamp
            );

        timestamp = stmt.get_column_int64 (8);
        card.use_date =
            mobius::core::datetime::new_datetime_from_unix_timestamp (
                timestamp
            );

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
    auto stmt = db.new_statement (_generate_sql (
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
        auto last_four = stmt.get_column_string (4);

        if (!last_four.empty ())
            card.card_number = std::string ("**** **** **** ") + last_four;

        auto timestamp = stmt.get_column_int64 (13);
        card.use_date =
            mobius::core::datetime::new_datetime_from_unix_timestamp (
                timestamp
            );

        timestamp = stmt.get_column_int64 (14);
        card.unmask_date =
            mobius::core::datetime::new_datetime_from_unix_timestamp (
                timestamp
            );

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
