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
#include <mobius/core/decoder/filetype.hpp>

namespace mobius::core::decoder
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get filetype according to file content
// @param reader Reader object
// @return Filetype
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_filetype (mobius::core::io::reader reader)
{
    std::string filetype;

    // Read bytes from the beginning of the stream
    auto head = reader.read (512);

    // Check empty stream
    if (!head)
        filetype = "empty";

    // Check datatypes
    else if (head.startswith ("\x89\x50\x4e\x47\x0d\x0a\x1a\x0a"))
        filetype = "image.png";

    else if (head.startswith ("\x4C\x00\x00\x00\x01\x14\x02\x00\x00\x00\x00\x00"
                              "\xC0\x00\x00\x00\x00\x00\x00\x46"))
        filetype = "data.ms.shllink";

    return filetype;
}

} // namespace mobius::core::decoder
