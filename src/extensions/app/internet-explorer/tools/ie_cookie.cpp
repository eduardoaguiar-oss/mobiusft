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
#include <mobius/core/io/file.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <unistd.h>
#include <iostream>
#include "../file_cookie.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: ie_cookie [OPTIONS] <path>" << std::endl;
    std::cerr << "e.g: ie_cookie '<cookie-file>.txt'" << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show MSIE Cache File info
// @param path Cookie file path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_file (const std::string &path)
{
    std::cout << std::endl;
    std::cout << ">> " << path << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Try to decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto f = mobius::core::io::new_file_by_path (path);
    auto reader = f.new_reader ();

    mobius::extension::app::internet_explorer::file_cookie dat (reader);
    if (!dat)
    {
        std::cerr << "\tFile is not an instance of a cookie file" << std::endl;
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show Cookies
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << "Cookies:" << path << std::endl;

    struct cookie
    {
        // @brief Cookie name
        std::string name;

        // @brief Cookie value
        std::string value;

        // @brief Cookie domain
        std::string domain;

        // @brief Cookie flags
        int flags = 0;

        // @brief Cookie creation time
        mobius::core::datetime::datetime creation_time;

        // @brief Cookie expiration time
        mobius::core::datetime::datetime expiration_time;
    };

    for (const auto &c : dat.get_cookies ())
    {
        std::cout << std::endl;
        std::cout << "\tName: " << c.name << std::endl;
        std::cout << "\tValue: " << c.value << std::endl;
        std::cout << "\tDomain: " << c.domain << std::endl;
        std::cout << "\tFlags: " << c.flags << std::endl;
        std::cout << "\tCreated: " << c.creation_time << std::endl;
        std::cout << "\tExpires: " << c.expiration_time << std::endl;
        std::cout << "\tIs Secure: " << (c.is_secure ? "yes" : "no") << std::endl;
        std::cout << "\tIs Session: " << (c.is_session ? "yes" : "no") << std::endl;
        std::cout << "\tIs Third Party: " << (c.is_third_party ? "yes" : "no") << std::endl;
        std::cout << "\tIs Restricted: " << (c.is_restricted ? "yes" : "no") << std::endl;
        std::cout << "\tIs IE6: " << (c.is_ie6 ? "yes" : "no") << std::endl;
        std::cout << "\tIs Legacy: " << (c.is_legacy ? "yes" : "no") << std::endl;
        std::cout << "\tIs HTTPOnly: " << (c.is_httponly ? "yes" : "no") << std::endl;
    }
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Main function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
main (int argc, char **argv)
{
    mobius::core::application app;
    mobius::core::set_logfile_path ("mobius.log");

    app.start ();

    std::cerr << app.get_name () << " v" << app.get_version () << std::endl;
    std::cerr << app.get_copyright () << std::endl;
    std::cerr << "Cookie File viewer v1.0" << std::endl;
    std::cerr << "by Eduardo Aguiar" << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Parse command line
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
        std::cerr << "Error: you must enter at least one path to cookie file" << std::endl;
        usage ();
        exit (EXIT_FAILURE);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show info
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    while (optind < argc)
    {
        try
        {
            show_file (argv[optind]);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what () << std::endl;
        }

        optind++;
    }

    app.stop ();

    return EXIT_SUCCESS;
}
