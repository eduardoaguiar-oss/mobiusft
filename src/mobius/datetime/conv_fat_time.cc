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
#include <mobius/datetime/datetime.h>
#include <mobius/datetime/timedelta.h>

namespace mobius
{
namespace datetime
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Convert FAT timestamp to datetime
//! \param d date value
//! \param t time value
//! \return datetime
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datetime
new_datetime_from_fat_time (std::uint16_t d, std::uint16_t t)
{
  mobius::datetime::datetime dt;

  if (d != 0 && d != 0xffff && t != 0 && t != 0xffff)
    dt = mobius::datetime::datetime (
           ((d >> 9) & 127) + 1980,
           (d >> 5) & 15,
           d & 31,
           (t >> 11) & 31,
           (t >> 5) & 63,
           (t & 31) * 2);

  return dt;
}

} // namespace datetime
} // namespace mobius
