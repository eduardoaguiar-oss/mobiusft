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
#include "hash_lm.h"
#include <mobius/crypt/cipher.h>
#include <mobius/string_functions.h>

namespace mobius::os::win
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Generate LM hash
// @param password Password
// @return LM hash
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
hash_lm (const std::string& password)
{
  const std::string u_password = mobius::string::toupper (password);
  mobius::bytearray lm_hash;

  mobius::bytearray key1 (u_password.substr (0, 7));
  key1.rpad (7);
  auto des1 = mobius::crypt::new_cipher_ecb ("des", key1);
  lm_hash = des1.encrypt ("KGS!@#$%");

  mobius::bytearray key2 (u_password.length () > 7 ? u_password.substr (7, 7) : "");
  key2.rpad (7);
  auto des2 = mobius::crypt::new_cipher_ecb ("des", key2);
  lm_hash += des2.encrypt ("KGS!@#$%");

  return lm_hash;
}

} // namespace mobius::os::win


