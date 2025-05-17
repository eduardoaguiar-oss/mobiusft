// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025
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
#include <chrono>
#include <mobius/core/datetime/datetime.hpp>

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
// @brief Subtract timedelta from datetime object
// @param dt datetime object
// @param delta timedelta object
// @return new datetime object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datetime
now ()
{
    auto clock_now = std::chrono::system_clock::now ();
    time_t t = std::chrono::system_clock::to_time_t (clock_now);

    return new_datetime_from_unix_timestamp (t);
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
// @brief Add timedelta to datetime object
// @param dt datetime object
// @param delta timedelta object
// @return new datetime object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datetime
operator+ (const datetime &dt, const timedelta &delta) noexcept
{
    using value_type = timedelta::value_type;
    value_type seconds =
        dt.get_time ().to_day_seconds () + delta.get_seconds ();
    value_type days = 0;

    constexpr value_type SECONDS_OF_DAY = 86400;

    if (seconds >= 0)
    {
        days = (seconds / SECONDS_OF_DAY) + delta.get_days ();
        seconds = seconds % SECONDS_OF_DAY;
    }
    else
    {
        seconds += SECONDS_OF_DAY;
        days--;
    }

    // create new timedelta, with seconds = 0
    mobius::core::datetime::timedelta delta2 (delta);
    delta2.set_days (days);
    delta2.set_seconds (0);

    // return new datetime
    return mobius::core::datetime::datetime (dt.get_date () + delta2,
                                             time (seconds % SECONDS_OF_DAY));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Subtract timedelta from datetime object
// @param dt datetime object
// @param delta timedelta object
// @return new datetime object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datetime
operator- (const datetime &dt, const timedelta &delta) noexcept
{
    return dt + timedelta (-delta.get_years (), -delta.get_days (),
                           -delta.get_seconds (), -delta.get_nanoseconds ());
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
