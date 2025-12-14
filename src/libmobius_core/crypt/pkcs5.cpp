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
#include <mobius/core/crypt/hash.hpp>
#include <mobius/core/crypt/hmac.hpp>
#include <mobius/core/crypt/pkcs5.hpp>
#include <mobius/core/exception.inc>
#include <stdexcept>

namespace mobius::core::crypt
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unpad plain text
// @param data Plain text data
// @return Unpadded data
// @see RFC 2898 (section 6.1.1.4)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
pkcs5_unpad (const mobius::core::bytearray &data)
{
    mobius::core::bytearray unpad_data;

    if (data)
    {
        auto pad_size = data[data.size () - 1];

        if (pad_size < data.size ())
            unpad_data = data.slice (0, data.size () - pad_size - 1);
    }

    return unpad_data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief PBKDF1 key derivation function
// @param hash_id Hash algorithm (either "md2", "md5" or "sha1")
// @param key Key
// @param salt Salt
// @param count Iterations
// @param dklen Derived key length in bytes
// @see RFC 2898
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
pbkdf1 (const std::string &hash_id, const mobius::core::bytearray &key,
        const mobius::core::bytearray &salt, std::uint32_t count,
        std::uint16_t dklen)
{
    // Validate input parameters
    if (dklen == 0)
        throw std::out_of_range (
            MOBIUS_EXCEPTION_MSG ("Derived key length must be > 0"));

    if (hash_id == "sha1")
    {
        if (dklen > 20)
            throw std::out_of_range (
                MOBIUS_EXCEPTION_MSG ("Derived key too long"));
    }

    else if (hash_id == "md2" || hash_id == "md5")
    {
        if (dklen > 16)
            throw std::out_of_range (
                MOBIUS_EXCEPTION_MSG ("Derived key too long"));
    }

    else
        throw std::out_of_range (
            MOBIUS_EXCEPTION_MSG ("Invalid hash algorithm"));

    // Calculate derived key
    mobius::core::crypt::hash h (hash_id);
    h.update (key);
    h.update (salt);
    mobius::core::bytearray t = h.get_digest ();

    for (std::uint32_t i = 1; i < count; i++)
    {
        mobius::core::crypt::hash h2 (hash_id);
        h2.update (t);
        t = h2.get_digest ();
    }

    return t.slice (0, dklen - 1);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief PBKDF2 key derivation function (with HMAC)
// @param hash_id hash algorithm (e.g. "md2", "md5", "sha1")
// @param key Key
// @param salt Salt
// @param count Iterations
// @param dklen Derived key length in bytes
// @see RFC 2898
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
pbkdf2_hmac (const std::string &hash_id, const mobius::core::bytearray &key,
             const mobius::core::bytearray &salt, std::uint32_t count,
             std::uint16_t dklen)
{
    // Validate input parameters
    if (dklen == 0)
        throw std::out_of_range (
            MOBIUS_EXCEPTION_MSG ("Derived key length must be > 0"));

    // Calculate derived key
    std::uint32_t i = 1;
    mobius::core::bytearray dk;
    mobius::core::crypt::hmac hmac (hash_id, key);

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
            u = hmac.get_digest ();
            t ^= u;
        }

        dk += t;
        ++i;
    }

    return dk.slice (0, dklen - 1);
}

} // namespace mobius::core::crypt
