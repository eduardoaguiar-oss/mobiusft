#ifndef MOBIUS_DATETIME_CONV_ISO_STRING_H
#define MOBIUS_DATETIME_CONV_ISO_STRING_H

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
#include <string>

namespace mobius
{
namespace datetime
{
std::string date_to_iso_string (const date&);
std::string time_to_iso_string (const time&);
std::string datetime_to_iso_string (const datetime&);
datetime new_datetime_from_iso_string (const std::string&);

} // namespace datetime
} // namespace mobius

#endif
