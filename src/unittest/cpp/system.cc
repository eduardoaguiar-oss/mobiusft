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
#include <mobius/system/group.h>
#include <mobius/system/user.h>
#include <stdexcept>

static void
testcase_group ()
{
  mobius::unittest test ("mobius::system::group");

  mobius::system::group g0;
  mobius::system::group g1 (0);

  // group ID
  ASSERT_EXCEPTION (test, g0.get_id (), std::runtime_error);
  test.ASSERT_EQUAL (g1.get_id (), 0);

  // group name
  ASSERT_EXCEPTION (test, g0.get_name (), std::runtime_error);
  test.ASSERT_EQUAL (g1.get_name (), "root");

  test.end ();
}

static void
testcase_user ()
{
  mobius::unittest test ("mobius::system::user");

  mobius::system::user u0;
  mobius::system::user u1 (0);

  // user ID
  ASSERT_EXCEPTION (test, u0.get_id (), std::runtime_error);
  test.ASSERT_EQUAL (u1.get_id (), 0);

  // user name
  ASSERT_EXCEPTION (test, u0.get_name (), std::runtime_error);
  test.ASSERT_EQUAL (u1.get_name (), "root");

  // group
  ASSERT_EXCEPTION (test, u0.get_group (), std::runtime_error);
  mobius::system::group g = u1.get_group ();

  test.ASSERT_EQUAL (g.get_id (), 0);
  test.ASSERT_EQUAL (g.get_name (), "root");

  test.end ();
}

void
unittest_system ()
{
  testcase_group ();
  testcase_user ();
}
