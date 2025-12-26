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
#include "../file_login_data.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: chromium_login_data [OPTIONS] <path>" << std::endl;
    std::cerr << "e.g: chromium_login_data 'Login Data'" << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show Chromium Login Data info
// @param path Login Data path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_login_data (const std::string &path)
{
    std::cout << std::endl;
    std::cout << ">> " << path << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Try to decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto f = mobius::core::io::new_file_by_path (path);
    auto reader = f.new_reader ();

    mobius::extension::app::chromium::file_login_data dat (reader);
    if (!dat)
    {
        std::cerr << "\tFile is not an instance of Login Data" << std::endl;
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show login data entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << "Login Data entries:" << std::endl;

    for (const auto &entry : dat.get_logins ())
    {
        std::cout << std::endl;
        std::cout << "\tIndex: " << entry.idx << std::endl;
        std::cout << "\tSchema version: " << dat.get_schema_version() << std::endl;
        std::cout << "\tAction URL: " << entry.action_url << std::endl;
        std::cout << "\tAvatar URL: " << entry.avatar_url << std::endl;
        std::cout << "\tBlacklisted by user: " << (entry.blacklisted_by_user ? "yes" : "no") << std::endl;
        std::cout << "\tDate created: " << entry.date_created << std::endl;
        std::cout << "\tDate last used: " << entry.date_last_used << std::endl;
        std::cout << "\tDate password modified: " << entry.date_password_modified << std::endl;
        std::cout << "\tDate received: " << entry.date_received << std::endl;
        std::cout << "\tDate synced: " << entry.date_synced << std::endl;
        std::cout << "\tDisplay name: " << entry.display_name << std::endl;
        std::cout << "\tFederation URL: " << entry.federation_url << std::endl;
        std::cout << "\tGeneration upload status: " << entry.generation_upload_status << std::endl;
        std::cout << "\tIcon URL: " << entry.icon_url << std::endl;
        std::cout << "\tID: " << entry.id << std::endl;
        std::cout << "\tIs zero click: " << (entry.is_zero_click ? "yes" : "no") << std::endl;
        std::cout << "\tKeychain identifier: " << entry.keychain_identifier << std::endl;
        std::cout << "\tOrigin URL: " << entry.origin_url << std::endl;
        std::cout << "\tPassword element: " << entry.password_element << std::endl;
        std::cout << "\tPassword type: " << entry.password_type << std::endl;
        std::cout << "\tPassword encrypted value: \n" << entry.password_value.dump(8) << std::endl;
        std::cout << "\tPreferred: " << (entry.preferred ? "yes" : "no") << std::endl;
        std::cout << "\tScheme: " << entry.scheme << std::endl;
        std::cout << "\tSender email: " << entry.sender_email << std::endl;
        std::cout << "\tSender name: " << entry.sender_name << std::endl;
        std::cout << "\tSender profile image URL: " << entry.sender_profile_image_url << std::endl;
        std::cout << "\tSharing notification displayed: " << (entry.sharing_notification_displayed ? "yes" : "no") << std::endl;
        std::cout << "\tSignon realm: " << entry.signon_realm << std::endl;
        std::cout << "\tSkip zero click: " << (entry.skip_zero_click ? "yes" : "no") << std::endl;
        std::cout << "\tSSL valid: " << (entry.ssl_valid ? "yes" : "no") << std::endl;
        std::cout << "\tSubmit element: " << entry.submit_element << std::endl;
        std::cout << "\tTimes used: " << entry.times_used << std::endl;
        std::cout << "\tUse additional auth: " << (entry.use_additional_auth ? "yes" : "no") << std::endl;
        std::cout << "\tUsername element: " << entry.username_element << std::endl;
        std::cout << "\tUsername value: " << entry.username_value << std::endl;
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
    std::cerr << "Chromium Login Data file viewer v1.0" << std::endl;
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
        std::cerr << "Error: you must enter at least one path to Login Data file"
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
            show_login_data (argv[optind]);
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
