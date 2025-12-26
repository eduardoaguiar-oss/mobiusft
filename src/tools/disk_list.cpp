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
#include <iomanip>
#include <iostream>
#include <mobius/core/application.hpp>
#include <mobius/core/system/device_list.hpp>
#include <unistd.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: disk_list" << std::endl;
    std::cerr << "e.g: disk_list" << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief main function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
main (int argc, char **argv)
{
    mobius::core::application app;
    std::cerr << app.get_name () << " v" << app.get_version () << std::endl;
    std::cerr << app.get_copyright () << std::endl;
    std::cerr << "DeviceList v1.0" << std::endl;
    std::cerr << "by Eduardo Aguiar" << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // parse command line
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    int opt;

    while ((opt = getopt (argc, argv, "h")) != EOF)
    {
        switch (opt)
        {
        case 'h':
            usage ();
            exit (EXIT_SUCCESS);
            break;

        default:
            usage ();
            exit (EXIT_FAILURE);
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // show disks information
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::system::device_list dev_list;

    for (auto dev : dev_list)
    {
        if (dev.get_type () == "disk" && dev.get_property ("ID_CDROM") != "1")
        {
            std::cout << std::endl;
            std::cout << "node: " << dev.get_node () << std::endl;
            std::cout << "vendor: " << dev.get_property ("ID_VENDOR")
                      << std::endl;
            std::cout << "model: " << dev.get_property ("ID_MODEL")
                      << std::endl;
            std::cout << "serial: " << dev.get_property ("ID_SERIAL_SHORT")
                      << std::endl;
            std::cout << "sectors: " << dev.get_sysattr ("size") << std::endl;
            std::cout << "sector size: "
                      << dev.get_sysattr ("queue/logical_block_size")
                      << " bytes" << std::endl;

            auto reader = dev.new_reader ();
            std::cout << "total size: " << reader.get_size () << " bytes"
                      << std::endl;
            std::cout << std::endl;

            decltype (reader)::offset_type pos = 0;

            for (int i = 0; i < 32; i++)
            {
                auto data = reader.read (16);
                std::cout << std::hex << std::setfill ('0') << std::setw (4)
                          << pos << ' ';

                for (auto c : data)
                    std::cout << " " << std::hex << std::setfill ('0')
                              << std::setw (2) << int (c) << std::dec;

                std::cout << "  ";
                for (auto c : data)
                    std::cout << ((c > 31 && c < 127) ? char (c) : '.');

                std::cout << std::endl;
                pos += 16;
            }
        }
    }
}
