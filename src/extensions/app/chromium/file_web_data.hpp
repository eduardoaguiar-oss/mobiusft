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
#include <mobius/core/io/reader.hpp>
#include <mobius/core/pod/map.hpp>
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
    // @brief Autofill profile structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct autofill_profile
    {
        // @brief Name
        std::string name;

        // @brief Email
        std::string email;

        // @brief Phone
        std::string phone;

        // @brief Address
        std::string address;

        // @brief City
        std::string city;

        // @brief State
        std::string state;

        // @brief Zip code
        std::string zip_code;

        // @brief Country
        std::string country;

        // @brief Date last used
        mobius::core::datetime::datetime date_last_used;

        // @brief Date created
        mobius::core::datetime::datetime date_created;

        // @brief Date modified
        mobius::core::datetime::datetime date_modified;

        // @brief Company name
        std::string company_name;

        // @brief Count
        std::uint32_t count = 0;

        // @brief Is deleted
        bool is_deleted = false;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Credit Card structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct credit_card
    {
        // @brief Number
        std::string number;

        // @brief Expiration month
        std::uint32_t expiration_month = 0;

        // @brief Expiration year
        std::uint32_t expiration_year = 0;

        // @brief CVV
        std::string cvv;

        // @brief Name
        std::string name;

        // @brief Address
        std::string address;

        // @brief Date last used
        mobius::core::datetime::datetime date_last_used;

        // @brief Date modified
        mobius::core::datetime::datetime date_modified;

        // @brief Card number encrypted
        mobius::core::bytearray card_number_encrypted;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_web_data (const mobius::core::io::reader &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if stream is an instance of web data file
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const noexcept { return is_instance_; }

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
    // @brief Get credit card entries
    // @return Vector of credit card entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<credit_card>
    get_credit_card_entries () const
    {
        return credit_cards_;
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

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

    // @brief Schema version
    std::uint32_t schema_version_ = 0;

    // @brief Autofill entries
    std::vector<autofill_entry> autofill_entries_;

    // @brief Credit card entries
    std::vector<credit_card> credit_cards_;

    // @brief Autofill profiles
    std::vector<autofill_profile> autofill_profiles_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_autofill_entries (mobius::core::database::database &);
};

} // namespace mobius::extension::app::chromium

#endif
