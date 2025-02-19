// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <mobius/charset.h>
#include <mobius/unittest.h>

void
testcase_conv_charset_to_utf8 ()
{
  mobius::unittest test ("mobius::conv_charset_to_utf8");
  mobius::bytearray b1 = "abcdef";
  mobius::bytearray b2 = "realiza\xe7\xe3o";
  mobius::bytearray b3 = {'a', 0, 'b', 0, 'c', 0, 'e', 0};

  test.ASSERT_EQUAL (mobius::conv_charset_to_utf8 (b1, "ASCII"), "abcdef");
  test.ASSERT_EQUAL (mobius::conv_charset_to_utf8 (b2, "CP1252"), "realização");
  test.ASSERT_EQUAL (mobius::conv_charset_to_utf8 (b3, "UTF-16LE"), "abce");

  test.end ();
}

void
testcase_conv_charset ()
{
  mobius::unittest test ("mobius::conv_charset");
  mobius::bytearray b1 = {'a', 0, 'b', 0, 'c', 0, 'e', 0};
  mobius::bytearray b2 = "ação";                                // utf-8
  mobius::bytearray b3 = {'a', 0, 0xe7, 0, 0xe3, 0, 'o', 0};    // utf-16le
  mobius::bytearray b4 = {'a', 0xe7, 0xe3, 'o'};                // cp1252

  test.ASSERT_EQUAL (mobius::conv_charset (b1, "UTF-16LE", "ASCII"), "abce");
  test.ASSERT_EQUAL (mobius::conv_charset (b2, "UTF-8", "UTF-16LE"), b3);
  test.ASSERT_EQUAL (mobius::conv_charset (b3, "UTF-16LE", "CP1252"), b4);
  test.ASSERT_EQUAL (mobius::conv_charset (b3, "UTF-16LE", "UTF-8"), b2);
  test.end ();
}

void
unittest_charset ()
{
  testcase_conv_charset_to_utf8 ();
  testcase_conv_charset ();
}
