#ifndef MOBIUS_EXTENSION_APP_CHROMIUM_FILE_COOKIES_HPP
#define MOBIUS_EXTENSION_APP_CHROMIUM_FILE_COOKIES_HPP

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

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Cookies file decoder
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_cookies
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Cookie structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct cookie
    {
        // @brief Record Index
        std::uint64_t idx = 0;

        // @brief Browser provenance
        std::string browser_provenance;

        // @brief Creation UTC
        mobius::core::datetime::datetime creation_utc;

        // @brief Encrypted value
        mobius::core::bytearray encrypted_value;

        // @brief Expires UTC
        mobius::core::datetime::datetime expires_utc;

        // @brief Has cross site ancestor
        bool has_cross_site_ancestor = false;

        // @brief Has expires
        bool has_expires = false;

        // @brief Host key
        std::string host_key;

        // @brief HTTP only
        bool httponly = false;

        // @brief Is HTTP only
        bool is_httponly = false;

        // @brief Is persistent
        bool is_persistent = false;

        // @brief Is same party
        bool is_same_party = false;

        // @brief Is secure
        bool is_secure = false;

        // @brief Last access UTC
        mobius::core::datetime::datetime last_access_utc;

        // @brief Last update UTC
        mobius::core::datetime::datetime last_update_utc;

        // @brief Name
        std::string name;

        // @brief Path
        std::string path;

        // @brief Persistent
        bool persistent = false;

        // @brief Priority
        std::uint8_t priority = 0;

        // @brief SameSite attribute
        bool samesite = false;

        // @brief Secure
        bool secure = false;

        // @brief Source port
        std::uint16_t source_port = 0;

        // @brief Source scheme
        std::string source_scheme;

        // @brief Source type
        std::uint8_t source_type = 0;

        // @brief Top frame site key
        std::string top_frame_site_key;

        // @brief Value
        mobius::core::bytearray value;

        // @brief File object
        mobius::core::io::file f;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_cookies (const mobius::core::io::reader &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if stream is an instance of Cookies file
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
    // @brief Get cookies
    // @return Vector of cookies
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<cookie>
    get_cookies () const
    {
        return cookies_;
    }

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

    // @brief Schema version
    std::uint32_t schema_version_ = 0;

    // @brief Cookies
    std::vector<cookie> cookies_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_cookies (mobius::core::database::database &);
};

} // namespace mobius::extension::app::chromium

#endif
