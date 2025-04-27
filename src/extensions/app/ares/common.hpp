#ifndef MOBIUS_EXTENSION_APP_ARES_COMMON_HPP
#define MOBIUS_EXTENSION_APP_ARES_COMMON_HPP

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
#include <mobius/bytearray.h>
#include <mobius/core/file_decoder/entry.hpp>
#include <mobius/pod/map.h>
#include <cstdint>
#include <map>

namespace mobius::extension::app::ares
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray decrypt (const mobius::bytearray&, std::uint16_t);
std::string media_type_to_string (std::uint8_t);
std::map <int, mobius::bytearray> decode_metadata (const mobius::bytearray&);
std::vector <std::pair <std::string, std::uint16_t>> decode_old_alt_sources (const mobius::bytearray&);
std::vector <std::pair <std::string, std::uint16_t>> decode_alt_sources (const mobius::bytearray&);

} // namespace mobius::extension::app::ares

#endif



