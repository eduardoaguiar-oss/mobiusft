// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
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
#include "../file_skype_db.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: skype_skype_db [OPTIONS] <path>" << std::endl;
    std::cerr << "e.g: skype_skype_db skype.db" << std::endl;
    std::cerr << std::endl;
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show Skype skype.db info
// @param path skype.db file path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_skype_db_info (const std::string &path)
{
    std::cout << std::endl;
    std::cout << ">> " << path << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Try to decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto f = mobius::core::io::new_file_by_path (path);
    auto reader = f.new_reader ();

    mobius::extension::app::skype::file_skype_db dat (reader);
    if (!dat)
    {
        std::cerr << "\tFile is not an instance of Skype DB" << std::endl;
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show account info
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto acc = dat.get_account ();

    std::cout << std::endl;
    std::cout << "Account Info:" << std::endl;
    std::cout << "   MRI: " << acc.mri <<  std::endl;
    std::cout << "   Skype Name: " << acc.skype_name <<  std::endl;
    std::cout << "   Balance Precision: " << acc.balance_precision <<  std::endl;
    std::cout << "   Balance Currency: " << acc.balance_currency <<  std::endl;
    std::cout << "   Full Name: " << acc.full_name <<  std::endl;
    std::cout << "   First Name: " << acc.first_name <<  std::endl;
    std::cout << "   Last Name: " << acc.last_name <<  std::endl;
    std::cout << "   Mood: " << acc.mood <<  std::endl;
    std::cout << "   Avatar URL: " << acc.avatar_url <<  std::endl;
    std::cout << "   Avatar File Path: " << acc.avatar_file_path <<  std::endl;
    std::cout << "   Conversation Last Sync Time: " << acc.conversation_last_sync_time <<  std::endl;
    std::cout << "   Last Seen Inbox Timestamp: " << acc.last_seen_inbox_timestamp <<  std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show contacts
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << std::endl;
    std::cout << "Contacts:" << std::endl;

    for (const auto &ct : dat.get_contacts ())
    {
        std::cout << std::endl;
        std::cout << "   Record Index: " << ct.idx << std::endl;
        std::cout << "   About Me: " << ct.about_me << std::endl;
        std::cout << "   Assigned Phonelabel 1: " << ct.assigned_phonelabel_1 << std::endl;
        std::cout << "   Assigned Phonelabel 2: " << ct.assigned_phonelabel_2 << std::endl;
        std::cout << "   Assigned Phonelabel 3: " << ct.assigned_phonelabel_3 << std::endl;
        std::cout << "   Assigned Phonenumber 1: " << ct.assigned_phonenumber_1 << std::endl;
        std::cout << "   Assigned Phonenumber 2: " << ct.assigned_phonenumber_2 << std::endl;
        std::cout << "   Assigned Phonenumber 3: " << ct.assigned_phonenumber_3 << std::endl;
        std::cout << "   Authorized: " << ct.authorized << std::endl;
        std::cout << "   Avatar Downloaded From: " << ct.avatar_downloaded_from << std::endl;
        std::cout << "   Avatar File Path: " << ct.avatar_file_path << std::endl;
        std::cout << "   Avatar Url: " << ct.avatar_url << std::endl;
        std::cout << "   Birthday: " << ct.birthday << std::endl;
        std::cout << "   Blocked: " << (ct.blocked ? "Yes" : "No") << std::endl;
        std::cout << "   City: " << ct.city << std::endl;
        std::cout << "   Contact Type: " << ct.contact_type << std::endl;
        std::cout << "   Country: " << ct.country << std::endl;
        std::cout << "   Display Name: " << ct.display_name << std::endl;
        std::cout << "   Full Name: " << ct.full_name << std::endl;
        std::cout << "   Gender: " << ct.gender << std::endl;
        std::cout << "   Homepage: " << ct.homepage << std::endl;
        std::cout << "   Is Buddy: " << (ct.is_buddy ? "Yes" : "No") << std::endl;
        std::cout << "   Is Favorite: " << (ct.is_favorite ? "Yes" : "No") << std::endl;
        std::cout << "   Is Suggested: " << (ct.is_suggested ? "Yes" : "No") << std::endl;
        std::cout << "   Mood: " << ct.mood << std::endl;
        std::cout << "   Mri: " << ct.mri << std::endl;
        std::cout << "   Phone Number Home: " << ct.phone_number_home << std::endl;
        std::cout << "   Phone Number Mobile: " << ct.phone_number_mobile << std::endl;
        std::cout << "   Phone Number Office: " << ct.phone_number_office << std::endl;
        std::cout << "   Province: " << ct.province << std::endl;
        std::cout << "   Recommendation Json: " << ct.recommendation_json << std::endl;
        std::cout << "   Recommendation Rank: " << ct.recommendation_rank << std::endl;
        std::cout << "   Unistore Version: " << ct.unistore_version << std::endl;
        std::cout << "   Update Version: " << ct.update_version << std::endl;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show SMS messages
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << std::endl;
    std::cout << "SMS Messages:" << std::endl;
    
    for (const auto &sm : dat.get_sms_messages ())
    {
        std::cout << std::endl;
        std::cout << "   Record Index: " << sm.idx << std::endl;
        std::cout << "   Author: " << sm.author << std::endl;
        std::cout << "   Clientmessageid: " << sm.clientmessageid << std::endl;
        std::cout << "   Content: " << sm.content << std::endl;
        std::cout << "   Convdbid: " << sm.convdbid << std::endl;
        std::cout << "   Dbid: " <<  sm.dbid << std::endl;
        std::cout << "   Editedtime: " << sm.editedtime << std::endl;
        std::cout << "   Id: " << sm.id << std::endl;
        std::cout << "   Is Preview: " << sm.is_preview << std::endl;
        std::cout << "   Json: " << sm.json << std::endl;
        std::cout << "   Messagetype: " << sm.messagetype << std::endl;
        std::cout << "   Originalarrivaltime: " << sm.original_arrival_time << std::endl;
        std::cout << "   Properties: " << sm.properties << std::endl;
        std::cout << "   Sendingstatus: " << sm.sendingstatus << std::endl;
        std::cout << "   Skypeguid: " << sm.skypeguid << std::endl;
        std::cout << "   Smsmessagedbid: " << sm.smsmessagedbid << std::endl;
        std::cout << "   Version: " << sm.version << std::endl;
        std::cout << "   Mmsdownloadstatus: " << sm.mmsdownloadstatus << std::endl;
        std::cout << "   Smstransportid: " << sm.smstransportid << std::endl;
        std::cout << "   Smstransportname: " << sm.smstransportname << std::endl;
        std::cout << "   Unistoreid: " << sm.unistoreid << std::endl;
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
    std::cerr << "Skype skype.db file viewer v1.0" << std::endl;
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
        std::cerr << "Error: you must enter at least one path to Skype skype.db file"
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
            show_skype_db_info (argv[optind]);
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
