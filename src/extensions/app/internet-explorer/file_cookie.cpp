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
#include "file_cookie.hpp"
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/string_functions.hpp>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Cookie flags
// @see https://github.com/wine-mirror/wine/blob/master/include/wininet.h
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
constexpr std::uint32_t INTERNET_COOKIE_IS_SECURE = 0x00000001;       // This is a secure cookie.
constexpr std::uint32_t INTERNET_COOKIE_IS_SESSION = 0x00000002;      // This is a session cookie.
constexpr std::uint32_t INTERNET_COOKIE_THIRD_PARTY = 0x00000010;     // This cookie is a third-party cookie.
constexpr std::uint32_t INTERNET_COOKIE_PROMPT_REQUIRED = 0x00000020; // Prompt is required for this cookie.
constexpr std::uint32_t INTERNET_COOKIE_EVALUATE_P3P = 0x00000040;    // Evaluate P3P policy for this cookie.
constexpr std::uint32_t INTERNET_COOKIE_APPLY_P3P = 0x00000080;       // Apply P3P policy to this cookie.
constexpr std::uint32_t INTERNET_COOKIE_P3P_ENABLED = 0x00000100;     // P3P is enabled for this cookie.
constexpr std::uint32_t INTERNET_COOKIE_IS_RESTRICTED =
    0x00000200;                                                  // This cookie is restricted to first-party contexts.
constexpr std::uint32_t INTERNET_COOKIE_IE6 = 0x00000400;        // This cookie is in IE6 mode.
constexpr std::uint32_t INTERNET_COOKIE_IS_LEGACY = 0x00000800;  // This is a legacy cookie.
constexpr std::uint32_t INTERNET_COOKIE_NON_SCRIPT = 0x00001000; // This cookie is non-scriptable.
constexpr std::uint32_t INTERNET_COOKIE_HTTPONLY = 0x00002000;   // This is an HTTP-only cookie.

} // namespace

namespace mobius::extension::app::internet_explorer
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader The reader object for the cookie file
//
// Each cookie is represented by a sequence of lines in the file, with the following order:
//   - name
//   - value
//   - domain
//   - flags
//   - expiration time (low and high parts)
//   - creation time (low and high parts)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_cookie::file_cookie (const mobius::core::io::reader &reader)
{
    mobius::core::log log (__FILE__, __func__);

    if (!reader || reader.get_size () < 8)
        return;

    auto fp = mobius::core::io::line_reader (reader, "utf-8", "\n");

    int state = 0;
    std::uint64_t timestamp = 0;
    std::string line;
    cookie c;

    while (fp.read (line))
    {
        line = mobius::core::string::rstrip (line);

        switch (state)
        {
            case 0:
                c = cookie ();
                c.name = line;
                state++;
                break;
            case 1:
                c.value = line;
                state++;
                break;
            case 2:
                c.domain = mobius::core::string::rstrip (line, "/");
                state++;
                break;
            case 3:
                c.flags = std::stoul (line);
                c.is_secure = (c.flags & INTERNET_COOKIE_IS_SECURE);
                c.is_session = (c.flags & INTERNET_COOKIE_IS_SESSION);
                c.is_third_party = (c.flags & INTERNET_COOKIE_THIRD_PARTY);
                c.is_restricted = (c.flags & INTERNET_COOKIE_IS_RESTRICTED);
                c.is_ie6 = (c.flags & INTERNET_COOKIE_IE6);
                c.is_legacy = (c.flags & INTERNET_COOKIE_IS_LEGACY);
                c.is_httponly = (c.flags & INTERNET_COOKIE_HTTPONLY);
                state++;
                break;
            case 4:
                timestamp = std::stoul (line);
                state++;
                break;
            case 5:
                timestamp = (static_cast<std::uint64_t> (std::stoul (line)) << 32) | timestamp;
                c.expiration_time = mobius::core::datetime::new_datetime_from_nt_timestamp (timestamp);
                state++;
                break;
            case 6:
                timestamp = std::stoul (line);
                state++;
                break;
            case 7:
                timestamp = (static_cast<std::uint64_t> (std::stoul (line)) << 32) | timestamp;
                c.creation_time = mobius::core::datetime::new_datetime_from_nt_timestamp (timestamp);
                cookies_.push_back (c);
                is_instance_ = true;
                state++;
                break;
            case 8:
                if (line == "*")
                    state = 0;
                break;
        }
    }

    // If either at least one cookie was successfully read or the file was read until the end, mark the instance as valid
    is_instance_ = true;
}

} // namespace mobius::extension::app::internet_explorer
