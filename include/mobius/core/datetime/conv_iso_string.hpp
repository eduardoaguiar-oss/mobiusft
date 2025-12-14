#ifndef MOBIUS_CORE_DATETIME_CONV_ISO_STRING_HPP
#define MOBIUS_CORE_DATETIME_CONV_ISO_STRING_HPP

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
#include <mobius/core/datetime/date.hpp>
#include <mobius/core/datetime/time.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <string>

namespace mobius::core::datetime
{
std::string date_to_iso_string (const date&);
std::string time_to_iso_string (const time&);
std::string datetime_to_iso_string (const datetime&);
datetime new_datetime_from_iso_string (const std::string&);

} // namespace mobius::core::datetime

#endif


