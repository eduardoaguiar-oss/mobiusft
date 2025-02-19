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
#include <mobius/io/file.h>
#include <mobius/pod/data.h>
#include <mobius/pod/map.h>

namespace
{

static std::vector <mobius::pod::data>
_get_v ()
{
  std::vector <mobius::pod::data> v;

  v.push_back (mobius::pod::data ());
  v.push_back ("abc");
  v.push_back (std::string ("xyz"));
  v.push_back (.52);
  v.push_back (5000000000);
  v.push_back (-500);
  v.push_back (false);
  v.push_back (true);
  v.push_back (mobius::bytearray { 0x05, 0x28, 0x41, 0x45 });
  v.push_back (mobius::datetime::datetime (2008, 1, 1, 12, 45, 56));
  v.push_back (mobius::pod::map {{"zbc",mobius::pod::data ()}, {"a123", true}, {"a23", 5}});
  v.push_back ({ true, mobius::pod::data (), 500.5, "abc", {1.5, false, 18} });

  std::vector <mobius::pod::data> l (v[11]);
  l.push_back ("xxx");
  l.push_back (mobius::datetime::datetime (2020, 1, 1, 1, 25, 30));
  v[11] = l;

  return v;
}

static void
testcase_pod_objects ()
{
  mobius::unittest test ("mobius::pod objects");

  // create objects
  auto v = _get_v ();

  // test data
  test.ASSERT_EQUAL (v[0], mobius::pod::data ());
  test.ASSERT_EQUAL (v[1], "abc");
  test.ASSERT_EQUAL (v[2], "xyz");
  test.ASSERT_EQUAL (v[3], .52);
  test.ASSERT_EQUAL (v[4], 5000000000);
  test.ASSERT_EQUAL (v[5], -500);
  test.ASSERT_EQUAL (v[6], false);
  test.ASSERT_EQUAL (v[7], true);
  test.ASSERT_EQUAL (v[8], (mobius::bytearray {0x05, 0x28, 0x41, 0x45}));
  test.ASSERT_EQUAL (v[9], mobius::datetime::datetime (2008, 1, 1, 12, 45, 56));

  test.ASSERT_TRUE (v[0].is_null ());
  test.ASSERT_TRUE (v[1].is_string ());
  test.ASSERT_TRUE (v[2].is_string ());
  test.ASSERT_TRUE (v[3].is_float ());
  test.ASSERT_TRUE (v[4].is_integer ());
  test.ASSERT_TRUE (v[5].is_integer ());
  test.ASSERT_TRUE (v[6].is_bool ());
  test.ASSERT_TRUE (v[7].is_bool ());
  test.ASSERT_TRUE (v[8].is_bytearray ());
  test.ASSERT_TRUE (v[9].is_datetime ());
  test.ASSERT_TRUE (v[10].is_map ());
  test.ASSERT_TRUE (v[11].is_list ());

  test.end ();
}

static void
testcase_pod_map ()
{
  mobius::unittest test ("mobius::pod::map");

  mobius::pod::map map;

  test.ASSERT_EQUAL (map.get_size (), 0);
  test.ASSERT_EQUAL (map.to_string (), "{}");
  test.ASSERT_FALSE (map.contains ("key"));
  test.ASSERT_TRUE (map.get ("key").is_null ());

  map.set ("x", 5);
  map.set ("y", "abc");
  map.set ("z", mobius::bytearray {1, 2, 3});

  test.ASSERT_EQUAL (map.get_size (), 3);
  test.ASSERT_NOT_EQUAL (map.to_string (), "{}");
  test.ASSERT_TRUE (map.contains ("x"));
  test.ASSERT_TRUE (map.contains ("y"));
  test.ASSERT_TRUE (map.contains ("z"));
  test.ASSERT_FALSE (map.contains ("a"));
  test.ASSERT_EQUAL (map.get ("x"), 5);
  test.ASSERT_EQUAL ((*map.begin ()).first, "x");
  test.ASSERT_EQUAL ((*--(map.end ())).first, "z");

  map.set ("x", "other value");
  test.ASSERT_EQUAL (map.get ("x"), "other value");
  test.ASSERT_EQUAL (map.get_size (), 3);
  test.ASSERT_EQUAL ((*map.begin ()).first, "y");
  test.ASSERT_EQUAL ((*--(map.end ())).first, "x");

  test.end ();
}

static void
testcase_pod_serialize_string ()
{
  mobius::unittest test ("mobius::pod::serialize/unserialize bytearray");

  // create objects
  auto v = _get_v ();

  // serialize/unserialize
  auto data = mobius::pod::serialize (v);
  std::vector <mobius::pod::data> v2 (mobius::pod::unserialize (data));

  test.ASSERT_EQUAL (v2[0], mobius::pod::data ());
  test.ASSERT_EQUAL (v2[1], "abc");
  test.ASSERT_EQUAL (v2[2], "xyz");
  test.ASSERT_TRUE (static_cast <long double> (v2[3]) - .52 < .00001);
  test.ASSERT_EQUAL (v2[4], 5000000000);
  test.ASSERT_EQUAL (v2[5], -500);
  test.ASSERT_EQUAL (v2[6], false);
  test.ASSERT_EQUAL (v2[7], true);
  test.ASSERT_EQUAL (v2[8], (mobius::bytearray {0x05, 0x28, 0x41, 0x45}));
  test.ASSERT_EQUAL (v2[9], mobius::datetime::datetime (2008, 1, 1, 12, 45, 56));
  test.ASSERT_EQUAL (v2[10], v[10]);
  test.ASSERT_EQUAL (v2[11], v[11]);

  test.end ();
}

static void
testcase_pod_serialize_reader ()
{
  mobius::unittest test ("mobius::pod::serialize/unserialize reader/writer");

  // create objects
  auto v = _get_v ();

  // serialize/unserialize
  auto f = mobius::io::new_file_by_path ("/tmp/data.pod");
  mobius::pod::serialize (f.new_writer (), v);

  std::vector <mobius::pod::data> v2 (mobius::pod::unserialize (f.new_reader ()));

  test.ASSERT_EQUAL (v2[0], mobius::pod::data ());
  test.ASSERT_EQUAL (v2[1], "abc");
  test.ASSERT_EQUAL (v2[2], "xyz");
  test.ASSERT_TRUE (static_cast <long double> (v2[3]) - .52 < .00001);
  test.ASSERT_EQUAL (v2[4], 5000000000);
  test.ASSERT_EQUAL (v2[5], -500);
  test.ASSERT_EQUAL (v2[6], false);
  test.ASSERT_EQUAL (v2[7], true);
  test.ASSERT_EQUAL (v2[8], (mobius::bytearray {0x05, 0x28, 0x41, 0x45}));
  test.ASSERT_EQUAL (v2[9], mobius::datetime::datetime (2008, 1, 1, 12, 45, 56));
  test.ASSERT_EQUAL (v2[10], v[10]);
  test.ASSERT_EQUAL (v2[11], v[11]);

  test.end ();
}

} // namespace

void
unittest_pod ()
{
  testcase_pod_objects ();
  testcase_pod_map ();
  testcase_pod_serialize_string ();
  testcase_pod_serialize_reader ();
}
