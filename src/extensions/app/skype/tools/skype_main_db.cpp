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
    // Show accounts
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

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show contacts
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << std::endl;
    std::cout << "Contacts:" << std::endl;

    for (const auto &ct : dat.get_contacts ())
    {
        std::cout << std::endl;
        std::cout << "   Record Index: " << ct.idx << std::endl;
        std::cout << "   About: " << ct.about << std::endl;
        std::cout << "   Account Modification Serial Nr: " << ct.account_modification_serial_nr << std::endl;
        std::cout << "   Added In Shared Group: " << ct.added_in_shared_group << std::endl;
        std::cout << "   Alertstring: " << ct.alertstring << std::endl;
        std::cout << "   Aliases: " << ct.aliases << std::endl;
        std::cout << "   Assigned Comment: " << ct.assigned_comment << std::endl;
        std::cout << "   Assigned Phone1: " << ct.assigned_phone1 << std::endl;
        std::cout << "   Assigned Phone1 Label: " << ct.assigned_phone1_label << std::endl;
        std::cout << "   Assigned Phone2: " << ct.assigned_phone2 << std::endl;
        std::cout << "   Assigned Phone2 Label: " << ct.assigned_phone2_label << std::endl;
        std::cout << "   Assigned Phone3: " << ct.assigned_phone3 << std::endl;
        std::cout << "   Assigned Phone3 Label: " << ct.assigned_phone3_label << std::endl;
        std::cout << "   Assigned Speeddial: " << ct.assigned_speeddial << std::endl;
        std::cout << "   Authorization Certificate:\n" << ct.authorization_certificate.dump (8) << std::endl;
        std::cout << "   Authorized Time: " << ct.authorized_time << std::endl;
        std::cout << "   Authreq Crc: " << ct.authreq_crc << std::endl;
        std::cout << "   Authreq History:\n" << ct.authreq_history.dump (8) << std::endl;
        std::cout << "   Authreq Initmethod: " << ct.authreq_initmethod << std::endl;
        std::cout << "   Authreq Nodeinfo:\n" << ct.authreq_nodeinfo.dump (8) << std::endl;
        std::cout << "   Authreq Src: " << ct.authreq_src << std::endl;
        std::cout << "   Authreq Timestamp: " << ct.authreq_timestamp << std::endl;
        std::cout << "   Authrequest Count: " << ct.authrequest_count << std::endl;
        std::cout << "   Availability: " << ct.availability << std::endl;
        std::cout << "   Avatar Hiresurl: " << ct.avatar_hiresurl << std::endl;
        std::cout << "   Avatar Hiresurl New: " << ct.avatar_hiresurl_new << std::endl;
        std::cout << "   Avatar Image:\n" << ct.avatar_image.dump (8) << std::endl;
        std::cout << "   Avatar Timestamp: " << ct.avatar_timestamp << std::endl;
        std::cout << "   Avatar Url: " << ct.avatar_url << std::endl;
        std::cout << "   Avatar Url New: " << ct.avatar_url_new << std::endl;
        std::cout << "   Birthday: " << ct.birthday << std::endl;
        std::cout << "   Buddyblob:\n" << ct.buddyblob.dump (8) << std::endl;
        std::cout << "   Buddystatus: " << ct.buddystatus << std::endl;
        std::cout << "   Capabilities:\n" << ct.capabilities.dump (8) << std::endl;
        std::cout << "   Cbl Future:\n" << ct.cbl_future.dump (8) << std::endl;
        std::cout << "   Certificate Send Count: " << ct.certificate_send_count << std::endl;
        std::cout << "   City: " << ct.city << std::endl;
        std::cout << "   Contactlist Track: " << ct.contactlist_track << std::endl;
        std::cout << "   Country: " << ct.country << std::endl;
        std::cout << "   Dirblob Last Search Time: " << ct.dirblob_last_search_time << std::endl;
        std::cout << "   Displayname: " << ct.displayname << std::endl;
        std::cout << "   Emails: " << ct.emails << std::endl;
        std::cout << "   External Id: " << ct.external_id << std::endl;
        std::cout << "   External System Id: " << ct.external_system_id << std::endl;
        std::cout << "   Extprop External Data: " << ct.extprop_external_data << std::endl;
        std::cout << "   Extprop Must Hide Avatar: " << ct.extprop_must_hide_avatar << std::endl;
        std::cout << "   Extprop Seen Birthday: " << ct.extprop_seen_birthday << std::endl;
        std::cout << "   Extprop SMS PSTN Contact Created: " << ct.extprop_sms_pstn_contact_created << std::endl;
        std::cout << "   Extprop SMS Target: " << ct.extprop_sms_target << std::endl;
        std::cout << "   Firstname: " << ct.firstname << std::endl;
        std::cout << "   Fullname: " << ct.fullname << std::endl;
        std::cout << "   Gender: " << ct.gender << std::endl;
        std::cout << "   Given Authlevel: " << ct.given_authlevel << std::endl;
        std::cout << "   Given Displayname: " << ct.given_displayname << std::endl;
        std::cout << "   Group Membership: " << ct.group_membership << std::endl;
        std::cout << "   Hashed Emails: " << ct.hashed_emails << std::endl;
        std::cout << "   Homepage: " << ct.homepage << std::endl;
        std::cout << "   Id: " << ct.id << std::endl;
        std::cout << "   In Shared Group: " << ct.in_shared_group << std::endl;
        std::cout << "   Ipcountry: " << ct.ipcountry << std::endl;
        std::cout << "   Is Auto Buddy: " << (ct.is_auto_buddy ? "Yes" : "No") << std::endl;
        std::cout << "   Is Mobile: " << (ct.is_mobile ? "Yes" : "No") << std::endl;
        std::cout << "   Is Permanent: " << (ct.is_permanent ? "Yes" : "No") << std::endl;
        std::cout << "   Is Trusted: " << (ct.is_trusted ? "Yes" : "No") << std::endl;
        std::cout << "   Isauthorized: " << (ct.isauthorized ? "Yes" : "No") << std::endl;
        std::cout << "   Isblocked: " << (ct.isblocked ? "Yes" : "No") << std::endl;
        std::cout << "   Languages: " << ct.languages << std::endl;
        std::cout << "   Last Used Networktime: " << ct.last_used_networktime << std::endl;
        std::cout << "   Lastname: " << ct.lastname << std::endl;
        std::cout << "   Lastonline Timestamp: " << ct.lastonline_timestamp << std::endl;
        std::cout << "   Lastused Timestamp: " << ct.lastused_timestamp << std::endl;
        std::cout << "   Liveid Cid: " << ct.liveid_cid << std::endl;
        std::cout << "   Main Phone: " << ct.main_phone << std::endl;
        std::cout << "   Mood Text: " << ct.mood_text << std::endl;
        std::cout << "   Mood Timestamp: " << ct.mood_timestamp << std::endl;
        std::cout << "   Mutual Friend Count: " << ct.mutual_friend_count << std::endl;
        std::cout << "   Network Availability: " << ct.network_availability << std::endl;
        std::cout << "   Node Capabilities: " << ct.node_capabilities << std::endl;
        std::cout << "   Node Capabilities And: " << ct.node_capabilities_and << std::endl;
        std::cout << "   Nr Of Buddies: " << ct.nr_of_buddies << std::endl;
        std::cout << "   Nrof Authed Buddies: " << ct.nrof_authed_buddies << std::endl;
        std::cout << "   Offline Authreq Id: " << ct.offline_authreq_id << std::endl;
        std::cout << "   Phone Home: " << ct.phone_home << std::endl;
        std::cout << "   Phone Home Normalized: " << ct.phone_home_normalized << std::endl;
        std::cout << "   Phone Mobile: " << ct.phone_mobile << std::endl;
        std::cout << "   Phone Mobile Normalized: " << ct.phone_mobile_normalized << std::endl;
        std::cout << "   Phone Office: " << ct.phone_office << std::endl;
        std::cout << "   Phone Office Normalized: " << ct.phone_office_normalized << std::endl;
        std::cout << "   Pop Score: " << ct.pop_score << std::endl;
        std::cout << "   Popularity Ord: " << ct.popularity_ord << std::endl;
        std::cout << "   Profile Attachments:\n" << ct.profile_attachments.dump (8) << std::endl;
        std::cout << "   Profile Etag: " << ct.profile_etag << std::endl;
        std::cout << "   Profile Json: " << ct.profile_json << std::endl;
        std::cout << "   Profile Timestamp: " << ct.profile_timestamp << std::endl;
        std::cout << "   Province: " << ct.province << std::endl;
        std::cout << "   Pstnnumber: " << ct.pstnnumber << std::endl;
        std::cout << "   Received Authrequest: " << ct.received_authrequest << std::endl;
        std::cout << "   Refreshing: " << ct.refreshing << std::endl;
        std::cout << "   Revoked Auth: " << ct.revoked_auth << std::endl;
        std::cout << "   Rich Mood Text: " << ct.rich_mood_text << std::endl;
        std::cout << "   Saved Directory Blob:\n" << ct.saved_directory_blob.dump (8) << std::endl;
        std::cout << "   Sent Authrequest: " << ct.sent_authrequest << std::endl;
        std::cout << "   Sent Authrequest Extrasbitmask: " << ct.sent_authrequest_extrasbitmask << std::endl;
        std::cout << "   Sent Authrequest Initmethod: " << ct.sent_authrequest_initmethod << std::endl;
        std::cout << "   Sent Authrequest Serial: " << ct.sent_authrequest_serial << std::endl;
        std::cout << "   Sent Authrequest Time: " << ct.sent_authrequest_time << std::endl;
        std::cout << "   Server Synced: " << ct.server_synced << std::endl;
        std::cout << "   Skypename: " << ct.skypename << std::endl;
        std::cout << "   Stack Version: " << ct.stack_version << std::endl;
        std::cout << "   Timezone: " << ct.timezone << std::endl;
        std::cout << "   Type: " << ct.type << std::endl;
        std::cout << "   Unified Servants: " << ct.unified_servants << std::endl;
        std::cout << "   Verified Company:\n" << ct.verified_company.dump (8) << std::endl;
        std::cout << "   Verified Email:\n" << ct.verified_email.dump (8) << std::endl;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show file transfers
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << std::endl;
    std::cout << "File transfers:" << std::endl;

    for (const auto &ft : dat.get_file_transfers ())
    {
        std::cout << std::endl;
        std::cout << "   Record Index: " << ft.idx << std::endl;
        std::cout << "   Accepttime: " << ft.accepttime << std::endl;
        std::cout << "   Bytespersecond: " << ft.bytespersecond << std::endl;
        std::cout << "   Bytestransferred: " << ft.bytestransferred << std::endl;
        std::cout << "   Chatmsg Guid: " << ft.chatmsg_guid << std::endl;
        std::cout << "   Chatmsg Index: " << ft.chatmsg_index << std::endl;
        std::cout << "   Convo Id: " << ft.convo_id << std::endl;
        std::cout << "   Extprop Handled By Chat: " << ft.extprop_handled_by_chat << std::endl;
        std::cout << "   Extprop Hide From History: " << ft.extprop_hide_from_history << std::endl;
        std::cout << "   Extprop Localfilename: " << ft.extprop_localfilename << std::endl;
        std::cout << "   Extprop Window Visible: " << ft.extprop_window_visible << std::endl;
        std::cout << "   Failurereason: " << ft.failurereason << std::endl;
        std::cout << "   Filename: " << ft.filename << std::endl;
        std::cout << "   Filepath: " << ft.filepath << std::endl;
        std::cout << "   Filesize: " << ft.filesize << std::endl;
        std::cout << "   Finishtime: " << ft.finishtime << std::endl;
        std::cout << "   Flags: " << ft.flags << std::endl;
        std::cout << "   Id: " << ft.id << std::endl;
        std::cout << "   Is Permanent: " << (ft.is_permanent ? "Yes" : "No") <<  std::endl;
        std::cout << "   Last Activity: " << ft.last_activity << std::endl;
        std::cout << "   Nodeid:\n" << ft.nodeid.dump (8) << std::endl;
        std::cout << "   Offer Send List: "  << std::endl;
        std::cout << "   Partner Dispname: " << ft.partner_dispname << std::endl;
        std::cout << "   Partner Handle: " << ft.partner_handle << std::endl;
        std::cout << "   Pk Id: " << ft.pk_id << std::endl;
        std::cout << "   Starttime: " << ft.starttime << std::endl;
        std::cout << "   Status: " << ft.status << std::endl;
        std::cout << "   Type: " << ft.type << std::endl;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show voicemails
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << std::endl;
    std::cout << "Voicemails:" << std::endl;

    for (const auto &vm : dat.get_voicemails ())
    {
        std::cout << std::endl;
        std::cout << "   Record Index: " << vm.idx << std::endl;
        std::cout << "   Allowed Duration: " << vm.allowed_duration << std::endl;
        std::cout << "   Chatmsg Guid: " << vm.chatmsg_guid << std::endl;
        std::cout << "   Convo Id: " << vm.convo_id << std::endl;
        std::cout << "   Duration: " << vm.duration << std::endl;
        std::cout << "   Extprop Hide From History: " << vm.extprop_hide_from_history << std::endl;
        std::cout << "   Failurereason: " << vm.failurereason << std::endl;
        std::cout << "   Failures: " << vm.failures << std::endl;
        std::cout << "   Flags: " << vm.flags << std::endl;
        std::cout << "   Id: " << vm.id << std::endl;
        std::cout << "   Is Permanent: " << (vm.is_permanent ? "Yes" : "No") << std::endl;
        std::cout << "   Notification Id: " << vm.notification_id << std::endl;
        std::cout << "   Partner Dispname: " << vm.partner_dispname << std::endl;
        std::cout << "   Partner Handle: " << vm.partner_handle << std::endl;
        std::cout << "   Path: " << vm.path << std::endl;
        std::cout << "   Playback Progress: " << vm.playback_progress << std::endl;
        std::cout << "   Size: " << vm.size << std::endl;
        std::cout << "   Status: " << vm.status << std::endl;
        std::cout << "   Subject: " << vm.subject << std::endl;
        std::cout << "   Timestamp: " << vm.timestamp << std::endl;
        std::cout << "   Type: " << vm.type << std::endl;
        std::cout << "   Vflags: " << vm.vflags << std::endl;
        std::cout << "   Xmsg: " << vm.xmsg << std::endl;
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
