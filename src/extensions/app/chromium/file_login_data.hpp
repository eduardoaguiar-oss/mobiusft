#ifndef MOBIUS_EXTENSION_APP_CHROMIUM_FILE_LOGIN_DATA_HPP
#define MOBIUS_EXTENSION_APP_CHROMIUM_FILE_LOGIN_DATA_HPP

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
#include <mobius/core/database/database.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/reader.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Login Data file decoder
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_login_data
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Login structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct login
    {
        // @brief Record Index
        std::uint64_t idx = 0;

        // @brief Schema version
        std::uint64_t schema_version = 0;

        // @brief Action URL
        std::string action_url;

        // @brief Actor Login Approved
        bool actor_login_approved;

        // @brief Avatar URL
        std::string avatar_url;

        // @brief Blacklisted by user
        bool blacklisted_by_user = false;

        // @brief Date created
        mobius::core::datetime::datetime date_created;

        // @brief Date last filled
        mobius::core::datetime::datetime date_last_filled;

        // @brief Date last used
        mobius::core::datetime::datetime date_last_used;

        // @brief Date password modified
        mobius::core::datetime::datetime date_password_modified;

        // @brief Date received
        mobius::core::datetime::datetime date_received;

        // @brief Date synced
        mobius::core::datetime::datetime date_synced;

        // @brief Display name
        std::string display_name;

        // @brief Federation URL
        std::string federation_url;

        // @brief Generation upload status
        std::uint64_t generation_upload_status = 0;

        // @brief Icon URL
        std::string icon_url;

        // @brief ID
        std::string id;

        // @brief Is zero click
        bool is_zero_click = false;

        // @brief Keychain identifier
        std::string keychain_identifier;

        // @brief Origin URL
        std::string origin_url;

        // @brief Password element
        std::string password_element;

        // @brief Password type
        std::uint64_t password_type = 0;

        // @brief Password value
        mobius::core::bytearray password_value;

        // @brief Preferred
        bool preferred = false;

        // @brief Scheme
        std::string scheme;

        // @brief Sender email
        std::string sender_email;

        // @brief Sender name
        std::string sender_name;

        // @brief Sender profile image URL
        std::string sender_profile_image_url;

        // @brief Sharing notification displayed
        bool sharing_notification_displayed = false;

        // @brief Signon realm
        std::string signon_realm;

        // @brief Skip zero click
        bool skip_zero_click = false;

        // @brief SSL valid
        bool ssl_valid = false;

        // @brief Submit element
        std::string submit_element;

        // @brief Times used
        std::uint64_t times_used = 0;

        // @brief Use additional auth
        bool use_additional_auth = false;

        // @brief Username element
        std::string username_element;

        // @brief Username value
        std::string username_value;

        // @brief File object
        mobius::core::io::file f;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_login_data (const mobius::core::io::reader &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if stream is an instance of Login Data file
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
    // @brief Get logins
    // @return Vector of logins
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<login>
    get_logins () const
    {
        return logins_;
    }

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

    // @brief Schema version
    std::uint32_t schema_version_ = 0;

    // @brief Logins
    std::vector<login> logins_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_logins (mobius::core::database::database &);
};

} // namespace mobius::extension::app::chromium

#endif
