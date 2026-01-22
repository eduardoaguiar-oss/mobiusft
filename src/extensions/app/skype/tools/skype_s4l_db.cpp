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
    // Show Account
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

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show calls
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << std::endl;
    std::cout << "Calls:" << std::endl;

    for (const auto &c : dat.get_calls ())
    {
        std::cout << std::endl;
        std::cout << "   Call ID: " << c.call_id << std::endl;
        std::cout << "   Call Type: " << c.call_type << std::endl;
        std::cout << "   Call Direction: " << c.call_direction << std::endl;
        std::cout << "   Call State: " << c.call_state << std::endl;
        std::cout << "   Connect Time: " << c.connect_time << std::endl;
        std::cout << "   End Time: " << c.end_time << std::endl;
        std::cout << "   Message ID: " << c.message_id << std::endl;
        std::cout << "   Message CUID: " << c.message_cuid << std::endl;
        std::cout << "   Originator: " << c.originator << std::endl;
        std::cout << "   Session Type: " << c.session_type << std::endl;
        std::cout << "   Start Time: " << c.start_time << std::endl;
        std::cout << "   Target: " << c.target << std::endl;
        std::cout << "   Thread ID: " << c.thread_id << std::endl;

        std::cout << "   Originator Participant: " << std::endl;
        std::cout << "      Skype Name: " << c.originator_participant.skype_name
                  << std::endl;
        std::cout << "      MRI: " << c.originator_participant.mri << std::endl;
        std::cout << "      Full Name: " << c.originator_participant.full_name
                  << std::endl;
        std::cout << "      Type: " << c.originator_participant.type
                  << std::endl;

        std::cout << "   Target Participants: " << std::endl;
        std::cout << "      Skype Name: " << c.target_participant.skype_name
                  << std::endl;
        std::cout << "      MRI: " << c.target_participant.mri << std::endl;
        std::cout << "      Full Name: " << c.target_participant.full_name
                  << std::endl;
        std::cout << "      Type: " << c.target_participant.type << std::endl;

        std::cout << "   Participants: " << std::endl;
        for (const auto &p : c.participants)
        {
            std::cout << std::endl;
            std::cout << "      Skype Name: " << p.skype_name << std::endl;
            std::cout << "      MRI: " << p.mri << std::endl;
            std::cout << "      Full Name: " << p.full_name << std::endl;
            std::cout << "      Type: " << p.type << std::endl;
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show contacts
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show messages
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << std::endl;
    std::cout << "Messages:" << std::endl;

    struct message
    {
        // @brief CUID
        std::string cuid;

        // @brief Display name
        std::string from_dispname;

        // @brief Is ephemeral
        bool is_ephemeral = false;

        // @brief Is my message
        bool is_my_message = false;

        // @brief NSP PK
        std::string nsp_pk;

        // @brief Type
        std::string type;

        // @brief Parsed Content
        std::vector<mobius::core::pod::map> parsed_content;
    };

    for (const auto &m : dat.get_messages ())
    {
        std::cout << std::endl;
        std::cout << "   Compose Time: " << m.compose_time << std::endl;
        std::cout << "   Content: " << m.content << std::endl;
        std::cout << "   Content type: " << m.content_type << std::endl;
        std::cout << "   Conversation Id: " << m.conversation_id << std::endl;
        std::cout << "   Created Time: " << m.created_time << std::endl;
        std::cout << "   Creator: " << m.creator << std::endl;
        std::cout << "   CUID: " << m.cuid << std::endl;
        std::cout << "   Is ephemeral: " << (m.is_ephemeral ? "yes" : "no")
                  << std::endl;
        std::cout << "   Is my message: " << (m.is_my_message ? "yes" : "no")
                  << std::endl;
        std::cout << "   NSP PK: " << m.nsp_pk << std::endl;
        std::cout << "   Type: " << m.type << std::endl;

        std::cout << "   Parsed Content: " << std::endl;
        for (const auto &pc : m.parsed_content)
        {
            std::cout << "      {" << std::endl;
            for (const auto &[key, value] : pc)
            {
                std::cout << "         " << key << ": " << value << std::endl;
            }
            std::cout << "      }" << std::endl;
        }
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
        std::cerr << "Error: you must enter at least one path to Skype "
                     "s4l-xxx.db file"
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
