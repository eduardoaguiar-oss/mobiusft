// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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
#include "base64.h"

namespace mobius::encoder
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Base 64 chars (RFC 4648, section 4)
static constexpr char BASE64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// @brief Base 64 chars, URL and filename (RFC 4648, section 5)
static constexpr char BASE64_URL[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

// @brief Base 32 chars (RFC 4648, section 6)
static constexpr char BASE32[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

// @brief Base 32 hex chars (RFC 4648, section 7)
static constexpr char BASE32_HEX[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";

// @brief Base 16 chars (RFC 4648, section 8)
static constexpr char BASE16[] = "0123456789ABCDEF";

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Encode bytes into base64 encoding
// @param data Data
// @return Base64 encoded data
// @see RFC 4648, section 4
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
base64 (const mobius::bytearray& data)
{
  std::string ret (((data.size () + 2) / 3) * 4, '=');
  std::uint64_t i = 0;
  std::uint32_t d = 0;
  std::uint32_t bits = 0;

  for (const auto c : data)
    {
      d = (d << 8) | c;
      bits += 8;

      while (bits >= 6)
        {
          bits -= 6;
          ret[i++] = BASE64[d >> bits];
          d = d & ((1 << bits) - 1);
        }
    }

  // padding, if necessary
  if (bits)
    ret[i] = BASE64[d << (6 - bits)];

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Encode bytes into base64 encoding, URL and Filename Safe Alphabet
// @param data Data
// @return Base64 encoded data
// @see RFC 4648, section 5
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
base64url (const mobius::bytearray& data)
{
  std::string ret (((data.size () + 2) / 3) * 4, '=');
  std::uint64_t i = 0;
  std::uint32_t d = 0;
  std::uint32_t bits = 0;

  for (const auto c : data)
    {
      d = (d << 8) | c;
      bits += 8;

      while (bits >= 6)
        {
          bits -= 6;
          ret[i++] = BASE64_URL[d >> bits];
          d = d & ((1 << bits) - 1);
        }
    }

  // padding, if necessary
  if (bits)
    ret[i] = BASE64_URL[d << (6 - bits)];

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Encode bytes into base32 encoding
// @param data Data
// @return Base32 encoded data
// @see RFC 4648, section 6
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
base32 (const mobius::bytearray& data)
{
  std::string ret (((data.size () + 4) / 5) * 8, '=');
  std::uint32_t d = 0;
  std::uint32_t bits = 0;
  std::uint64_t i = 0;

  for (const auto c : data)
    {
      d = (d << 8) | c;
      bits += 8;

      while (bits >= 5)
        {
          bits -= 5;
          ret[i++] = BASE32[d >> bits];
          d = d & ((1 << bits) - 1);
        }
    }

  // padding, if necessary
  if (bits)
    ret[i] = BASE32[d << (5 - bits)];

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Encode bytes into base32hex encoding
// @param data Data
// @return Base32hex encoded data
// @see RFC 4648, section 7
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
base32hex (const mobius::bytearray& data)
{
  std::string ret (((data.size () + 4) / 5) * 8, '=');
  std::uint32_t d = 0;
  std::uint32_t bits = 0;
  std::uint64_t i = 0;

  for (const auto c : data)
    {
      d = (d << 8) | c;
      bits += 8;

      while (bits >= 5)
        {
          bits -= 5;
          ret[i++] = BASE32_HEX[d >> bits];
          d = d & ((1 << bits) - 1);
        }
    }

  // padding, if necessary
  if (bits)
    ret[i] = BASE32_HEX[d << (5 - bits)];

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Encode bytes into base16 encoding
// @param data Data
// @return Base16 encoded data
// @see RFC 4648, section 8
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
base16 (const mobius::bytearray& data)
{
  std::string ret (data.size () * 2, '\x00');
  std::uint64_t i = 0;

  for (const auto c : data)
    {
      ret[i++] = BASE16[c >> 4];
      ret[i++] = BASE16[c & 0x0f];
    }

  return ret;
}

} // namespace mobius::encoder




