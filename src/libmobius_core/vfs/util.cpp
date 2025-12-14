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
#include <mobius/core/string_functions.hpp>
#include <mobius/core/vfs/util.hpp>

namespace mobius::core::vfs
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Normalize drive info
// @param drive_vendor drive vendor
// @param drive_model drive model
// @param drive_serial_number drive serial number
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
normalize_drive_info (std::string &vendor, std::string &model,
                      std::string &serial_number)
{
    // get drive vendor, if any
    size_t pos = model.find (' ');

    if (pos != std::string::npos)
    {
        vendor = mobius::core::string::capitalize (model.substr (0, pos));
        ++pos;
    }

    else
        pos = 0;

    // format drive model
    model = model.substr (pos);
    pos = model.find ('-');

    if (pos != std::string::npos)
        model.erase (pos);

    if (mobius::core::string::startswith (model, "ST"))
        vendor = "Seagate";

    else if (mobius::core::string::startswith (model, "WDC_"))
    {
        model = model.substr (4);
        vendor = "Western Digital";
    }

    // format serial number
    if (mobius::core::string::startswith (serial_number, "WD-"))
    {
        serial_number = serial_number.substr (3);
        vendor = "Western Digital";
    }
}

} // namespace mobius::core::vfs
