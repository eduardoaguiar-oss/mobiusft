// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
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
#include <mobius/core/charset.hpp>
#include <mobius/core/crypt/hmac.hpp>
#include <mobius/core/os/win/hash_msdcc1.hpp>
#include <mobius/core/os/win/hash_msdcc2.hpp>
#include <mobius/core/string_functions.hpp>

namespace mobius::core::os::win
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Generate MSDCC2 hash
// @param password Password
// @param username User name
// @param iterations Number of iterations
// @return MSDCC2 hash
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
hash_msdcc2 (const std::string &password, const std::string &username,
             std::uint32_t iterations)
{
    const mobius::core::bytearray pass = hash_msdcc1 (password, username);
    const mobius::core::bytearray salt = mobius::core::conv_charset (
        mobius::core::string::tolower (username), "UTF-8", "UTF-16LE");

    mobius::core::crypt::hmac hmac_sha1 ("sha1", pass);
    hmac_sha1.update (salt);
    hmac_sha1.update ({0, 0, 0, 1});

    mobius::core::bytearray temp = hmac_sha1.get_digest ();
    mobius::core::bytearray out = temp.slice (0, 15);

    for (std::uint32_t i = 1; i < iterations; i++)
    {
        mobius::core::crypt::hmac hmac_sha1 ("sha1", pass);
        hmac_sha1.update (temp);
        temp = hmac_sha1.get_digest ();
        out ^= temp.slice (0, 15);
    }

    return out;
}

} // namespace mobius::core::os::win
