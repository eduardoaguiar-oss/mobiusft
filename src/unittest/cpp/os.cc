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
#include <mobius/os/win/hash_lm.h>
#include <mobius/os/win/hash_msdcc1.h>
#include <mobius/os/win/hash_msdcc2.h>
#include <mobius/os/win/hash_nt.h>
#include <mobius/unittest.h>

static void
testcase_hash_nt ()
{
  mobius::unittest test ("mobius::os::win::hash_nt");

  auto hash_nt = [] (const std::string& pwd) -> std::string { return mobius::os::win::hash_nt (pwd).to_hexstring (); };

  test.ASSERT_EQUAL (hash_nt (""), "31d6cfe0d16ae931b73c59d7e0c089c0");
  test.ASSERT_EQUAL (hash_nt ("123456"), "32ed87bdb5fdc5e9cba88547376818d4");
  test.ASSERT_EQUAL (hash_nt ("user"), "57d583aa46d571502aad4bb7aea09c70");

  test.end ();
}

static void
testcase_hash_lm ()
{
  mobius::unittest test ("mobius::os::win::hash_lm");

  auto hash_lm = [] (const std::string& pwd) -> std::string { return mobius::os::win::hash_lm (pwd).to_hexstring (); };

  test.ASSERT_EQUAL (hash_lm (""), "aad3b435b51404eeaad3b435b51404ee");
  test.ASSERT_EQUAL (hash_lm ("123456"), "44efce164ab921caaad3b435b51404ee");
  test.ASSERT_EQUAL (hash_lm ("user"), "22124ea690b83bfbaad3b435b51404ee");
  test.ASSERT_EQUAL (hash_lm ("USER"), "22124ea690b83bfbaad3b435b51404ee");

  test.end ();
}

static void
testcase_hash_msdcc1 ()
{
  mobius::unittest test ("mobius::os::win::hash_msdcc1");

  auto hash_msdcc1 = [] (const std::string& pwd, const std::string& username) -> std::string { return mobius::os::win::hash_msdcc1 (pwd, username).to_hexstring (); };

  // test cases from JTR (http://www.openwall.com/john)
  test.ASSERT_EQUAL (hash_msdcc1 ("", "root"), "176a4c2bd45ac73687676c2f09045353");
  test.ASSERT_EQUAL (hash_msdcc1 ("test2", "test2"), "ab60bdb4493822b175486810ac2abe63");
  test.ASSERT_EQUAL (hash_msdcc1 ("test2", "TEST2"), "ab60bdb4493822b175486810ac2abe63");

  test.end ();
}

static void
testcase_hash_msdcc2 ()
{
  mobius::unittest test ("mobius::os::win::hash_msdcc2");

  auto hash_msdcc2 = [] (const std::string& pwd, const std::string& username, int i) -> std::string { return mobius::os::win::hash_msdcc2 (pwd, username, i).to_hexstring (); };

  // test cases from JTR (http://www.openwall.com/john)
  test.ASSERT_EQUAL (hash_msdcc2 ("", "bin", 10240), "c0cbe0313a861062e29f92ede58f9b36");
  test.ASSERT_EQUAL (hash_msdcc2 ("test1", "test1", 10240), "607bbe89611e37446e736f7856515bf8");
  test.ASSERT_EQUAL (hash_msdcc2 ("qerwt", "Joe", 10240), "e09b38f84ab0be586b730baf61781e30");
  test.ASSERT_EQUAL (hash_msdcc2 ("12345", "Joe", 10240), "6432f517a900b3fc34ffe57f0f346e16");

  test.end ();
}

void
unittest_os ()
{
  testcase_hash_nt ();
  testcase_hash_lm ();
  testcase_hash_msdcc1 ();
  testcase_hash_msdcc2 ();
}
