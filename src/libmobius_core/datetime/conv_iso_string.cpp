// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008-2026 Eduardo Aguiar
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
#include <cstdio>
#include <mobius/core/datetime/conv_iso_string.hpp>
#include <mobius/core/exception.inc>
#include <sstream>
#include <stdexcept>

namespace mobius::core::datetime
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert ISO 8601 formatted string to datetime
// @param str string
// @return datetime
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datetime
new_datetime_from_iso_string (const std::string &str)
{
    if (str.empty ())
        return {};

    int year, month, day;
    int hours, minutes, seconds;
    const char *format = nullptr;
    constexpr int NUMBER_OF_ITEMS = 6;

    if (str.length () > 10 && str[10] == 'T')
        format = "%04d-%02d-%02dT%02d:%02d:%02d";

    else if (str.length () > 8 && str[8] == 'T')
        format = "%04d%02d%02dT%02d%02d%02d";

    else
        format = "%04d-%02d-%02d %02d:%02d:%02d";

    if (sscanf (str.c_str (), format, &year, &month, &day, &hours, &minutes,
                &seconds) != NUMBER_OF_ITEMS)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG (
            ("Invalid datetime string: " + str).c_str ()));

    return datetime (year, month, day, hours, minutes, seconds);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert date object to ISO 8601 string
// @param d date object
// @return date formatted as string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
date_to_iso_string (const date &d)
{
    char buffer[64] = {0};

    if (d)
        sprintf (buffer, "%04d-%02d-%02d", d.get_year (), d.get_month (),
                 d.get_day ());

    return buffer;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert time object to ISO 8601 string
// @param t time object
// @return time formatted as 'HH:MM:SS'
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
time_to_iso_string (const time &t)
{
    char buffer[64] = {0};

    if (t)
        sprintf (buffer, "%02d:%02d:%02d", t.get_hour (), t.get_minute (),
                 t.get_second ());

    return buffer;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert datetime object to ISO 8601 format
// @param dt datetime object
// @return datetime formatted as 'YYYY-MM-DD HH:MM:SS'
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
datetime_to_iso_string (const datetime &dt)
{
    std::string str;

    if (dt)
        str = date_to_iso_string (dt.get_date ()) + 'T' +
              time_to_iso_string (dt.get_time ()) + 'Z';

    return str;
}

} // namespace mobius::core::datetime
