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
#include <mobius/bytearray.h>
#include <mobius/unittest.h>
#include <stdexcept>
#include <cstdint>

mobius::bytearray
build ()
{
  return std::move (mobius::bytearray {'A', 'Z', '0', '1', 13, 10, ' '});
}

void
unittest_bytearray ()
{
  mobius::unittest test ("mobius::bytearray");

  uint8_t c_array[] = {65, 69, 65, 48};
  mobius::bytearray b0;
  mobius::bytearray b1 = "abc123";
  mobius::bytearray b2 = b1;
  mobius::bytearray b3 (c_array, 4);
  mobius::bytearray b4;
  mobius::bytearray b5 = b3;
  mobius::bytearray b6 = b0;
  mobius::bytearray b7 = {'A', 'Z', '0', '1', 13, 10, ' '};
  mobius::bytearray b8 (100);
  mobius::bytearray b9 = {0, 1, 2, 3, 4};
  mobius::bytearray b10 = std::move (b9);
  mobius::bytearray b11;
  mobius::bytearray b12 = {0, 1, 2, 3, 4};
  b11 = build ();
  b4 = b3;

  // size
  test.ASSERT_EQUAL (b0.size (), 0);
  test.ASSERT_EQUAL (b1.size (), 6);
  test.ASSERT_EQUAL (b2.size (), b1.size ());
  test.ASSERT_EQUAL (b3.size (), 4);
  test.ASSERT_EQUAL (b4.size (), b3.size ());
  test.ASSERT_EQUAL (b5.size (), b3.size ());
  test.ASSERT_EQUAL (b6.size (), b0.size ());
  test.ASSERT_EQUAL (b7.size (), 7);
  test.ASSERT_EQUAL (b8.size (), 100);
  test.ASSERT_EQUAL (b9.size (), 0);
  test.ASSERT_EQUAL (b10.size (), 5);

  // empty
  test.ASSERT_TRUE (b0.empty ());
  test.ASSERT_FALSE (b1.empty ());
  test.ASSERT_FALSE (b2.empty ());
  test.ASSERT_FALSE (b3.empty ());
  test.ASSERT_FALSE (b4.empty ());
  test.ASSERT_FALSE (b5.empty ());
  test.ASSERT_TRUE (b6.empty ());
  test.ASSERT_FALSE (b7.empty ());
  test.ASSERT_FALSE (b8.empty ());
  test.ASSERT_TRUE (b9.empty ());
  test.ASSERT_FALSE (b10.empty ());

  // operator[] const
  test.ASSERT_EQUAL (b1[0], 'a');
  test.ASSERT_EQUAL (b1[5], '3');
  test.ASSERT_EQUAL (b2[0], b1[0]);
  test.ASSERT_EQUAL (b2[5], b1[5]);
  test.ASSERT_EQUAL (b3[0], 'A');
  test.ASSERT_EQUAL (b3[3], '0');
  test.ASSERT_EQUAL (b4[0], b3[0]);
  test.ASSERT_EQUAL (b4[3], b3[3]);
  test.ASSERT_EQUAL (b5[0], b3[0]);
  test.ASSERT_EQUAL (b5[3], b3[3]);
  test.ASSERT_EQUAL (b7[0], 'A');
  test.ASSERT_EQUAL (b7[6], ' ');
  test.ASSERT_EQUAL (b10[0], 0);
  ASSERT_EXCEPTION (test, b0[0], std::out_of_range);
  ASSERT_EXCEPTION (test, b1[6], std::out_of_range);
  ASSERT_EXCEPTION (test, b6[0], std::out_of_range);
  ASSERT_EXCEPTION (test, b9[0], std::out_of_range);

  // operator[]
  b4[0] = 'B';
  test.ASSERT_EQUAL (b4[0], 'B');
  test.ASSERT_NOT_EQUAL (b3[0], b4[0]);
  test.ASSERT_NOT_EQUAL (b5[0], b4[0]);
  test.ASSERT_EQUAL (b3[0], b5[0]);

  // operator+
  mobius::bytearray sum = b12 + mobius::bytearray {5, 6, 7, 8, 9};
  test.ASSERT_FALSE (sum.empty ());
  test.ASSERT_EQUAL (sum.size (), 10);

  for (auto i = 0; i < 10; i++)
    test.ASSERT_EQUAL (sum[i], i);

  // operator<<
  mobius::bytearray l1 = {0x81, 0xc4, 0x57, 0x83};
  test.ASSERT_EQUAL (l1 << 0, l1);
  test.ASSERT_EQUAL ((l1 << 32), (mobius::bytearray (4)));
  test.ASSERT_EQUAL (l1 << 8, (mobius::bytearray {0xc4, 0x57, 0x83, 0x00}));
  test.ASSERT_EQUAL (l1 << 9, (mobius::bytearray {0x88, 0xaf, 0x06, 0x00}));
  test.ASSERT_EQUAL (l1 << 1, (mobius::bytearray {0x03, 0x88, 0xaf, 0x06}));
  test.ASSERT_EQUAL (l1 << 2, (mobius::bytearray {0x07, 0x11, 0x5e, 0x0c}));
  test.ASSERT_EQUAL (l1 << 7, (mobius::bytearray {0xe2, 0x2b, 0xc1, 0x80}));
  test.ASSERT_EQUAL (l1 << 24, (mobius::bytearray {0x83, 0x00, 0x00, 0x00}));
  test.ASSERT_EQUAL (l1 << 31, (mobius::bytearray {0x80, 0x00, 0x00, 0x00}));

  // operator>>
  mobius::bytearray l2 = {0x81, 0xc4, 0x57, 0x73};
  test.ASSERT_EQUAL (l2 >> 0, l2);
  test.ASSERT_EQUAL (l2 >> 32, (mobius::bytearray (4)));
  test.ASSERT_EQUAL (l2 >> 8, (mobius::bytearray {0x00, 0x81, 0xc4, 0x57}));
  test.ASSERT_EQUAL (l2 >> 9, (mobius::bytearray {0x00, 0x40, 0xe2, 0x2b}));
  test.ASSERT_EQUAL (l2 >> 1, (mobius::bytearray {0x40, 0xe2, 0x2b, 0xb9}));
  test.ASSERT_EQUAL (l2 >> 2, (mobius::bytearray {0x20, 0x71, 0x15, 0xdc}));
  test.ASSERT_EQUAL (l2 >> 7, (mobius::bytearray {0x01, 0x03, 0x88, 0xae}));
  test.ASSERT_EQUAL (l2 >> 24, (mobius::bytearray {0x00, 0x00, 0x00, 0x81}));
  test.ASSERT_EQUAL (l2 >> 31, (mobius::bytearray {0x00, 0x00, 0x00, 0x01}));

  // fill
  b3.fill ('X');
  test.ASSERT_EQUAL (b3.size (), 4);
  test.ASSERT_EQUAL (b3[0], 'X');
  test.ASSERT_EQUAL (b3[3], 'X');
  test.ASSERT_NOT_EQUAL (b5[0], b3[0]);

  // resize
  b11.resize (9);
  test.ASSERT_EQUAL (b11.size (), 9);
  test.ASSERT_EQUAL (b11[0], 'A');
  test.ASSERT_EQUAL (b11[6], ' ');

  b11.resize (5);
  test.ASSERT_EQUAL (b11[0], 'A');
  test.ASSERT_EQUAL (b11[4], 13);
  test.ASSERT_EQUAL (b11.size (), 5);

  // slice
  mobius::bytearray slice1 = b7.slice (1, 3);
  mobius::bytearray b13;

  test.ASSERT_EQUAL (b7.slice (0, b7.size ()), b7);
  test.ASSERT_EQUAL (b13.slice (0, 0), mobius::bytearray ());
  test.ASSERT_EQUAL (slice1.size (), 3);
  test.ASSERT_EQUAL (slice1, mobius::bytearray ({'Z', '0', '1'}));
  test.ASSERT_TRUE (b7.slice (3, 1).empty ());

  // const_iterator
  for (mobius::bytearray::const_iterator iter = b3.begin (); iter != b3.end (); ++iter)
    test.ASSERT_EQUAL (*iter, 'X');

  // iterator
  for (mobius::bytearray::iterator iter = b3.begin (); iter != b3.end (); ++iter)
    {
      *iter = '0';
      test.ASSERT_EQUAL (*iter, '0');
      test.ASSERT_EQUAL (*iter, b3[iter - b3.begin ()]);
    }

  // bytearray <-> hexstring
  mobius::bytearray b14;
  const std::string b14_s = "0123456789abcdef";
  b14.from_hexstring (b14_s);

  test.ASSERT_EQUAL (b14.size (), 8);
  test.ASSERT_EQUAL (b14, mobius::bytearray ({0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef}));
  test.ASSERT_EQUAL (b14.to_hexstring (), b14_s);

  // operator +=
  test.ASSERT_EQUAL (mobius::bytearray () + b14, b14);

  test.end ();
}
