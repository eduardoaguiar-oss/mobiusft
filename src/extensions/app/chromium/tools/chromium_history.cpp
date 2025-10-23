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
#include "../file_history.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: chromium_history [OPTIONS] <path>" << std::endl;
    std::cerr << "e.g: chromium_history 'History'" << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show Chromium History info
// @param path History path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_history (const std::string &path)
{
    std::cout << std::endl;
    std::cout << ">> " << path << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Try to decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto f = mobius::core::io::new_file_by_path (path);
    auto reader = f.new_reader ();

    mobius::extension::app::chromium::file_history dat (reader);
    if (!dat)
    {
        std::cerr << "\tFile is not an instance of History" << std::endl;
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show history entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << "History entries:" << std::endl;

    for (const auto &entry : dat.get_history_entries ())
    {
        std::cout << std::endl;
        std::cout << "\tIndex: " << entry.idx << std::endl;
        std::cout << "\tSchema version: " << entry.schema_version << std::endl;
        std::cout << "\tFavicon ID: " << entry.favicon_id << std::endl;
        std::cout << "\tHidden: " << (entry.hidden ? "Yes" : "No") << std::endl;
        std::cout << "\tID: " << entry.id << std::endl;
        std::cout << "\tLast visit time: " << entry.last_visit_time << std::endl;
        std::cout << "\tTitle: " << entry.title << std::endl;
        std::cout << "\tTyped count: " << entry.typed_count << std::endl;
        std::cout << "\tURL: " << entry.url << std::endl;
        std::cout << "\tVisit count: " << entry.visit_count << std::endl;
        std::cout << "\tApp ID: " << entry.app_id << std::endl;
        std::cout << "\tConsider for NTP most visited: "
                  << (entry.consider_for_ntp_most_visited ? "Yes" : "No")
                  << std::endl;
        std::cout << "\tExternal referrer URL: " << entry.external_referrer_url
                  << std::endl;
        std::cout << "\tFrom visit ID: " << entry.from_visit << std::endl;
        std::cout << "\tVisit ID: " << entry.visit_id << std::endl;
        std::cout << "\tIncremented omnibox typed score: "
                  << (entry.incremented_omnibox_typed_score ? "Yes" : "No")
                  << std::endl;
        std::cout << "\tIs known to sync: " << (entry.is_known_to_sync ? "Yes" : "No")
                  << std::endl;
        std::cout << "\tOpener visit ID: " << entry.opener_visit << std::endl;
        std::cout << "\tOriginator cache GUID: " << entry.originator_cache_guid
                  << std::endl;
        std::cout << "\tOriginator from visit ID: " << entry.originator_from_visit
                  << std::endl;
        std::cout << "\tOriginator opener visit ID: " << entry.originator_opener_visit
                  << std::endl;
        std::cout << "\tOriginator visit ID: " << entry.originator_visit_id
                  << std::endl;
        std::cout << "\tPublicly routable: "
                  << (entry.publicly_routable ? "Yes" : "No") << std::endl;
        std::cout << "\tSegment ID: " << entry.segment_id << std::endl;
        std::cout << "\tTransition type: " << entry.transition << std::endl;
        std::cout << "\tVisit URL: " << entry.visit_url << std::endl;
        std::cout << "\tVisit duration: " << entry.visit_duration << std::endl;
        std::cout << "\tVisit time: " << entry.visit_time << std::endl;
        std::cout << "\tVisited link ID: " << entry.visited_link_id << std::endl;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show download entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << std::endl;
    std::cout << "Download entries:" << std::endl;

    for (const auto &entry : dat.get_downloads ())
    {
        std::cout << std::endl;
        std::cout << "\tIndex: " << entry.idx << std::endl;
        std::cout << "\tBy extension ID: " << entry.by_ext_id << std::endl;
        std::cout << "\tBy extension name: " << entry.by_ext_name << std::endl;
        std::cout << "\tBy web app ID: " << entry.by_web_app_id << std::endl;
        std::cout << "\tCurrent path: " << entry.current_path << std::endl;
        std::cout << "\tDanger type: " << entry.danger_type << std::endl;
        std::cout << "\tEmbedder download data: "
                  << entry.embedder_download_data << std::endl;
        std::cout << "\tEnd time: " << entry.end_time << std::endl;
        std::cout << "\tETag: " << entry.etag << std::endl;
        std::cout << "\tFull path: " << entry.full_path << std::endl;
        std::cout << "\tGUID: " << entry.guid << std::endl;
        std::cout << "\tHash: " << entry.hash << std::endl;
        std::cout << "\tHTTP method: " << entry.http_method << std::endl;
        std::cout << "\tID: " << entry.id << std::endl;
        std::cout << "\tInterrupt reason: " << entry.interrupt_reason
                  << std::endl;
        std::cout << "\tLast access time: " << entry.last_access_time
                  << std::endl;
        std::cout << "\tLast modified: " << entry.last_modified << std::endl;
        std::cout << "\tMIME type: " << entry.mime_type << std::endl;
        std::cout << "\tOpened: " << (entry.opened ? "Yes" : "No") << std::endl;
        std::cout << "\tOriginal MIME type: " << entry.original_mime_type
                  << std::endl;
        std::cout << "\tReceived bytes: " << entry.received_bytes << std::endl;
        std::cout << "\tReferrer: " << entry.referrer << std::endl;
        std::cout << "\tSite URL: " << entry.site_url << std::endl;
        std::cout << "\tStart time: " << entry.start_time << std::endl;
        std::cout << "\tState: " << entry.state << std::endl;
        std::cout << "\tTab referrer URL: " << entry.tab_referrer_url
                  << std::endl;
        std::cout << "\tTab URL: " << entry.tab_url << std::endl;
        std::cout << "\tTarget path: " << entry.target_path << std::endl;
        std::cout << "\tTotal bytes: " << entry.total_bytes << std::endl;
        std::cout << "\tTransient: " << (entry.transient ? "Yes" : "No") << std::endl;
        std::cout << "\tURL: " << entry.url << std::endl;
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
    std::cerr << "Chromium History file viewer v1.0" << std::endl;
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
        std::cerr << "Error: you must enter at least one path to History file"
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
            show_history (argv[optind]);
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
