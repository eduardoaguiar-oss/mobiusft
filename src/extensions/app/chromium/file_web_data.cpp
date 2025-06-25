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
// - autofill: autofill entries
//   40-48: name, value, pair_id, count
//   55-134: name, value, date_created, date_last_used, count
//
// - autofill_dates: autofill entry dates
//   40-48: pair_id, date_created
//   55: - (deleted)
//
// - autofill_profiles: autofill profiles
//   40-48: guid, company_name, address_line_1, address_line_2, city, state,
//   zipcode, country, country_code, date_modified
//   55: guid, company_name, street_address, dependent_locality, city, state,
//   zipcode, country_code, date_modified, origin
//   56-58: guid, company_name, street_address, dependent_locality, city, state,
//   zipcode, country_code, date_modified, origin, language_code
//   60-113: guid, company_name, street_address, dependent_locality, city,
//   state, zipcode, country_code, date_modified, origin, language_code,
//   full_name 116: - (deleted)
//
// - autofill_profile_addresses: autofill profile addresses
//   88-: guid, street_address, street_name, dependent_street_name,
//   house_number, subpremise, premise_name, street_address_status,
//   street_name_status, dependent_street_name_status, house_number_status,
//   subpremise_status, premise_name_status
//   90-: guid, street_address, street_name, dependent_street_name,
//   house_number, subpremise, premise_name, street_address_status,
//   street_name_status, dependent_street_name_status, house_number_status,
//   subpremise_status, premise_name_status, dependent_locality, city, state,
//   zip_code, country_code, dependent_locality_status, city_status,
//   state_status, zip_code_status, country_code_status
//   91-113: guid, street_address, street_name, dependent_street_name,
//   house_number, subpremise, premise_name, street_address_status,
//   street_name_status, dependent_street_name_status, house_number_status,
//   subpremise_status, premise_name_status, dependent_locality, city, state,
//   zip_code, country_code, dependent_locality_status, city_status,
//   state_status, zip_code_status, country_code_status, apartment_number,
//   floor, apartment_number_status, floor_status 116: - (deleted)
//
// - autofill_profile_emails: autofill profile emails
//   40-113: guid, email
//   116: - (deleted)
//
// - autofill_profile_names: autofill profile names
//   40-56: guid, first_name, middle_name, last_name
//   ??-58: guid, first_name, middle_name, last_name, full_name
//
// - autofill_profile_phones: autofill profile phones
//   40-48: guid, type, number
//   ??-113: guid, number
//   116: - (deleted)
//
// - autofill_profiles_trash: autofill entries in trash
//   40-98: guid
//   100: - (deleted)
//
// - credit_cards: credit card entries
//   40-48: guid, name_on_card, expiration_month, expiration_year,
//   card_number_encrypted, date_modified
//   ??-55: guid, name_on_card, expiration_month, expiration_year,
//   card_number_encrypted, date_modified, origin
//
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

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

    is_instance_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load autofill entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_web_data::_load_autofill_entries (mobius::core::database::database &db)
{
    mobius::core::database::statement stmt;

    if (schema_version_ < 55)
    {
        stmt = db.new_statement (
            "SELECT a.name, a.value, a.count, d.date_created, NULL "
            "FROM autofill a, autofill_dates d "
            "WHERE a.pair_id = d.pair_id"
        );
    }

    else
    {
        stmt = db.new_statement (
            "SELECT name, value, count, date_created, date_last_used "
            "FROM autofill"
        );
    }

    while (stmt.fetch_row ())
    {
        autofill_entry entry;
        entry.idx = autofill_entries_.size ();
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

} // namespace mobius::extension::app::chromium
