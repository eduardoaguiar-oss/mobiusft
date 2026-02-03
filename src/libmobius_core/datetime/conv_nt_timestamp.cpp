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
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/datetime/timedelta.hpp>

namespace mobius::core::datetime
{
static constexpr datetime EPOCH_NT_DATETIME (1601, 1, 1, 0, 0, 0);
static constexpr datetime EPOCH_DOT_NET_DATETIME (1, 1, 1, 0, 0, 0);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert NT timestamp to datetime
// @param timestamp NT timestamp (64 bits)
// @return datetime
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datetime
new_datetime_from_nt_timestamp (std::uint64_t timestamp)
{
    datetime dt;

    if (timestamp)
    {
        timedelta delta (timestamp / 10000000, (timestamp % 10000000) * 100);
        dt = EPOCH_NT_DATETIME + delta;
    }

    return dt;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert .NET timestamp to datetime
// @param timestamp .NET timestamp (64 bits)
// @return datetime
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datetime
new_datetime_from_dot_net_timestamp (std::uint64_t timestamp)
{
    datetime dt;

    if (timestamp)
    {
        timedelta delta (timestamp / 10000000, (timestamp % 10000000) * 100);
        dt = EPOCH_DOT_NET_DATETIME + delta;
    }

    return dt;
}

} // namespace mobius::core::datetime
