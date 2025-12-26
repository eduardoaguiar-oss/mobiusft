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
#include <mobius/core/crypt/cipher.hpp>
#include <mobius/core/os/win/hash_lm.hpp>
#include <mobius/core/string_functions.hpp>

namespace mobius::core::os::win
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Generate LM hash
// @param password Password
// @return LM hash
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
hash_lm (const std::string &password)
{
    const std::string u_password = mobius::core::string::toupper (password);
    mobius::core::bytearray lm_hash;

    mobius::core::bytearray key1 (u_password.substr (0, 7));
    key1.rpad (7);
    auto des1 = mobius::core::crypt::new_cipher_ecb ("des", key1);
    lm_hash = des1.encrypt ("KGS!@#$%");

    mobius::core::bytearray key2 (
        u_password.length () > 7 ? u_password.substr (7, 7) : "");
    key2.rpad (7);
    auto des2 = mobius::core::crypt::new_cipher_ecb ("des", key2);
    lm_hash += des2.encrypt ("KGS!@#$%");

    return lm_hash;
}

} // namespace mobius::core::os::win
