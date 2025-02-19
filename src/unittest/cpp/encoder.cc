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
#include <mobius/unittest.h>
#include <mobius/encoder/base64.h>

static void
testcase_base16 ()
{
  mobius::unittest test ("mobius::encoder::base16");

  test.ASSERT_EQUAL (mobius::encoder::base16 (""), "");
  test.ASSERT_EQUAL (mobius::encoder::base16 ("f"), "66");
  test.ASSERT_EQUAL (mobius::encoder::base16 ("fo"), "666F");
  test.ASSERT_EQUAL (mobius::encoder::base16 ("foo"), "666F6F");
  test.ASSERT_EQUAL (mobius::encoder::base16 ("foob"), "666F6F62");
  test.ASSERT_EQUAL (mobius::encoder::base16 ("fooba"), "666F6F6261");
  test.ASSERT_EQUAL (mobius::encoder::base16 ("foobar"), "666F6F626172");

  test.end ();
}

static void
testcase_base32 ()
{
  mobius::unittest test ("mobius::encoder::base32");

  test.ASSERT_EQUAL (mobius::encoder::base32 (""), "");
  test.ASSERT_EQUAL (mobius::encoder::base32 ("f"), "MY======");
  test.ASSERT_EQUAL (mobius::encoder::base32 ("fo"), "MZXQ====");
  test.ASSERT_EQUAL (mobius::encoder::base32 ("foo"), "MZXW6===");
  test.ASSERT_EQUAL (mobius::encoder::base32 ("foob"), "MZXW6YQ=");
  test.ASSERT_EQUAL (mobius::encoder::base32 ("fooba"), "MZXW6YTB");
  test.ASSERT_EQUAL (mobius::encoder::base32 ("foobar"), "MZXW6YTBOI======");

  test.end ();
}

static void
testcase_base32hex ()
{
  mobius::unittest test ("mobius::encoder::base32hex");

  test.ASSERT_EQUAL (mobius::encoder::base32hex (""), "");
  test.ASSERT_EQUAL (mobius::encoder::base32hex ("f"), "CO======");
  test.ASSERT_EQUAL (mobius::encoder::base32hex ("fo"), "CPNG====");
  test.ASSERT_EQUAL (mobius::encoder::base32hex ("foo"), "CPNMU===");
  test.ASSERT_EQUAL (mobius::encoder::base32hex ("foob"), "CPNMUOG=");
  test.ASSERT_EQUAL (mobius::encoder::base32hex ("fooba"), "CPNMUOJ1");
  test.ASSERT_EQUAL (mobius::encoder::base32hex ("foobar"), "CPNMUOJ1E8======");

  test.end ();
}

static void
testcase_base64 ()
{
  mobius::unittest test ("mobius::encoder::base64");

  test.ASSERT_EQUAL (mobius::encoder::base64 (""), "");
  test.ASSERT_EQUAL (mobius::encoder::base64 ("f"), "Zg==");
  test.ASSERT_EQUAL (mobius::encoder::base64 ("fo"), "Zm8=");
  test.ASSERT_EQUAL (mobius::encoder::base64 ("foo"), "Zm9v");
  test.ASSERT_EQUAL (mobius::encoder::base64 ("foob"), "Zm9vYg==");
  test.ASSERT_EQUAL (mobius::encoder::base64 ("fooba"), "Zm9vYmE=");
  test.ASSERT_EQUAL (mobius::encoder::base64 ("foobar"), "Zm9vYmFy");

  test.end ();
}

void
unittest_encoder ()
{
  testcase_base16 ();
  testcase_base32 ();
  testcase_base32hex ();
  testcase_base64 ();
}
