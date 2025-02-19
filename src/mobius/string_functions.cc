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
#include "string_functions.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <fnmatch.h>

namespace mobius::string
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief replace string's s1 substrings with s2
//! \param str string
//! \param s1 string
//! \param s2 string
//! \return new string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
replace (const std::string& str, const std::string& s1, const std::string& s2)
{
  if (str.empty () || s1.empty ())
    return str;

  std::string tmp (str);
  std::string::size_type pos = tmp.find (s1);

  while (pos != std::string::npos)
    {
      tmp = tmp.replace (pos, s1.length (), s2);
      pos = tmp.find (s1, pos + s2.length ());
    }

  return tmp;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief convert ASCII string to lowercase
//! \param str string
//! \return lowercased string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
tolower (const std::string& str)
{
  std::string tmp (str.length (), 0);
  std::transform (str.begin (), str.end (), tmp.begin (), ::tolower);
  return tmp;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief convert ASCII string to capitalize mode
//! \param str string
//! \return capitalized string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
capitalize (const std::string& str)
{
  std::string tmp (tolower (str));

  if (tmp.length () > 0)
    tmp[0] = ::toupper (tmp[0]);

  return tmp;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief convert ASCII string to uppercase
//! \param str string
//! \return uppercased string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
toupper (const std::string& str)
{
  std::string tmp (str.length (), 0);
  std::transform (str.begin (), str.end (), tmp.begin (), ::toupper);
  return tmp;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief check if a string starts with another one
//! \param str string
//! \param starting starting string
//! \return true if str starts with starting
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
startswith (const std::string& str, const std::string& starting)
{
  return starting.length () > 0 &&
         str.length () >= starting.length () &&
         str.compare (0, starting.length (), starting) == 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief check if a string ends with another one
//! \param str string
//! \param ending ending string
//! \return true if str ends with ending
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
endswith (const std::string& str, const std::string& ending)
{
  return ending.length () > 0 &&
         str.length () >= ending.length () &&
         str.compare (str.length () - ending.length (), ending.length (), ending) == 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Check if a string matches a given pattern
//! \param patter fnmatch patter
//! \param str string
//! \return true if str matches, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
fnmatch (const std::string& pattern, const std::string& str)
{
  return ::fnmatch (pattern.c_str (), str.c_str (), FNM_NOESCAPE) == 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Check if a string matches a given pattern (case insensitive)
//! \param patter fnmatch patter
//! \param str string
//! \return true if str matches, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
case_insensitive_fnmatch (const std::string& pattern, const std::string& str)
{
  const std::string i_pattern = tolower (pattern);
  const std::string i_str = tolower (str);

  return ::fnmatch (i_pattern.c_str (), i_str.c_str (), FNM_NOESCAPE) == 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Check if two strings are equal, case insensitive
//! \param a First string
//! \param b Second string
//! \return True if strings match, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
case_insensitive_match (const std::string& a, const std::string& b)
{
  return (a.size () == b.size () &&
          std::equal (a.begin (), a.end (), b.begin (),
                      [](char c1, char c2){ return (c1 == c2 || std::toupper (c1) == std::toupper (c2)); }
                     )
         );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Check if two strings are equal, case sensitive
//! \param a First string
//! \param b Second string
//! \return True if strings match, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
case_sensitive_match (const std::string& a, const std::string& b)
{
  return (a == b);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief strip chars from both ends of a string
//! \param str string
//! \param chars chars
//! \return a new string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
strip (const std::string& str, const std::string& chars)
{
  const auto pos1 = str.find_first_not_of (chars);
  const auto pos2 = str.find_last_not_of (chars);

  if (pos1 == std::string::npos || pos2 == std::string::npos)
    return std::string ();

  return str.substr (pos1, pos2 - pos1 + 1);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief strip chars from the beginning of a string
//! \param str string
//! \param chars chars
//! \return a new string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
lstrip (const std::string& str, const std::string& chars)
{
  const auto pos = str.find_first_not_of (chars);

  if (pos == std::string::npos)
    return std::string ();

  return str.substr (pos);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief strip chars from the end of a string
//! \param str string
//! \param chars chars
//! \return a new string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
rstrip (const std::string& str, const std::string& chars)
{
  const auto pos = str.find_last_not_of (chars);

  if (pos == std::string::npos)
    return std::string ();

  return str.substr (0, pos + 1);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief split string into parts
//! \param str string
//! \param sep separator string
//! \return vector containing parts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <std::string>
split (const std::string& str, const std::string& sep)
{
  std::vector <std::string> result;
  std::string::size_type start = 0;
  std::string::size_type end = str.find (sep);

  while (end != std::string::npos)
    {
      result.push_back (str.substr (start, end - start));
      start = end + sep.length ();
      end = str.find (sep, start);
    }

  result.push_back (str.substr (start));
  return result;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get n word from string
//! \param str string
//! \param n word index (negative values count from right to left)
//! \param sep separator string
//! \return word
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
word (const std::string& str, int n, const std::string& sep)
{
  std::string word;

  // split string
  auto d = split (str, sep);

  if (n < 0)
    n = d.size () + n;

  // get n word
  if (n >= 0 && n < int (d.size ()))
    word = d[n];

  return word;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief remove all occurrences of 'c' in str
//! \param str string
//! \param c char
//! \return a new string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
remove_char (const std::string& str, char c)
{
  std::string tmp (str);
  std::string::iterator end_pos = std::remove (tmp.begin(), tmp.end(), c);
  tmp.erase (end_pos, tmp.end ());
  return tmp;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief pad string to the left with a given char
//! \param str string
//! \param siz string size
//! \param c fill char
//! \return a new string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
lpad (const std::string& str, std::string::size_type siz, char c)
{
  if (str.length () < siz)
    return std::string (siz - str.length (), c) + str;

  return str;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief pad string to the right with a given char
//! \param str string
//! \param siz string size
//! \param c fill char
//! \return a new string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
rpad (const std::string& str, std::string::size_type siz, char c)
{
  if (str.length () < siz)
    return str + std::string (siz - str.length (), c);

  return str;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief convert value to hexadecimal string
//! \param value value
//! \param digits min digits
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
to_hex (std::uint64_t value, unsigned int digits)
{
  std::ostringstream stream;
  stream << std::hex << std::setw (digits) << std::setfill ('0') << value << std::dec;
  return stream.str ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief convert value to string
//! \param value value
//! \param digits min digits
//! \param fill fill char
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
to_string (std::uint64_t value, unsigned int digits, char fill)
{
  std::ostringstream stream;
  stream << std::setw (digits) << std::setfill (fill) << value;
  return stream.str ();
}

} // namespace mobius::string
