#ifndef MOBIUS_CORE_CHARSET_HPP
#define MOBIUS_CORE_CHARSET_HPP

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
#include <utility>

namespace mobius::core
{
std::string conv_charset_to_utf8 (const mobius::core::bytearray&, const std::string&);

std::pair <std::string,mobius::core::bytearray> conv_charset_to_utf8_partial (const mobius::core::bytearray&, const std::string&);

mobius::core::bytearray conv_charset (const mobius::core::bytearray&, const std::string&, const std::string&);

std::pair <mobius::core::bytearray,mobius::core::bytearray> conv_charset_partial (const mobius::core::bytearray&, const std::string&, const std::string&);
} // namespace mobius::core

#endif


