// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <cmath>
#include <mobius/core/datetime/conv_julian.hpp>

namespace mobius::core::datetime
{
static constexpr julian_type EPOCH_GREGORIAN = 1721425.5; // 0001-01-01 00:00:00

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert julian days to date
// @param jd julian days
// @return date
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
date
date_from_julian (julian_type jd) noexcept
{
    julian_type wjd = floor (jd - 0.5) + 0.5;
    int depoch = wjd - EPOCH_GREGORIAN;
    int quadricent = depoch / 146097;
    int dqc = depoch % 146097;
    int cent = dqc / 36524;
    int dcent = dqc % 36524;
    int quad = dcent / 1461;
    int dquad = dcent % 1461;
    int yindex = dquad / 365;

    int year = (quadricent * 400) + (cent * 100) + (quad * 4) + yindex;
    if (!((cent == 4) || (yindex == 4)))
        year++;

    int yearday = wjd - date_to_julian (date (year, 1, 1));
    int leapadj = 0;

    date d (year, 3, 1);
    if (wjd >= date_to_julian (d))
        leapadj = d.is_leap_year () ? 1 : 2;

    int month = (((yearday + leapadj) * 12) + 373) / 367;
    int day = wjd - date_to_julian (date (year, month, 1)) + 1;

    return date (year, month, day);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert date to julian days
// @param d date
// @return julian days
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
julian_type
date_to_julian (const date &d) noexcept
{
    int offset = 0;

    if (d.get_month () > 2)
    {
        if (d.is_leap_year ())
            offset = -1;
        else
            offset = -2;
    }

    return (EPOCH_GREGORIAN - 1) + (365 * (d.get_year () - 1)) +
           floor ((d.get_year () - 1) / 4) - floor ((d.get_year () - 1) / 100) +
           floor ((d.get_year () - 1) / 400) +
           floor (((367 * d.get_month ()) - 362) / 12) + offset + d.get_day ();
}

} // namespace mobius::core::datetime
