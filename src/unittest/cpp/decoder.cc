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
#include <mobius/decoder/base64.h>
#include <mobius/decoder/data_decoder.h>
#include <mobius/decoder/hexstring.h>
#include <mobius/io/file.h>

#include <iostream>

static void
testcase_base16 ()
{
  mobius::unittest test ("mobius::decoder::base16");

  test.ASSERT_EQUAL (mobius::decoder::base16 (""), "");
  test.ASSERT_EQUAL (mobius::decoder::base16 ("66"), "f");
  test.ASSERT_EQUAL (mobius::decoder::base16 ("666F"), "fo");
  test.ASSERT_EQUAL (mobius::decoder::base16 ("666F6F"), "foo");
  test.ASSERT_EQUAL (mobius::decoder::base16 ("666F6F62"), "foob");
  test.ASSERT_EQUAL (mobius::decoder::base16 ("666F6F6261"), "fooba");
  test.ASSERT_EQUAL (mobius::decoder::base16 ("666F6F626172"), "foobar");

  test.end ();
}

static void
testcase_base32 ()
{
  mobius::unittest test ("mobius::decoder::base32");

  test.ASSERT_EQUAL (mobius::decoder::base32 (""), "");
  test.ASSERT_EQUAL (mobius::decoder::base32 ("MY======"), "f");
  test.ASSERT_EQUAL (mobius::decoder::base32 ("MZXQ===="), "fo");
  test.ASSERT_EQUAL (mobius::decoder::base32 ("MZXW6==="), "foo");
  test.ASSERT_EQUAL (mobius::decoder::base32 ("MZXW6YQ="), "foob");
  test.ASSERT_EQUAL (mobius::decoder::base32 ("MZXW6YTB"), "fooba");
  test.ASSERT_EQUAL (mobius::decoder::base32 ("MZXW6YTBOI======"), "foobar");

  test.end ();
}

static void
testcase_base32hex ()
{
  mobius::unittest test ("mobius::decoder::base32hex");

  test.ASSERT_EQUAL (mobius::decoder::base32hex (""), "");
  test.ASSERT_EQUAL (mobius::decoder::base32hex ("CO======"), "f");
  test.ASSERT_EQUAL (mobius::decoder::base32hex ("CPNG===="), "fo");
  test.ASSERT_EQUAL (mobius::decoder::base32hex ("CPNMU==="), "foo");
  test.ASSERT_EQUAL (mobius::decoder::base32hex ("CPNMUOG="), "foob");
  test.ASSERT_EQUAL (mobius::decoder::base32hex ("CPNMUOJ1"), "fooba");
  test.ASSERT_EQUAL (mobius::decoder::base32hex ("CPNMUOJ1E8======"), "foobar");

  test.end ();
}

static void
testcase_base64 ()
{
  mobius::unittest test ("mobius::decoder::base64");

  test.ASSERT_EQUAL (mobius::decoder::base64 (""), "");
  test.ASSERT_EQUAL (mobius::decoder::base64 ("Zg=="), "f");
  test.ASSERT_EQUAL (mobius::decoder::base64 ("Zm8="), "fo");
  test.ASSERT_EQUAL (mobius::decoder::base64 ("Zm9v"), "foo");
  test.ASSERT_EQUAL (mobius::decoder::base64 ("Zm9vYg=="), "foob");
  test.ASSERT_EQUAL (mobius::decoder::base64 ("Zm9\r\nvYg=="), "foob");
  test.ASSERT_EQUAL (mobius::decoder::base64 ("Zm9vYmE="), "fooba");
  test.ASSERT_EQUAL (mobius::decoder::base64 ("Zm9vYmFy"), "foobar");
  test.ASSERT_EQUAL (mobius::decoder::base64 ("Zm9v\r\nYmF\r\ny"), "foobar");

  test.end ();
}

static std::string
get_ipv6 (const mobius::bytearray& data)
{
  mobius::decoder::data_decoder decoder (data);
  return decoder.get_ipv6 ();
}

static std::string
get_ipv4_mapped_ipv6 (const mobius::bytearray& data)
{
  mobius::decoder::data_decoder decoder (data);
  return decoder.get_ipv4_mapped_ipv6 ();
}

static void
testcase_data_decoder ()
{
  mobius::unittest test ("mobius::decoder::data_decoder");
  auto f = mobius::io::new_file_by_path ("/tmp/unittest.dat");

  {
    auto writer = f.new_writer ();
    writer.write (mobius::bytearray {0x08,0x00,0x00,0x00});     // uint32_le = 8
    writer.write (mobius::bytearray {0x00,0x00,0x00,0x08});     // uint32_be = 8
    writer.write (mobius::bytearray {0xff,0xff});               // uint16_le = 65535
    writer.write ("abc");
    writer.write (mobius::bytearray {0, 0, 0, 0, 0, 0, 0, 0});  // nt_datetime 1601-01-01 00:00:00
    writer.write ("def");
    writer.write (mobius::bytearray {0});                       // c_string
    writer.write (mobius::bytearray {'g', 0, 'h', 0, 'i', 0, 0, 0}); // c_string (utf-16)
  }

  auto reader = f.new_reader ();
  mobius::decoder::data_decoder decoder (reader);

  test.ASSERT_EQUAL (decoder.get_uint32_le (), 8);
  test.ASSERT_EQUAL (decoder.get_uint32_be (), 8);
  test.ASSERT_EQUAL (decoder.get_uint16_le (), 65535);
  test.ASSERT_EQUAL (decoder.get_string_by_size (3), "abc");
  test.ASSERT_EQUAL (decoder.get_nt_datetime (), mobius::datetime::datetime ());
  test.ASSERT_EQUAL (decoder.get_c_string (), "def");
  test.ASSERT_EQUAL (decoder.get_c_string ("UTF-16LE"), "ghi");

  // ipv6
  test.ASSERT_EQUAL (get_ipv6 ({0xfe, 0x80, 0x77, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x44, 0x00, 0x00, 0x33, 0x22}), "FE80:7788::5544:0:3322");
  test.ASSERT_EQUAL (get_ipv6 ({0xfe, 0x80, 0x77, 0x88, 0x50, 0x30, 0x10, 0x20, 0x40, 0x60, 0x55, 0x44, 0x00, 0x00, 0x33, 0x22}), "FE80:7788:5030:1020:4060:5544::3322");
  test.ASSERT_EQUAL (get_ipv6 ({0xfe, 0x80, 0x77, 0x88, 0x50, 0x30, 0x10, 0x20, 0x40, 0x60, 0x55, 0x44, 0x01, 0x01, 0x00, 0x22}), "FE80:7788:5030:1020:4060:5544:101:22");
  test.ASSERT_EQUAL (get_ipv6 ({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}), "::");
  test.ASSERT_EQUAL (get_ipv6 ({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}), "::1");
  test.ASSERT_EQUAL (get_ipv4_mapped_ipv6 ({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xc0, 0x44, 0x00, 0x01}), "192.68.0.1");
  test.ASSERT_EQUAL (get_ipv4_mapped_ipv6 ({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x44, 0x00, 0x01}), "::192.68.0.1");
  test.ASSERT_EQUAL (get_ipv4_mapped_ipv6 ({0xfe, 0x80, 0x77, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x44, 0xc0, 0x44, 0x00, 0x01}), "FE80:7788::5544:192.68.0.1");

  test.end ();
}

static void
testcase_hexstring ()
{
  mobius::unittest test ("mobius::decoder::hexstring");

  mobius::bytearray b1 = {0x03, 0xf5};

  test.ASSERT_EQUAL (mobius::decoder::hexstring ("3F5"), b1);
  test.ASSERT_EQUAL (mobius::decoder::hexstring ("03f5"), b1);
  test.ASSERT_EQUAL (mobius::decoder::hexstring ("03F5"), b1);
  test.ASSERT_EQUAL (mobius::decoder::hexstring ("03f5"), b1);
  test.ASSERT_EQUAL (mobius::decoder::hexstring ("3f50000"), mobius::bytearray ({0x3, 0xf5, 0, 0}));
  test.ASSERT_EQUAL (mobius::decoder::hexstring ("0f3f5"), mobius::bytearray ({0, 0xf3, 0xf5}));

  test.end ();
}


void
unittest_decoder ()
{
  testcase_base16 ();
  testcase_base32 ();
  testcase_base32hex ();
  testcase_base64 ();
  testcase_data_decoder ();
  testcase_hexstring ();
}
