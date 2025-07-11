#ifndef MOBIUS_EXTENSION_APP_CHROMIUM_FILE_WEB_DATA_HPP
#define MOBIUS_EXTENSION_APP_CHROMIUM_FILE_WEB_DATA_HPP

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
#include <mobius/core/bytearray.hpp>
#include <mobius/core/database/database.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/reader.hpp>
#include <string>
#include <vector>

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Web Data file decoder
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_web_data
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Autofill entry structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct autofill_entry
    {
        // @brief Record Index
        std::uint64_t idx = 0;

        // @brief Name
        std::string name;

        // @brief Value
        mobius::core::bytearray value;

        // @brief Date created
        mobius::core::datetime::datetime date_created;

        // @brief Date last used
        mobius::core::datetime::datetime date_last_used;

        // @brief Count
        std::uint32_t count = 0;

        // @brief Is encrypted
        bool is_encrypted = false;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Autofill profile address structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct autofill_profile_address
    {
        // @brief Address line 1
        std::string address_line_1;

        // @brief Address line 2
        std::string address_line_2;

        // @brief Street address
        std::string street_address;

        // @brief Street name
        std::string street_name;

        // @brief Dependent street name
        std::string dependent_street_name;

        // @brief House number
        std::string house_number;

        // @brief Subpremise
        std::string subpremise;

        // @brief Premise name
        std::string premise_name;

        // @brief Dependent locality
        std::string dependent_locality;

        // @brief City
        std::string city;

        // @brief State
        std::string state;

        // @brief Zip code
        std::string zip_code;

        // @brief Country code
        std::string country_code;

        // @brief Apartment number
        std::string apartment_number;

        // @brief Floor
        std::string floor;

        // @brief Country
        std::string country;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Autofill profile name structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct autofill_profile_name
    {
        // @brief First name
        std::string first_name;

        // @brief Middle name
        std::string middle_name;

        // @brief Last name
        std::string last_name;

        // @brief Full name
        std::string full_name;

        // @brief Honorific prefix
        std::string honorific_prefix;

        // @brief First last name
        std::string first_last_name;

        // @brief Conjunction last name
        std::string conjunction_last_name;

        // @brief Second last name
        std::string second_last_name;

        // @brief Full name with honorific prefix
        std::string full_name_with_honorific_prefix;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Autofill profile phone structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct autofill_profile_phone
    {
        // @brief Phone type
        std::string type;

        // @brief Phone number
        std::string number;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Autofill profile structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct autofill_profile
    {
        // @brief Record Index
        std::uint64_t idx = 0;

        // @brief GUID
        std::string guid;

        // @brief company name
        std::string company_name;

        // @brief Use count
        std::uint32_t use_count = 0;

        // @brief Last time used
        mobius::core::datetime::datetime date_last_used;

        // @brief Date modified
        mobius::core::datetime::datetime date_modified;

        // @brief Origin
        std::string origin;

        // @brief Language code
        std::string language_code;

        // @brief Names
        std::vector<autofill_profile_name> names;

        // @brief Addresses
        std::vector<autofill_profile_address> addresses;

        // @brief Emails
        std::vector<std::string> emails;

        // @brief Phones
        std::vector<autofill_profile_phone> phones;

        // @brief Is in trash
        bool is_in_trash = false;

        // @brief Source file
        mobius::core::io::file f;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Credit Card Tag structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct credit_card_tag
    {
        // @brief Date modified
        mobius::core::datetime::datetime date_modified;

        // @brief Encrypted tag
        mobius::core::bytearray tag_encrypted;

        // @brief Tag
        std::string tag;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Credit Card structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct credit_card
    {
        // @brief Record Index
        std::uint64_t idx = 0;

        // @brief GUID
        std::string guid;

        // @brief Encrypted card number
        mobius::core::bytearray card_number_encrypted;

        // @brief Encrypted CVC
        mobius::core::bytearray cvc_encrypted;

        // @brief Encrypted name on card
        mobius::core::bytearray name_on_card_encrypted;

        // @brief Encrypted IBAN
        mobius::core::bytearray iban_encrypted;

        // @brief IBAN
        std::string iban;

        // @brief Card number
        std::string card_number;

        // @brief CVC
        std::string cvc;

        // @brief Last four digits
        std::string last_four;

        // @brief Expiration month
        std::uint32_t expiration_month = 0;

        // @brief Expiration year
        std::uint32_t expiration_year = 0;

        // @brief Name on card
        std::string name_on_card;

        // @brief Date modified
        mobius::core::datetime::datetime date_modified;

        // @brief Unmask date
        mobius::core::datetime::datetime unmask_date;

        // @brief Use count
        std::uint32_t use_count = 0;

        // @brief Use date
        mobius::core::datetime::datetime use_date;

        // @brief ID
        std::string id;

        // @brief Billing address ID
        std::string billing_address_id;

        // @brief Nickname
        std::string nickname;

        // @brief Origin
        std::string origin;

        // @brief Bank name
        std::string bank_name;

        // @brief Card art URL
        std::string card_art_url;

        // @brief Card info retrieval enrollment state
        std::string card_info_retrieval_enrollment_state;

        // @brief Card issuer
        std::string card_issuer;

        // @brief Card issuer ID
        std::uint64_t card_issuer_id = 0;

        // @brief Instrument ID
        std::string instrument_id;

        // @brief Network
        std::string network;

        // @brief Product description
        std::string product_description;

        // @brief Product terms URL
        std::string product_terms_url;

        // @brief Status
        std::string status;

        // @brief Type
        std::string type;

        // @brief Virtual card enrollment state
        std::string virtual_card_enrollment_state;

        // @brief Virtual card enrollment type
        std::string virtual_card_enrollment_type;

        // @brief Tags
        std::vector<credit_card_tag> tags;

        // @brief Source file
        mobius::core::io::file f;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_web_data (const mobius::core::io::reader &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if stream is an instance of web data file
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const noexcept
    {
        return is_instance_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get schema version
    // @return Schema version
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_schema_version () const
    {
        return schema_version_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get autofill entries
    // @return Vector of autofill entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<autofill_entry>
    get_autofill_entries () const
    {
        return autofill_entries_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get autofill profiles
    // @return Vector of autofill profiles
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<autofill_profile>
    get_autofill_profiles () const
    {
        return autofill_profiles_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get credit cards
    // @return Vector of credit cards
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<credit_card>
    get_credit_cards () const
    {
        return credit_cards_;
    }

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

    // @brief Schema version
    std::uint32_t schema_version_ = 0;

    // @brief Autofill entries
    std::vector<autofill_entry> autofill_entries_;

    // @brief Autofill profiles
    std::vector<autofill_profile> autofill_profiles_;

    // @brief Credit cards
    std::vector<credit_card> credit_cards_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_autofill_entries (mobius::core::database::database &);
    void _load_autofill_profiles (mobius::core::database::database &);
    void _load_credit_cards (mobius::core::database::database &);
    void _load_masked_credit_cards (mobius::core::database::database &);
};

} // namespace mobius::extension::app::chromium

#endif
