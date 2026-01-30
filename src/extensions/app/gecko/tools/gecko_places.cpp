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
#include "../file_places_sqlite.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: gecko_places [OPTIONS] <path>" << std::endl;
    std::cerr << "e.g: gecko_places 'places.sqlite'" << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show Gecko Places info
// @param path Places path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_places (const std::string &path)
{
    std::cout << std::endl;
    std::cout << ">> " << path << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Try to decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto f = mobius::core::io::new_file_by_path (path);
    auto reader = f.new_reader ();

    mobius::extension::app::gecko::file_places_sqlite dat (reader);
    if (!dat)
    {
        std::cerr << "\tFile is not an instance of places.sqlite" << std::endl;
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show bookmarks
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << "Bookmarks:" << std::endl;

    for (const auto &entry : dat.get_bookmarks ())
    {
        std::cout << std::endl;
        std::cout << "\tIndex: " << entry.idx << std::endl;
        std::cout << "\tDate Added: " << entry.date_added << std::endl;
        std::cout << "\tFK: " << entry.fk << std::endl;
        std::cout << "\tFolder Type: " << entry.folder_type << std::endl;
        std::cout << "\tGUID: " << entry.guid << std::endl;
        std::cout << "\tID: " << entry.id << std::endl;
        std::cout << "\tKeyword ID: " << entry.keyword_id << std::endl;
        std::cout << "\tLast Modified: " << entry.last_modified << std::endl;
        std::cout << "\tParent: " << entry.parent << std::endl;
        std::cout << "\tParent Name: " << entry.parent_name << std::endl;
        std::cout << "\tPosition: " << entry.position << std::endl;
        std::cout << "\tSync Change Counter: " << entry.sync_change_counter << std::endl;
        std::cout << "\tSync Status: " << entry.sync_status << std::endl;
        std::cout << "\tTitle: " << entry.title << std::endl;
        std::cout << "\tType: " << entry.type << std::endl;
        std::cout << "\tURL: " << entry.url << std::endl;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show visited URLs
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << std::endl;
    std::cout << "Visited URLs:" << std::endl;

    for (const auto &entry : dat.get_visited_urls ())
    {
        std::cout << std::endl;
        std::cout << "\tFrom Visit: " << entry.from_visit << std::endl;
        std::cout << "\tVisit ID: " << entry.visit_id << std::endl;
        std::cout << "\tPlace ID: " << entry.place_id << std::endl;
        std::cout << "\tSession: " << entry.session << std::endl;
        std::cout << "\tSource: " << entry.source << std::endl;
        std::cout << "\tTriggering Place ID: " << entry.triggering_place_id << std::endl;
        std::cout << "\tVisit Date: " << entry.visit_date << std::endl;
        std::cout << "\tVisit Type: " << entry.visit_type << std::endl;
        std::cout << "\tAlt Frecency: " << entry.alt_frecency << std::endl;
        std::cout << "\tDescription: " << entry.description << std::endl;
        std::cout << "\tFavicon ID: " << entry.favicon_id << std::endl;
        std::cout << "\tForeign Count: " << entry.foreign_count << std::endl;
        std::cout << "\tFrecency: " << entry.frecency << std::endl;
        std::cout << "\tGUID: " << entry.guid << std::endl;
        std::cout << "\tHidden: " << entry.hidden << std::endl;
        std::cout << "\tPlaces ID: " << entry.places_id << std::endl;
        std::cout << "\tLast Visit Date: " << entry.last_visit_date << std::endl;
        std::cout << "\tOrigin ID: " << entry.origin_id << std::endl;
        std::cout << "\tPreview Image URL: " << entry.preview_image_url << std::endl;
        std::cout << "\tRecalc Alt Frecency: " << entry.recalc_alt_frecency << std::endl;
        std::cout << "\tRecalc Frecency: " << entry.recalc_frecency << std::endl;
        std::cout << "\tRev Host: " << entry.rev_host << std::endl;
        std::cout << "\tSite Name: " << entry.site_name << std::endl;
        std::cout << "\tTitle: " << entry.title << std::endl;
        std::cout << "\tTyped: " << entry.typed << std::endl;
        std::cout << "\tURL: " << entry.url << std::endl;
        std::cout << "\tURL Hash: " << entry.url_hash << std::endl;
        std::cout << "\tVisit Count: " << entry.visit_count << std::endl;
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
    std::cerr << "Gecko Places file viewer v1.0" << std::endl;
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
        std::cerr << "Error: you must enter at least one path to places.sqlite file"
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
            show_places (argv[optind]);
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
