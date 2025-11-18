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
#include <mobius/core/datetime/timedelta.hpp>

namespace mobius::core::datetime
{
namespace
{
// @brief Seconds of the day
constexpr timedelta::value_type SECONDS_PER_DAY = 86400;
constexpr timedelta::value_type NANOSECONDS_PER_SECOND = 1000000000;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get total seconds
// @return Timedelta in seconds
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
timedelta::value_type
timedelta::to_seconds () const noexcept
{
    return seconds_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get total nanoseconds
// @return Timedelta in nanoseconds
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
timedelta::value_type
timedelta::to_nanoseconds () const noexcept
{
    return seconds_ * NANOSECONDS_PER_SECOND + nanoseconds_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get total days
// @return Timedelta in days
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
timedelta::value_type
timedelta::to_days () const noexcept
{
    return seconds_ / SECONDS_PER_DAY;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set total seconds
// @param value Timedelta in seconds
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
timedelta::from_seconds (value_type value) noexcept
{
    seconds_ = value;
    nanoseconds_ = 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set total nanoseconds
// @param value Timedelta in nanoseconds
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
timedelta::from_nanoseconds (value_type value) noexcept
{
    seconds_ = value / NANOSECONDS_PER_SECOND;
    nanoseconds_ = value % NANOSECONDS_PER_SECOND;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set total days
// @param value Timedelta in days
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
timedelta::from_days (value_type value) noexcept
{
    seconds_ = SECONDS_PER_DAY * value;
    nanoseconds_ = 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Operator+=
// @param td Timedelta object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
timedelta &
timedelta::operator+= (const timedelta &td) noexcept
{
    nanoseconds_ += td.nanoseconds_;
    seconds_ += td.seconds_;

    _normalize ();

    return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Operator-=
// @param td Timedelta object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
timedelta &
timedelta::operator-= (const timedelta &td) noexcept
{
    nanoseconds_ -= td.nanoseconds_;
    seconds_ -= td.seconds_;

    _normalize ();

    return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Normalize timedelta
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
timedelta::_normalize () noexcept
{
    if (nanoseconds_ >= NANOSECONDS_PER_SECOND ||
        nanoseconds_ <= -NANOSECONDS_PER_SECOND)
    {
        seconds_ += nanoseconds_ / NANOSECONDS_PER_SECOND;
        nanoseconds_ %= NANOSECONDS_PER_SECOND;
    }

    if (nanoseconds_ < 0 && seconds_ > 0)
    {
        seconds_--;
        nanoseconds_ = NANOSECONDS_PER_SECOND - nanoseconds_;
    }

    else if (nanoseconds_ > 0 && seconds_ < 0)
    {
        seconds_++;
        nanoseconds_ = nanoseconds_ - NANOSECONDS_PER_SECOND;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Operator+
// @param ta Timedelta object A
// @param tb Timedelta object B
// @return New timedelta object as the sum of <i>ta</i> and <i>tb</i>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
timedelta
operator+ (const timedelta &ta, const timedelta &tb) noexcept
{
    timedelta s = ta;
    s += tb;
    return s;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Operator-
// @param ta Timedelta object A
// @param tb Timedelta object B
// @return New timedelta object as the difference of <i>ta</i> and <i>tb</i>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
timedelta
operator- (const timedelta &ta, const timedelta &tb) noexcept
{
    timedelta s = ta;
    s -= tb;
    return s;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new timedelta object from nanoseconds
// @param value Value in nanoseconds
// @return New timedelta object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
timedelta
new_timedelta_from_nanoseconds (timedelta::value_type value)
{
    timedelta d;
    d.from_nanoseconds (value);
    return d;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new timedelta object from seconds
// @param value Value in seconds
// @return New timedelta object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
timedelta
new_timedelta_from_seconds (timedelta::value_type value)
{
    timedelta d;
    d.from_seconds (value);
    return d;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new timedelta object from days
// @param value Value in days
// @return New timedelta object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
timedelta
new_timedelta_from_days (timedelta::value_type value)
{
    timedelta d;
    d.from_days (value);
    return d;
}

} // namespace mobius::core::datetime
