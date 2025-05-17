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
#include "../file_key_index_dat.hpp"
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
    std::cerr << "use: emule_file_key_index_dat [OPTIONS] <path>" << std::endl;
    std::cerr << "e.g: emule_file_key_index_dat KeyIndex.dat" << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show name structure
// @param n Name structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
show_name (const mobius::extension::app::emule::file_key_index_dat::name &n)
{
    std::cerr << std::endl;
    std::cerr << "\t\t\t\tLifetime: " << n.lifetime << std::endl;

    std::cerr << "\t\t\t\tFile names:" << std::endl;
    for (const auto &[name, popularity] : n.filenames)
        std::cerr << "\t\t\t\t\t" << popularity << '\t' << name << std::endl;

    std::cerr << "\t\t\t\tIPs:" << std::endl;
    for (const auto &ip : n.ips)
        std::cerr << "\t\t\t\t\t" << ip.value << '\t' << ip.last_published
                  << std::endl;

    std::cerr << "\t\t\t\tTags: " << n.tags.size () << std::endl;

    for (const auto &tag : n.tags)
    {
        std::cerr << "\t\t\t\t" << int (tag.get_id ()) << '\t'
                  << mobius::core::string::to_hex (tag.get_type (), 2) << '\t'
                  << tag.get_name () << '\t' << tag.get_value ().to_string ()
                  << std::endl;
    }

    std::cerr << "\t\t\t\tAICH hashes:" << std::endl;
    for (const auto &h : n.aich_hashes)
        std::cerr << "\t\t\t\t\t" << h << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show source structure
// @param s Source structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
show_source (const mobius::extension::app::emule::file_key_index_dat::source &s)
{
    std::cerr << std::endl;
    std::cerr << "\t\t\tID: " << s.id << std::endl;
    std::cerr << "\t\t\tNames:" << std::endl;

    for (const auto &n : s.names)
        show_name (n);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show key structure
// @param k Key structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
show_key (const mobius::extension::app::emule::file_key_index_dat::key &k)
{
    std::cerr << std::endl;
    std::cerr << "\t>> Key" << std::endl;
    std::cerr << "\t\tID: " << k.id << std::endl;
    std::cerr << "\t\tSources:" << std::endl;

    for (const auto &s : k.sources)
        show_source (s);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show Emule KeyIndex.dat info
// @param path KeyIndex.dat path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_key_index_dat (const std::string &path)
{
    std::cout << std::endl;
    std::cout << ">> " << path << std::endl;

    auto f = mobius::core::io::new_file_by_path (path);
    auto reader = f.new_reader ();

    mobius::extension::app::emule::file_key_index_dat key_index (reader);
    if (!key_index)
    {
        std::cerr << "\tFile is not an instance of KeyIndex.dat" << std::endl;
        return;
    }

    std::cerr << "\tVersion: " << int (key_index.get_version ()) << std::endl;
    std::cerr << "\tSave time: " << key_index.get_save_time () << std::endl;
    std::cerr << "\tClient ID: " << key_index.get_client_id () << std::endl;

    for (const auto &k : key_index.get_keys ())
        show_key (k);
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
    std::cerr << "Emule KeyIndex.dat viewer v1.0" << std::endl;
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
            << "Error: you must enter at least one path to KeyIndex.dat file"
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
            show_key_index_dat (argv[optind]);
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
