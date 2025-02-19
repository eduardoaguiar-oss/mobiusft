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
#include <mobius/core/application.h>
#include <mobius/core/resource.h>
#include <mobius/core/thread_guard.h>
#include <thread>


static void
testcase_application ()
{
  mobius::unittest test ("mobius::core::application");
  mobius::core::application app;
  mobius::core::application app2;

  // constants
  test.ASSERT_EQUAL (app.get_name (), "Mobius Forensic Toolkit");
  test.ASSERT_NOT_EQUAL (app.get_version (), "");
  test.ASSERT_NOT_EQUAL (app.get_title (), "");
  test.ASSERT_NOT_EQUAL (app.get_copyright (), "");

  // unique reference
  test.ASSERT_EQUAL (app.get_name (), app2.get_name ());
  test.ASSERT_EQUAL (app.get_version (), app2.get_version ());
  test.ASSERT_EQUAL (app.get_title (), app2.get_title ());
  test.ASSERT_EQUAL (app.get_copyright (), app2.get_copyright ());

  // paths
  test.ASSERT_TRUE (app.get_config_path ("").length () > 0);
  test.ASSERT_EQUAL (app.get_config_path ("./"), app.get_config_path (""));

  test.ASSERT_TRUE (app.get_cache_path ("").length () > 0);
  test.ASSERT_EQUAL (app.get_cache_path ("./"), app.get_cache_path (""));

  test.ASSERT_TRUE (app.get_data_path ("").length () > 0);
  test.ASSERT_EQUAL (app.get_data_path ("./"), app.get_data_path (""));

  ASSERT_EXCEPTION (test, app.get_config_path ("../usr/lib"), std::invalid_argument);
  ASSERT_EXCEPTION (test, app.get_cache_path ("../usr/lib"), std::invalid_argument);

  test.end ();
}

static void
testcase_resource ()
{
  mobius::unittest test ("mobius::core::resource");

  mobius::core::add_resource ("unittest.x", "x var", 5);
  mobius::core::add_resource ("unittest.y", "y var", std::string ("abc"));

  test.ASSERT_TRUE (mobius::core::has_resource ("unittest.x"));
  test.ASSERT_TRUE (mobius::core::has_resource ("unittest.y"));
  test.ASSERT_FALSE (mobius::core::has_resource ("unittest.z"));

  test.ASSERT_EQUAL (mobius::core::get_resource_value <int> ("unittest.x"), 5);
  test.ASSERT_EQUAL (mobius::core::get_resource_value <std::string> ("unittest.y"), "abc");
  ASSERT_EXCEPTION (test, mobius::core::get_resource_value <std::string> ("unittest.x"), std::invalid_argument);
  ASSERT_EXCEPTION (test, mobius::core::get_resource_value <int> ("unittest.z"), std::runtime_error);

  auto resources = mobius::core::get_resources ("unittest");
  test.ASSERT_EQUAL (resources.size (), 2);

  mobius::core::remove_resource ("unittest.z");
  mobius::core::remove_resource ("unittest.x");
  test.ASSERT_FALSE (mobius::core::has_resource ("unittest.x"));
  test.ASSERT_TRUE (mobius::core::has_resource ("unittest.y"));

  resources = mobius::core::get_resources ("unittest");
  test.ASSERT_EQUAL (resources.size (), 1);

  test.end ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Test thread_guard class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
thread_func (mobius::unittest& test)
{
  {
  auto guard = mobius::core::thread_guard ();

  mobius::core::set_thread_resource("x", 5);
  test.ASSERT_TRUE (mobius::core::has_thread_resource ("x"));
  test.ASSERT_EQUAL (mobius::core::get_thread_resource<int> ("x"), 5);

  mobius::core::remove_thread_resource ("x");
  test.ASSERT_FALSE (mobius::core::has_thread_resource ("x"));
  }

  {
  auto guard2 = mobius::core::thread_guard ();
  mobius::core::set_thread_resource("x", 1024);
  test.ASSERT_TRUE (mobius::core::has_thread_resource ("x"));
  test.ASSERT_EQUAL (mobius::core::get_thread_resource<int> ("x"), 1024);
  }
}

static void
testcase_thread_guard ()
{
  mobius::unittest test ("mobius::core::thread_guard");

  ASSERT_EXCEPTION (test, mobius::core::thread_guard (), std::runtime_error);

  mobius::core::set_thread_resource ("x", 8);
  test.ASSERT_TRUE (mobius::core::has_thread_resource ("x"));
  test.ASSERT_EQUAL (mobius::core::get_thread_resource<int> ("x"), 8);

  mobius::core::remove_thread_resource ("x");
  test.ASSERT_FALSE (mobius::core::has_thread_resource ("x"));

  auto t = std::thread (thread_func, std::ref(test));
  t.join();

  mobius::core::set_thread_resource ("x", std::string {"abc"});
  test.ASSERT_TRUE (mobius::core::has_thread_resource ("x"));
  test.ASSERT_EQUAL (mobius::core::get_thread_resource<std::string> ("x"), "abc");

  test.end ();
}

void
unittest_core ()
{
  testcase_application ();
  testcase_resource ();
  testcase_thread_guard ();
}
