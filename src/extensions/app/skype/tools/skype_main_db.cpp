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

    struct account
    {
        // @brief Record Index
        std::uint64_t idx = 0;

        // @brief Schema version
        std::uint64_t schema_version = 0;

        // @brief About
        std::string about;

        // @brief Ad Policy
        std::int64_t ad_policy = 0;

        // @brief Added In Shared Group
        std::int64_t added_in_shared_group = 0;

        // @brief Alertstring
        std::string alertstring;

        // @brief Aliases
        std::string aliases;

        // @brief Assigned Comment
        std::string assigned_comment;

        // @brief Assigned Speeddial
        std::string assigned_speeddial;

        // @brief Authorized Time
        std::int64_t authorized_time = 0;

        // @brief Authreq History
        mobius::core::bytearray authreq_history;

        // @brief Authreq Timestamp
        std::int64_t authreq_timestamp = 0;

        // @brief Authrequest Count
        std::int64_t authrequest_count = 0;

        // @brief Authrequest Policy
        std::int64_t authrequest_policy = 0;

        // @brief Availability
        std::int64_t availability = 0;

        // @brief Avatar Image
        mobius::core::bytearray avatar_image;

        // @brief Avatar Policy
        std::int64_t avatar_policy = 0;

        // @brief Avatar Timestamp
        mobius::core::datetime::datetime avatar_timestamp;

        // @brief Birthday
        std::int64_t birthday = 0;

        // @brief Buddyblob
        mobius::core::bytearray buddyblob;

        // @brief Buddycount Policy
        std::int64_t buddycount_policy = 0;

        // @brief Capabilities
        mobius::core::bytearray capabilities;

        // @brief Cbl Future
        mobius::core::bytearray cbl_future;

        // @brief Cblsyncstatus
        std::int64_t cblsyncstatus = 0;

        // @brief Chat Policy
        std::int64_t chat_policy = 0;

        // @brief City
        std::string city;

        // @brief Cobrand Id
        std::int64_t cobrand_id = 0;

        // @brief Commitstatus
        std::int64_t commitstatus = 0;

        // @brief Country
        std::string country;

        // @brief Displayname
        std::string displayname;

        // @brief Emails
        std::string emails;

        // @brief Federated Presence Policy
        std::int64_t federated_presence_policy = 0;  // skype 6

        // @brief Flamingo Xmpp Status
        std::int64_t flamingo_xmpp_status = 0; // skype 6

        // @brief Fullname
        std::string fullname;

        // @brief Gender
        std::int64_t gender = 0;

        // @brief Given Authlevel
        std::int64_t given_authlevel = 0;

        // @brief Given Displayname
        std::string given_displayname;

        // @brief Homepage
        std::string homepage;

        // @brief Id
        std::int64_t id = 0;

        // @brief In Shared Group
        std::int64_t in_shared_group = 0;

        // @brief Ipcountry
        std::string ipcountry;

        // @brief Is Permanent
        std::int64_t is_permanent = 0;

        // @brief Languages
        std::string languages;

        // @brief Lastonline Timestamp
        mobius::core::datetime::datetime lastonline_timestamp;

        // @brief Lastused Timestamp
        mobius::core::datetime::datetime lastused_timestamp;

        // @brief Liveid Membername
        std::string liveid_membername;

        // @brief Logoutreason
        std::int64_t logoutreason = 0;

        // @brief Mood Text
        std::string mood_text;

        // @brief Mood Timestamp
        mobius::core::datetime::datetime mood_timestamp;

        // @brief Node Capabilities
        std::int64_t node_capabilities = 0;

        // @brief Node Capabilities And
        std::int64_t node_capabilities_and = 0;

        // @brief Nr Of Other Instances
        std::int64_t nr_of_other_instances = 0;

        // @brief Nrof Authed Buddies
        std::int64_t nrof_authed_buddies = 0;

        // @brief Offline Authreq Id
        std::int64_t offline_authreq_id = 0;

        // @brief Offline Callforward
        std::string offline_callforward;

        // @brief Options Change Future
        mobius::core::bytearray options_change_future;  // skype 6

        // @brief Owner Under Legal Age
        std::int64_t owner_under_legal_age = 0;

        // @brief Partner Channel Status
        std::string partner_channel_status;  // skype 6

        // @brief Partner Optedout
        std::string partner_optedout;

        // @brief Phone Home
        std::string phone_home;

        // @brief Phone Mobile
        std::string phone_mobile;

        // @brief Phone Office
        std::string phone_office;

        // @brief Phonenumbers Policy
        std::int64_t phonenumbers_policy = 0;

        // @brief Profile Attachments
        mobius::core::bytearray profile_attachments;

        // @brief Profile Timestamp
        mobius::core::datetime::datetime profile_timestamp;

        // @brief Province
        std::string province;

        // @brief Pstn Call Policy
        std::int64_t pstn_call_policy = 0;

        // @brief Pstnnumber
        std::string pstnnumber;

        // @brief Pwdchangestatus
        std::int64_t pwdchangestatus = 0;

        // @brief Received Authrequest
        std::string received_authrequest;

        // @brief Refreshing
        std::int64_t refreshing = 0;

        // @brief Registration Timestamp
        mobius::core::datetime::datetime registration_timestamp;

        // @brief Revoked Auth
        std::int64_t revoked_auth = 0;

        // @brief Rich Mood Text
        std::string rich_mood_text;

        // @brief Roaming History Enabled
        bool roaming_history_enabled = false;  // skype 6

        // @brief Sent Authrequest
        std::string sent_authrequest;

        // @brief Sent Authrequest Serial
        std::int64_t sent_authrequest_serial = 0;

        // @brief Sent Authrequest Time
        std::int64_t sent_authrequest_time = 0;

        // @brief Service Provider Info
        std::string service_provider_info;

        // @brief Set Availability
        std::int64_t set_availability = 0;

        // @brief Shortcircuit Sync
        std::int64_t shortcircuit_sync = 0;   // skype 6

        // @brief Skype Call Policy
        std::int64_t skype_call_policy = 0;

        // @brief Skypein Numbers
        std::string skypein_numbers;

        // @brief Skypename
        std::string skypename;

        // @brief Skypeout Balance
        std::int64_t skypeout_balance = 0;

        // @brief Skypeout Balance Currency
        std::string skypeout_balance_currency;

        // @brief Skypeout Precision
        std::int64_t skypeout_precision = 0;

        // @brief Stack Version
        std::int64_t stack_version = 0;

        // @brief Status
        std::int64_t status = 0;

        // @brief Subscriptions
        std::string subscriptions;

        // @brief Suggested Skypename
        std::string suggested_skypename;

        // @brief Synced Email
        mobius::core::bytearray synced_email;

        // @brief Timezone
        std::int64_t timezone = 0;

        // @brief Timezone Policy
        std::int64_t timezone_policy = 0;

        // @brief Type
        std::int64_t type = 0;

        // @brief Uses Jcs
        std::int64_t uses_jcs = 0;  // skype 6

        // @brief Verified Company
        mobius::core::bytearray verified_company;  // skype 6

        // @brief Verified Email
        mobius::core::bytearray verified_email;  // skype 6

        // @brief Voicemail Policy
        std::int64_t voicemail_policy = 0;

        // @brief Webpresence Policy
        std::int64_t webpresence_policy = 0;
    };

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
