#ifndef MOBIUS_CORE_OS_WIN_DPAPI_CIPHER_INFO_HPP
#define MOBIUS_CORE_OS_WIN_DPAPI_CIPHER_INFO_HPP

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
#include <string>

namespace mobius::core::os::win::dpapi
{
std::string get_cipher_id (std::uint32_t);
std::uint32_t get_cipher_key_length (std::uint32_t);
std::uint32_t get_cipher_salt_length (std::uint32_t);

} // namespace mobius::core::os::win::dpapi

#endif


