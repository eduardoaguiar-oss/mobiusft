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
#include "../file_main_db.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: skype_main_db [OPTIONS] <path>" << std::endl;
    std::cerr << "e.g: skype_main_db main.db" << std::endl;
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

    mobius::extension::app::skype::file_main_db dat (reader);
    if (!dat)
    {
        std::cerr << "\tFile is not an instance of Main DB" << std::endl;
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show main db entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << "Accounts:" << std::endl;

    for (const auto &acc : dat.get_accounts ())
    {
        std::cout << std::endl;
        std::cout << "   Record Index: " << acc.idx << std::endl;
        std::cout << "   Schema version: " << acc.schema_version << std::endl;
        std::cout << "   About: " << acc.about << std::endl;
        std::cout << "   Ad Policy: " << acc.ad_policy << std::endl;
        std::cout << "   Added In Shared Group: " << acc.added_in_shared_group << std::endl;
        std::cout << "   Alertstring: " << acc.alertstring << std::endl;
        std::cout << "   Aliases: " << acc.aliases << std::endl;
        std::cout << "   Assigned Comment: " << acc.assigned_comment << std::endl;
        std::cout << "   Assigned Speeddial: " << acc.assigned_speeddial << std::endl;
        std::cout << "   Authorized Time: " << acc.authorized_time << std::endl;
        std::cout << "   Authrequest History:\n" << acc.authreq_history.dump (8) << std::endl;
        std::cout << "   Authreq Timestamp: " << acc.authreq_timestamp << std::endl;
        std::cout << "   Authrequest Count: " <<  acc.authrequest_count << std::endl;
        std::cout << "   Availability: " << acc.availability << std::endl;
        std::cout << "   Avatar Image:\n" << acc.avatar_image.dump (8) << std::endl;
        std::cout << "   Avatar Policy: " << acc.avatar_policy << std::endl;
        std::cout << "   Avatar Timestamp: " << acc.avatar_timestamp << std::endl;
        std::cout << "   Birthday: " << acc.birthday << std::endl;
        std::cout << "   Buddyblob:\n" << acc.buddyblob.dump (8) << std::endl;
        std::cout << "   Buddycount Policy: " << acc.buddycount_policy << std::endl;
        std::cout << "   Capabilities:\n" << acc.capabilities.dump (8) << std::endl;
        std::cout << "   Cbl Future:\n" << acc.cbl_future.dump (8) << std::endl;
        std::cout << "   Cblsyncstatus: " << acc.cblsyncstatus << std::endl;
        std::cout << "   Chat Policy: " << acc.chat_policy << std::endl;
        std::cout << "   City: " << acc.city << std::endl;
        std::cout << "   Commitstatus: " << acc.commitstatus << std::endl;
        std::cout << "   Country: " << acc.country << std::endl;
        std::cout << "   Displayname: " << acc.displayname << std::endl;
        std::cout << "   Emails: " << acc.emails << std::endl;
        std::cout << "   Federated Presence Policy: " << acc.federated_presence_policy << std::endl;
        std::cout << "   Flamingo Xmpp Status: " << acc.flamingo_xmpp_status << std::endl;
        std::cout << "   Fullname: " << acc.fullname << std::endl;
        std::cout << "   Gender: " << acc.gender << std::endl;
        std::cout << "   Given Authlevel: " << acc.given_authlevel << std::endl;
        std::cout << "   Given Displayname: " << acc.given_displayname << std::endl;
        std::cout << "   Homepage: " << acc.homepage << std::endl;
        std::cout << "   Id: " << acc.id << std::endl;
        std::cout << "   In Shared Group: " << acc.in_shared_group << std::endl;
        std::cout << "   Ipcountry: " << acc.ipcountry << std::endl;
        std::cout << "   Is Permanent: " << (acc.is_permanent ? "Yes" : "No") << std::endl;
        std::cout << "   Languages: " << acc.languages << std::endl;
        std::cout << "   Lastonline Timestamp: " << acc.lastonline_timestamp << std::endl;
        std::cout << "   Lastused Timestamp: " << acc.lastused_timestamp << std::endl;
        std::cout << "   Logoutreason: " << acc.logoutreason << std::endl;
        std::cout << "   Mood Text: " << acc.mood_text << std::endl;
        std::cout << "   Mood Timestamp: " << acc.mood_timestamp << std::endl;
        std::cout << "   Node Capabilities: " << acc.node_capabilities << std::endl;
        std::cout << "   Node Capabilities And: " << acc.node_capabilities_and << std::endl;
        std::cout << "   Nr Of Other Instances: " << acc.nr_of_other_instances << std::endl;
        std::cout << "   Nrof Authed Buddies: " << acc.nrof_authed_buddies << std::endl;
        std::cout << "   Offline Authreq Id: " << acc.offline_authreq_id << std::endl;
        std::cout << "   Offline Callforward: " << acc.offline_callforward << std::endl;
        std::cout << "   Options Change Future:\n" << acc.options_change_future.dump (8) << std::endl;
        std::cout << "   Owner Under Legal Age: " << acc.owner_under_legal_age << std::endl;
        std::cout << "   Partner Channel Status: " << acc.partner_channel_status << std::endl;
        std::cout << "   Partner Optedout: " << acc.partner_optedout << std::endl;
        std::cout << "   Phone Home: " << acc.phone_home << std::endl;
        std::cout << "   Phone Mobile: " << acc.phone_mobile << std::endl;
        std::cout << "   Phone Office: " << acc.phone_office << std::endl;
        std::cout << "   Phonenumbers Policy: " << acc.phonenumbers_policy << std::endl;
        std::cout << "   Profile Attachments:\n" << acc.profile_attachments.dump (8) << std::endl;
        std::cout << "   Profile Timestamp: " << acc.profile_timestamp << std::endl;
        std::cout << "   Province: " << acc.province << std::endl;
        std::cout << "   Pstn Call Policy: " << acc.pstn_call_policy << std::endl;
        std::cout << "   Pstnnumber: " << acc.pstnnumber << std::endl;
        std::cout << "   Pwdchangestatus: " << acc.pwdchangestatus << std::endl;
        std::cout << "   Received Authrequest: " << acc.received_authrequest << std::endl;
        std::cout << "   Refreshing: " << acc.refreshing << std::endl;
        std::cout << "   Registration Timestamp: " << acc.registration_timestamp << std::endl;
        std::cout << "   Revoked Auth: " << acc.revoked_auth << std::endl;
        std::cout << "   Rich Mood Text: " << acc.rich_mood_text << std::endl;
        std::cout << "   Roaming History Enabled: " << acc.roaming_history_enabled << std::endl;
        std::cout << "   Sent Authrequest: " << acc.sent_authrequest << std::endl;
        std::cout << "   Sent Authrequest Serial: " << acc.sent_authrequest_serial << std::endl;
        std::cout << "   Sent Authrequest Time: " << acc.sent_authrequest_time << std::endl;
        std::cout << "   Service Provider Info: " << acc.service_provider_info << std::endl;
        std::cout << "   Set Availability: " << acc.set_availability << std::endl;
        std::cout << "   Shortcircuit Sync: " << acc.shortcircuit_sync << std::endl;
        std::cout << "   Skype Call Policy: " << acc.skype_call_policy << std::endl;
        std::cout << "   Skypein Numbers: " << acc.skypein_numbers << std::endl;
        std::cout << "   Skypename: " << acc.skypename << std::endl;
        std::cout << "   Skypeout Balance: " << acc.skypeout_balance << std::endl;
        std::cout << "   Skypeout Balance Currency: " << acc.skypeout_balance_currency << std::endl;
        std::cout << "   Skypeout Precision: " << acc.skypeout_precision << std::endl;
        std::cout << "   Stack Version: " << acc.stack_version << std::endl;
        std::cout << "   Status: " << acc.status << std::endl;
        std::cout << "   Subscriptions: " << acc.subscriptions << std::endl;
        std::cout << "   Suggested Skypename: " << acc.suggested_skypename << std::endl;
        std::cout << "   Synced Email:\n" << acc.synced_email.dump (8) << std::endl;
        std::cout << "   Timezone: " << acc.timezone << std::endl;
        std::cout << "   Timezone Policy: " << acc.timezone_policy << std::endl;
        std::cout << "   Type: " << acc.type << std::endl;
        std::cout << "   Uses Jcs: " << acc.uses_jcs << std::endl;
        std::cout << "   Verified Company:\n" << acc.verified_company.dump (8) << std::endl;
        std::cout << "   Verified Email:\n" << acc.verified_email.dump (8) << std::endl;
        std::cout << "   Voicemail Policy: " << acc.voicemail_policy << std::endl;
        std::cout << "   Webpresence Policy: " << acc.webpresence_policy << std::endl;
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
    std::cerr << "Skype main.db file viewer v1.0" << std::endl;
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
        std::cerr << "Error: you must enter at least one path to Skype main.db file"
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
