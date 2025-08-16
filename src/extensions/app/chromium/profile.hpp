#ifndef MOBIUS_EXTENSION_APP_CHROMIUM_PROFILE_HPP
#define MOBIUS_EXTENSION_APP_CHROMIUM_PROFILE_HPP

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
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/folder.hpp>
#include <mobius/core/io/reader.hpp>
#include <mobius/core/pod/map.hpp>
#include <cstdint>
#include <memory>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "file_cookies.hpp"
#include "file_history.hpp"
#include "file_login_data.hpp"
#include "file_web_data.hpp"

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Chromium profile class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class profile
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Datatypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    using autofill_profile = file_web_data::autofill_profile;
    using cookie = file_cookies::cookie;
    using credit_card = file_web_data::credit_card;
    using download = file_history::download;
    using history_entry = file_history::history_entry;
    using login = file_login_data::login;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Account structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct account
    {
        // @brief GUID
        std::string id;

        // @brief Name
        std::string name;

        // @brief Email
        std::vector<std::string> emails;

        // @brief Phone number
        std::vector<std::string> phone_numbers;

        // @brief Organizations
        std::vector<std::string> organizations;

        // @brief Address
        std::vector<std::string> addresses;

        // @brief Names
        std::vector<std::string> names;

        // @brief Metadata
        mobius::core::pod::map metadata;

        // @brief Source file
        mobius::core::io::file f;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Autofill structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct autofill
    {
        // @brief Record number
        std::uint64_t idx = 0;

        // #brief DB Schema version
        std::int64_t schema_version = 0;

        // @brief Name
        std::string name;

        // @brief Encrypted value
        mobius::core::bytearray encrypted_value;

        // @brief Value
        std::string value;

        // @brief Count
        std::uint32_t count = 0;

        // @brief Date created
        mobius::core::datetime::datetime date_created;

        // @brief Date last used
        mobius::core::datetime::datetime date_last_used;

        // @brief Metadata
        mobius::core::pod::map metadata;

        // @brief Is encrypted
        bool is_encrypted = false;

        // @brief Source file
        mobius::core::io::file f;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    profile ();
    profile (const profile &) noexcept = default;
    profile (profile &&) noexcept = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    profile &operator= (const profile &) noexcept = default;
    profile &operator= (profile &&) noexcept = default;
    operator bool () const noexcept;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string get_username () const;
    void set_username (const std::string &);
    mobius::core::io::folder get_folder () const;
    std::string get_path () const;
    std::string get_app_id () const;
    std::string get_app_name () const;
    std::string get_profile_name () const;
    std::string get_created_by_version () const;
    mobius::core::datetime::datetime get_creation_time () const;
    mobius::core::datetime::datetime get_last_modified_time () const;
    mobius::core::datetime::datetime get_last_engagement_time () const;
    std::vector<account> get_accounts () const;
    std::size_t size_accounts () const;
    std::vector<autofill> get_autofill_entries () const;
    std::size_t size_autofill_entries () const;
    std::vector<autofill_profile> get_autofill_profiles () const;
    std::size_t size_autofill_profiles () const;
    std::vector<cookie> get_cookies () const;
    std::size_t size_cookies () const;
    std::vector<credit_card> get_credit_cards () const;
    std::size_t size_credit_cards () const;
    std::vector<download> get_downloads () const;
    std::size_t size_downloads () const;
    std::vector<history_entry> get_history_entries () const;
    std::size_t size_history_entries () const;
    std::vector<login> get_logins () const;
    std::size_t size_logins () const;

    void set_folder (const mobius::core::io::folder &);
    void add_bookmarks_file (const mobius::core::io::file &);
    void add_cookies_file (const mobius::core::io::file &);
    void add_history_file (const mobius::core::io::file &);
    void add_login_data_file (const mobius::core::io::file &);
    void add_preferences_file (const mobius::core::io::file &);
    void add_web_data_file (const mobius::core::io::file &);

  private:
    // @brief Forward declaration
    class impl;

    // @brief Implementation pointer
    std::shared_ptr<impl> impl_;
};

} // namespace mobius::extension::app::chromium

#endif
