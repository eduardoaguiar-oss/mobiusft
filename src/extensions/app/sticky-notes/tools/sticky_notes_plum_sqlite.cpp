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
#include "../file_plum_sqlite.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: sticky_notes_plum_sqlite [OPTIONS] <path>" << std::endl;
    std::cerr << "e.g: sticky_notes_plum_sqlite 'plum.sqlite'" << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show Plum.sqlite info
// @param path Plum.sqlite path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_plum_sqlite (const std::string &path)
{
    std::cout << std::endl;
    std::cout << ">> " << path << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Try to decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto f = mobius::core::io::new_file_by_path (path);
    auto reader = f.new_reader ();

    mobius::extension::app::sticky_notes::file_plum_sqlite dat (reader);
    if (!dat)
    {
        std::cerr << "\tFile is not an instance of plum.sqlite" << std::endl;
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show notes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << "Notes:" << std::endl;

    for (const auto &entry : dat.get_notes ())
    {
        std::cout << std::endl;
        std::cout << "\tIndex: " << entry.idx << std::endl;
        std::cout << "\tChange Key: " << entry.change_key << std::endl;
        std::cout << "\tCreated At: " << entry.created_at << std::endl;
        std::cout << "\tCreation Note ID Anchor: " << entry.creation_note_id_anchor << std::endl;
        std::cout << "\tDeleted At: " << entry.deleted_at << std::endl;
        std::cout << "\tID: " << entry.id << std::endl;
        std::cout << "\tIs Always On Top: " << (entry.is_always_on_top ? "true" : "false") << std::endl;
        std::cout << "\tIs Future Note: " << (entry.is_future_note ? "true" : "false") << std::endl;
        std::cout << "\tIs Open: " << (entry.is_open ? "true" : "false") << std::endl;
        std::cout << "\tIs Remote Data Invalid: " << (entry.is_remote_data_invalid ? "true" : "false") << std::endl;
        std::cout << "\tLast Server Version: " << entry.last_server_version << std::endl;
        std::cout << "\tParent ID: " << entry.parent_id << std::endl;
        std::cout << "\tPending Insights Scan: " << entry.pending_insights_scan << std::endl;
        std::cout << "\tRemote ID: " << entry.remote_id << std::endl;
        std::cout << "\tRemote Schema Version: " << entry.remote_schema_version << std::endl;
        std::cout << "\tText: " << entry.text << std::endl;
        std::cout << "\tTheme: " << entry.theme << std::endl;
        std::cout << "\tType: " << entry.type << std::endl;
        std::cout << "\tUpdated At: " << entry.updated_at << std::endl;
        std::cout << "\tWindow Position: " << entry.window_position << std::endl;
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
    std::cerr << "Sticky Notes Plum.sqlite file viewer v1.0" << std::endl;
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
        std::cerr << "Error: you must enter at least one path to plum.sqlite file"
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
            show_plum_sqlite (argv[optind]);
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
