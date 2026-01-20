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
    std::cout << "   MRI: " << acc.mri << std::endl;
    std::cout << "   Skype Name: " << acc.skype_name << std::endl;
    std::cout << "   Balance Precision: " << acc.balance_precision << std::endl;
    std::cout << "   Balance Currency: " << acc.balance_currency << std::endl;
    std::cout << "   Full Name: " << acc.full_name << std::endl;
    std::cout << "   First Name: " << acc.first_name << std::endl;
    std::cout << "   Last Name: " << acc.last_name << std::endl;
    std::cout << "   Mood: " << acc.mood << std::endl;
    std::cout << "   Avatar URL: " << acc.avatar_url << std::endl;
    std::cout << "   Avatar File Path: " << acc.avatar_file_path << std::endl;
    std::cout << "   Conversation Last Sync Time: "
              << acc.conversation_last_sync_time << std::endl;
    std::cout << "   Last Seen Inbox Timestamp: "
              << acc.last_seen_inbox_timestamp << std::endl;

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
        std::cout << "   Assigned Phonelabel 1: " << ct.assigned_phonelabel_1
                  << std::endl;
        std::cout << "   Assigned Phonelabel 2: " << ct.assigned_phonelabel_2
                  << std::endl;
        std::cout << "   Assigned Phonelabel 3: " << ct.assigned_phonelabel_3
                  << std::endl;
        std::cout << "   Assigned Phonenumber 1: " << ct.assigned_phonenumber_1
                  << std::endl;
        std::cout << "   Assigned Phonenumber 2: " << ct.assigned_phonenumber_2
                  << std::endl;
        std::cout << "   Assigned Phonenumber 3: " << ct.assigned_phonenumber_3
                  << std::endl;
        std::cout << "   Authorized: " << ct.authorized << std::endl;
        std::cout << "   Avatar Downloaded From: " << ct.avatar_downloaded_from
                  << std::endl;
        std::cout << "   Avatar File Path: " << ct.avatar_file_path
                  << std::endl;
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
        std::cout << "   Is Buddy: " << (ct.is_buddy ? "Yes" : "No")
                  << std::endl;
        std::cout << "   Is Favorite: " << (ct.is_favorite ? "Yes" : "No")
                  << std::endl;
        std::cout << "   Is Suggested: " << (ct.is_suggested ? "Yes" : "No")
                  << std::endl;
        std::cout << "   Mood: " << ct.mood << std::endl;
        std::cout << "   Mri: " << ct.mri << std::endl;
        std::cout << "   Phone Number Home: " << ct.phone_number_home
                  << std::endl;
        std::cout << "   Phone Number Mobile: " << ct.phone_number_mobile
                  << std::endl;
        std::cout << "   Phone Number Office: " << ct.phone_number_office
                  << std::endl;
        std::cout << "   Province: " << ct.province << std::endl;
        std::cout << "   Recommendation Json: " << ct.recommendation_json
                  << std::endl;
        std::cout << "   Recommendation Rank: " << ct.recommendation_rank
                  << std::endl;
        std::cout << "   Unistore Version: " << ct.unistore_version
                  << std::endl;
        std::cout << "   Update Version: " << ct.update_version << std::endl;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show corelib messages
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << std::endl;
    std::cout << "Corelib Messages:" << std::endl;

    for (const auto &cm : dat.get_corelib_messages ())
    {
        std::cout << std::endl;
        std::cout << "   Record Index: " << cm.idx << std::endl;
        std::cout << "   Annotation Version: " << cm.annotation_version
                  << std::endl;
        std::cout << "   Author: " << cm.author << std::endl;
        std::cout << "   Author Was Live: " << cm.author_was_live << std::endl;
        std::cout << "   Body Is Rawxml: " << cm.body_is_rawxml << std::endl;
        std::cout << "   Body Xml: " << cm.body_xml << std::endl;
        std::cout << "   Bots Settings: " << cm.bots_settings << std::endl;
        std::cout << "   Call Guid: " << cm.call_guid << std::endl;
        std::cout << "   Charmsg Type: " << cm.charmsg_type << std::endl;
        std::cout << "   Chatmsg Status: " << cm.chatmsg_status << std::endl;
        std::cout << "   Chatname: " << cm.chatname << std::endl;
        std::cout << "   Consumption Status: " << cm.consumption_status
                  << std::endl;
        std::cout << "   Content Flags: " << cm.content_flags << std::endl;
        std::cout << "   Conversation Identity: " << cm.conversation_identity
                  << std::endl;
        std::cout << "   Conversation Name: " << cm.conversation_display_name
                  << std::endl;
        std::cout << "   Convo Id: " << cm.convo_id << std::endl;
        std::cout << "   Crc: " << cm.crc << std::endl;
        std::cout << "   Dialog Partner: " << cm.dialog_partner << std::endl;
        std::cout << "   Edited By: " << cm.edited_by << std::endl;
        std::cout << "   Edited Timestamp: " << cm.edited_timestamp
                  << std::endl;
        std::cout << "   Error Code: " << cm.error_code << std::endl;
        std::cout << "   Extprop Mms Msg Metadata: "
                  << cm.extprop_mms_msg_metadata << std::endl;
        std::cout << "   Extprop Sms Server Id: " << cm.extprop_sms_server_id
                  << std::endl;
        std::cout << "   Extprop Sms Src Msg Id: " << cm.extprop_sms_src_msg_id
                  << std::endl;
        std::cout << "   Extprop Sms Sync Global Id: "
                  << cm.extprop_sms_sync_global_id << std::endl;
        std::cout << "   From Dispname: " << cm.from_dispname << std::endl;
        std::cout << "   Guid: " << cm.guid << std::endl;
        std::cout << "   Id: " << cm.id << std::endl;
        std::cout << "   Identities: " << cm.identities << std::endl;
        std::cout << "   Is Parmanent: " << cm.is_parmanent << std::endl;
        std::cout << "   Language: " << cm.language << std::endl;
        std::cout << "   Leavereason: " << cm.leavereason << std::endl;
        std::cout << "   Newoptions: " << cm.newoptions << std::endl;
        std::cout << "   Newrole: " << cm.newrole << std::endl;
        std::cout << "   Oldoptions: " << cm.oldoptions << std::endl;
        std::cout << "   Option Bits: " << cm.option_bits << std::endl;
        std::cout << "   Param Key: " << cm.param_key << std::endl;
        std::cout << "   Param Value: " << cm.param_value << std::endl;
        std::cout << "   Participant Count: " << cm.participant_count
                  << std::endl;
        std::cout << "   Pk Id: " << cm.pk_id << std::endl;
        std::cout << "   Reaction Thread: " << cm.reaction_thread << std::endl;
        std::cout << "   Reason: " << cm.reason << std::endl;
        std::cout << "   Remote Id: " << cm.remote_id << std::endl;
        std::cout << "   Sending Status: " << cm.sending_status << std::endl;
        std::cout << "   Server Id: " << cm.server_id << std::endl;
        std::cout << "   Timestamp: " << cm.timestamp << std::endl;
        std::cout << "   Timestamp ms: " << cm.timestamp_ms << std::endl;
        std::cout << "   Type: " << cm.type << std::endl;
        std::cout << "   Parsed Content: " << std::endl;

        for (const auto &pc : cm.parsed_content)
        {
            std::cout << "      {" << std::endl;
            for (const auto &[key, value] : pc)
            {
                std::cout << "         " << key << ": " << value << std::endl;
            }
            std::cout << "      }" << std::endl;
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show messages
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << std::endl;
    std::cout << "Messages:" << std::endl;

    for (const auto &m : dat.get_messages ())
    {
        std::cout << std::endl;
        std::cout << "   Record Index: " << m.idx << std::endl;
        std::cout << "   Author: " << m.author << std::endl;
        std::cout << "   Clientmessageid: " << m.clientmessageid << std::endl;
        std::cout << "   Content: " << m.content << std::endl;
        std::cout << "   Convdbid: " << m.convdbid << std::endl;
        std::cout << "   Conversation Type: " << m.conversation_type
                  << std::endl;
        std::cout << "   Conversation Identity: " << m.conversation_id
                  << std::endl;
        std::cout << "   Conversation MRI: " << m.conversation_mri << std::endl;
        std::cout << "   Dbid: " << m.dbid << std::endl;
        std::cout << "   Editedtime: " << m.editedtime << std::endl;
        std::cout << "   Id: " << m.id << std::endl;
        std::cout << "   Is Preview: " << m.is_preview << std::endl;
        std::cout << "   Json: " << m.json << std::endl;
        std::cout << "   Messagetype: " << m.messagetype << std::endl;
        std::cout << "   Originalarrivaltime: " << m.originalarrivaltime
                  << std::endl;
        std::cout << "   Properties: " << m.properties << std::endl;
        std::cout << "   Sendingstatus: " << m.sendingstatus << std::endl;
        std::cout << "   Skypeguid: " << m.skypeguid << std::endl;
        std::cout << "   Version: " << m.version << std::endl;

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
        std::cout << "   Dbid: " << sm.dbid << std::endl;
        std::cout << "   Editedtime: " << sm.editedtime << std::endl;
        std::cout << "   Id: " << sm.id << std::endl;
        std::cout << "   Is Preview: " << sm.is_preview << std::endl;
        std::cout << "   Json: " << sm.json << std::endl;
        std::cout << "   Messagetype: " << sm.messagetype << std::endl;
        std::cout << "   Originalarrivaltime: " << sm.original_arrival_time
                  << std::endl;
        std::cout << "   Properties: " << sm.properties << std::endl;
        std::cout << "   Sendingstatus: " << sm.sendingstatus << std::endl;
        std::cout << "   Skypeguid: " << sm.skypeguid << std::endl;
        std::cout << "   Smsmessagedbid: " << sm.smsmessagedbid << std::endl;
        std::cout << "   Version: " << sm.version << std::endl;
        std::cout << "   Mmsdownloadstatus: " << sm.mmsdownloadstatus
                  << std::endl;
        std::cout << "   Smstransportid: " << sm.smstransportid << std::endl;
        std::cout << "   Smstransportname: " << sm.smstransportname
                  << std::endl;
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
        std::cerr
            << "Error: you must enter at least one path to Skype skype.db file"
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
