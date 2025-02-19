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
#include <mobius/vfs/imagefile.h>

static void
testcase_imagefile_null ()
{
  mobius::unittest test ("mobius::vfs::imagefile (null)");

  mobius::vfs::imagefile imagefile;

  // get_type
  ASSERT_EXCEPTION (test, imagefile.get_type (), std::runtime_error);

  // get_size
  ASSERT_EXCEPTION (test, imagefile.get_size (), std::runtime_error);

  // get_sectors
  ASSERT_EXCEPTION (test, imagefile.get_sectors (), std::runtime_error);

  // get_sector_size
  ASSERT_EXCEPTION (test, imagefile.get_sector_size (), std::runtime_error);

  // new_reader
  ASSERT_EXCEPTION (test, imagefile.new_reader (), std::runtime_error);

  // new_writer
  ASSERT_EXCEPTION (test, imagefile.new_writer (), std::runtime_error);

  test.end ();
}

static void
testcase_imagefile_raw ()
{
  mobius::unittest test ("mobius::vfs::imagefile (raw)");

  const std::string url = "file:///etc/hosts";
  auto imagefile = mobius::vfs::new_imagefile_by_url (url, "raw");

  // get_type
  test.ASSERT_EQUAL (imagefile.get_type (), "raw");

  // get_size
  test.ASSERT_NOT_EQUAL (imagefile.get_size (), 0);

  // get_sectors
  test.ASSERT_NOT_EQUAL (imagefile.get_sectors (), 0);

  // get_sector_size
  test.ASSERT_EQUAL (imagefile.get_sector_size (), 512);

  // get_last_metadata_time
  test.ASSERT_TRUE (imagefile.get_attribute ("last_metadata_time").is_datetime ());

  // get_last_modification_time
  test.ASSERT_TRUE (imagefile.get_attribute ("last_modification_time").is_datetime ());

  // get_last_access_time
  test.ASSERT_TRUE (imagefile.get_attribute ("last_access_time").is_datetime ());

  // new_reader
  auto reader = imagefile.new_reader ();
  auto data = reader.read (2);
  test.ASSERT_EQUAL (data.size (), 2);

  test.end ();
}

static void
testcase_imagefile_split ()
{
  mobius::unittest test ("mobius::vfs::imagefile (split)");
  const std::string url = "file:///tmp/unittest.001";

  // create imagefile
  {
    auto imagefile = mobius::vfs::new_imagefile_by_url (url, "split");
    imagefile.set_attribute ("segment_size", 100000);

    auto writer = imagefile.new_writer ();
    writer.seek (100);
    writer.write ("abc\r\n");

    writer.seek (0);
    writer.write ("def");

    writer.seek (99998);
    writer.write ("1234");

    writer.seek (3);
    writer.write ("ghi");
  }

  // read imagefile created
  auto imagefile = mobius::vfs::new_imagefile_by_url (url);

  // get_type
  test.ASSERT_EQUAL (imagefile.get_type (), "split");

  // get_size
  test.ASSERT_EQUAL (imagefile.get_size (), 100002);

  // get_sectors
  test.ASSERT_EQUAL (imagefile.get_sectors (), 196);

  // get_sector_size
  test.ASSERT_EQUAL (imagefile.get_sector_size (), 512);

  // get_segments
  test.ASSERT_EQUAL (imagefile.get_attribute ("segments"), 2);

  // get_segment_size
  test.ASSERT_EQUAL (imagefile.get_attribute ("segment_size"), 100000);

  // get_acquisition_user
  test.ASSERT_NOT_EQUAL (imagefile.get_attribute ("acquisition_user"), std::string ());

  // get_acquisition_time
  test.ASSERT_TRUE (imagefile.get_attribute ("acquisition_time").is_datetime ());

  // data
  auto reader = imagefile.new_reader ();

  mobius::bytearray b1 = reader.read (6);
  test.ASSERT_EQUAL (b1, "defghi");

  reader.seek (100);
  mobius::bytearray b2 = reader.read (5);
  test.ASSERT_EQUAL (b2, "abc\r\n");

  reader.seek (99998);
  mobius::bytearray b3 = reader.read (4);
  test.ASSERT_EQUAL (b3, "1234");

  test.end ();
}

void
unittest_imagefile ()
{
  testcase_imagefile_null ();
  testcase_imagefile_raw ();
  testcase_imagefile_split ();
}
