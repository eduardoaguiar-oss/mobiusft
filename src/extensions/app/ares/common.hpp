#ifndef MOBIUS_EXTENSION_APP_ARES_COMMON_HPP
#define MOBIUS_EXTENSION_APP_ARES_COMMON_HPP

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
#include <cstdint>
#include <map>
#include <mobius/core/bytearray.hpp>
#include <mobius/core/file_decoder/entry.hpp>

namespace mobius::extension::app::ares
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray decrypt (const mobius::core::bytearray &,
                                 std::uint16_t);
std::string media_type_to_string (std::uint8_t);
std::map<int, mobius::core::bytearray>
decode_metadata (const mobius::core::bytearray &);
std::vector<std::pair<std::string, std::uint16_t>>
decode_old_alt_sources (const mobius::core::bytearray &);
std::vector<std::pair<std::string, std::uint16_t>>
decode_alt_sources (const mobius::core::bytearray &);

} // namespace mobius::extension::app::ares

#endif
