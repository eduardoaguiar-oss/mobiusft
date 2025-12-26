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
#include "../file_settings_dat.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: utorrent_file_settings_dat [OPTIONS] <path>"
              << std::endl;
    std::cerr << "e.g: utorrent_file_settings_dat settings.dat" << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show uTorrent settings.dat info
// @param path settings.dat path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_settings_dat (const std::string &path)
{
    std::cout << std::endl;
    std::cout << ">> " << path << std::endl;

    auto f = mobius::core::io::new_file_by_path (path);
    auto reader = f.new_reader ();

    mobius::extension::app::utorrent::file_settings_dat settings (reader);
    if (!settings)
    {
        std::cerr << "\tFile is not an instance of settings.dat" << std::endl;
        return;
    }

    std::cout << "\tAutostart flag: "
              << (settings.get_autostart () ? "Enabled" : "Disabled")
              << std::endl;
    std::cout << "\tBind port: " << settings.get_bind_port () << std::endl;
    std::cout << "\tCached host: " << settings.get_cached_host () << std::endl;
    std::cout << "\tComputer ID: " << settings.get_computer_id () << std::endl;
    std::cout << "\tDir active downloads: "
              << settings.get_dir_active_downloads () << std::endl;
    std::cout << "\tDir completed downloads: "
              << settings.get_dir_completed_downloads () << std::endl;
    std::cout << "\tDir torrent files: " << settings.get_dir_torrent_files ()
              << std::endl;
    std::cout << "\tExe path: " << settings.get_exe_path () << std::endl;
    std::cout << "\tExecution count: " << settings.get_execution_count ()
              << std::endl;
    std::cout << "\tExternal IP: " << settings.get_external_ip () << std::endl;
    std::cout << "\tInstallation time: " << settings.get_installation_time ()
              << std::endl;
    std::cout << "\tInstallation version: "
              << settings.get_installation_version () << std::endl;
    std::cout << "\tLanguage: " << settings.get_language () << std::endl;
    std::cout << "\tLast used time: " << settings.get_last_used_time ()
              << std::endl;
    std::cout << "\tLast bin change time: "
              << settings.get_last_bin_change_time () << std::endl;
    std::cout << "\tRuntime (seconds): " << settings.get_runtime ()
              << std::endl;
    std::cout << "\tSave path: " << settings.get_save_path () << std::endl;
    std::cout << "\tSettings saved time: "
              << settings.get_settings_saved_time () << std::endl;
    std::cout << "\tStatistics time: "
              << settings.get_statistics_time () << std::endl;
    std::cout << "\tSSDP UUID: " << settings.get_ssdp_uuid () << std::endl;
    std::cout << "\tTotal downloaded bytes: "
              << settings.get_total_bytes_downloaded () << std::endl;
    std::cout << "\tTotal uploaded bytes: "
              << settings.get_total_bytes_uploaded () << std::endl;
    std::cout << "\tUsername: " << settings.get_username () << std::endl;
    std::cout << "\tVersion: " << settings.get_version () << std::endl;

    std::cout << "\tMetadata:" << std::endl;
    for (const auto &[mkey, mvalue] : settings.get_metadata ())
    {
        std::cout << "\t\t" << mkey << ": " << mvalue.to_string () << std::endl;
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
    std::cerr << "µTorrent settings.dat viewer v1.0" << std::endl;
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
            << "Error: you must enter at least one path to settings.dat file"
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
            show_settings_dat (argv[optind]);
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
