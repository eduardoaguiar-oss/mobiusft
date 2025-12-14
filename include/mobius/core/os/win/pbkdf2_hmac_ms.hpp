#ifndef MOBIUS_OS_WIN_PBKDF2_HMAC_MS_H
#define MOBIUS_OS_WIN_PBKDF2_HMAC_MS_H

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
#include <cstdint>
#include <string>

namespace mobius::core::os::win
{

mobius::core::bytearray pbkdf2_hmac_ms (
   const mobius::core::bytearray&,
   const mobius::core::bytearray&,
   std::uint32_t,
   std::uint16_t,
   const std::string&);

} // namespace mobius::core::os::win

#endif


