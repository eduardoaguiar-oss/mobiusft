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
#include <mobius/exception.inc>
#include <stdexcept>

namespace mobius::datetime
{
static constexpr datetime EPOCH_UNIX_DATETIME (1970, 1, 1, 0, 0, 0);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert UNIX timestamp to datetime
// @param timestamp UNIX timestamp
// @return datetime
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datetime
new_datetime_from_unix_timestamp (std::uint64_t timestamp)
{
  datetime dt;

  if (timestamp)
    {
      timedelta delta;
      delta.set_seconds (timestamp);
      dt = EPOCH_UNIX_DATETIME + delta;
    }

  return dt;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert datetime to UNIX timestamp
// @param dt datetime
// @return UNIX timestamp
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*
std::uint64_t
datetime_from_unix_timestamp (const datetime& dt)
{
  if (dt < EPOCH_UNIX_DATETIME)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("cannot convert datetime to Unix timestamp"));

  // ! \todo waiting for datetime.operator-
  //timedelta delta = dt - EPOCH_UNIX_DATETIME;

  return 0; //EPOCH_UNIX_DATETIME + delta;
}*/

} // namespace mobius::datetime


