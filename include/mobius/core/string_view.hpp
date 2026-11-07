#ifndef MOBIUS_CORE_STRING_VIEW_HPP
#define MOBIUS_CORE_STRING_VIEW_HPP

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
#include <vector>

namespace mobius::core::string_view
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// test functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool is_digit (std::string_view);
bool is_integer (std::string_view);
bool is_hex (std::string_view);
bool is_alpha (std::string_view);
bool is_alnum (std::string_view);
bool is_lower (std::string_view);
bool is_upper (std::string_view);
bool is_space (std::string_view);
bool is_numeric_cpf (std::string_view);
bool is_numeric_cnpj (std::string_view);
bool is_email (std::string_view);
bool is_mobile_phone (std::string_view);
bool is_url (std::string_view);
bool is_ipv4 (std::string_view);
bool is_ipv6 (std::string_view);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Remove chars functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string_view rstrip (std::string_view, std::string_view = " \t\r\n");
std::string_view lstrip (std::string_view, std::string_view = " \t\r\n");
std::string_view strip (std::string_view, std::string_view = " \t\r\n");

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Word functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <std::string_view> split (std::string_view, std::string_view);
std::string_view word (std::string_view, int, std::string_view = " ");

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Conversion functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int64_t to_integer (std::string_view);

} // namespace mobius::core::string_view

#endif


