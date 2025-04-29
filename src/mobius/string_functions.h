#ifndef MOBIUS_CORE_STRING_HPP
#define MOBIUS_CORE_STRING_HPP

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
#include <cstdint>
#include <string>
#include <vector>

namespace mobius::string
{
// case functions
std::string tolower (const std::string&);
std::string toupper (const std::string&);
std::string capitalize (const std::string&);

// remove chars functions
std::string lstrip (const std::string&, const std::string& = " \t\r\n");
std::string rstrip (const std::string&, const std::string& = " \t\r\n");
std::string strip (const std::string&, const std::string& = " \t\r\n");
std::string remove_char (const std::string&, char);

// format functions
std::string lpad (const std::string&, std::string::size_type, char = ' ');
std::string rpad (const std::string&, std::string::size_type, char = ' ');
std::string replace (const std::string&, const std::string&, const std::string&);

// parse functions
std::vector <std::string> split (const std::string&, const std::string& = " ");
std::string word (const std::string&, int, const std::string& = " ");

// compare functions
bool startswith (const std::string&, const std::string&);
bool endswith (const std::string&, const std::string&);
bool fnmatch (const std::string&, const std::string&);
bool case_insensitive_fnmatch (const std::string&, const std::string&);
bool case_insensitive_match (const std::string&, const std::string&);
bool case_sensitive_match (const std::string&, const std::string&);

// representation functions
std::string to_hex (std::uint64_t, unsigned int = 1);
std::string to_string (std::uint64_t, unsigned int = 1, char = '0');

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Try to convert value to string
// @param v Value
// @return String representation of value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T> std::string
to_string (const T& t)
{
  if constexpr (std::is_same_v <T, std::string> || std::is_same_v <T, const char *>)
    return t;

  else
    return std::to_string (t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get first non-empty string from arguments
// @param args Arguments
// @return First non-empty argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T> std::string
first_of (const T& t)
{
  return to_string (t);
}

template <typename T, typename ...Args> std::string
first_of (T t, Args... args)
{
  std::string s = first_of (t);

  if (s.empty ())
    return first_of (args...);

  else
    return s;
}

} // namespace mobius::string

#endif


