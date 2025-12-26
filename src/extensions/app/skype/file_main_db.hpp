#ifndef MOBIUS_EXTENSION_APP_SKYPE_FILE_MAIN_DB_HPP
#define MOBIUS_EXTENSION_APP_SKYPE_FILE_MAIN_DB_HPP

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
#include <mobius/core/database/database.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/reader.hpp>
#include <mobius/core/pod/map.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace mobius::extension::app::skype
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief main.db file decoder
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_main_db
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Account structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct account
    {
        // @brief Record Index
        std::uint64_t idx = 0;

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
        mobius::core::datetime::datetime authorized_time;

        // @brief Authreq History
        mobius::core::bytearray authreq_history;

        // @brief Authreq Timestamp
        mobius::core::datetime::datetime authreq_timestamp;

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
        std::string birthday;

        // @brief Buddyblob
        mobius::core::bytearray buddyblob;

        // @brief Buddycount Policy
        std::int64_t buddycount_policy = 0;

        // @brief Capabilities
        mobius::core::bytearray capabilities;

        // @brief Cbl Future
        mobius::core::bytearray cbl_future;

        // @brief Cbl Profile Blob
        mobius::core::bytearray cbl_profile_blob;

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

        // @brief Contactssyncstatus
        std::int64_t contactssyncstatus = 0;

        // @brief Country
        std::string country;

        // @brief Displayname
        std::string displayname;

        // @brief Emails
        std::string emails;

        // @brief Federated Presence Policy
        std::int64_t federated_presence_policy = 0; // skype 6

        // @brief Forward Starttime
        mobius::core::datetime::datetime forward_starttime;

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

        // @brief Hidden Expression Tabs
        std::string hidden_expression_tabs;

        // @brief Homepage
        std::string homepage;

        // @brief Id
        std::int64_t id = 0;

        // @brief In Shared Group
        std::int64_t in_shared_group = 0;

        // @brief Ipcountry
        std::string ipcountry;

        // @brief Is Permanent
        bool is_permanent = false;

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

        // @brief Msa Pmn
        std::string msa_pmn;

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

        // @brief Option Ui Color
        std::string option_ui_color;

        // @brief Options Change Future
        mobius::core::bytearray options_change_future; // skype 6

        // @brief Owner Under Legal Age
        std::int64_t owner_under_legal_age = 0;

        // @brief Partner Channel Status
        std::string partner_channel_status; // skype 6

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

        // @brief Read Receipt Optout
        std::int64_t read_receipt_optout;

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
        bool roaming_history_enabled = false; // skype 6

        // @brief Sent Authrequest
        std::string sent_authrequest;

        // @brief Sent Authrequest Serial
        std::int64_t sent_authrequest_serial = 0;

        // @brief Sent Authrequest Time
        mobius::core::datetime::datetime sent_authrequest_time;

        // @brief Service Provider Info
        std::string service_provider_info;

        // @brief Set Availability
        std::int64_t set_availability = 0;

        // @brief Shortcircuit Sync
        std::int64_t shortcircuit_sync = 0; // skype 6

        // @brief Signin Name
        std::string signin_name;

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
        std::string timezone;

        // @brief Timezone Policy
        std::int64_t timezone_policy = 0;

        // @brief Type
        std::int64_t type = 0;

        // @brief Uses Jcs
        std::int64_t uses_jcs = 0; // skype 6

        // @brief Verified Company
        mobius::core::bytearray verified_company; // skype 6

        // @brief Verified Email
        mobius::core::bytearray verified_email; // skype 6

        // @brief Voicemail Policy
        std::int64_t voicemail_policy = 0;

        // @brief Webpresence Policy
        std::int64_t webpresence_policy = 0;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief call_members structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct call_member
    {
        // @brief Record index number
        std::uint64_t idx = 0;

        // @brief Accepted By
        std::string accepted_by;

        // @brief Admit Failure Reason
        std::int64_t admit_failure_reason;

        // @brief Balance Update
        std::string balance_update;

        // @brief Call Db Id
        std::int64_t call_db_id;

        // @brief Call Duration
        std::int64_t call_duration;

        // @brief Call End Diagnostics Code
        std::string call_end_diagnostics_code;

        // @brief Call Name
        std::string call_name;

        // @brief Call Session Guid
        std::string call_session_guid;

        // @brief Capabilities
        std::int64_t capabilities;

        // @brief Content Sharing Role
        std::int64_t content_sharing_role;

        // @brief Country
        std::string country;

        // @brief Creation Timestamp
        mobius::core::datetime::datetime creation_timestamp;

        // @brief Debuginfo
        std::string debuginfo;

        // @brief Dispname
        std::string dispname;

        // @brief Dominant Speaker Rank
        std::int64_t dominant_speaker_rank;

        // @brief Endpoint Details
        std::string endpoint_details;

        // @brief Endpoint Type
        std::int64_t endpoint_type;

        // @brief Failurereason
        std::int64_t failurereason;

        // @brief Fallback In Progress
        std::int64_t fallback_in_progress;

        // @brief Forward Targets
        std::string forward_targets;

        // @brief Forwarded By
        std::string forwarded_by;

        // @brief Group Calling Capabilities
        std::int64_t group_calling_capabilities;

        // @brief Guid
        std::string guid;

        // @brief Id
        std::int64_t id;

        // @brief Identity
        std::string identity;

        // @brief Identity Type
        std::int64_t identity_type;

        // @brief Ip Address
        std::string ip_address;

        // @brief Is Active Speaker
        bool is_active_speaker = false;

        // @brief Is Conference
        bool is_conference = false;

        // @brief Is Multiparty Video Capable
        bool is_multiparty_video_capable = false;

        // @brief Is Permanent
        bool is_permanent = false;

        // @brief Is Premium Video Sponsor
        bool is_premium_video_sponsor = false;

        // @brief Is Read Only
        bool is_read_only = false;

        // @brief Is Seamlessly Upgraded Call
        bool is_seamlessly_upgraded_call;

        // @brief Is Server Muted
        bool is_server_muted = false;

        // @brief Is Video Codec Compatible
        bool is_video_codec_compatible = false;

        // @brief Languages
        std::string languages;

        // @brief Light Weight Meeting Role
        std::int64_t light_weight_meeting_role;

        // @brief Limiting Factor
        std::int64_t limiting_factor;

        // @brief Mike Status
        std::int64_t mike_status;

        // @brief Mri Identity
        std::string mri_identity;

        // @brief Next Redial Time
        std::int64_t next_redial_time;

        // @brief Nonse Word
        std::string nonse_word;

        // @brief Nr Of Delivered Push Notifications
        std::int64_t nr_of_delivered_push_notifications;

        // @brief Nrof Redials Done
        std::int64_t nrof_redials_done;

        // @brief Nrof Redials Left
        std::int64_t nrof_redials_left;

        // @brief Participant Sponsor
        std::string participant_sponsor;

        // @brief Payment Category
        std::string payment_category;

        // @brief Pk Status
        std::int64_t pk_status;

        // @brief Price Currency
        std::string price_currency;

        // @brief Price Per Minute
        std::int64_t price_per_minute;

        // @brief Price Precision
        std::int64_t price_precision;

        // @brief Prime Status
        std::int64_t prime_status;

        // @brief Pstn Feedback
        std::string pstn_feedback;

        // @brief Pstn Statustext
        std::string pstn_statustext;

        // @brief Quality Problems
        std::string quality_problems;

        // @brief Quality Status
        std::int64_t quality_status;

        // @brief Real Identity
        std::string real_identity;

        // @brief Recovery In Progress
        std::int64_t recovery_in_progress;

        // @brief Role
        std::string role;

        // @brief Seconds Left
        std::int64_t seconds_left;

        // @brief Sounderror Code
        std::int64_t sounderror_code;

        // @brief Soundlevel
        std::int64_t soundlevel;

        // @brief Start Timestamp
        mobius::core::datetime::datetime start_timestamp;

        // @brief Stats Xml
        std::string stats_xml;

        // @brief Status
        std::int64_t status;

        // @brief Target Identity
        std::string target_identity;

        // @brief Tenant Id
        std::string tenant_id;

        // @brief Transfer Active
        std::int64_t transfer_active;

        // @brief Transfer Status
        std::int64_t transfer_status;

        // @brief Transfer Topic
        std::string transfer_topic;

        // @brief Transferred By
        std::string transferred_by;

        // @brief Transferred To
        std::string transferred_to;

        // @brief Type
        std::int64_t type;

        // @brief Version String
        std::string version_string;

        // @brief Video Count Changed
        std::int64_t video_count_changed;

        // @brief Videostatus
        std::int64_t videostatus;

        // @brief Voicechannel
        std::int64_t voicechannel;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Phone call structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct call
    {
        // @brief Record index number
        std::uint64_t idx = 0;

        // @brief Access Token
        std::string access_token;

        // @brief Active Members
        std::int64_t active_members;

        // @brief Begin Timestamp
        mobius::core::datetime::datetime begin_timestamp;

        // @brief Broadcast Metadata
        std::string broadcast_metadata;

        // @brief Caller Mri Identity
        std::string caller_mri_identity;

        // @brief Conf Participants
        mobius::core::bytearray conf_participants;

        // @brief Content Sharing Session Count Changed
        std::int64_t content_sharing_session_count_changed;

        // @brief Conv Dbid
        std::int64_t conv_dbid;

        // @brief Conversation Type
        std::string conversation_type;

        // @brief Current Video Audience
        std::string current_video_audience;

        // @brief Datachannel Object Id
        std::int64_t datachannel_object_id;

        // @brief Duration
        std::int64_t duration;

        // @brief Endpoint Details
        std::string endpoint_details;

        // @brief Failurecode
        std::int64_t failurecode;

        // @brief Failurereason
        std::int64_t failurereason;

        // @brief Forwarding Destination Type
        std::string forwarding_destination_type;

        // @brief Host Identity
        std::string host_identity;

        // @brief Id
        std::int64_t id;

        // @brief Incoming Type
        std::string incoming_type;

        // @brief Is Active
        bool is_active = false;

        // @brief Is Conference
        bool is_conference = false;

        // @brief Is Hostless
        bool is_hostless = false;

        // @brief Is Incoming
        bool is_incoming = false;

        // @brief Is Incoming One On One Video Call
        bool is_incoming_one_on_one_video_call = false;

        // @brief Is Muted
        bool is_muted = false;

        // @brief Is Muted Speaker
        bool is_muted_speaker = false;

        // @brief Is On Hold
        bool is_on_hold = false;

        // @brief Is Permanent
        bool is_permanent = false;

        // @brief Is Premium Video Sponsor
        bool is_premium_video_sponsor = false;

        // @brief Is Server Muted
        bool is_server_muted = false;

        // @brief Is Unseen Missed
        bool is_unseen_missed = false;

        // @brief Joined Existing
        std::int64_t joined_existing;

        // @brief Leg Id
        std::string leg_id;

        // @brief Light Weight Meeting Count Changed
        std::int64_t light_weight_meeting_count_changed;

        // @brief Max Videoconfcall Participants
        std::int64_t max_videoconfcall_participants;

        // @brief Meeting Details
        std::string meeting_details;

        // @brief Member Count Changed
        std::int64_t member_count_changed;

        // @brief Members
        mobius::core::bytearray members;

        // @brief Message Id
        std::string message_id;

        // @brief Mike Status
        std::int64_t mike_status;

        // @brief Name
        std::string name;

        // @brief Old Duration
        std::int64_t old_duration;

        // @brief Old Members
        mobius::core::bytearray old_members;

        // @brief Onbehalfof Mri
        std::string onbehalfof_mri;

        // @brief Optimal Remote Videos In Conference
        std::int64_t optimal_remote_videos_in_conference;

        // @brief Partner Dispname
        std::string partner_dispname;

        // @brief Partner Handle
        std::string partner_handle;

        // @brief Premium Video Is Grace Period
        std::int64_t premium_video_is_grace_period;

        // @brief Premium Video Sponsor List
        std::string premium_video_sponsor_list;

        // @brief Premium Video Status
        std::int64_t premium_video_status;

        // @brief Pstn Number
        std::string pstn_number;

        // @brief Pstn Status
        std::string pstn_status;

        // @brief Quality Problems
        std::string quality_problems;

        // @brief Queue Info
        std::string queue_info;

        // @brief Role
        std::string role;

        // @brief Server Identity
        std::string server_identity;

        // @brief Soundlevel
        std::int64_t soundlevel;

        // @brief Start Timestamp
        mobius::core::datetime::datetime start_timestamp;

        // @brief Status
        std::int64_t status;

        // @brief Technology
        std::int64_t technology;

        // @brief Tenant Id
        std::string tenant_id;

        // @brief Thread Id
        std::string thread_id;

        // @brief Topic
        std::string topic;

        // @brief Transfer Failure Reason
        std::int64_t transfer_failure_reason;

        // @brief Transfer Status
        std::int64_t transfer_status;

        // @brief Transferor Displayname
        std::string transferor_displayname;

        // @brief Transferor Mri
        std::string transferor_mri;

        // @brief Transferor Type
        std::string transferor_type;

        // @brief Type
        std::int64_t type;

        // @brief Vaa Input Status
        std::int64_t vaa_input_status;

        // @brief Video Disabled
        std::int64_t video_disabled;

        // @brief Members
        std::vector<call_member> call_members;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Contact structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct contact
    {
        // @brief Record index number
        std::uint64_t idx = 0;

        // @brief About
        std::string about;

        // @brief Account Modification Serial Nr
        std::int64_t account_modification_serial_nr;

        // @brief Added In Shared Group
        std::int64_t added_in_shared_group;

        // @brief Alertstring
        std::string alertstring;

        // @brief Aliases
        std::string aliases;

        // @brief Assigned Comment
        std::string assigned_comment;

        // @brief Assigned Phone1
        std::string assigned_phone1;

        // @brief Assigned Phone1 Label
        std::string assigned_phone1_label;

        // @brief Assigned Phone2
        std::string assigned_phone2;

        // @brief Assigned Phone2 Label
        std::string assigned_phone2_label;

        // @brief Assigned Phone3
        std::string assigned_phone3;

        // @brief Assigned Phone3 Label
        std::string assigned_phone3_label;

        // @brief Assigned Speeddial
        std::string assigned_speeddial;

        // @brief Authorization Certificate
        mobius::core::bytearray authorization_certificate;

        // @brief Authorized Time
        std::int64_t authorized_time;

        // @brief Authreq Crc
        std::int64_t authreq_crc;

        // @brief Authreq History
        mobius::core::bytearray authreq_history;

        // @brief Authreq Initmethod
        std::int64_t authreq_initmethod;

        // @brief Authreq Nodeinfo
        mobius::core::bytearray authreq_nodeinfo;

        // @brief Authreq Src
        std::int64_t authreq_src;

        // @brief Authreq Timestamp
        std::int64_t authreq_timestamp;

        // @brief Authrequest Count
        std::int64_t authrequest_count;

        // @brief Availability
        std::int64_t availability;

        // @brief Avatar Hiresurl
        std::string avatar_hiresurl;

        // @brief Avatar Hiresurl New
        std::string avatar_hiresurl_new;

        // @brief Avatar Image
        mobius::core::bytearray avatar_image;

        // @brief Avatar Timestamp
        mobius::core::datetime::datetime avatar_timestamp;

        // @brief Avatar Url
        std::string avatar_url;

        // @brief Avatar Url New
        std::string avatar_url_new;

        // @brief Birthday
        std::string birthday;

        // @brief Buddyblob
        mobius::core::bytearray buddyblob;

        // @brief Buddystatus
        std::int64_t buddystatus;

        // @brief Capabilities
        mobius::core::bytearray capabilities;

        // @brief Cbl Future
        mobius::core::bytearray cbl_future;

        // @brief Certificate Send Count
        std::int64_t certificate_send_count;

        // @brief City
        std::string city;

        // @brief Contactlist Track
        std::int64_t contactlist_track;

        // @brief Country
        std::string country;

        // @brief Dirblob Last Search Time
        mobius::core::datetime::datetime dirblob_last_search_time;

        // @brief Displayname
        std::string displayname;

        // @brief Emails
        std::string emails;

        // @brief External Id
        std::string external_id;

        // @brief External System Id
        std::string external_system_id;

        // @brief Extprop Can Show Avatar
        std::int64_t extprop_can_show_avatar;

        // @brief Extprop Contact Ab Uuid
        std::string extprop_contact_ab_uuid;

        // @brief Extprop External Data
        std::string extprop_external_data;

        // @brief Extprop Last Sms Number
        std::string extprop_last_sms_number;

        // @brief Extprop Must Hide Avatar
        std::int64_t extprop_must_hide_avatar;

        // @brief Extprop Seen Birthday
        std::int64_t extprop_seen_birthday;

        // @brief Extprop Sms Pstn Contact Created
        std::int64_t extprop_sms_pstn_contact_created;

        // @brief Extprop Sms Target
        std::int64_t extprop_sms_target;

        // @brief Extprop Viral Upgrade Campaign Id
        std::int64_t extprop_viral_upgrade_campaign_id;

        // @brief Firstname
        std::string firstname;

        // @brief Fullname
        std::string fullname;

        // @brief Gender
        std::int64_t gender;

        // @brief Given Authlevel
        std::int64_t given_authlevel;

        // @brief Given Displayname
        std::string given_displayname;

        // @brief Group Membership
        std::int64_t group_membership;

        // @brief Hashed Emails
        std::string hashed_emails;

        // @brief Homepage
        std::string homepage;

        // @brief Id
        std::int64_t id;

        // @brief In Shared Group
        std::int64_t in_shared_group;

        // @brief Ipcountry
        std::string ipcountry;

        // @brief Is Auto Buddy
        std::int64_t is_auto_buddy;

        // @brief Is Mobile
        std::int64_t is_mobile;

        // @brief Is Permanent
        std::int64_t is_permanent;

        // @brief Is Trusted
        std::int64_t is_trusted;

        // @brief Isauthorized
        std::int64_t isauthorized;

        // @brief Isblocked
        std::int64_t isblocked;

        // @brief Languages
        std::string languages;

        // @brief Last Used Networktime
        mobius::core::datetime::datetime last_used_networktime;

        // @brief Lastname
        std::string lastname;

        // @brief Lastonline Timestamp
        mobius::core::datetime::datetime lastonline_timestamp;

        // @brief Lastused Timestamp
        mobius::core::datetime::datetime lastused_timestamp;

        // @brief Liveid Cid
        std::string liveid_cid;

        // @brief Main Phone
        std::string main_phone;

        // @brief Mood Text
        std::string mood_text;

        // @brief Mood Timestamp
        mobius::core::datetime::datetime mood_timestamp;

        // @brief Mutual Friend Count
        std::int64_t mutual_friend_count;

        // @brief Network Availability
        std::int64_t network_availability;

        // @brief Node Capabilities
        std::int64_t node_capabilities;

        // @brief Node Capabilities And
        std::int64_t node_capabilities_and;

        // @brief Nr Of Buddies
        std::int64_t nr_of_buddies;

        // @brief Nrof Authed Buddies
        std::int64_t nrof_authed_buddies;

        // @brief Offline Authreq Id
        std::int64_t offline_authreq_id;

        // @brief Phone Home
        std::string phone_home;

        // @brief Phone Home Normalized
        std::string phone_home_normalized;

        // @brief Phone Mobile
        std::string phone_mobile;

        // @brief Phone Mobile Normalized
        std::string phone_mobile_normalized;

        // @brief Phone Office
        std::string phone_office;

        // @brief Phone Office Normalized
        std::string phone_office_normalized;

        // @brief Pop Score
        std::int64_t pop_score;

        // @brief Popularity Ord
        std::int64_t popularity_ord;

        // @brief Profile Attachments
        mobius::core::bytearray profile_attachments;

        // @brief Profile Etag
        std::string profile_etag;

        // @brief Profile Json
        std::string profile_json;

        // @brief Profile Timestamp
        mobius::core::datetime::datetime profile_timestamp;

        // @brief Province
        std::string province;

        // @brief Pstnnumber
        std::string pstnnumber;

        // @brief Received Authrequest
        std::string received_authrequest;

        // @brief Refreshing
        std::int64_t refreshing;

        // @brief Revoked Auth
        std::int64_t revoked_auth;

        // @brief Rich Mood Text
        std::string rich_mood_text;

        // @brief Saved Directory Blob
        mobius::core::bytearray saved_directory_blob;

        // @brief Sent Authrequest
        std::string sent_authrequest;

        // @brief Sent Authrequest Extrasbitmask
        std::int64_t sent_authrequest_extrasbitmask;

        // @brief Sent Authrequest Initmethod
        std::int64_t sent_authrequest_initmethod;

        // @brief Sent Authrequest Serial
        std::int64_t sent_authrequest_serial;

        // @brief Sent Authrequest Time
        mobius::core::datetime::datetime sent_authrequest_time;

        // @brief Server Synced
        std::int64_t server_synced;

        // @brief Skypename
        std::string skypename;

        // @brief Stack Version
        std::int64_t stack_version;

        // @brief Timezone
        std::string timezone;

        // @brief Type
        std::int64_t type;

        // @brief Unified Servants
        std::string unified_servants;

        // @brief Verified Company
        mobius::core::bytearray verified_company;

        // @brief Verified Email
        mobius::core::bytearray verified_email;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief File transfer structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct file_transfer
    {
        // @brief Record index number
        std::uint64_t idx = 0;

        // @brief Accepttime
        mobius::core::datetime::datetime accepttime;

        // @brief Bytespersecond
        std::int64_t bytespersecond;

        // @brief Bytestransferred
        std::string bytestransferred;

        // @brief Chatmsg Guid
        std::string chatmsg_guid;

        // @brief Chatmsg Index
        std::int64_t chatmsg_index;

        // @brief Convo Id
        std::int64_t convo_id;

        // @brief Extprop Handled By Chat
        std::int64_t extprop_handled_by_chat;

        // @brief Extprop Hide From History
        std::int64_t extprop_hide_from_history;

        // @brief Extprop Localfilename
        std::string extprop_localfilename;

        // @brief Extprop Transfer Alias
        mobius::core::bytearray extprop_transfer_alias;

        // @brief Extprop Window Visible
        std::int64_t extprop_window_visible;

        // @brief Failurereason
        std::int64_t failurereason;

        // @brief Filename
        std::string filename;

        // @brief Filepath
        std::string filepath;

        // @brief Filesize
        std::string filesize;

        // @brief Finishtime
        mobius::core::datetime::datetime finishtime;

        // @brief Flags
        std::int64_t flags;

        // @brief Id
        std::int64_t id;

        // @brief Is Permanent
        bool is_permanent;

        // @brief Last Activity
        std::int64_t last_activity;

        // @brief Nodeid
        mobius::core::bytearray nodeid;

        // @brief Offer Send List
        std::string offer_send_list;

        // @brief Old Filepath
        std::int64_t old_filepath;

        // @brief Old Status
        std::int64_t old_status;

        // @brief Parent Id
        std::int64_t parent_id;

        // @brief Partner Dispname
        std::string partner_dispname;

        // @brief Partner Handle
        std::string partner_handle;

        // @brief Pk Id
        std::int64_t pk_id;

        // @brief Starttime
        mobius::core::datetime::datetime starttime;

        // @brief Status
        std::int64_t status;

        // @brief Type
        std::int64_t type;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Chat messsage participant
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct message_participant
    {
        // @brief Record index number
        std::uint64_t idx = 0;

        // @brief Adder
        std::string adder;

        // @brief Adding In Progress Since
        std::int64_t adding_in_progress_since;

        // @brief Convo Id
        std::int64_t convo_id = 0;

        // @brief Debuginfo
        std::string debuginfo;

        // @brief Dominant Speaker Rank
        std::int64_t dominant_speaker_rank;

        // @brief Endpoint Details
        std::string endpoint_details;

        // @brief Extprop Default Identity
        std::int64_t extprop_default_identity;

        // @brief Extprop Identity To Use
        std::int64_t extprop_identity_to_use;

        // @brief Group Calling Capabilities
        std::int64_t group_calling_capabilities;

        // @brief Id
        std::int64_t id = 0;

        // @brief Identity
        std::string identity;

        // @brief Is Active Speaker
        bool is_active_speaker = false;

        // @brief Is Multiparty Video Capable
        bool is_multiparty_video_capable = false;

        // @brief Is Multiparty Video Updatable
        bool is_multiparty_video_updatable = false;

        // @brief Is Permanent
        bool is_permanent = false;

        // @brief Is Premium Video Sponsor
        bool is_premium_video_sponsor = false;

        // @brief Is Seamlessly Upgraded Call
        bool is_seamlessly_upgraded_call = false;

        // @brief Is Video Codec Compatible
        bool is_video_codec_compatible = false;

        // @brief Last Leavereason
        std::int64_t last_leavereason;

        // @brief Last Voice Error
        std::string last_voice_error;

        // @brief Live Country
        std::string live_country;

        // @brief Live Fwd Identities
        std::string live_fwd_identities;

        // @brief Live Identity
        std::string live_identity;

        // @brief Live Identity To Use
        std::string live_identity_to_use;

        // @brief Live Ip Address
        std::string live_ip_address;

        // @brief Live Price For Me
        std::string live_price_for_me;

        // @brief Live Start Timestamp
        std::int64_t live_start_timestamp;

        // @brief Live Type
        std::int64_t live_type;

        // @brief Live Voicechannel
        std::int64_t live_voicechannel;

        // @brief Livesession Fallback In Progress
        std::int64_t livesession_fallback_in_progress;

        // @brief Livesession Recovery In Progress
        std::int64_t livesession_recovery_in_progress;

        // @brief Messaging Mode
        std::int64_t messaging_mode;

        // @brief Next Redial Time
        std::int64_t next_redial_time;

        // @brief Nrof Redials Left
        std::int64_t nrof_redials_left;

        // @brief Quality Problems
        std::string quality_problems;

        // @brief Rank
        std::int64_t rank;

        // @brief Read Horizon
        std::int64_t read_horizon;

        // @brief Real Identity
        std::string real_identity;

        // @brief Requested Rank
        std::int64_t requested_rank;

        // @brief Sound Level
        std::int64_t sound_level;

        // @brief Sponsor
        std::string sponsor;

        // @brief Text Status
        std::int64_t text_status;

        // @brief Transferred By
        std::string transferred_by;

        // @brief Transferred To
        std::string transferred_to;

        // @brief Video Status
        std::int64_t video_status;

        // @brief Voice Status
        std::int64_t voice_status;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Chat message
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct message
    {
        // @brief Record index number
        std::uint64_t idx = 0;

        // @brief Annotation Version
        std::int64_t annotation_version;

        // @brief Author
        std::string author;

        // @brief Author Was Live
        std::int64_t author_was_live;

        // @brief Body Is Rawxml
        std::int64_t body_is_rawxml;

        // @brief Body Xml
        std::string body_xml;

        // @brief Bots Settings
        std::string bots_settings;

        // @brief Call Guid
        std::string call_guid;

        // @brief Chatmsg Status
        std::int64_t chatmsg_status;

        // @brief Chatmsg Type
        std::int64_t chatmsg_type;

        // @brief Chatname
        std::string chatname;

        // @brief Consumption Status
        std::int64_t consumption_status;

        // @brief Content Flags
        std::int64_t content_flags;

        // @brief Convo Id
        std::int64_t convo_id;

        // @brief Crc
        std::int64_t crc;

        // @brief Dialog Partner
        std::string dialog_partner;

        // @brief Edited By
        std::string edited_by;

        // @brief Edited Timestamp
        mobius::core::datetime::datetime edited_timestamp;

        // @brief Error Code
        std::int64_t error_code;

        // @brief Extprop Chatmsg Ft Index Timestamp
        mobius::core::datetime::datetime extprop_chatmsg_ft_index_timestamp;

        // @brief Extprop Chatmsg Is Pending
        std::int64_t extprop_chatmsg_is_pending;

        // @brief Extprop Contact Received Stamp
        std::int64_t extprop_contact_received_stamp;

        // @brief Extprop Contact Review Date
        std::string extprop_contact_review_date;

        // @brief Extprop Contact Reviewed
        std::int64_t extprop_contact_reviewed;

        // @brief Extprop Mms Msg Metadata
        std::string extprop_mms_msg_metadata;

        // @brief Extprop Sms Server Id
        std::string extprop_sms_server_id;

        // @brief Extprop Sms Src Msg Id
        std::string extprop_sms_src_msg_id;

        // @brief Extprop Sms Sync Global Id
        std::string extprop_sms_sync_global_id;

        // @brief From Dispname
        std::string from_dispname;

        // @brief Guid
        mobius::core::bytearray guid;

        // @brief Id
        std::int64_t id;

        // @brief Identities
        std::string identities;

        // @brief Is Permanent
        bool is_permanent = false;

        // @brief Language
        std::string language;

        // @brief Leavereason
        std::int64_t leavereason;

        // @brief Newoptions
        std::int64_t newoptions;

        // @brief Newrole
        std::int64_t newrole;

        // @brief Oldoptions
        std::int64_t oldoptions;

        // @brief Option Bits
        std::int64_t option_bits;

        // @brief Param Key
        std::int64_t param_key;

        // @brief Param Value
        std::int64_t param_value;

        // @brief Participant Count
        std::int64_t participant_count;

        // @brief Pk Id
        std::int64_t pk_id;

        // @brief Reaction Thread
        std::string reaction_thread;

        // @brief Reason
        std::string reason;

        // @brief Remote Id
        std::int64_t remote_id;

        // @brief Sending Status
        std::int64_t sending_status;

        // @brief Server Id
        std::int64_t server_id;

        // @brief Timestamp
        mobius::core::datetime::datetime timestamp;

        // @brief Timestamp  Ms
        std::int64_t timestamp__ms;

        // @brief Type
        std::int64_t type;

        // @brief Parsed content
        std::vector<mobius::core::pod::map> content;

        // @brief Message Participants
        std::vector<message_participant> participants;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief SMS structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct sms
    {
        // @brief Record index number
        std::uint64_t idx = 0;

        // @brief Body
        std::string body;

        // @brief Chatmsg Id
        std::int64_t chatmsg_id;

        // @brief Convo Name
        std::string convo_name;

        // @brief Error Category
        std::int64_t error_category;

        // @brief Event Flags
        std::int64_t event_flags;

        // @brief Extprop Extended
        std::int64_t extprop_extended;

        // @brief Extprop Hide From History
        std::int64_t extprop_hide_from_history;

        // @brief Failurereason
        std::int64_t failurereason;

        // @brief Id
        std::int64_t id;

        // @brief Identity
        std::string identity;

        // @brief Is Failed Unseen
        bool is_failed_unseen;

        // @brief Is Permanent
        bool is_permanent;

        // @brief Notification Id
        std::int64_t notification_id;

        // @brief Outgoing Reply Type
        std::int64_t outgoing_reply_type;

        // @brief Price
        std::int64_t price;

        // @brief Price Currency
        std::string price_currency;

        // @brief Price Precision
        std::int64_t price_precision;

        // @brief Reply Id Number
        std::string reply_id_number;

        // @brief Reply To Number
        std::string reply_to_number;

        // @brief Status
        std::int64_t status;

        // @brief Target Numbers
        std::string target_numbers;

        // @brief Target Statuses
        mobius::core::bytearray target_statuses;

        // @brief Timestamp
        mobius::core::datetime::datetime timestamp;

        // @brief Type
        std::int64_t type;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Voicemail structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct voicemail
    {
        // @brief Record index number
        std::uint64_t idx = 0;

        // @brief Allowed Duration
        std::int64_t allowed_duration;

        // @brief Chatmsg Guid
        std::string chatmsg_guid;

        // @brief Convo Id
        std::int64_t convo_id;

        // @brief Duration
        std::int64_t duration;

        // @brief Extprop Hide From History
        std::int64_t extprop_hide_from_history;

        // @brief Failurereason
        std::int64_t failurereason;

        // @brief Failures
        std::int64_t failures;

        // @brief Flags
        std::int64_t flags;

        // @brief Id
        std::int64_t id;

        // @brief Is Permanent
        bool is_permanent;

        // @brief Notification Id
        std::int64_t notification_id;

        // @brief Partner Dispname
        std::string partner_dispname;

        // @brief Partner Handle
        std::string partner_handle;

        // @brief Path
        std::string path;

        // @brief Playback Progress
        std::int64_t playback_progress;

        // @brief Size
        std::int64_t size;

        // @brief Status
        std::int64_t status;

        // @brief Subject
        std::string subject;

        // @brief Timestamp
        mobius::core::datetime::datetime timestamp;

        // @brief Type
        std::int64_t type;

        // @brief Vflags
        std::int64_t vflags;

        // @brief Xmsg
        std::string xmsg;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_main_db (const mobius::core::io::reader &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if stream is an instance of main.db file
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const noexcept
    {
        return is_instance_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get schema version
    // @return Schema version
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_schema_version () const
    {
        return schema_version_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get accounts
    // @return Vector of accounts
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<account>
    get_accounts () const
    {
        return accounts_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get calls
    // @return Vector of calls
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<call>
    get_calls () const
    {
        return calls_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get contacts
    // @return Vector of contacts
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<contact>
    get_contacts () const
    {
        return contacts_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get file transfers
    // @return Vector of file transfers
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<file_transfer>
    get_file_transfers () const
    {
        return file_transfers_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get messages
    // @return Vector of messages
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<message>
    get_messages () const
    {
        return messages_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get SMS messages
    // @return Vector of SMS messages
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<sms>
    get_sms_messages () const
    {
        return sms_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get voicemails
    // @return Vector of voicemails
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<voicemail>
    get_voicemails () const
    {
        return voicemails_;
    }

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

    // @brief Schema version
    std::uint32_t schema_version_ = 0;

    // @brief Accounts
    std::vector<account> accounts_;

    // @brief Calls
    std::vector<call> calls_;

    // @brief Contacts
    std::vector<contact> contacts_;

    // @brief File Transfers
    std::vector<file_transfer> file_transfers_;

    // @brief Messages
    std::vector<message> messages_;

    // @brief SMS
    std::vector<sms> sms_;

    // @brief Voicemails
    std::vector<voicemail> voicemails_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_accounts (mobius::core::database::database &);
    void _load_calls (mobius::core::database::database &);
    void _load_contacts (mobius::core::database::database &);
    void _load_file_transfers (mobius::core::database::database &);
    void _load_messages (mobius::core::database::database &);
    void _load_sms (mobius::core::database::database &);
    void _load_voicemails (mobius::core::database::database &);
};

} // namespace mobius::extension::app::skype

#endif
