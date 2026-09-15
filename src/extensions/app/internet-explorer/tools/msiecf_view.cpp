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
#include "../file_msiecf.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: msiecf_view [OPTIONS] <path>" << std::endl;
    std::cerr << "e.g: msiecf_view 'index.dat'" << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show MSIE Cache File info
// @param path index.dat path
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

    mobius::extension::app::internet_explorer::file_msiecf dat (reader);
    if (!dat)
    {
        std::cerr << "\tFile is not an instance of index.dat" << std::endl;
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show file metadata
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << std::endl;
    std::cout << "File metadata:" << std::endl;
    std::cout << "\tPath: " << path << std::endl;
    std::cout << "\tSignature: " << dat.get_signature () << std::endl;
    std::cout << "\tVersion: " << dat.get_version () << std::endl;
    std::cout << "\tSize: " << dat.get_size () << std::endl;
    std::cout << "\tHash table offset: " << dat.get_hash_table_offset () << std::endl;
    std::cout << "\tTotal blocks: " << dat.get_total_blocks () << std::endl;
    std::cout << "\tAllocated blocks: " << dat.get_allocated_blocks () << std::endl;
    std::cout << "\tCache size limit: " << dat.get_cache_size_limit () << std::endl;
    std::cout << "\tCache size: " << dat.get_cache_size () << std::endl;
    std::cout << "\tCache size non releasable: " << dat.get_cache_size_non_releasable () << std::endl;
    std::cout << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show cache directories
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << "Cache directories:" << std::endl;
    auto directories = dat.get_directories ();

    for (const auto &dir : directories)
        std::cout << "\t" << dir.dirname << '\t' << dir.file_count << std::endl;

    std::cout << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show URLs
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << "URLs:" << std::endl;

    for (const auto &url : dat.get_urls ())
    {
        std::cout << std::endl;
        std::cout << "\tURL: " << url.location << std::endl;
        std::cout << "\t\tRecord Type: " << static_cast<int> (url.record_type) << std::endl;
        std::cout << "\t\tRecord Offset: " << url.record_offset << " (0x"
                  << mobius::core::string::to_hex (url.record_offset, 8) << ")" << std::endl;
        std::cout << "\t\tRecord Hash: 0x" << mobius::core::string::to_hex (url.record_hash, 8) << std::endl;
        std::cout << "\t\tRecord Original Hash: 0x" << mobius::core::string::to_hex (url.record_original_hash, 8)
                  << std::endl;
        std::cout << "\t\tRecord Calculated Hash: 0x" << mobius::core::string::to_hex (url.calculated_hash, 8)
                  << std::endl;
        std::cout << "\t\tRecord Hash Table Index: " << url.record_hash_table_idx << std::endl;
        std::cout << "\t\tRecord Hash Entry Index: " << url.record_hash_entry_idx << std::endl;

        std::cout << std::endl;
        std::cout << "\t\tAccess Time: " << url.access_time << std::endl;
        std::cout << "\t\tModification Time: " << url.modification_time << std::endl;
        std::cout << "\t\tExpiration Time: " << url.expiration_time << std::endl;
        std::cout << "\t\tLast Sync Time: " << url.last_sync_time << std::endl;
        std::cout << "\t\tLocal Time: " << url.local_time << std::endl;

        std::cout << std::endl;
        std::cout << "\t\tLocation: " << url.location << std::endl;
        std::cout << "\t\tLocation Type: " << url.location_type << std::endl;
        std::cout << "\t\tLocation Username: " << url.location_username << std::endl;
        std::cout << "\t\tLocation Value: " << url.location_value << std::endl;
        std::cout << "\t\tPage Title: " << url.page_title << std::endl;
        std::cout << "\t\tQuery String: " << url.query_string << std::endl;
        std::cout << "\t\tHTTP Response: " << url.http_response << std::endl;

        std::cout << std::endl;
        std::cout << "\t\tFlags: 0x" << mobius::core::string::to_hex (url.flags, 8) << std::endl;
        std::cout << "\t\tIs Deleted: " << (url.is_deleted ? "true" : "false") << std::endl;
        std::cout << "\t\tIs Reallocated: " << (url.is_reallocated ? "true" : "false") << std::endl;

        std::cout << "\t\tLocation Offset: " << url.location_offset;
        if (url.location_offset)
            std::cout << " (0x" << mobius::core::string::to_hex (url.record_offset + url.location_offset, 8) << ")";
        std::cout << std::endl;

        std::cout << "\t\tFilename Offset: " << url.filename_offset;
        if (url.filename_offset)
            std::cout << " (0x" << mobius::core::string::to_hex (url.record_offset + url.filename_offset, 8) << ")";
        std::cout << std::endl;

        std::cout << "\t\tData Offset: " << url.data_offset;
        if (url.data_offset)
            std::cout << " (0x" << mobius::core::string::to_hex (url.record_offset + url.data_offset, 8) << ")";
        std::cout << std::endl;

        std::cout << "\t\tData Size: " << url.data_size << std::endl;
        std::cout << "\t\tHits: " << url.hits << std::endl;

        std::cout << std::endl;
        std::cout << "\t\tCache Dir Index: " << url.cache_dir_index << std::endl;
        std::cout << "\t\tDirname: " << url.dirname << std::endl;
        std::cout << "\t\tFile Name: " << url.filename << std::endl;
        std::cout << "\t\tFile Size: " << url.cached_file_size << std::endl;
        std::cout << "\t\tFile Creation Time: " << url.cached_file_creation_time << std::endl;

        std::cout << "\t\tTags: " << url.tags.size () << " entries" << std::endl;

        std::vector<std::pair<std::uint8_t, mobius::core::pod::data>> sorted_tags (url.tags.begin (), url.tags.end ());
        std::sort (sorted_tags.begin (), sorted_tags.end (), [] (const auto &a, const auto &b) { return a.first < b.first; });

        for (const auto &[k, v] : sorted_tags) // show sorted
            std::cout << "\t\t\t[0x" << mobius::core::string::to_hex (k, 2) << "]: " << v.to_string () << std::endl;
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
    std::cerr << "MSIE Cache File viewer v1.0" << std::endl;
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
        std::cerr << "Error: you must enter at least one path to index.dat file" << std::endl;
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
