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
#include <mobius/zlib_functions.h>
#include <mobius/unittest.h>

void
testcase_zlib_functions ()
{
  mobius::unittest test ("mobius::zlib_functions");
  mobius::bytearray b1 (65536);

  for (int i = 0; i < 65536; i++)
    b1[i] = i & 0xff;

  mobius::bytearray c = zlib_compress (b1, 6);
  mobius::bytearray b2 = zlib_decompress (c);

  test.ASSERT_EQUAL (b2, b1);
  test.end ();
}

void
unittest_zlib_functions ()
{
  testcase_zlib_functions ();
}
