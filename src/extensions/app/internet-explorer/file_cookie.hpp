#ifndef MOBIUS_EXTENSION_APP_INTERNET_EXPLORER_FILE_COOKIE_HPP
#define MOBIUS_EXTENSION_APP_INTERNET_EXPLORER_FILE_COOKIE_HPP

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
#include <mobius/core/io/reader.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/line_reader.hpp>
#include <string>
#include <vector>

namespace mobius::extension::app::internet_explorer
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Internet Explorer cookie file decoder
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_cookie
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Cookie data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct cookie
    {
        // @brief Cookie name
        std::string name;

        // @brief Cookie value
        std::string value;

        // @brief Cookie domain
        std::string domain;

        // @brief Cookie flags
        std::uint32_t flags = 0;

        // @brief Cookie is secure
        bool is_secure = false;

        // @brief Cookie is a session cookie
        bool is_session = false;

        // @brief Cookie is a third-party cookie
        bool is_third_party = false;

        // @brief Cookie is restricted to first-party contexts
        bool is_restricted = false;

        // @brief Cookie is in IE6 mode
        bool is_ie6 = false;

        // @brief Cookie is a legacy cookie
        bool is_legacy = false;

        // @brief Cookie is HTTP-only
        bool is_httponly = false;

        // @brief Cookie creation time
        mobius::core::datetime::datetime creation_time;

        // @brief Cookie expiration time
        mobius::core::datetime::datetime expiration_time;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_cookie (const mobius::core::io::reader &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if stream is an instance of cookie file
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const noexcept
    {
        return is_instance_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get cookies
    // @return Cookies
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<cookie>
    get_cookies () const
    {
        return cookies_;
    }

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

    // @brief Cookies
    std::vector<cookie> cookies_;
};

} // namespace mobius::extension::app::internet_explorer

#endif
