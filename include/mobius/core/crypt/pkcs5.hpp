#ifndef MOBIUS_CRYPT_PKCS5_H
#define MOBIUS_CRYPT_PKCS5_H

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
#include <mobius/core/bytearray.hpp>
#include <string>
#include <cstdint>

namespace mobius::core::crypt
{

mobius::core::bytearray pkcs5_unpad (const mobius::core::bytearray&);

mobius::core::bytearray pbkdf1 (const std::string&, const mobius::core::bytearray&, const mobius::core::bytearray&, std::uint32_t, std::uint16_t);

mobius::core::bytearray pbkdf2_hmac (const std::string&, const mobius::core::bytearray&, const mobius::core::bytearray&, std::uint32_t, std::uint16_t);

} // namespace mobius::core::crypt

#endif


