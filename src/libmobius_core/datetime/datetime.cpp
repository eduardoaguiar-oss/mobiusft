// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
// Eduardo Aguiar
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
#include <mobius/core/datetime/conv_julian.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <chrono>

#include <iostream>

namespace mobius::core::datetime
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Construct datetime from date and time
// @param d date object
// @param t time object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datetime::datetime (const date &d, const time &t) noexcept
    : date_ (d),
      time_ (t),
      is_null_ (false)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add timedelta to datetime object
// @param delta timedelta object
// @return Datetime reference
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datetime &
datetime::operator+= (const timedelta &delta) noexcept
{
    timedelta::value_type seconds =
        static_cast<timedelta::value_type> (time_.to_day_seconds ()) +
        delta.to_seconds ();

    constexpr timedelta::value_type SECONDS_PER_DAY = 86400;
    auto seconds_in_day = seconds % SECONDS_PER_DAY;
    auto days = seconds / SECONDS_PER_DAY;

    if (seconds_in_day < 0)
    {
        seconds_in_day = SECONDS_PER_DAY + seconds_in_day;
        days--;
    }

    timedelta d_delta;
    d_delta.from_days (days);

    date_ += d_delta;
    time_.from_day_seconds (seconds_in_day);

    return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Subtract timedelta from datetime object
// @param delta timedelta object
// @return Datetime reference
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datetime &
datetime::operator-= (const timedelta &delta) noexcept
{
    *this += timedelta (-delta.to_seconds ());
    return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get current date/time
// @return Current date/time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datetime
now ()
{
    auto clock_now = std::chrono::system_clock::now ();
    time_t t = std::chrono::system_clock::to_time_t (clock_now);

    return new_datetime_from_unix_timestamp (t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add timedelta to datetime object
// @param dt datetime object
// @param delta timedelta object
// @return New datetime object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datetime
operator+ (const datetime &dt, const timedelta &delta) noexcept
{
    datetime d (dt);
    d += delta;
    return d;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Subtract timedelta fromdatetime object
// @param dt datetime object
// @param delta timedelta object
// @return New datetime object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datetime
operator- (const datetime &dt, const timedelta &delta) noexcept
{
    datetime d (dt);
    d -= delta;
    return d;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Subtract two datetime objects
// @param da datetime object A
// @param db datetime object B
// @return timedelta object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
timedelta
operator- (const datetime &da, const datetime &db) noexcept
{
    return (da.get_date () - db.get_date ()) +
           (da.get_time () - db.get_time ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Compare two datetimes
// @param d1 datetime object
// @param d2 datetime object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator== (const datetime &d1, const datetime &d2) noexcept
{
    return bool (d1) == bool (d2) && d1.get_date () == d2.get_date () &&
           d1.get_time () == d2.get_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert datetime to string
// @param dt datetime object
// @return string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
to_string (const datetime &dt)
{
    if (dt)
        return to_string (dt.get_date ()) + ' ' + to_string (dt.get_time ());

    return std::string ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Write datetime representation to std::ostream
// @param stream ostream reference
// @param dt datetime object
// @return reference to ostream
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::ostream &
operator<< (std::ostream &stream, const datetime &dt)
{
    stream << to_string (dt);

    return stream;
}

} // namespace mobius::core::datetime
