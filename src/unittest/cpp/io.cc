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
#include <mobius/io/path.h>
#include <mobius/io/file.h>
#include <mobius/io/folder.h>
#include <mobius/io/uri.h>
#include <mobius/io/sequential_reader_adaptor.h>
#include <mobius/io/bytearray_io.h>

static void
testcase_path ()
{
  mobius::unittest test ("mobius::io::path");

  // constructor (separator = '/')
  mobius::io::path p1a = "/usr/lib64/Libc.so";
  mobius::io::path p2a = "//usr/lib64/Libc.so";
  mobius::io::path p3a = "//usr////lib64/////./libc.so";
  mobius::io::path p4a = "////usr///lib/../lib64/////./libc.so";
  mobius::io::path p5a = "////usr///lib/../lib64/////./libc.so/../libc2.so";
  mobius::io::path p6a = "////usr///lib/../lib64/////./libc.so/../libc2.so//";
  mobius::io::path p7a = "./usr/lib64/libc.so";

  // constructor (separator = '\\')
  mobius::io::path p1b = mobius::io::new_path_from_win (R"(\usr\lib64\Libc.so)");
  mobius::io::path p2b = mobius::io::new_path_from_win (R"(\\usr\lib64\libc.so)");
  mobius::io::path p3b = mobius::io::new_path_from_win (R"(\\usr\\\\lib64\\\\\.\libc.so)");
  mobius::io::path p4b = mobius::io::new_path_from_win (R"(\\\\usr\\\lib\..\lib64\\\\\.\libc.so)");
  mobius::io::path p5b = mobius::io::new_path_from_win (R"(\\\\usr\\\lib\..\lib64\\\\\.\libc.so\..\libc2.so)");
  mobius::io::path p6b = mobius::io::new_path_from_win (R"(\\\\usr\\\lib\..\lib64\\\\\.\libc.so\..\libc2.so\\)");
  mobius::io::path p7b = mobius::io::new_path_from_win (R"(.\usr\lib64\libc.so)");

  // to_string
  test.ASSERT_EQUAL (to_string (p1a), "/usr/lib64/Libc.so");
  test.ASSERT_EQUAL (to_string (p2a), "/usr/lib64/Libc.so");
  test.ASSERT_EQUAL (to_string (p3a), "/usr/lib64/libc.so");
  test.ASSERT_EQUAL (to_string (p4a), "/usr/lib64/libc.so");
  test.ASSERT_EQUAL (to_string (p5a), "/usr/lib64/libc2.so");
  test.ASSERT_EQUAL (to_string (p6a), "/usr/lib64/libc2.so/");
  test.ASSERT_EQUAL (to_string (p7a), "usr/lib64/libc.so");
  test.ASSERT_EQUAL (to_string (p1b), "/usr/lib64/Libc.so");
  test.ASSERT_EQUAL (to_string (p2b), "/usr/lib64/libc.so");
  test.ASSERT_EQUAL (to_string (p3b), "/usr/lib64/libc.so");
  test.ASSERT_EQUAL (to_string (p4b), "/usr/lib64/libc.so");
  test.ASSERT_EQUAL (to_string (p5b), "/usr/lib64/libc2.so");
  test.ASSERT_EQUAL (to_string (p6b), "/usr/lib64/libc2.so/");
  test.ASSERT_EQUAL (to_string (p7b), "usr/lib64/libc.so");

  // dirname
  test.ASSERT_EQUAL (p1a.get_dirname (), "/usr/lib64");
  test.ASSERT_EQUAL (p2a.get_dirname (), "/usr/lib64");
  test.ASSERT_EQUAL (p3a.get_dirname (), "/usr/lib64");
  test.ASSERT_EQUAL (p4a.get_dirname (), "/usr/lib64");
  test.ASSERT_EQUAL (p5a.get_dirname (), "/usr/lib64");
  test.ASSERT_EQUAL (p6a.get_dirname (), "/usr/lib64/libc2.so");
  test.ASSERT_EQUAL (p7a.get_dirname (), "usr/lib64");
  test.ASSERT_EQUAL (p1b.get_dirname (), "/usr/lib64");
  test.ASSERT_EQUAL (p2b.get_dirname (), "/usr/lib64");
  test.ASSERT_EQUAL (p3b.get_dirname (), "/usr/lib64");
  test.ASSERT_EQUAL (p4b.get_dirname (), "/usr/lib64");
  test.ASSERT_EQUAL (p5b.get_dirname (), "/usr/lib64");
  test.ASSERT_EQUAL (p6b.get_dirname (), "/usr/lib64/libc2.so");
  test.ASSERT_EQUAL (p7b.get_dirname (), "usr/lib64");

  // filename
  test.ASSERT_EQUAL (p1a.get_filename (), "Libc.so");
  test.ASSERT_EQUAL (p2a.get_filename (), "Libc.so");
  test.ASSERT_EQUAL (p3a.get_filename (), "libc.so");
  test.ASSERT_EQUAL (p4a.get_filename (), "libc.so");
  test.ASSERT_EQUAL (p5a.get_filename (), "libc2.so");
  test.ASSERT_EQUAL (p6a.get_filename (), "");
  test.ASSERT_EQUAL (p7a.get_filename (), "libc.so");
  test.ASSERT_EQUAL (p1b.get_filename (), "Libc.so");
  test.ASSERT_EQUAL (p2b.get_filename (), "libc.so");
  test.ASSERT_EQUAL (p3b.get_filename (), "libc.so");
  test.ASSERT_EQUAL (p4b.get_filename (), "libc.so");
  test.ASSERT_EQUAL (p5b.get_filename (), "libc2.so");
  test.ASSERT_EQUAL (p6b.get_filename (), "");
  test.ASSERT_EQUAL (p7b.get_filename (), "libc.so");

  // prefix
  test.ASSERT_EQUAL (p1a.get_prefix (), "/usr/lib64/Libc");
  test.ASSERT_EQUAL (p2a.get_prefix (), "/usr/lib64/Libc");
  test.ASSERT_EQUAL (p3a.get_prefix (), "/usr/lib64/libc");
  test.ASSERT_EQUAL (p4a.get_prefix (), "/usr/lib64/libc");
  test.ASSERT_EQUAL (p5a.get_prefix (), "/usr/lib64/libc2");
  test.ASSERT_EQUAL (p6a.get_prefix (), "");
  test.ASSERT_EQUAL (p7a.get_prefix (), "usr/lib64/libc");
  test.ASSERT_EQUAL (p1b.get_prefix (), "/usr/lib64/Libc");
  test.ASSERT_EQUAL (p2b.get_prefix (), "/usr/lib64/libc");
  test.ASSERT_EQUAL (p3b.get_prefix (), "/usr/lib64/libc");
  test.ASSERT_EQUAL (p4b.get_prefix (), "/usr/lib64/libc");
  test.ASSERT_EQUAL (p5b.get_prefix (), "/usr/lib64/libc2");
  test.ASSERT_EQUAL (p6b.get_prefix (), "");
  test.ASSERT_EQUAL (p7b.get_prefix (), "usr/lib64/libc");

  // extension
  test.ASSERT_EQUAL (p1a.get_extension (), "so");
  test.ASSERT_EQUAL (p2a.get_extension (), "so");
  test.ASSERT_EQUAL (p3a.get_extension (), "so");
  test.ASSERT_EQUAL (p4a.get_extension (), "so");
  test.ASSERT_EQUAL (p5a.get_extension (), "so");
  test.ASSERT_EQUAL (p6a.get_extension (), "");
  test.ASSERT_EQUAL (p7a.get_extension (), "so");
  test.ASSERT_EQUAL (p1b.get_extension (), "so");
  test.ASSERT_EQUAL (p2b.get_extension (), "so");
  test.ASSERT_EQUAL (p3b.get_extension (), "so");
  test.ASSERT_EQUAL (p4b.get_extension (), "so");
  test.ASSERT_EQUAL (p5b.get_extension (), "so");
  test.ASSERT_EQUAL (p6b.get_extension (), "");
  test.ASSERT_EQUAL (p7b.get_extension (), "so");

  // is_absolute
  test.ASSERT_TRUE (p1a.is_absolute ());
  test.ASSERT_TRUE (p2a.is_absolute ());
  test.ASSERT_TRUE (p3a.is_absolute ());
  test.ASSERT_TRUE (p4a.is_absolute ());
  test.ASSERT_TRUE (p5a.is_absolute ());
  test.ASSERT_TRUE (p6a.is_absolute ());
  test.ASSERT_FALSE (p7a.is_absolute ());
  test.ASSERT_TRUE (p1b.is_absolute ());
  test.ASSERT_TRUE (p2b.is_absolute ());
  test.ASSERT_TRUE (p3b.is_absolute ());
  test.ASSERT_TRUE (p4b.is_absolute ());
  test.ASSERT_TRUE (p5b.is_absolute ());
  test.ASSERT_TRUE (p6b.is_absolute ());
  test.ASSERT_FALSE (p7b.is_absolute ());

  // filename_match
  test.ASSERT_TRUE (p1a.filename_match ("Libc.*"));
  test.ASSERT_TRUE (p1a.filename_match ("*.so"));
  test.ASSERT_TRUE (p1a.filename_match ("*.so"));
  test.ASSERT_TRUE (p1a.filename_match ("L*.so"));
  test.ASSERT_FALSE (p1a.filename_match ("*.so1"));
  test.ASSERT_FALSE (p1a.filename_match ("l?bx.so"));
  test.ASSERT_FALSE (p6a.filename_match ("libc.*"));
  test.ASSERT_FALSE (p6a.filename_match ("*.so"));
  test.ASSERT_FALSE (p6a.filename_match ("*.so"));
  test.ASSERT_FALSE (p6a.filename_match ("l*.so"));
  test.ASSERT_FALSE (p6a.filename_match ("*.so1"));
  test.ASSERT_FALSE (p1b.filename_match ("LIBC.*"));
  test.ASSERT_FALSE (p1b.filename_match ("*.So"));
  test.ASSERT_FALSE (p1b.filename_match ("*.SO"));
  test.ASSERT_TRUE (p1b.filename_match ("L*.so"));

  // to_win_path
  test.ASSERT_EQUAL (to_win_path (p1a), R"(\usr\lib64\Libc.so)");
  test.ASSERT_EQUAL (to_win_path (p2a), R"(\usr\lib64\Libc.so)");
  test.ASSERT_EQUAL (to_win_path (p3a), R"(\usr\lib64\libc.so)");
  test.ASSERT_EQUAL (to_win_path (p4a), R"(\usr\lib64\libc.so)");
  test.ASSERT_EQUAL (to_win_path (p5a), R"(\usr\lib64\libc2.so)");
  test.ASSERT_EQUAL (to_win_path (p6a), R"(\usr\lib64\libc2.so\)");
  test.ASSERT_EQUAL (to_win_path (p7a), R"(usr\lib64\libc.so)");
  test.ASSERT_EQUAL (to_win_path (p1b), R"(\usr\lib64\Libc.so)");
  test.ASSERT_EQUAL (to_win_path (p2b), R"(\usr\lib64\libc.so)");
  test.ASSERT_EQUAL (to_win_path (p3b), R"(\usr\lib64\libc.so)");
  test.ASSERT_EQUAL (to_win_path (p4b), R"(\usr\lib64\libc.so)");
  test.ASSERT_EQUAL (to_win_path (p5b), R"(\usr\lib64\libc2.so)");
  test.ASSERT_EQUAL (to_win_path (p6b), R"(\usr\lib64\libc2.so\)");
  test.ASSERT_EQUAL (to_win_path (p7b), R"(usr\lib64\libc.so)");

  // join
  test.ASSERT_EQUAL (to_string (join (p1a, p1a)), "/usr/lib64/Libc.so");
  test.ASSERT_EQUAL (to_string (join (p1a, mobius::io::path ("ext"))), "/usr/lib64/Libc.so/ext");
  test.ASSERT_EQUAL (to_string (join (p1a, mobius::io::path ("ext/lib2/usr"))), "/usr/lib64/Libc.so/ext/lib2/usr");
          
  test.end ();
}

static void
testcase_file ()
{
  mobius::unittest test ("mobius::io::file");

  mobius::io::file f0;
  auto f1 = mobius::io::new_file_by_path ("/tmp/unittest.dat");
  ASSERT_EXCEPTION (test, mobius::io::new_file_by_url ("invalid:///invalid/uri"), std::invalid_argument);
  auto f3 = mobius::io::new_file_by_path ("/etc/hosts");

  // is_valid
  test.ASSERT_FALSE (f0);
  test.ASSERT_TRUE (f1);
  test.ASSERT_TRUE (f3);

  // is_deleted
  ASSERT_EXCEPTION (test, f0.is_deleted (), std::runtime_error);
  test.ASSERT_FALSE (f3.is_deleted ());

  // is_reallocated
  ASSERT_EXCEPTION (test, f0.is_reallocated (), std::runtime_error);
  test.ASSERT_FALSE (f3.is_reallocated ());

  // is_hidden
  ASSERT_EXCEPTION (test, f0.is_hidden (), std::runtime_error);
  test.ASSERT_FALSE (f3.is_hidden ());

  // is_regular_file
  ASSERT_EXCEPTION (test, f0.is_regular_file (), std::runtime_error);
  test.ASSERT_TRUE (f3.is_regular_file ());

  // exists
  ASSERT_EXCEPTION (test, f0.exists (), std::runtime_error);
  test.ASSERT_TRUE (f3.exists ());

  // get_size
  ASSERT_EXCEPTION (test, f0.get_size (), std::runtime_error);
  test.ASSERT_NOT_EQUAL (f3.get_size (), 0);

  // user_id
  ASSERT_EXCEPTION (test, f0.get_user_id (), std::runtime_error);
  test.ASSERT_EQUAL (f3.get_user_id (), 0);

  // user_name
  ASSERT_EXCEPTION (test, f0.get_user_name (), std::runtime_error);
  test.ASSERT_EQUAL (f3.get_user_name (), "root");

  // group_id
  ASSERT_EXCEPTION (test, f0.get_group_id (), std::runtime_error);
  test.ASSERT_EQUAL (f3.get_group_id (), 0);

  // group_name
  ASSERT_EXCEPTION (test, f0.get_group_name (), std::runtime_error);
  test.ASSERT_EQUAL (f3.get_group_name (), "root");

  // access time
  ASSERT_EXCEPTION (test, f0.get_access_time (), std::runtime_error);
  test.ASSERT_TRUE (f3.get_access_time ());

  // modification time
  ASSERT_EXCEPTION (test, f0.get_modification_time (), std::runtime_error);
  test.ASSERT_TRUE (f3.get_modification_time ());

  // last metadata time
  ASSERT_EXCEPTION (test, f0.get_metadata_time (), std::runtime_error);
  test.ASSERT_TRUE (f3.get_metadata_time ());

  // write and read back
  ASSERT_EXCEPTION (test, f0.new_writer (), std::runtime_error);

  {
    auto writer = f1.new_writer ();
    writer.write ("abc");
    writer.write (mobius::bytearray {0x0d,0x0a});
  }

  ASSERT_EXCEPTION (test, f0.new_reader (), std::runtime_error);

  {
    auto reader = f1.new_reader ();
    mobius::bytearray b1 = reader.read (3);
    mobius::bytearray b2 = reader.read (2);

    test.ASSERT_EQUAL (b1, "abc");
    test.ASSERT_EQUAL (b2, "\r\n");
  }

  // append to the end and read back
  {
    auto writer = f1.new_writer (false);
    writer.write ("def");
    writer.write (mobius::bytearray {0x0d,0x0a});
  }

  {
    auto reader = f1.new_reader ();

    test.ASSERT_EQUAL (reader.read (3), "abc");
    test.ASSERT_EQUAL (reader.read (2), "\r\n");
    test.ASSERT_EQUAL (reader.read (3), "def");
    test.ASSERT_EQUAL (reader.read (2), "\r\n");
  }

  // remove
  auto f2 = mobius::io::new_file_by_path ("/tmp/unittest.dat");
  test.ASSERT_TRUE (f2.exists ());
  f2.remove ();
  test.ASSERT_FALSE (f2.exists ());

  // rename
  f2 = mobius::io::new_file_by_path ("/tmp/unittest.dat");
  {
    auto writer = f2.new_writer ();
    writer.write ("abc");
  }

  f2.rename ("unittest2.dat");
  test.ASSERT_EQUAL (f2.get_name (), "unittest2.dat");
  test.ASSERT_TRUE (f2.exists ());
  f2.remove ();
  test.ASSERT_FALSE (f2.exists ());

  test.end ();
}

static void
testcase_folder ()
{
  mobius::unittest test ("mobius::io::folder");

  mobius::io::folder f0;
  auto f1 = mobius::io::new_folder_by_path ("/etc");
  ASSERT_EXCEPTION (test, mobius::io::new_folder_by_url ("invalid:///invalid/uri"), std::invalid_argument);

  // is_valid
  test.ASSERT_FALSE (f0);
  test.ASSERT_TRUE (f1);

  // exists
  ASSERT_EXCEPTION (test, f0.exists (), std::runtime_error);
  test.ASSERT_TRUE (f1.exists ());

  // user_id
  ASSERT_EXCEPTION (test, f0.get_user_id (), std::runtime_error);
  test.ASSERT_EQUAL (f1.get_user_id (), 0);

  // user_name
  ASSERT_EXCEPTION (test, f0.get_user_name (), std::runtime_error);
  test.ASSERT_EQUAL (f1.get_user_name (), "root");

  // group_id
  ASSERT_EXCEPTION (test, f0.get_group_id (), std::runtime_error);
  test.ASSERT_EQUAL (f1.get_group_id (), 0);

  // group_name
  ASSERT_EXCEPTION (test, f0.get_group_name (), std::runtime_error);
  test.ASSERT_EQUAL (f1.get_group_name (), "root");

  // last access time
  ASSERT_EXCEPTION (test, f0.get_access_time (), std::runtime_error);
  test.ASSERT_TRUE (f1.get_access_time ());

  // last modification time
  ASSERT_EXCEPTION (test, f0.get_modification_time (), std::runtime_error);
  test.ASSERT_TRUE (f1.get_modification_time ());

  // last metadata time
  ASSERT_EXCEPTION (test, f0.get_metadata_time (), std::runtime_error);
  test.ASSERT_TRUE (f1.get_metadata_time ());

  // test children
  ASSERT_EXCEPTION (test, f0.get_children (), std::runtime_error);

  test.end ();
}

static void
testcase_reader ()
{
  mobius::unittest test ("mobius::io::reader");
  auto f = mobius::io::new_file_by_path ("/dev/zero");
  auto reader = f.new_reader ();

  // capabilities
  test.ASSERT_TRUE (reader.is_seekable ());
  test.ASSERT_TRUE (reader.is_rewindable ());
  test.ASSERT_TRUE (reader.is_sizeable ());

  // initial values
  test.ASSERT_EQUAL (reader.get_size (), 0);
  test.ASSERT_EQUAL (reader.eof (), true);
  test.ASSERT_EQUAL (reader.tell (), 0);

  // read bytes
  mobius::bytearray data = reader.read (16);
  test.ASSERT_EQUAL (data.size (), 16);
  test.ASSERT_EQUAL (data[0], 0);
  test.ASSERT_EQUAL (data[15], 0);

  // values
  test.ASSERT_EQUAL (reader.eof (), true);
  test.ASSERT_EQUAL (reader.tell (), 16);

  // seek
  reader.seek (100);
  test.ASSERT_EQUAL (reader.tell (), 100);

  // read more bytes
  data = reader.read (16);
  test.ASSERT_EQUAL (data.size (), 16);
  test.ASSERT_EQUAL (data[0], 0);
  test.ASSERT_EQUAL (data[15], 0);

  // values
  test.ASSERT_EQUAL (reader.eof (), true);
  test.ASSERT_EQUAL (reader.tell (), 116);

  test.end ();
}

static void
testcase_writer ()
{
  mobius::unittest test ("mobius::io::writer");
  auto f = mobius::io::new_file_by_path ("/tmp/unittest.dat");

  {
    auto writer = f.new_writer ();
    writer.write ("abc");
    writer.write (mobius::bytearray {0x0d,0x0a});
  }

  test.ASSERT_TRUE (f.exists ());

  auto reader = f.new_reader ();
  mobius::bytearray b1 = reader.read (3);
  mobius::bytearray b2 = reader.read (2);

  test.ASSERT_EQUAL (b1, "abc");
  test.ASSERT_EQUAL (b2, "\r\n");

  f.remove ();
  test.ASSERT_FALSE (f.exists ());

  test.end ();
}

static std::string
join (const mobius::io::uri& base, const std::string& rel)
{
  mobius::io::uri target = join (base, mobius::io::uri (rel));
  return target.get_value ();
}

static void
testcase_uri ()
{
  mobius::unittest test ("mobius::io::uri");

  // examples from RFC 3986 - section 1.1.2
  mobius::io::uri uri1 ("ftp://ftp.is.co.za/rfc/rfc1808.txt");
  mobius::io::uri uri2 ("http://www.ietf.org/rfc/rfc2396.txt");
  mobius::io::uri uri3 ("ldap://[2001:db8::7]/c=GB?objectClass?one");
  mobius::io::uri uri4 ("mailto:John.Doe@example.com");
  mobius::io::uri uri5 ("news:comp.infosystems.www.servers.unix");
  mobius::io::uri uri6 ("tel:+1-816-555-1212");
  mobius::io::uri uri7 ("telnet://192.0.2.16:80/");
  mobius::io::uri uri8 ("urn:oasis:names:specification:docbook:dtd:xml:4.1.2");

  // copy constructor and copy assignment
  mobius::io::uri uri_c1 = uri2;
  mobius::io::uri uri_c2;
  uri_c2 = uri2;

  // URI build from its parts
  mobius::io::uri uri9 ("https", "aguiar", "mypassword", "www.gnu.org", "80", "/test/subdir 1", "value=55&p=#14", "anchor1");

  // full URI
  mobius::io::uri uri10 ("https://aguiar:mypassword@www.gnu.org:80/test/subdir%201?value=55#anchor1");

  // copy constructor and copy assignment
  test.ASSERT_EQUAL (uri_c1, uri2);
  test.ASSERT_EQUAL (uri_c2, uri2);
  test.ASSERT_NOT_EQUAL (uri1, uri2);

  // empty URI
  mobius::io::uri uri11;
  
  // URI from path
  auto uri12 = mobius::io::new_uri_from_path ("/test with space%/a%b");

  // value
  test.ASSERT_EQUAL (uri1.get_value (), "ftp://ftp.is.co.za/rfc/rfc1808.txt");
  test.ASSERT_EQUAL (uri2.get_value (), "http://www.ietf.org/rfc/rfc2396.txt");
  test.ASSERT_EQUAL (uri3.get_value (), "ldap://[2001:db8::7]/c=GB?objectClass?one");
  test.ASSERT_EQUAL (uri4.get_value (), "mailto:John.Doe@example.com");
  test.ASSERT_EQUAL (uri5.get_value (), "news:comp.infosystems.www.servers.unix");
  test.ASSERT_EQUAL (uri6.get_value (), "tel:+1-816-555-1212");
  test.ASSERT_EQUAL (uri7.get_value (), "telnet://192.0.2.16:80/");
  test.ASSERT_EQUAL (uri8.get_value (), "urn:oasis:names:specification:docbook:dtd:xml:4.1.2");
  test.ASSERT_EQUAL (uri9.get_value (), "https://aguiar:mypassword@www.gnu.org:80/test/subdir%201?value=55&p=%2314#anchor1");
  test.ASSERT_EQUAL (uri10.get_value (), "https://aguiar:mypassword@www.gnu.org:80/test/subdir%201?value=55#anchor1");
  test.ASSERT_EQUAL (uri12.get_value (), "file:///test%20with%20space%25/a%25b");

  // is_empty
  test.ASSERT_FALSE (uri1.is_empty ());
  test.ASSERT_FALSE (uri2.is_empty ());
  test.ASSERT_FALSE (uri3.is_empty ());
  test.ASSERT_FALSE (uri4.is_empty ());
  test.ASSERT_FALSE (uri5.is_empty ());
  test.ASSERT_FALSE (uri6.is_empty ());
  test.ASSERT_FALSE (uri7.is_empty ());
  test.ASSERT_FALSE (uri8.is_empty ());
  test.ASSERT_FALSE (uri9.is_empty ());
  test.ASSERT_FALSE (uri10.is_empty ());
  test.ASSERT_TRUE (uri11.is_empty ());
  test.ASSERT_FALSE (uri12.is_empty ());

  // is_relative
  test.ASSERT_FALSE (uri1.is_relative ());
  test.ASSERT_FALSE (uri2.is_relative ());
  test.ASSERT_FALSE (uri3.is_relative ());
  test.ASSERT_FALSE (uri4.is_relative ());
  test.ASSERT_FALSE (uri5.is_relative ());
  test.ASSERT_FALSE (uri6.is_relative ());
  test.ASSERT_FALSE (uri7.is_relative ());
  test.ASSERT_FALSE (uri8.is_relative ());
  test.ASSERT_FALSE (uri9.is_relative ());
  test.ASSERT_FALSE (uri10.is_relative ());
  test.ASSERT_FALSE (uri11.is_relative ());
  test.ASSERT_FALSE (uri12.is_relative ());

  // is_absolute
  test.ASSERT_TRUE (uri1.is_absolute ());
  test.ASSERT_TRUE (uri2.is_absolute ());
  test.ASSERT_TRUE (uri3.is_absolute ());
  test.ASSERT_TRUE (uri4.is_absolute ());
  test.ASSERT_TRUE (uri5.is_absolute ());
  test.ASSERT_TRUE (uri6.is_absolute ());
  test.ASSERT_TRUE (uri7.is_absolute ());
  test.ASSERT_TRUE (uri8.is_absolute ());
  test.ASSERT_FALSE (uri9.is_absolute ());
  test.ASSERT_FALSE (uri10.is_absolute ());
  test.ASSERT_FALSE (uri11.is_absolute ());
  test.ASSERT_TRUE (uri12.is_absolute ());

  // scheme
  test.ASSERT_EQUAL (uri1.get_scheme (), "ftp");
  test.ASSERT_EQUAL (uri2.get_scheme (), "http");
  test.ASSERT_EQUAL (uri3.get_scheme (), "ldap");
  test.ASSERT_EQUAL (uri4.get_scheme (), "mailto");
  test.ASSERT_EQUAL (uri5.get_scheme (), "news");
  test.ASSERT_EQUAL (uri6.get_scheme (), "tel");
  test.ASSERT_EQUAL (uri7.get_scheme (), "telnet");
  test.ASSERT_EQUAL (uri8.get_scheme (), "urn");
  test.ASSERT_EQUAL (uri9.get_scheme (), "https");
  test.ASSERT_EQUAL (uri10.get_scheme (), "https");
  test.ASSERT_EQUAL (uri12.get_scheme (), "file");

  // authority
  test.ASSERT_EQUAL (uri1.get_authority (), "ftp.is.co.za");
  test.ASSERT_EQUAL (uri2.get_authority (), "www.ietf.org");
  test.ASSERT_EQUAL (uri3.get_authority (), "[2001:db8::7]");
  test.ASSERT_EQUAL (uri4.get_authority (), "");
  test.ASSERT_EQUAL (uri5.get_authority (), "");
  test.ASSERT_EQUAL (uri6.get_authority (), "");
  test.ASSERT_EQUAL (uri7.get_authority (), "192.0.2.16:80");
  test.ASSERT_EQUAL (uri8.get_authority (), "");
  test.ASSERT_EQUAL (uri9.get_authority (), "aguiar:mypassword@www.gnu.org:80");
  test.ASSERT_EQUAL (uri10.get_authority (), "aguiar:mypassword@www.gnu.org:80");
  test.ASSERT_EQUAL (uri12.get_authority (), "");

  // path
  test.ASSERT_EQUAL (uri1.get_path (), "/rfc/rfc1808.txt");
  test.ASSERT_EQUAL (uri2.get_path (), "/rfc/rfc2396.txt");
  test.ASSERT_EQUAL (uri3.get_path (), "/c=GB");
  test.ASSERT_EQUAL (uri4.get_path (), "John.Doe@example.com");
  test.ASSERT_EQUAL (uri5.get_path (), "comp.infosystems.www.servers.unix");
  test.ASSERT_EQUAL (uri6.get_path (), "+1-816-555-1212");
  test.ASSERT_EQUAL (uri7.get_path (), "/");
  test.ASSERT_EQUAL (uri8.get_path (), "oasis:names:specification:docbook:dtd:xml:4.1.2");
  test.ASSERT_EQUAL (uri9.get_path (), "/test/subdir%201");
  test.ASSERT_EQUAL (uri9.get_path ("utf-8"), "/test/subdir 1");
  test.ASSERT_EQUAL (uri10.get_path (), "/test/subdir%201");
  test.ASSERT_EQUAL (uri10.get_path ("utf-8"), "/test/subdir 1");
  test.ASSERT_EQUAL (uri12.get_path (), "/test%20with%20space%25/a%25b");
  test.ASSERT_EQUAL (uri12.get_path ("utf-8"), "/test with space%/a%b");

  // query
  test.ASSERT_EQUAL (uri1.get_query (), "");
  test.ASSERT_EQUAL (uri2.get_query (), "");
  test.ASSERT_EQUAL (uri3.get_query (), "objectClass?one");
  test.ASSERT_EQUAL (uri4.get_query (), "");
  test.ASSERT_EQUAL (uri5.get_query (), "");
  test.ASSERT_EQUAL (uri6.get_query (), "");
  test.ASSERT_EQUAL (uri7.get_query (), "");
  test.ASSERT_EQUAL (uri8.get_query (), "");
  test.ASSERT_EQUAL (uri9.get_query (), "value=55&p=%2314");
  test.ASSERT_EQUAL (uri9.get_query ("utf-8"), "value=55&p=#14");
  test.ASSERT_EQUAL (uri10.get_query (), "value=55");
  test.ASSERT_EQUAL (uri12.get_query (), "");

  // fragment
  test.ASSERT_EQUAL (uri1.get_fragment (), "");
  test.ASSERT_EQUAL (uri2.get_fragment (), "");
  test.ASSERT_EQUAL (uri3.get_fragment (), "");
  test.ASSERT_EQUAL (uri4.get_fragment (), "");
  test.ASSERT_EQUAL (uri5.get_fragment (), "");
  test.ASSERT_EQUAL (uri6.get_fragment (), "");
  test.ASSERT_EQUAL (uri7.get_fragment (), "");
  test.ASSERT_EQUAL (uri8.get_fragment (), "");
  test.ASSERT_EQUAL (uri9.get_fragment (), "anchor1");
  test.ASSERT_EQUAL (uri10.get_fragment (), "anchor1");
  test.ASSERT_EQUAL (uri12.get_fragment (), "");

  // username
  test.ASSERT_EQUAL (uri1.get_username (), "");
  test.ASSERT_EQUAL (uri2.get_username (), "");
  test.ASSERT_EQUAL (uri3.get_username (), "");
  test.ASSERT_EQUAL (uri4.get_username (), "");
  test.ASSERT_EQUAL (uri5.get_username (), "");
  test.ASSERT_EQUAL (uri6.get_username (), "");
  test.ASSERT_EQUAL (uri7.get_username (), "");
  test.ASSERT_EQUAL (uri8.get_username (), "");
  test.ASSERT_EQUAL (uri9.get_username (), "aguiar");
  test.ASSERT_EQUAL (uri10.get_username (), "aguiar");
  test.ASSERT_EQUAL (uri12.get_username (), "");

  // password
  test.ASSERT_EQUAL (uri1.get_password (), "");
  test.ASSERT_EQUAL (uri2.get_password (), "");
  test.ASSERT_EQUAL (uri3.get_password (), "");
  test.ASSERT_EQUAL (uri4.get_password (), "");
  test.ASSERT_EQUAL (uri5.get_password (), "");
  test.ASSERT_EQUAL (uri6.get_password (), "");
  test.ASSERT_EQUAL (uri7.get_password (), "");
  test.ASSERT_EQUAL (uri8.get_password (), "");
  test.ASSERT_EQUAL (uri9.get_password (), "mypassword");
  test.ASSERT_EQUAL (uri10.get_password (), "mypassword");
  test.ASSERT_EQUAL (uri12.get_password (), "");

  // host
  test.ASSERT_EQUAL (uri1.get_host (), "ftp.is.co.za");
  test.ASSERT_EQUAL (uri2.get_host (), "www.ietf.org");
  test.ASSERT_EQUAL (uri3.get_host (), "[2001:db8::7]");
  test.ASSERT_EQUAL (uri4.get_host (), "");
  test.ASSERT_EQUAL (uri5.get_host (), "");
  test.ASSERT_EQUAL (uri6.get_host (), "");
  test.ASSERT_EQUAL (uri7.get_host (), "192.0.2.16");
  test.ASSERT_EQUAL (uri8.get_host (), "");
  test.ASSERT_EQUAL (uri9.get_host (), "www.gnu.org");
  test.ASSERT_EQUAL (uri10.get_host (), "www.gnu.org");
  test.ASSERT_EQUAL (uri12.get_host (), "");

  // port
  test.ASSERT_EQUAL (uri1.get_port (), "");
  test.ASSERT_EQUAL (uri2.get_port (), "");
  test.ASSERT_EQUAL (uri3.get_port (), "");
  test.ASSERT_EQUAL (uri4.get_port (), "");
  test.ASSERT_EQUAL (uri5.get_port (), "");
  test.ASSERT_EQUAL (uri6.get_port (), "");
  test.ASSERT_EQUAL (uri7.get_port (), "80");
  test.ASSERT_EQUAL (uri8.get_port (), "");
  test.ASSERT_EQUAL (uri9.get_port (), "80");
  test.ASSERT_EQUAL (uri10.get_port (), "80");
  test.ASSERT_EQUAL (uri12.get_port (), "");

  // parent
  test.ASSERT_EQUAL (uri1.get_parent (), mobius::io::uri ("ftp://ftp.is.co.za/rfc"));
  test.ASSERT_EQUAL (uri2.get_parent (), mobius::io::uri ("http://www.ietf.org/rfc"));
  test.ASSERT_EQUAL (uri3.get_parent (), mobius::io::uri ("ldap://[2001:db8::7]"));
  test.ASSERT_EQUAL (uri4.get_parent (), mobius::io::uri ());
  test.ASSERT_EQUAL (uri5.get_parent (), mobius::io::uri ());
  test.ASSERT_EQUAL (uri6.get_parent (), mobius::io::uri ());
  test.ASSERT_EQUAL (uri7.get_parent (), mobius::io::uri ("telnet://192.0.2.16:80"));
  test.ASSERT_EQUAL (uri8.get_parent (), mobius::io::uri ());
  test.ASSERT_EQUAL (uri9.get_parent (), mobius::io::uri ("https://aguiar:mypassword@www.gnu.org:80/test"));
  test.ASSERT_EQUAL (uri10.get_parent (), mobius::io::uri ("https://aguiar:mypassword@www.gnu.org:80/test"));
  test.ASSERT_EQUAL (uri12.get_parent (), mobius::io::uri ("file:///test%20with%20space%25"));

  // filename
  test.ASSERT_EQUAL (uri1.get_filename (), "rfc1808.txt");
  test.ASSERT_EQUAL (uri2.get_filename (), "rfc2396.txt");
  test.ASSERT_EQUAL (uri3.get_filename (), "c=GB");
  test.ASSERT_EQUAL (uri4.get_filename (), "");
  test.ASSERT_EQUAL (uri5.get_filename (), "");
  test.ASSERT_EQUAL (uri6.get_filename (), "");
  test.ASSERT_EQUAL (uri7.get_filename (), "");
  test.ASSERT_EQUAL (uri8.get_filename (), "");
  test.ASSERT_EQUAL (uri9.get_filename (), "subdir%201");
  test.ASSERT_EQUAL (uri9.get_filename ("utf8"), "subdir 1");
  test.ASSERT_EQUAL (uri10.get_filename (), "subdir%201");
  test.ASSERT_EQUAL (uri10.get_filename ("utf8"), "subdir 1");
  test.ASSERT_EQUAL (uri12.get_filename (), "a%25b");
  test.ASSERT_EQUAL (uri12.get_filename ("utf8"), "a%b");

  // extension
  test.ASSERT_EQUAL (uri1.get_extension (), "txt");
  test.ASSERT_EQUAL (uri2.get_extension (), "txt");
  test.ASSERT_EQUAL (uri3.get_extension (), "");
  test.ASSERT_EQUAL (uri4.get_extension (), "");
  test.ASSERT_EQUAL (uri5.get_extension (), "");
  test.ASSERT_EQUAL (uri6.get_extension (), "");
  test.ASSERT_EQUAL (uri7.get_extension (), "");
  test.ASSERT_EQUAL (uri8.get_extension (), "");
  test.ASSERT_EQUAL (uri9.get_extension (), "");
  test.ASSERT_EQUAL (uri10.get_extension (), "");
  test.ASSERT_EQUAL (uri12.get_extension (), "");

  // join - normal examples - RFC 3986 - section 5.4.1
  mobius::io::uri base ("http://a/b/c/d;p?q");

  test.ASSERT_EQUAL (join (base, "g:h"), "g:h");
  test.ASSERT_EQUAL (join (base, "g"), "http://a/b/c/g");
  test.ASSERT_EQUAL (join (base, "./g"), "http://a/b/c/g");
  test.ASSERT_EQUAL (join (base, "g/"), "http://a/b/c/g/");
  test.ASSERT_EQUAL (join (base, "/g"), "http://a/g");
  test.ASSERT_EQUAL (join (base, "//g"), "http://g");
  test.ASSERT_EQUAL (join (base, "?y"), "http://a/b/c/d;p?y");
  test.ASSERT_EQUAL (join (base, "g?y"), "http://a/b/c/g?y");
  test.ASSERT_EQUAL (join (base, "#s"), "http://a/b/c/d;p?q#s");
  test.ASSERT_EQUAL (join (base, "g#s"), "http://a/b/c/g#s");
  test.ASSERT_EQUAL (join (base, "g?y#s"), "http://a/b/c/g?y#s");
  test.ASSERT_EQUAL (join (base, ";x"), "http://a/b/c/;x");
  test.ASSERT_EQUAL (join (base, "g;x"), "http://a/b/c/g;x");
  test.ASSERT_EQUAL (join (base, "g;x?y#s"), "http://a/b/c/g;x?y#s");
  test.ASSERT_EQUAL (join (base, ""), "http://a/b/c/d;p?q");
  test.ASSERT_EQUAL (join (base, "."), "http://a/b/c/");
  test.ASSERT_EQUAL (join (base, "./"), "http://a/b/c/");
  test.ASSERT_EQUAL (join (base, ".."), "http://a/b/");
  test.ASSERT_EQUAL (join (base, "../"), "http://a/b/");
  test.ASSERT_EQUAL (join (base, "../g"), "http://a/b/g");
  test.ASSERT_EQUAL (join (base, "../.."), "http://a/");
  test.ASSERT_EQUAL (join (base, "../../"), "http://a/");
  test.ASSERT_EQUAL (join (base, "../../g"), "http://a/g");

  // join - abnormal examples - RFC 3986 - section 5.4.2
  test.ASSERT_EQUAL (join (base, "../../../g"), "http://a/g");
  test.ASSERT_EQUAL (join (base, "../../../../g"), "http://a/g");

  test.ASSERT_EQUAL (join (base, "/./g"), "http://a/g");
  test.ASSERT_EQUAL (join (base, "/../g"), "http://a/g");
  test.ASSERT_EQUAL (join (base, "g."), "http://a/b/c/g.");
  test.ASSERT_EQUAL (join (base, ".g"), "http://a/b/c/.g");
  test.ASSERT_EQUAL (join (base, "g.."), "http://a/b/c/g..");
  test.ASSERT_EQUAL (join (base, "..g"), "http://a/b/c/..g");

  test.ASSERT_EQUAL (join (base, "./../g"), "http://a/b/g");
  test.ASSERT_EQUAL (join (base, "./g/."), "http://a/b/c/g/");
  test.ASSERT_EQUAL (join (base, "g/./h"), "http://a/b/c/g/h");
  test.ASSERT_EQUAL (join (base, "g/../h"), "http://a/b/c/h");
  test.ASSERT_EQUAL (join (base, "g;x=1/./y"), "http://a/b/c/g;x=1/y");
  test.ASSERT_EQUAL (join (base, "g;x=1/../y"), "http://a/b/c/y");

  test.ASSERT_EQUAL (join (base, "g?y/./x"), "http://a/b/c/g?y/./x");
  test.ASSERT_EQUAL (join (base, "g?y/../x"), "http://a/b/c/g?y/../x");
  test.ASSERT_EQUAL (join (base, "g#s/./x"), "http://a/b/c/g#s/./x");
  test.ASSERT_EQUAL (join (base, "g#s/../x"), "http://a/b/c/g#s/../x");

  // normalization
  mobius::io::uri uri20 ("HTTP://WWW.%65xample.COM/src/lib/../doc/index.html");

  test.ASSERT_EQUAL (uri20.get_scheme (), "http");
  test.ASSERT_EQUAL (uri20.get_authority (), "www.example.com");
  test.ASSERT_EQUAL (uri20.get_path (), "/src/doc/index.html");
  test.ASSERT_EQUAL (uri20.get_value (), "http://www.example.com/src/doc/index.html");

  // is same document
  mobius::io::uri uri21 ("HTTP://www.%65xample.com/src/lib/../doc/index.html#chapter1");
  mobius::io::uri uri22 ("HTTP://www.%65xample.com/src/lib/../doc/index.html#appendixa");

  test.ASSERT_NOT_EQUAL (uri21, uri22);
  test.ASSERT_TRUE (mobius::io::is_same_document (uri21, uri22));

  test.end ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Test sequential_reader_adaptor class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
testcase_sequential_reader_adaptor ()
{
  mobius::unittest test ("mobius::io::sequential_reader_adaptor");

  mobius::bytearray data ("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  mobius::io::sequential_reader_adaptor adaptor (mobius::io::new_bytearray_reader (data));

  test.ASSERT_EQUAL (adaptor.tell (), 0);
  test.ASSERT_EQUAL (adaptor.eof (), false);
  test.ASSERT_EQUAL (adaptor.peek (), 'A');
  test.ASSERT_EQUAL (adaptor.peek (), 'A');
  test.ASSERT_EQUAL (adaptor.tell (), 0);
  test.ASSERT_EQUAL (adaptor.get (), 'A');

  test.ASSERT_EQUAL (adaptor.tell (), 1);
  test.ASSERT_EQUAL (adaptor.peek (), 'B');
  test.ASSERT_EQUAL (adaptor.peek (), 'B');
  test.ASSERT_EQUAL (adaptor.tell (), 1);
  test.ASSERT_EQUAL (adaptor.get (), 'B');

  adaptor.skip (2);

  test.ASSERT_EQUAL (adaptor.tell (), 4);
  test.ASSERT_EQUAL (adaptor.eof (), false);
  test.ASSERT_EQUAL (adaptor.peek (), 'E');
  test.ASSERT_EQUAL (adaptor.peek (), 'E');
  test.ASSERT_EQUAL (adaptor.get (), 'E');
  
  test.ASSERT_EQUAL (adaptor.tell (), 5);
  test.ASSERT_EQUAL (adaptor.eof (), false);
  test.ASSERT_EQUAL (adaptor.peek (3), "FGH");
  test.ASSERT_EQUAL (adaptor.peek (3), "FGH");
  test.ASSERT_EQUAL (adaptor.get (3), "FGH");

  adaptor.skip (15);
  test.ASSERT_EQUAL (adaptor.tell (), 23);
  test.ASSERT_EQUAL (adaptor.peek (), 'X');
  test.ASSERT_EQUAL (adaptor.peek (3), "XYZ");
  test.ASSERT_EQUAL (adaptor.peek (), 'X');
  test.ASSERT_EQUAL (adaptor.peek (5), "XYZ");
  test.ASSERT_EQUAL (adaptor.tell (), 23);
  test.ASSERT_EQUAL (adaptor.get (5), "XYZ");
  test.ASSERT_EQUAL (adaptor.tell (), 26);
  test.ASSERT_EQUAL (adaptor.eof (), true);

  test.end ();
}

void
unittest_io ()
{
  testcase_path ();
  testcase_uri ();
  testcase_file ();
  testcase_folder ();
  testcase_reader ();
  testcase_writer ();
  testcase_sequential_reader_adaptor ();
}
