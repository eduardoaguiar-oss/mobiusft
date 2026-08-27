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
#include <mobius/core/application.hpp>
#include <mobius/core/decoder/plist.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/pod/map.hpp>
#include <unistd.h>
#include <iostream>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: plist_view file1 [file2 ...]" << std::endl;
    std::cerr << "e.g: plist_view example.plist" << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show plist data
// @param data Pod data
// @param indent Indentation level
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_plist_data (const mobius::core::pod::data &data, int indent = 0)
{
    std::string indent_str (indent, ' ');

    if (data.is_null ())
    {
        std::cout << "<null>" << std::endl;
        return;
    }

    else if (data.is_bool ())
    {
        std::cout << "<bool> " << (data.to_bool () ? "true" : "false") << std::endl;
        return;
    }

    else if (data.is_integer ())
    {
        std::cout << "<integer> " << data.to_integer () << std::endl;
        return;
    }

    else if (data.is_float ())
    {
        std::cout << "<float> " << data.to_float () << std::endl;
        return;
    }

    else if (data.is_string ())
    {
        std::cout << "<string> " << "\"" << data.to_string () << "\"" << std::endl;
        return;
    }

    else if (data.is_bytearray ())
    {
        std::cout << "<bytearray> " << data.to_bytearray ().size () << " bytes" << std::endl;
        std::cout << data.to_bytearray ().dump (indent + 2);
        return;
    }

    else if (data.is_datetime ())
    {
        std::cout << "<datetime> " << data.to_datetime () << std::endl;
        return;
    }

    else if (data.is_list ())
    {
        const auto vec = data.to_list ();
        auto size = vec.size ();
        std::cout << "<array> (" << (size == 1 ? "1 item" : std::to_string (size) + " items") << ")" << std::endl;

        for (const auto &item : vec)
            show_plist_data (item, indent + 2);

        return;
    }

    else if (data.is_map ())
    {
        const auto map = data.to_map ();
        auto size = map.get_size ();

        std::cout << indent_str << "<dict> (" << (size == 1 ? "1 item" : std::to_string (size) + " items") << ")" << std::endl;
        
        for (const auto &[k, v] : map)
        {
            std::cout << indent_str << "  " << k << ": ";
            show_plist_data (v, indent + 2);
        }

        return;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show plist file
// @param path Path to plist file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_plist (const std::string &path)
{
    auto f = mobius::core::io::new_file_by_path (path);
    auto data = mobius::core::decoder::plist (f.new_reader ());

    show_plist_data (data);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief main function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
main (int argc, char **argv)
{
    mobius::core::application app;
    std::cerr << app.get_name () << " v" << app.get_version () << std::endl;
    std::cerr << app.get_copyright () << std::endl;
    std::cerr << "Plist View v1.0" << std::endl;
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

    if (optind >= argc)
    {
        std::cerr << std::endl;
        std::cerr << "Error: you must enter a valid path to a plist file" << std::endl;
        usage ();
        exit (EXIT_FAILURE);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show plist data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (int i = optind; i < argc; ++i)
    {
        std::string path = argv[i];
        std::cout << std::endl;
        std::cout << "Plist file: " << path << std::endl;

        try
        {
            show_plist (path);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what () << std::endl;
        }
    }
}
