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
#include "file_part_met_txtsrc.hpp"
#include <mobius/core/io/line_reader.hpp>
#include <mobius/core/log.hpp>

namespace mobius::extension::app::emule
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse .part.met.txtsrc line
// @param line Text line
// @return source structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static file_part_met_txtsrc::source
parse_txtsrc_line (const std::string &line)
{
    file_part_met_txtsrc::source src;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get IP
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    size_t colon_pos = line.find (':');

    if (colon_pos == std::string::npos)
        return {};

    src.ip = line.substr (0, colon_pos);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get port
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    size_t comma_pos = line.find (',', colon_pos);

    if (comma_pos == std::string::npos)
        return {};

    std::string port_str =
        line.substr (colon_pos + 1, comma_pos - colon_pos - 1);
    try
    {
        src.port = std::stoi (port_str);
    }
    catch (const std::exception &e)
    {
        return {};
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get date
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    size_t semicolon_pos1 = line.find (';', comma_pos);

    if (semicolon_pos1 == std::string::npos)
        return {};

    src.date = line.substr (comma_pos + 1, semicolon_pos1 - comma_pos - 1);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get datetime
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    size_t semicolon_pos2 = line.find (';', semicolon_pos1 + 1);

    if (semicolon_pos2 == std::string::npos)
        src.datetime = line.substr (semicolon_pos1 + 1);
    else
        src.datetime = line.substr (semicolon_pos1 + 1,
                                    semicolon_pos2 - semicolon_pos1 - 1);

    return src;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_part_met_txtsrc::file_part_met_txtsrc (
    const mobius::core::io::reader &reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (!reader || reader.get_size () < 22)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto line_reader = mobius::core::io::line_reader (reader, "utf-8", "\r\n");
    bool first = true;
    std::string line;

    while (line_reader.read (line))
    {
        if (first)
        {
            if (line.substr (0, 22) != "#format: a.b.c.d:port,")
                return;
            first = false;
        }

        else if (line.size () > 0 && line[0] != '#')
        {
            auto src = parse_txtsrc_line (line);

            if (!src.ip.empty ())
                sources_.push_back (src);
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // End decoding
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    is_instance_ = true;
}

} // namespace mobius::extension::app::emule
