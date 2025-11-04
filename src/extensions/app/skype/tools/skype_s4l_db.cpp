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
#include "../file_s4l_db.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: skype_s4l_db [OPTIONS] <path>" << std::endl;
    std::cerr << "e.g: skype_s4l_db s4l-xxx.db" << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show Skype Main DB info
// @param path Main DB file path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_main_db_info (const std::string &path)
{
    std::cout << std::endl;
    std::cout << ">> " << path << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Try to decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto f = mobius::core::io::new_file_by_path (path);
    auto reader = f.new_reader ();

    mobius::extension::app::skype::file_s4l_db dat (reader);
    if (!dat)
    {
        std::cerr << "\tFile is not an instance of s4l-xxx.db" << std::endl;
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show main db entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << "Account:" << std::endl;

    auto acc = dat.get_account ();
    std::cout << "   Skype Name: " << acc.skype_name << std::endl;
    std::cout << "   Primary Member Name: " << acc.primary_member_name
              << std::endl;
    std::cout << "   Full Name: " << acc.full_name << std::endl;
    std::cout << "   Birthdate: " << acc.birthdate << std::endl;
    std::cout << "   Country: " << acc.country << std::endl;
    std::cout << "   Province: " << acc.province << std::endl;
    std::cout << "   City: " << acc.city << std::endl;
    std::cout << "   Timezone: " << acc.timezone << std::endl;
    std::cout << "   Locale: " << acc.locale << std::endl;
    std::cout << "   Device ID: " << acc.device_id << std::endl;
    std::cout << "   MS Account ID from Sign In: " << acc.msaid_from_signin
              << std::endl;
    std::cout << "   MS Account CID: " << acc.msa_cid << std::endl;
    std::cout << "   MS Account CID Hex: " << acc.msa_cid_hex << std::endl;
    std::cout << "   MS Account ID: " << acc.msa_id << std::endl;
    std::cout << "   Emails: ";
    for (const auto &e : acc.emails)
        std::cout << e << " ";
    std::cout << std::endl;
    std::cout << "   Phone Numbers: ";
    for (const auto &p : acc.phone_numbers)
        std::cout << p << " ";
    std::cout << std::endl;
    std::cout << "   Mood Text: " << acc.mood_text << std::endl;
    std::cout << "   Thumbnail URL: " << acc.thumbnail_url << std::endl;
    std::cout << "   App Version: " << acc.app_version << std::endl;

    std::cout << std::endl;
    std::cout << "Contacts:" << std::endl;

    for (const auto &c : dat.get_contacts ())
    {
        std::cout << std::endl;
        std::cout << "   Skype Name: " << c.skype_name << std::endl;
        std::cout << "   MRI: " << c.mri << std::endl;
        std::cout << "   Full Name: " << c.full_name << std::endl;
        std::cout << "   Birthdate: " << c.birthdate << std::endl;
        std::cout << "   Gender: " << c.gender << std::endl;
        std::cout << "   Country: " << c.country << std::endl;
        std::cout << "   Province: " << c.province << std::endl;
        std::cout << "   City: " << c.city << std::endl;
        std::cout << "   Emails: ";
        for (const auto &e : c.emails)
            std::cout << e << " ";
        std::cout << std::endl;
        std::cout << "   Phone Numbers: ";
        for (const auto &p : c.phone_numbers)
            std::cout << p << " ";
        std::cout << std::endl;
        std::cout << "   Mood Text: " << c.mood_text << std::endl;
        std::cout << "   Thumbnail URL: " << c.thumbnail_url << std::endl;
        std::cout << "   Fetched Time: " << c.fetched_time << std::endl;
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
    std::cerr << "Skype s4l.db file viewer v1.0" << std::endl;
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
            << "Error: you must enter at least one path to Skype s4l-xxx.db file"
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
            show_main_db_info (argv[optind]);
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
