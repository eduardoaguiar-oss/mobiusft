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
#include <cstdint>
#include <map>
#include <mobius/core/exception.inc>
#include <mobius/core/os/win/dpapi/cipher_info.hpp>
#include <stdexcept>
#include <string>

namespace mobius::core::os::win::dpapi
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Cipher information map
// @see https://docs.microsoft.com/en-us/windows/desktop/seccrypto/alg-id
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct cipher_info
{
    std::string id;             // crypt::cipher cipher_id
    std::uint32_t key_length;   // key length in bytes
    std::uint32_t block_length; // block length in bytes
    std::uint32_t iv_length;    // IV length in bytes
};

static const std::map<std::uint32_t, cipher_info> CIPHERS = {
    {0x6601, {"des", 8, 8, 8}},    {0x6603, {"3des", 24, 8, 8}},
    {0x660e, {"aes", 16, 16, 16}}, {0x660f, {"aes", 24, 16, 16}},
    {0x6610, {"aes", 32, 16, 16}}, {0x6801, {"rc4", 16, 8, 16}}};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get cipher ID by MS alg ID
// @param ms_alg_id MS alg ID
// @return cipher ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_cipher_id (std::uint32_t ms_alg_id)
{
    auto iter = CIPHERS.find (ms_alg_id);

    if (iter != CIPHERS.end ())
        return iter->second.id;

    else
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("Unknown MS alg ID"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get cipher key length by MS alg ID
// @param ms_alg_id MS alg ID
// @return Key length in bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
get_cipher_key_length (std::uint32_t ms_alg_id)
{
    auto iter = CIPHERS.find (ms_alg_id);

    if (iter != CIPHERS.end ())
        return iter->second.key_length;

    else
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("Unknown MS alg ID"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get cipher salt length by MS alg ID
// @param ms_alg_id MS alg ID
// @return Salt length in bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
get_cipher_salt_length (std::uint32_t ms_alg_id)
{
    auto iter = CIPHERS.find (ms_alg_id);

    if (iter != CIPHERS.end ())
        return iter->second.iv_length;

    else
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("Unknown MS alg ID"));
}

} // namespace mobius::core::os::win::dpapi
