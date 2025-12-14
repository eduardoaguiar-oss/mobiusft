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
#include <mobius/core/crypt/cipher_impl_zip.hpp>
#include <mobius/core/crypt/crc32.hpp>

namespace
{

constexpr std::uint32_t ZIPCONST = 0x08088405U;

} // namespace

namespace mobius::core::crypt
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param passwd Encryption/decryption password
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
cipher_impl_zip::cipher_impl_zip (const bytearray &passwd)
    : passwd_ (passwd)
{
    reset ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Encrypt data
// @param data Data to be encrypted
// @return Encrypted data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
cipher_impl_zip::encrypt (const mobius::core::bytearray &data)
{
    mobius::core::bytearray out (data.size ());
    auto o_iter = out.begin ();

    for (std::uint8_t b : data)
    {
        std::uint16_t temp = k2_ | 2;
        std::uint8_t d = (temp * (temp ^ 1)) >> 8;
        std::uint8_t o = b ^ d;
        *o_iter++ = o;

        k0_ = crc32 (k0_, b);
        k1_ = (k1_ + (k0_ & 0xff)) * ZIPCONST + 1;
        k2_ = crc32 (k2_, k1_ >> 24);
    }

    return out;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt data
// @param data Data to be decrypted
// @return Decrypted data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
cipher_impl_zip::decrypt (const mobius::core::bytearray &data)
{
    mobius::core::bytearray out (data.size ());
    auto o_iter = out.begin ();

    for (std::uint8_t b : data)
    {
        std::uint16_t temp = k2_ | 2;
        std::uint8_t d = (temp * (temp ^ 1)) >> 8;
        std::uint8_t o = b ^ d;
        *o_iter++ = o;

        k0_ = crc32 (k0_, o);
        k1_ = (k1_ + (k0_ & 0xff)) * ZIPCONST + 1;
        k2_ = crc32 (k2_, k1_ >> 24);
    }

    return out;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset state
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
cipher_impl_zip::reset () noexcept
{
    k0_ = 0x12345678;
    k1_ = 0x23456789;
    k2_ = 0x34567890;

    for (std::uint8_t b : passwd_)
    {
        k0_ = crc32 (k0_, b);
        k1_ = (k1_ + (k0_ & 0xff)) * ZIPCONST + 1;
        k2_ = crc32 (k2_, k1_ >> 24);
    }
}

} // namespace mobius::core::crypt
