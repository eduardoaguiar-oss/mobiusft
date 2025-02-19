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
#include <mobius/datetime/date.h>
#include <mobius/datetime/time.h>
#include <mobius/datetime/datetime.h>
#include <mobius/datetime/timedelta.h>
#include <mobius/datetime/conv_iso_string.h>
#include <mobius/datetime/conv_julian.h>
#include <mobius/unittest.h>
#include <stdexcept>

using namespace mobius::datetime;

static void
testcase_date ()
{
  mobius::unittest test ("mobius::datetime::date");

  date d1;
  test.ASSERT_EQUAL (d1.get_year (), 0);
  test.ASSERT_EQUAL (d1.get_month (), 0);
  test.ASSERT_EQUAL (d1.get_day (), 0);
  test.ASSERT_FALSE (d1);

  date d2 (1980, 11, 23);
  test.ASSERT_EQUAL (d2.get_year (), 1980);
  test.ASSERT_EQUAL (d2.get_month (), 11);
  test.ASSERT_EQUAL (d2.get_day (), 23);
  test.ASSERT_TRUE (d2);

  test.ASSERT_EQUAL (d2, date (1980, 11, 23));
  test.ASSERT_NOT_EQUAL (d2, date (1980, 11, 22));

  test.ASSERT_TRUE (d2 == date (1980, 11, 23));
  test.ASSERT_TRUE (d2 != date (1980, 11, 22));

  test.ASSERT_TRUE (d2 < date (1980, 11, 24));
  test.ASSERT_TRUE (d2 < date (1980, 12, 1));
  test.ASSERT_TRUE (d2 < date (1981, 1, 2));

  test.ASSERT_TRUE (d2 <= date (1980, 11, 23));
  test.ASSERT_TRUE (d2 <= date (1980, 11, 24));
  test.ASSERT_TRUE (d2 <= date (1980, 12, 1));
  test.ASSERT_TRUE (d2 <= date (1981, 1, 2));

  test.ASSERT_TRUE (d2 > date (1980, 11, 22));
  test.ASSERT_TRUE (d2 > date (1979, 12, 1));
  test.ASSERT_TRUE (d2 > date (1980, 1, 2));

  test.ASSERT_TRUE (d2 >= date (1980, 11, 23));
  test.ASSERT_TRUE (d2 >= date (1980, 11, 22));
  test.ASSERT_TRUE (d2 >= date (1979, 12, 1));
  test.ASSERT_TRUE (d2 >= date (1980, 1, 2));

  test.end ();
}

static void
testcase_time ()
{
  mobius::unittest test ("mobius::datetime::time");

  mobius::datetime::time t;
  test.ASSERT_EQUAL (t.to_day_seconds (), 0);
  test.ASSERT_FALSE (t);
  test.end ();
}

static void
testcase_datetime ()
{
  mobius::unittest test ("mobius::datetime::datetime");

  datetime dt1;
  test.ASSERT_EQUAL (dt1.get_date ().get_year (), 0);
  test.ASSERT_EQUAL (dt1.get_date ().get_month (), 0);
  test.ASSERT_EQUAL (dt1.get_date ().get_day (), 0);
  test.ASSERT_EQUAL (dt1.get_time ().get_hour (), 0);
  test.ASSERT_EQUAL (dt1.get_time ().get_minute (), 0);
  test.ASSERT_EQUAL (dt1.get_time ().get_second (), 0);
  test.ASSERT_FALSE (dt1);

  datetime dt3 = new_datetime_from_nt_timestamp (0);
  test.ASSERT_EQUAL (dt3, datetime ());
  test.ASSERT_FALSE (dt3);

  datetime dt4 = new_datetime_from_unix_timestamp (0);
  test.ASSERT_EQUAL (dt4, datetime ());
  test.ASSERT_FALSE (dt4);
  test.end ();
}

static void
testcase_timedelta ()
{
  mobius::unittest test ("mobius::datetime::timedelta");

  datetime dt1 (1970, 1, 1, 0, 0, 0);
  test.ASSERT_EQUAL (dt1 + timedelta (5, 0, 7200), datetime (1975, 1, 1, 2, 0, 0));
  test.ASSERT_EQUAL (dt1 + timedelta (0, 0, -7200), datetime (1969, 12, 31, 22, 0, 0));
  test.ASSERT_EQUAL (dt1 - timedelta (5, 0, 7200), datetime (1964, 12, 31, 22, 0, 0));
  test.ASSERT_EQUAL (dt1 - timedelta (0, 0, 7200), datetime (1969, 12, 31, 22, 0, 0));

  date dt2 (1980, 11, 23);
  test.ASSERT_EQUAL ((dt2 - date (1980, 11, 1)).get_days (), 22);
  test.ASSERT_EQUAL ((dt2 - date (1979, 11, 23)).get_days (), 366);
  test.ASSERT_EQUAL (date (1980, 11, 1) + timedelta (0, 22), date (1980,11,23));
  test.ASSERT_EQUAL (date (1980, 11, 23) + timedelta (0, -22), date (1980,11,1));

  test.end ();
}

static void
testcase_conv_iso_string ()
{
  mobius::unittest test ("mobius::datetime::conv_iso_string functions");

  datetime dt (1980, 11, 23, 10, 15, 45);
  test.ASSERT_EQUAL (date_to_iso_string (dt.get_date ()), "1980-11-23");
  test.ASSERT_EQUAL (time_to_iso_string (dt.get_time ()), "10:15:45");
  test.ASSERT_EQUAL (datetime_to_iso_string (dt), "1980-11-23T10:15:45Z");

  ASSERT_EXCEPTION (test, new_datetime_from_iso_string ("2012.05.01 05:05:05"), std::runtime_error);
  ASSERT_EXCEPTION (test, new_datetime_from_iso_string ("2012-05-01X05:05:05"), std::runtime_error);
  datetime dt2 = new_datetime_from_iso_string ("2012-05-22T23:55:12Z");
  test.ASSERT_EQUAL (dt2, datetime (2012,05,22, 23, 55, 12));

  test.end ();
}

static void
testcase_conv_julian ()
{
  mobius::unittest test ("mobius::datetime::conv_julian functions");
  constexpr julian_type EPOCH_NT_DATETIME = 2305813.5;
  constexpr julian_type EPOCH_UNIX_DATETIME = 2440587.5;

  date d1 = date_from_julian (EPOCH_NT_DATETIME);
  test.ASSERT_EQUAL (d1, date (1601, 1, 1));
  test.ASSERT_EQUAL (EPOCH_NT_DATETIME, date_to_julian (d1));

  date d2 = date_from_julian (EPOCH_UNIX_DATETIME);
  test.ASSERT_EQUAL (d2, date (1970, 1, 1));
  test.ASSERT_EQUAL (EPOCH_UNIX_DATETIME, date_to_julian (d2));

  test.end ();
}

static void
testcase_conv_nt_timestamp ()
{
  mobius::unittest test ("mobius::datetime::conv_nt_timestamp functions");
  datetime d1 = new_datetime_from_nt_timestamp (0);
  test.ASSERT_EQUAL (d1, datetime ());

  datetime d2 = new_datetime_from_nt_timestamp (129319043083437500);
  test.ASSERT_EQUAL (d2, datetime (2010, 10, 18, 19, 38, 28));

  test.end ();
}

static void
testcase_conv_unix_timestamp ()
{
  mobius::unittest test ("mobius::datetime::conv_unix_timestamp functions");
  datetime d1 = new_datetime_from_unix_timestamp (0);
  test.ASSERT_EQUAL (d1, datetime ());

  datetime d2 = new_datetime_from_unix_timestamp (1448722013);
  test.ASSERT_EQUAL (d2, datetime (2015, 11, 28, 14, 46, 53));

  test.end ();
}

void
unittest_datetime ()
{
  testcase_date ();
  testcase_time ();
  testcase_datetime ();
  testcase_timedelta ();
  testcase_conv_iso_string ();
  testcase_conv_julian ();
  testcase_conv_nt_timestamp ();
  testcase_conv_unix_timestamp ();
}
