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
#include <mobius/core/crypt/hmac.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/os/win/pbkdf2_hmac_ms.hpp>
#include <stdexcept>

namespace mobius::core::os::win
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief PBKDF2 key derivation function (with HMAC) - MS version
// @param password password
// @param salt salt
// @param count iterations count
// @param dklen derived key length in bytes
// @param hash_id hash algorithm (e.g. "md2", "md5", "sha1", "sha2-256")
// This function implements MS version, which is not compatible with RFC 2898
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
pbkdf2_hmac_ms (const mobius::core::bytearray &password,
                const mobius::core::bytearray &salt, std::uint32_t count,
                std::uint16_t dklen, const std::string &hash_id)
{
    // Validate input parameters
    if (dklen == 0)
        throw std::out_of_range (
            MOBIUS_EXCEPTION_MSG ("derived key length must be > 0"));

    // Calculate derived key
    std::uint32_t i = 1;
    mobius::core::bytearray dk;
    mobius::core::crypt::hmac hmac (hash_id, password);

    while (dk.size () < dklen)
    {
        hmac.reset ();
        hmac.update (salt);
        hmac.update ({static_cast<uint8_t> (i >> 24),
                      static_cast<uint8_t> (i >> 16),
                      static_cast<uint8_t> (i >> 8), static_cast<uint8_t> (i)});

        auto t = hmac.get_digest ();
        auto u = t;

        for (std::uint32_t j = 1; j < count; j++)
        {
            hmac.reset ();
            hmac.update (u);
            u ^= hmac.get_digest ();
        }

        dk += u;
        ++i;
    }

    return dk.slice (0, dklen - 1);
}

} // namespace mobius::core::os::win
