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
#include "../file_downloads_sqlite.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: gecko_downloads [OPTIONS] <path>" << std::endl;
    std::cerr << "e.g: gecko_downloads 'downloads.sqlite'" << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show Gecko Downloads info
// @param path Downloads path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_downloads (const std::string &path)
{
    std::cout << std::endl;
    std::cout << ">> " << path << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Try to decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto f = mobius::core::io::new_file_by_path (path);
    auto reader = f.new_reader ();

    mobius::extension::app::gecko::file_downloads_sqlite dat (reader);
    if (!dat)
    {
        std::cerr << "\tFile is not an instance of downloads.sqlite" << std::endl;
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show downloads
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << "Downloads:" << std::endl;

    for (const auto &entry : dat.get_downloads ())
    {
        std::cout << std::endl;
        std::cout << "\tIndex: " << entry.idx << std::endl;
        std::cout << "\tAuto resume: " << entry.auto_resume << std::endl;
        std::cout << "\tCurr bytes: " << entry.curr_bytes << std::endl;
        std::cout << "\tEnd time: " << entry.end_time << std::endl;
        std::cout << "\tEntity ID: " << entry.entity_id << std::endl;
        std::cout << "\tGuid: " << entry.guid << std::endl;
        std::cout << "\tId: " << entry.id << std::endl;
        std::cout << "\tMax bytes: " << entry.max_bytes << std::endl;
        std::cout << "\tMimetype: " << entry.mime_type << std::endl;
        std::cout << "\tName: " << entry.name << std::endl;
        std::cout << "\tPreferred action: " << entry.preferred_action << std::endl;
        std::cout << "\tPreferred application: " << entry.preferred_application << std::endl;
        std::cout << "\tReferrer: " << entry.referrer << std::endl;
        std::cout << "\tSource: " << entry.source << std::endl;
        std::cout << "\tStart time: " << entry.start_time << std::endl;
        std::cout << "\tState: " << entry.state << std::endl;
        std::cout << "\tTarget: " << entry.target << std::endl;
        std::cout << "\tTemp path: " << entry.temp_path << std::endl;
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
    std::cerr << "Gecko Downloads file viewer v1.0" << std::endl;
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
        std::cerr << "Error: you must enter at least one path to downloads.sqlite file"
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
            show_downloads (argv[optind]);
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
