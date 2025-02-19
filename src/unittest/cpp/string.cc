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
#include <mobius/string_functions.h>
#include <mobius/unittest.h>

using namespace mobius::string;

void
unittest_string ()
{
  mobius::unittest test ("mobius::string");

  // replace
  test.ASSERT_EQUAL (replace (std::string (), std::string (), std::string ()), std::string ());
  test.ASSERT_EQUAL (replace ("abc", "a", "x"), "xbc");
  test.ASSERT_EQUAL (replace ("abc", "d", "x"), "abc");
  test.ASSERT_EQUAL (replace ("abc", "c", "a"), "aba");
  test.ASSERT_EQUAL (replace ("abcd", "bc", "efgh"), "aefghd");
  test.ASSERT_EQUAL (replace ("abcd", "ab", "efgh"), "efghcd");
  test.ASSERT_EQUAL (replace ("abcd", "d", "efgh"), "abcefgh");

  // toupper
  test.ASSERT_EQUAL (toupper (std::string ()), std::string ());
  test.ASSERT_EQUAL (toupper ("ABC"), "ABC");
  test.ASSERT_EQUAL (toupper ("abc"), "ABC");
  test.ASSERT_EQUAL (toupper ("aBC"), "ABC");

  // tolower
  test.ASSERT_EQUAL (tolower (std::string ()), std::string ());
  test.ASSERT_EQUAL (tolower ("abc"), "abc");
  test.ASSERT_EQUAL (tolower ("ABC"), "abc");
  test.ASSERT_EQUAL (tolower ("Abc"), "abc");

  // startswith
  test.ASSERT_FALSE (startswith ("abc", "x"));
  test.ASSERT_FALSE (startswith ("abc", "xy"));
  test.ASSERT_TRUE (startswith ("abc", "a"));
  test.ASSERT_TRUE (startswith ("abc", "ab"));
  test.ASSERT_TRUE (startswith ("abc", "abc"));
  test.ASSERT_FALSE (startswith ("abc", ""));
  test.ASSERT_FALSE (startswith ("", "a"));

  // endswith
  test.ASSERT_FALSE (endswith ("abc", "x"));
  test.ASSERT_FALSE (endswith ("abc", "xy"));
  test.ASSERT_TRUE (endswith ("abc", "c"));
  test.ASSERT_TRUE (endswith ("abc", "bc"));
  test.ASSERT_TRUE (endswith ("abc", "abc"));
  test.ASSERT_FALSE (endswith ("abc", ""));
  test.ASSERT_FALSE (endswith ("", "a"));

  // strip
  test.ASSERT_EQUAL (strip (std::string ()), std::string ());
  test.ASSERT_EQUAL (strip ("   "), std::string ());
  test.ASSERT_EQUAL (strip ("abc"), "abc");
  test.ASSERT_EQUAL (strip ("abc "), "abc");
  test.ASSERT_EQUAL (strip (" abc"), "abc");
  test.ASSERT_EQUAL (strip (" abc "), "abc");
  test.ASSERT_EQUAL (strip ("x x x x x  abc x x xxx x x x", "x "), "abc");
  test.ASSERT_EQUAL (strip ("x x x x x  ", "x "), std::string ());

  // lstrip
  test.ASSERT_EQUAL (lstrip (std::string ()), std::string ());
  test.ASSERT_EQUAL (lstrip ("   "), std::string ());
  test.ASSERT_EQUAL (lstrip ("abc"), "abc");
  test.ASSERT_EQUAL (lstrip ("abc "), "abc ");
  test.ASSERT_EQUAL (lstrip (" abc"), "abc");
  test.ASSERT_EQUAL (lstrip (" abc "), "abc ");
  test.ASSERT_EQUAL (lstrip ("x x x x x  abc x x", "x "), "abc x x");
  test.ASSERT_EQUAL (lstrip ("x x x x x  ", "x "), std::string ());

  // rstrip
  test.ASSERT_EQUAL (rstrip (std::string ()), std::string ());
  test.ASSERT_EQUAL (rstrip ("   "), std::string ());
  test.ASSERT_EQUAL (rstrip ("abc"), "abc");
  test.ASSERT_EQUAL (rstrip ("abc "), "abc");
  test.ASSERT_EQUAL (rstrip (" abc"), " abc");
  test.ASSERT_EQUAL (rstrip (" abc "), " abc");
  test.ASSERT_EQUAL (rstrip ("x x abc x x", "x "), "x x abc");
  test.ASSERT_EQUAL (rstrip ("x x x x x  ", "x "), std::string ());

  // remove char
  test.ASSERT_EQUAL (remove_char (std::string (), 'a'), std::string ());
  test.ASSERT_EQUAL (remove_char ("   ", ' '), std::string ());
  test.ASSERT_EQUAL (remove_char ("abc", 'b'), "ac");
  test.ASSERT_EQUAL (remove_char ("abc ", 'a'), "bc ");
  test.ASSERT_EQUAL (remove_char (" abc", 'c'), " ab");
  test.ASSERT_EQUAL (remove_char (" a b c ", ' '), "abc");
  test.ASSERT_EQUAL (remove_char ("xxxxx axbxc", 'x'), " abc");
  test.ASSERT_EQUAL (remove_char ("x x x x x  ", ' '), "xxxxx");

  // split
  auto v1 = split ("abc", "/");
  test.ASSERT_EQUAL (v1.size (), 1);
  test.ASSERT_EQUAL (v1[0], "abc");

  auto v2 = split ("a/b/cd", "/");
  test.ASSERT_EQUAL (v2.size (), 3);
  test.ASSERT_EQUAL (v2[0], "a");
  test.ASSERT_EQUAL (v2[1], "b");
  test.ASSERT_EQUAL (v2[2], "cd");

  auto v3 = split ("::a::b::cd::", "::");
  test.ASSERT_EQUAL (v3.size (), 5);
  test.ASSERT_EQUAL (v3[0], "");
  test.ASSERT_EQUAL (v3[1], "a");
  test.ASSERT_EQUAL (v3[2], "b");
  test.ASSERT_EQUAL (v3[3], "cd");
  test.ASSERT_EQUAL (v3[4], "");

  // fnmatch
  test.ASSERT_TRUE (fnmatch ("*", "."));
  test.ASSERT_TRUE (fnmatch ("*", ".abc"));
  test.ASSERT_TRUE (fnmatch ("*", "abc"));
  test.ASSERT_TRUE (fnmatch ("*", ""));
  test.ASSERT_TRUE (fnmatch ("a*", "a"));
  test.ASSERT_TRUE (fnmatch ("a*", "abc"));
  test.ASSERT_FALSE (fnmatch ("a*a", "abc"));
  test.ASSERT_TRUE (fnmatch ("a*a", "aba"));
  test.ASSERT_TRUE (fnmatch ("a*a", "abba"));
  test.ASSERT_TRUE (fnmatch ("[0-9]", "1"));
  test.ASSERT_FALSE (fnmatch ("[0-9]", "a"));
  test.ASSERT_TRUE (fnmatch ("[0-9]*", "9a"));

  test.end ();
}
