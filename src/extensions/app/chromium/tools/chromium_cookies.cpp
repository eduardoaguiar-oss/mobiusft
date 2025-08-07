// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025
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
#include <mobius/core/application.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <unistd.h>
#include <iostream>
#include "../file_cookies.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: chromium_cookies [OPTIONS] <path>" << std::endl;
    std::cerr << "e.g: chromium_cookies 'Cookies'" << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show Chromium Cookies info
// @param path Cookies path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_cookies (const std::string &path)
{
    std::cout << std::endl;
    std::cout << ">> " << path << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Try to decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto f = mobius::core::io::new_file_by_path (path);
    auto reader = f.new_reader ();

    mobius::extension::app::chromium::file_cookies dat (reader);
    if (!dat)
    {
        std::cerr << "\tFile is not an instance of Cookies" << std::endl;
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show cookies
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << "Cookies:" << std::endl;

    for (const auto &entry : dat.get_cookies ())
    {
        std::cout << std::endl;
        std::cout << "\tIndex: " << entry.idx << std::endl;
        std::cout << "\tSchema version: " << dat.get_schema_version ()
                  << std::endl;
        std::cout << "\tCreation UTC: " << entry.creation_utc << std::endl;
        std::cout << "\tEncrypted value: \n" << entry.encrypted_value.dump (12);
        std::cout << "\tExpires UTC: " << entry.expires_utc << std::endl;
        std::cout << "\tHas cross site ancestor: "
                  << (entry.has_cross_site_ancestor ? "yes" : "no")
                  << std::endl;
        std::cout << "\tHas expires: " << (entry.has_expires ? "yes" : "no")
                  << std::endl;
        std::cout << "\tHost key: " << entry.host_key << std::endl;
        std::cout << "\tHTTP only: " << (entry.httponly ? "yes" : "no")
                  << std::endl;
        std::cout << "\tIs HTTP only: " << (entry.is_httponly ? "yes" : "no")
                  << std::endl;
        std::cout << "\tIs persistent: " << (entry.is_persistent ? "yes" : "no")
                  << std::endl;
        std::cout << "\tIs same party: " << (entry.is_same_party ? "yes" : "no")
                  << std::endl;
        std::cout << "\tIs secure: " << (entry.is_secure ? "yes" : "no")
                  << std::endl;
        std::cout << "\tLast access UTC: " << entry.last_access_utc
                  << std::endl;
        std::cout << "\tLast update UTC: " << entry.last_update_utc
                  << std::endl;
        std::cout << "\tName: " << entry.name << std::endl;
        std::cout << "\tPath: " << entry.path << std::endl;
        std::cout << "\tPersistent: " << (entry.persistent ? "yes" : "no")
                  << std::endl;
        std::cout << "\tPriority: " << static_cast<int> (entry.priority)
                  << std::endl;
        std::cout << "\tSameSite: " << (entry.samesite ? "yes" : "no")
                  << std::endl;
        std::cout << "\tSecure: " << (entry.secure ? "yes" : "no") << std::endl;
        std::cout << "\tSource port: " << entry.source_port << std::endl;
        std::cout << "\tSource scheme: " << entry.source_scheme << std::endl;
        std::cout << "\tSource type: " << static_cast<int> (entry.source_type)
                  << std::endl;
        std::cout << "\tTop frame site key: " << entry.top_frame_site_key
                  << std::endl;
        std::cout << "\tValue: \n" << entry.value.dump (12);
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
    std::cerr << "Chromium Cookies file viewer v1.0" << std::endl;
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
        std::cerr
            << "Error: you must enter at least one path to Cookies file"
            << std::endl;
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
            show_cookies (argv[optind]);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what () << std::endl;
            exit (EXIT_FAILURE);
        }

        optind++;
    }

    app.stop ();

    return EXIT_SUCCESS;
}
