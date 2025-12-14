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
#include <mobius/core/datetime/date.hpp>

namespace mobius::core::datetime
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if year is a leap year
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
date::is_leap_year () const noexcept
{
    return ((year_ % 4) == 0) &&
           (!(((year_ % 100) == 0) && ((year_ % 400) != 0)));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get date weekday
// @return Weekday (0 = Monday, 6 = Sunday)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
date::get_weekday () const noexcept
{
    return (static_cast<resolution_type> (date_to_julian (*this)) + 1) % 7;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add timedelta to a date
// @return date object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
date&
date::operator+= (const timedelta &delta) noexcept
{
    *this = date_from_julian (date_to_julian (*this) + delta.to_days ());
    return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Subtract timedelta from a date
// @return date object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
date&
date::operator-= (const timedelta &delta) noexcept
{
    *this = date_from_julian (date_to_julian (*this) - delta.to_days ());
    return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Compare two dates
// @param d1 date object
// @param d2 date object
// @return true if the two dates are equal
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator== (
    const mobius::core::datetime::date &d1,
    const mobius::core::datetime::date &d2
) noexcept
{
    return bool (d1) == bool (d2) && d1.get_year () == d2.get_year () &&
           d1.get_month () == d2.get_month () && d1.get_day () == d2.get_day ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Compare two dates
// @param d1 date object
// @param d2 date object
// @return true if the d1 < d2
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator< (
    const mobius::core::datetime::date &d1,
    const mobius::core::datetime::date &d2
) noexcept
{
    return bool (d1) == bool (d2) && (d1.get_year () < d2.get_year () ||
                                      (d1.get_year () == d2.get_year () &&
                                       (d1.get_month () < d2.get_month () ||
                                        (d1.get_month () == d2.get_month () &&
                                         d1.get_day () < d2.get_day ()))));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Calculate the number of days between two dates
// @return timedelta object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
timedelta
operator- (const date &d1, const date &d2) noexcept
{
    timedelta delta;
    delta.from_days (date_to_julian (d1) - date_to_julian (d2));

    return delta;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add timedelta to a date
// @return date object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
date
operator+ (const date &d, const timedelta &delta) noexcept
{
    return date_from_julian (date_to_julian (d) + delta.to_days ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Subtract timedelta from a date
// @return date object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
date
operator- (const date &d, const timedelta &delta) noexcept
{
    return date_from_julian (date_to_julian (d) - delta.to_days ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert date to string
// @param d date object
// @return string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
to_string (const date &d)
{
    std::string str;

    if (d)
    {
        char buffer[64] = {0};

        sprintf (
            buffer, "%04d-%02d-%02d", d.get_year (), d.get_month (),
            d.get_day ()
        );

        str = buffer;
    }

    return str;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Write datetime representation to std::ostream
// @param stream ostream reference
// @param dt datetime object
// @return reference to ostream
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::ostream &
operator<< (std::ostream &stream, const date &d)
{
    stream << to_string (d);

    return stream;
}

} // namespace mobius::core::datetime
