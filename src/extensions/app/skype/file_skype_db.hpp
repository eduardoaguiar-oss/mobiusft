#ifndef MOBIUS_EXTENSION_APP_SKYPE_FILE_SKYPE_DB_HPP
#define MOBIUS_EXTENSION_APP_SKYPE_FILE_SKYPE_DB_HPP

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
#include <mobius/core/database/database.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/reader.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace mobius::extension::app::skype
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief skype.db file decoder
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_skype_db
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Contact structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct contact
    {
        // @brief Record index number
        std::uint64_t idx = 0;

        // @brief About Me
        std::string about_me;

        // @brief Assigned Phonelabel 1
        std::int64_t assigned_phonelabel_1;

        // @brief Assigned Phonelabel 2
        std::int64_t assigned_phonelabel_2;

        // @brief Assigned Phonelabel 3
        std::int64_t assigned_phonelabel_3;

        // @brief Assigned Phonenumber 1
        std::string assigned_phonenumber_1;

        // @brief Assigned Phonenumber 2
        std::string assigned_phonenumber_2;

        // @brief Assigned Phonenumber 3
        std::string assigned_phonenumber_3;

        // @brief Authorized
        mobius::core::bytearray authorized;

        // @brief Avatar Downloaded From
        std::string avatar_downloaded_from;

        // @brief Avatar File Path
        std::string avatar_file_path;

        // @brief Avatar Url
        std::string avatar_url;

        // @brief Birthday
        std::string birthday;

        // @brief Blocked
        bool blocked;

        // @brief City
        std::string city;

        // @brief Contact Type
        std::int64_t contact_type;

        // @brief Country
        std::string country;

        // @brief Display Name
        std::string display_name;

        // @brief Full Name
        std::string full_name;

        // @brief Gender
        std::int64_t gender;

        // @brief Homepage
        std::string homepage;

        // @brief Is Buddy
        bool is_buddy;

        // @brief Is Favorite
        bool is_favorite;

        // @brief Is Suggested
        bool is_suggested;

        // @brief Mood
        std::string mood;

        // @brief Mri
        std::string mri;

        // @brief Phone Number Home
        std::string phone_number_home;

        // @brief Phone Number Mobile
        std::string phone_number_mobile;

        // @brief Phone Number Office
        std::string phone_number_office;

        // @brief Province
        std::string province;

        // @brief Recommendation Json
        std::string recommendation_json;

        // @brief Recommendation Rank
        std::int64_t recommendation_rank;

        // @brief Unistore Version
        std::int64_t unistore_version;

        // @brief Update Version
        std::int64_t update_version;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_skype_db (const mobius::core::io::reader &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if stream is an instance of skype.db file
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
    // @brief Get contacts
    // @return Vector of contacts
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<contact>
    get_contacts () const
    {
        return contacts_;
    }

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

    // @brief Schema version
    std::uint32_t schema_version_ = 0;

    // @brief Contacts
    std::vector<contact> contacts_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_contacts (mobius::core::database::database &);
};

} // namespace mobius::extension::app::skype

#endif
