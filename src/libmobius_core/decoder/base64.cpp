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
#include <mobius/core/decoder/base64.hpp>
#include <mobius/core/exception.inc>
#include <stdexcept>

namespace mobius::core::decoder
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Base 16 chars (RFC 4648, section 8)
static constexpr std::uint8_t BASE16[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0a, 0x0b, 0x0c,
    0x0d, 0x0e, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

// @brief Base 32 chars (RFC 4648, section 6)
static constexpr std::uint8_t BASE32[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01, 0x02,
    0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

// @brief Base 32 hex chars (RFC 4648, section 7)
static constexpr std::uint8_t BASE32_HEX[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0a, 0x0b, 0x0c,
    0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

// @brief Base 64 chars (RFC 4648, section 4)
static constexpr std::uint8_t BASE64[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3e,
    0xff, 0xff, 0xff, 0x3f, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
    0x3c, 0x3d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01, 0x02,
    0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c,
    0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff};

// @brief Base 64 chars, URL and filename (RFC 4648, section 5)
static constexpr std::uint8_t BASE64_URL[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x3e, 0xff, 0xff, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
    0x3c, 0x3d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01, 0x02,
    0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xff,
    0xff, 0xff, 0xff, 0x3f, 0xff, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c,
    0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode base64 string
// @param s Base64 encoded string
// @return Data
// @see RFC 4648, section 4
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
base64 (const std::string &s)
{
    std::uint64_t l = s.size ();

    // ignore pad chars
    while (l > 0 && (s[l - 1] == '=' || s[l - 1] == '\n' || s[l - 1] == '\r'))
        l--;

    // calculate output size
    std::uint64_t obits = 0;

    for (std::uint64_t i = 0; i < l; i++)
    {
        char c = s[i];

        if (c == '\n' || c == '\r') // ignore
            ;

        else if (c < 32 || BASE64[c - 32] == 0xff)
            throw std::invalid_argument (
                MOBIUS_EXCEPTION_MSG ("invalid base64 encoded string"));

        else
            obits += 6;
    }

    // convert
    mobius::core::bytearray ret (obits >> 3);
    std::uint32_t d = 0;
    std::uint64_t idx = 0;
    std::uint32_t bits = 0;

    for (std::uint64_t i = 0; i < l; i++)
    {
        char c = s[i];

        if (c != '\n' && c != '\r') // ignore
        {
            d = (d << 6) | BASE64[c - 32];
            bits += 6;

            while (bits >= 8)
            {
                bits -= 8;
                ret[idx++] = d >> bits;
                d = d & ((1 << bits) - 1);
            }
        }
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode base64 encoded string, URL and Filename Safe Alphabet
// @param s Base64url encoded string
// @return Data
// @see RFC 4648, section 5
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
base64url (const std::string &s)
{
    // check input size
    std::uint64_t l = s.size ();

    if (l % 4)
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("invalid base64url encoded string"));

    // calculate output size
    std::uint32_t obits = s.size () * 6;

    while (l > 0 && s[l - 1] == '=')
    {
        obits -= 6;
        l--;
    }

    // convert
    mobius::core::bytearray ret (obits / 8);
    std::uint64_t i = 0;
    std::uint32_t d = 0;
    std::uint64_t idx = 0;
    std::uint32_t bits = 0;

    while (i < l)
    {
        char c = s[i++];

        if (c < 32 || BASE64_URL[c - 32] == 0xff)
            throw std::invalid_argument (
                MOBIUS_EXCEPTION_MSG ("invalid base64url encoded string"));

        d = (d << 6) | BASE64_URL[c - 32];
        bits += 6;

        while (bits >= 8)
        {
            bits -= 8;
            ret[idx++] = d >> bits;
            d = d & ((1 << bits) - 1);
        }
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode base32 encoded string
// @param s Base32 encoded string
// @return Data
// @see RFC 4648, section 6
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
base32 (const std::string &s)
{
    // check input size
    std::uint64_t l = s.size ();

    if (l % 8)
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("invalid base32 encoded string"));

    // calculate output size
    std::uint32_t obits = s.size () * 5;

    while (l > 0 && s[l - 1] == '=')
    {
        obits -= 5;
        l--;
    }

    // convert
    mobius::core::bytearray ret (obits / 8);
    std::uint64_t i = 0;
    std::uint32_t d = 0;
    std::uint64_t idx = 0;
    std::uint32_t bits = 0;

    while (i < l)
    {
        char c = s[i++];

        if (c < 32 || BASE32[c - 32] == 0xff)
            throw std::invalid_argument (
                MOBIUS_EXCEPTION_MSG ("invalid base32 encoded string"));

        d = (d << 5) | BASE32[c - 32];
        bits += 5;

        while (bits >= 8)
        {
            bits -= 8;
            ret[idx++] = d >> bits;
            d = d & ((1 << bits) - 1);
        }
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode base32hex encoded string
// @param s Base32hex encoded string
// @return Data
// @see RFC 4648, section 7
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
base32hex (const std::string &s)
{
    // check input size
    std::uint64_t l = s.size ();

    if (l % 8)
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("invalid base32hex encoded string"));

    // calculate output size
    std::uint32_t obits = s.size () * 5;

    while (l > 0 && s[l - 1] == '=')
    {
        obits -= 5;
        l--;
    }

    // convert
    mobius::core::bytearray ret (obits / 8);
    std::uint64_t i = 0;
    std::uint32_t d = 0;
    std::uint64_t idx = 0;
    std::uint32_t bits = 0;

    while (i < l)
    {
        char c = s[i++];

        if (c < 32 || BASE32_HEX[c - 32] == 0xff)
            throw std::invalid_argument (
                MOBIUS_EXCEPTION_MSG ("invalid base32hex encoded string"));

        d = (d << 5) | BASE32_HEX[c - 32];
        bits += 5;

        while (bits >= 8)
        {
            bits -= 8;
            ret[idx++] = d >> bits;
            d = d & ((1 << bits) - 1);
        }
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode base16 encoded string
// @param s Base16 encoded string
// @return Data
// @see RFC 4648, section 8
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
base16 (const std::string &s)
{
    // check input size
    std::uint64_t l = s.size ();

    if (l % 2)
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("invalid base16 encoded string"));

    // convert
    mobius::core::bytearray ret (l / 2);

    for (std::uint64_t i = 0; i < l; i += 2)
    {
        char c1 = s[i];
        char c2 = s[i + 1];

        if (c1 < 32 || BASE16[c1 - 32] == 0xff || c2 < 32 ||
            BASE16[c2 - 32] == 0xff)
            throw std::invalid_argument (
                MOBIUS_EXCEPTION_MSG ("invalid base16 encoded string"));

        ret[i / 2] = (BASE16[c1 - 32] << 4) | BASE16[c2 - 32];
    }

    return ret;
}

} // namespace mobius::core::decoder
