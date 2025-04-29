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
#include "hash_msdcc2.h"
#include "hash_msdcc1.h"
#include <mobius/core/charset.hpp>
#include <mobius/core/crypt/hmac.hpp>
#include <mobius/string_functions.h>

namespace mobius::os::win
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Generate MSDCC2 hash
// @param password Password
// @param username User name
// @param iterations Number of iterations
// @return MSDCC2 hash
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
hash_msdcc2 (
  const std::string& password,
  const std::string& username,
  std::uint32_t iterations)
{
  const mobius::bytearray pass = hash_msdcc1 (password, username);
  const mobius::bytearray salt = mobius::core::conv_charset (mobius::string::tolower (username), "UTF-8", "UTF-16LE");

  mobius::core::crypt::hmac hmac_sha1 ("sha1", pass);
  hmac_sha1.update (salt);
  hmac_sha1.update ({0, 0, 0, 1});

  mobius::bytearray temp = hmac_sha1.get_digest ();
  mobius::bytearray out = temp.slice (0, 15);

  for (std::uint32_t i = 1; i < iterations; i++)
    {
      mobius::core::crypt::hmac hmac_sha1 ("sha1", pass);
      hmac_sha1.update (temp);
      temp = hmac_sha1.get_digest ();
      out ^= temp.slice (0, 15);
    }

  return out;
}

} // namespace mobius::os::win


