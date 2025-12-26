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
#include "../file_stored_searches_met.hpp"
#include <iostream>
#include <mobius/core/application.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <unistd.h>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: emule_stored_searches_met [OPTIONS] <path>" << std::endl;
    std::cerr << "e.g: emule_stored_searches_met StoredSearches.met"
              << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show CSearchFile data
// @param sf CSearchFile data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
show_file (
    const mobius::extension::app::emule::file_stored_searches_met::CSearchFile
        &sf)
{
    std::cerr << std::endl;
    std::cerr << "\t\t>> File" << std::endl;
    std::cerr << "\t\tHash ED2K: " << sf.hash_ed2k << std::endl;
    std::cerr << "\t\tUser IP: " << sf.user_ip << std::endl;
    std::cerr << "\t\tUser port: " << sf.user_port << std::endl;
    std::cerr << "\t\t\tTags: " << sf.tags.size () << std::endl;

    for (const auto &tag : sf.tags)
    {
        std::cerr << "\t\t\t\t" << int (tag.get_id ()) << '\t'
                  << mobius::core::string::to_hex (tag.get_type (), 2) << '\t'
                  << tag.get_name () << '\t' << tag.get_value ().to_string ()
                  << std::endl;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show search data
// @param s Search data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
show_search (
    const mobius::extension::app::emule::file_stored_searches_met::search &s)
{
    std::cerr << std::endl;
    std::cerr << "\t>> Search" << std::endl;
    std::cerr << "\t\tID: " << s.id << std::endl;
    std::cerr << "\t\tE-type: " << int (s.e_type) << std::endl;
    std::cerr << "\t\tClient search files: " << s.b_client_search_files
              << std::endl;
    std::cerr << "\t\tSpecial title: " << s.special_title << std::endl;
    std::cerr << "\t\tExpression: " << s.expression << std::endl;
    std::cerr << "\t\tFile type: " << s.filetype << std::endl;

    for (const auto &sf : s.files)
        show_file (sf);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show Emule StoredSearches.met info
// @param path StoredSearches.met path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_stored_searches_met (const std::string &path)
{
    std::cout << std::endl;
    std::cout << ">> " << path << std::endl;

    auto f = mobius::core::io::new_file_by_path (path);
    auto reader = f.new_reader ();

    mobius::extension::app::emule::file_stored_searches_met met (reader);
    if (!met)
    {
        std::cerr << "\tFile is not an instance of StoredSearches.met"
                  << std::endl;
        return;
    }

    std::cerr << "\tVersion: " << int (met.get_version ()) << std::endl;

    for (const auto &s : met.get_searches ())
        show_search (s);
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
    std::cerr << "Emule StoredSearches.met viewer v1.0" << std::endl;
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
        std::cerr << "Error: you must enter at least one path to "
                     "StoredSearches.met file"
                  << std::endl;
        usage ();
        exit (EXIT_FAILURE);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show hive info
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    while (optind < argc)
    {
        try
        {
            show_stored_searches_met (argv[optind]);
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
