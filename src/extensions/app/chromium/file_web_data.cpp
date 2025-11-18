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
#include <unordered_map>
#include <unordered_set>
#include "common.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
// @see https://atropos4n6.com/other/chrome-web-data-forensics/
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Web Data file tables
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// - autofill: autofill entries
//      - count: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - date_created: 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - date_last_used: 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - name: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - pair_id: 40, 43, 45, 48, 52
//      - value: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - value_lower: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//
// - autofill_dates: autofill entry dates
//      - date_created: 40, 43, 45, 48, 52
//      - pair_id: 40, 43, 45, 48, 52
//
// - autofill_profiles: autofill profiles
//      - address_line_1: 40, 43, 45, 48, 52
//      - address_line_2: 40, 43, 45, 48, 52
//      - city: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - company_name: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - country: 40, 43, 45, 48, 52
//      - country_code: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - date_modified: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - dependent_locality: 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - disallow_settings_visible_updates: 96-98, 100, 104, 107-113
//      - guid: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - is_client_validity_states_updated: 80-84, 86-88, 90-92, 96-98
//      - label: 96-98, 100, 104, 107-113
//      - language_code: 56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - origin: 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - sorting_code: 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - state: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - street_address: 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - use_count: 61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - use_date: 61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - validity_bitfield: 76-78, 80-84, 86-88, 90-92, 96-98
//      - zipcode: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//
// - autofill_profile_addresses: autofill profile addresses
//      - apartment_number: 91-92, 96-98, 100, 104, 107-113
//      - apartment_number_status: 91-92, 96-98, 100, 104, 107-113
//      - city: 90-92, 96-98, 100, 104, 107-113
//      - city_status: 90-92, 96-98, 100, 104, 107-113
//      - country_code: 90-92, 96-98, 100, 104, 107-113
//      - country_code_status: 90-92, 96-98, 100, 104, 107-113
//      - dependent_locality: 90-92, 96-98, 100, 104, 107-113
//      - dependent_locality_status: 90-92, 96-98, 100, 104, 107-113
//      - dependent_street_name: 88, 90-92, 96-98, 100, 104, 107-113
//      - dependent_street_name_status: 88, 90-92, 96-98, 100, 104, 107-113
//      - floor: 91-92, 96-98, 100, 104, 107-113
//      - floor_status: 91-92, 96-98, 100, 104, 107-113
//      - guid: 88, 90-92, 96-98, 100, 104, 107-113
//      - house_number: 88, 90-92, 96-98, 100, 104, 107-113
//      - house_number_status: 88, 90-92, 96-98, 100, 104, 107-113
//      - premise_name: 88, 90-92, 96-98, 100, 104, 107-113
//      - premise_name_status: 88, 90-92, 96-98, 100, 104, 107-113
//      - sorting_code: 90-92, 96-98, 100, 104, 107-113
//      - sorting_code_status: 90-92, 96-98, 100, 104, 107-113
//      - state: 90-92, 96-98, 100, 104, 107-113
//      - state_status: 90-92, 96-98, 100, 104, 107-113
//      - street_address: 88, 90-92, 96-98, 100, 104, 107-113
//      - street_address_status: 88, 90-92, 96-98, 100, 104, 107-113
//      - street_name: 88, 90-92, 96-98, 100, 104, 107-113
//      - street_name_status: 88, 90-92, 96-98, 100, 104, 107-113
//      - subpremise: 88, 90-92, 96-98, 100, 104, 107-113
//      - subpremise_status: 88, 90-92, 96-98, 100, 104, 107-113
//      - zip_code: 90-92, 96-98, 100, 104, 107-113
//      - zip_code_status: 90-92, 96-98, 100, 104, 107-113
//
// - autofill_profile_birthdates
//      - day: 104, 107-113
//      - guid: 104, 107-113
//      - month: 104, 107-113
//      - year: 104, 107-113
//
// - autofill_profile_emails: autofill profile emails
//      - email: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - guid: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//
// - autofill_profile_names: autofill profile names
//      - conjunction_last_name: 88, 90-92, 96-98, 100, 104, 107-113
//      - conjunction_last_name_status: 88, 90-92, 96-98, 100, 104, 107-113
//      - date_of_birth: 58, 65, 80, 88, 96
//      - first_last_name: 88, 90-92, 96-98, 100, 104, 107-113
//      - first_last_name_status: 88, 90-92, 96-98, 100, 104, 107-113
//      - first_name: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - first_name_status: 88, 90-92, 96-98, 100, 104, 107-113
//      - full_name: 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - full_name_status: 88, 90-92, 96-98, 100, 104, 107-113
//      - full_name_with_honorific_prefix: 92, 96-98, 100, 104, 107-113
//      - full_name_with_honorific_prefix_status: 92, 96-98, 100, 104, 107-113
//      - gender: 58, 65, 80, 88, 96
//      - guid: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - honorific_prefix: 88, 90-92, 96-98, 100, 104, 107-113
//      - honorific_prefix_status: 88, 90-92, 96-98, 100, 104, 107-113
//      - last_name: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - last_name_status: 88, 90-92, 96-98, 100, 104, 107-113
//      - middle_name: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - middle_name_status: 88, 90-92, 96-98, 100, 104, 107-113
//      - second_last_name: 88, 90-92, 96-98, 100, 104, 107-113
//      - second_last_name_status: 88, 90-92, 96-98, 100, 104, 107-113
//
// - autofill_profile_phones: autofill profile phones
//      - guid: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - number: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113
//      - type: 40, 43, 45, 48, 52
//
// - autofill_profile_usernames
//      - guid: 58, 65, 80, 88, 96
//      - username: 58, 65, 80, 88, 96
//
// - autofill_profiles_trash: autofill entries in trash
//      - guid: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98
//
// - credit_cards: credit card entries
//      - billing_address_id: 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - card_number_encrypted: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - date_modified: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - expiration_month: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - expiration_year: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - guid: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - name_on_card: 40, 43, 45, 48, 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - nickname: 87-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - origin: 52, 55-56, 58, 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - use_count: 61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - use_date: 61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143

// - credit_card_tags
//      - date_modified: 90, 92
//      - guid: 90, 92
//      - tag: 90, 92
//
// - credit_card_tags_v2
//      - date_modified: 83-84, 87-88
//      - guid: 83-84, 87-88
//      - tag: 83-84, 87-88
//
// - ibans: International Bank Account Numbers
//      - guid: 104, 107-113, 116-117
//      - nickname: 104, 107-113, 116-117
//      - use_count: 104, 107-113, 116-117
//      - use_date: 104, 107-113, 116-117
//      - value: 104, 107-113
//      - value_encrypted: 116-117
//
// - masked_credit_cards: masked credit card entries
//      - bank_name: 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - billing_address_id: 67, 70
//      - card_art_url: 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - card_benefit_source: 141, 143
//      - card_info_retrieval_enrollment_state: 135, 137-138, 140-141, 143
//      - card_issuer: 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - card_issuer_id: 108-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - exp_month: 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - exp_year: 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - id: 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - instrument_id: 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - last_four: 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - name_on_card: 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - network: 72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - nickname: 84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - product_description: 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - product_terms_url: 123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - status: 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-97
//      - type: 60-61, 64-65, 67, 70-71, 74, 76-78, 80-82
//      - virtual_card_enrollment_state: 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - virtual_card_enrollment_type: 111-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//
// - server_card_metadata
//      - billing_address_id: 71-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - id: 65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - use_count: 65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - use_date: 65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//
// - unmasked_credit_cards: unmasked credit card entries
//      - card_number_encrypted: 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - id: 60-61, 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - unmask_date: 64-65, 67, 70-72, 74, 76-78, 80-84, 86-88, 90-92, 96-98, 100, 104, 107-113, 116-117, 119-120, 122-123, 125, 127-128, 130, 132, 134-135, 137-138, 140-141, 143
//      - use_count: 64-65, 67, 70-72, 74, 76-78, 80-84
//      - use_date: 64-65, 67, 70-72, 74, 76-78, 80-84
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
    1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,
    31,  32,  33,  34,  35,  36,  37,  38,  39,  41,  42,  44,  46,  47,  49,
    50,  51,  53,  54,  57,  59,  62,  63,  66,  68,  69,  73,  75,  79,
    85,  89,  93,  94,  95,  99,  101, 102, 103, 105, 106, 114, 115, 118,
    121, 124, 126, 129, 131, 133, 136, 139, 142
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Last known schema version
// This constant represents the last schema version that is known and handled
// by the current implementation. Any schema version greater than this value
// will be considered unsupported and will trigger a warning in the log.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::int64_t LAST_KNOWN_SCHEMA_VERSION = 143;

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

    mobius::core::database::statement stmt = db.new_statement (
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
        "${dependent_locality:90-*}, "
        "${city:90-*}, "
        "${state:90-*}, "
        "${zip_code:90-*}, "
        "${country_code:90-*}, "
        "${dependent_locality_status:90-*}, "
        "${city_status:90-*}, "
        "${state_status:90-*}, "
        "${zip_code_status:90-*}, "
        "${country_code_status:90-*}, "
        "${apartment_number:91-*}, "
        "${floor:91-*}, "
        "${apartment_number_status:91-*}, "
        "${floor_status:91-*} "
        "FROM autofill_profile_addresses "
        "WHERE guid = ?",
        schema_version
    );

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
    mobius::core::database::statement stmt = db.new_statement (
        "SELECT email "
        "FROM autofill_profile_emails "
        "WHERE guid = ?",
        schema_version
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

    // Prepare statement to retrieve names from autofill_profile_names table
    mobius::core::database::statement stmt = db.new_statement (
        "SELECT first_name, "
        "middle_name, "
        "last_name, "
        "${full_name:58-*}, "
        "${honorific_prefix:88-*}, "
        "${first_last_name:88-*}, "
        "${conjunction_last_name:88-*}, "
        "${second_last_name:88-*}, "
        "${full_name_with_honorific_prefix:92-*} "
        "FROM autofill_profile_names "
        "WHERE guid = ?",
        schema_version
    );

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
    mobius::core::database::statement stmt = db.new_statement (
        "SELECT ${type:40-52}, "
        "number "
        "FROM autofill_profile_phones "
        "WHERE guid = ?",
        schema_version
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
            // Prepare statement to retrieve server card metadata from the
            // database
            mobius::core::database::statement stmt =
                db.new_statement (
                    "SELECT id, "
                    "${billing_address_id:71-*}, "
                    "use_count, "
                    "use_date "
                    "FROM server_card_metadata ",
                    schema_version
                );

            // Retrieve records from server_card_metadata table
            while (stmt.fetch_row ())
            {
                server_card_metadata metadata;

                metadata.id = stmt.get_column_string (0);
                metadata.billing_address_id = stmt.get_column_string (1);
                metadata.use_count = stmt.get_column_int64 (2);
                metadata.use_date = get_datetime (stmt.get_column_int64 (3));

                // Add metadata to the map
                server_card_metadata_map[metadata.id] = metadata;
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
            mobius::core::database::statement stmt =
                db.new_statement (
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
        mobius::core::database::statement stmt =
            db.new_statement (
                "SELECT guid, "
                "company_name, "
                "${address_line_1:40-52}, "
                "${address_line_2:40-52}, "
                "${street_address:55-*}, "
                "${dependent_locality:55-*}, "
                "city, "
                "state, "
                "zipcode, "
                "country_code, "
                "${country:40-52}, "
                "${date_modified:40-*}, "
                "${origin:55-*}, "
                "${language_code:56-*}, "
                "${use_count:61-*}, "
                "${use_date:61-*} "
                "FROM autofill_profiles p",
                schema_version_
            );

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
            profile.names =
                _get_profile_names (db, schema_version_, profile.guid);
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
        mobius::core::database::statement stmt =
            db.new_statement (
                "SELECT guid, "
                "name_on_card, "
                "expiration_month, "
                "expiration_year, "
                "card_number_encrypted, "
                "${date_modified:30-*}, "
                "${origin:52-*}, "
                "${use_count:61-*}, "
                "${use_date:61-*}, "
                "${billing_address_id:66-*}, "
                "${nickname:87-*} "
                "FROM credit_cards",
                schema_version_
            );

        // Retrieve records from credit_cards table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            credit_card card;

            card.idx = idx++;
            card.guid = stmt.get_column_string (0);
            card.expiration_month = stmt.get_column_int64 (2);
            card.expiration_year = stmt.get_column_int64 (3);
            card.card_number_encrypted = stmt.get_column_bytearray (4);
            card.date_modified = get_datetime (stmt.get_column_int64 (5));
            card.origin = stmt.get_column_string (6);
            card.use_count = stmt.get_column_int64 (7);
            card.use_date = get_datetime (stmt.get_column_int64 (8));
            card.billing_address_id = stmt.get_column_string (9);
            card.nickname = stmt.get_column_string (10);

            // Set name on card
            auto name_on_card = stmt.get_column_bytearray (1);

            if (name_on_card.startswith ("v10") ||
                name_on_card.startswith ("v20"))
                card.name_on_card_encrypted = name_on_card;

            else
                card.name_on_card = name_on_card.to_string ();

            // Add card to the list
            credit_cards_.emplace_back (std::move (card));
        }
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
        auto stmt = db.new_statement (
            "SELECT ${bank_name:74-*}, "
            "${billing_address_id:67-70}, "
            "${card_art_url:96-*}, "
            "${card_benefit_source:141-*}, "
            "${card_info_retrieval_enrollment_state:135-*}, "
            "${card_issuer:86-*}, "
            "${card_issuer_id:108-*}, "
            "exp_month, "
            "exp_year, "
            "id, "
            "${instrument_id:90-*}, "
            "last_four, "
            "name_on_card, "
            "${network:72-*}, "
            "${nickname:84-*}, "
            "${product_description:102-*}, "
            "${product_terms_url:125-*}, "
            "${status:60-97}, "
            "${type:60-82}, "
            "${virtual_card_enrollment_state:96-*}, "
            "${virtual_card_enrollment_type:111-*} "
            "FROM masked_credit_cards",
            schema_version_
        );

        // Retrieve records from masked_credit_cards and unmasked_credit_cards
        // tables
        auto idx = 0;

        while (stmt.fetch_row ())
        {
            credit_card card;

            // Fill data from masked_credit_cards table
            card.idx = idx++;
            card.bank_name = stmt.get_column_string (0);
            card.billing_address_id = stmt.get_column_string (1);
            card.card_art_url = stmt.get_column_string (2);
            card.card_benefit_source = stmt.get_column_string (3);
            card.card_info_retrieval_enrollment_state =
                stmt.get_column_int64 (4);
            card.card_issuer = stmt.get_column_string (5);
            card.card_issuer_id = stmt.get_column_int64 (6);
            card.expiration_month = stmt.get_column_int64 (7);
            card.expiration_year = stmt.get_column_int64 (8);
            card.id = stmt.get_column_string (9);
            card.instrument_id = stmt.get_column_string (10);
            card.last_four = stmt.get_column_string (11);
            card.name_on_card = stmt.get_column_string (12);
            card.network = stmt.get_column_string (13);
            card.nickname = stmt.get_column_string (14);
            card.product_description = stmt.get_column_string (15);
            card.product_terms_url = stmt.get_column_string (16);
            card.status = stmt.get_column_string (17);
            card.type = stmt.get_column_string (18);
            card.virtual_card_enrollment_state = stmt.get_column_int64 (19);
            card.virtual_card_enrollment_type = stmt.get_column_int64 (20);

            if (!card.last_four.empty ())
                card.card_number =
                    std::string ("**** **** **** ") + card.last_four;

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
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::chromium
