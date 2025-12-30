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
#include "file_main_db.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <format>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include "common.hpp"
#include "message_parser.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// main.db file tables
//
// - Accounts: Skype account
// - CallMembers
// - Calls
// - Contacts
// - Messages: Chat messages
// - Participants: Chat participants
// - SMSes: SMS/MMS messages
// - Transfers: File transfers
// - Voicemails
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief DEBUG flag
static constexpr bool DEBUG = false;

// @brief Last known schema version
static constexpr std::int64_t LAST_KNOWN_SCHEMA_VERSION = 308;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unknown schema versions
// This set contains schema versions that are not recognized or not handled
// by the current implementation. It is used to identify unsupported versions
// of the main.db schema in Skype applications.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::unordered_set<std::int64_t> UNKNOWN_SCHEMA_VERSIONS = {
    1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,
    31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,
    46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,
    61,  62,  63,  64,  65,  68,  70,  71,  73,  74,  75,  76,  78,  79,  80,
    82,  83,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  98,  99,
    101, 102, 103, 104, 105, 106, 108, 109, 110, 111, 112, 113, 115, 116, 117,
    118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132,
    134, 135, 136, 137, 138, 139, 140, 142, 143, 144, 145, 146, 147, 148, 150,
    151, 154, 155, 156, 157, 158, 159, 160, 161, 163, 165, 166, 167, 169, 170,
    171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185,
    186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 197, 198, 199, 200, 201,
    202, 203, 204, 205, 206, 207, 208, 210, 211, 212, 213, 214, 215, 216, 217,
    218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232,
    233, 234, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248,
    249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 260, 261, 262, 263, 264,
    265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279,
    280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294,
    295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief System messages
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::unordered_map<std::int64_t, std::string> SYSTEM_MESSAGES = {
    {4, "Conference call started"},
    {10, "Chat member added"},
    {12, "Chat member removed"},
    {13, "Chat ended"},
    {30, "Call started"},
    {39, "Call ended"},
    {50, "Authorization requested"},
    {51, "Authorization given"},
    {53, "User blocked"},
    {63, "Contacts info sent"},
    {64, "SMS sent"},
    {68, "Files sent"},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get database schema version
// @param db Database object
// @return Schema version or 0 if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::int64_t
get_db_schema_version (mobius::core::database::database &db)
{
    std::int64_t schema_version = 0;

    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        if (!db.has_table ("AppSchemaVersion"))
        {
            log.info (
                __LINE__,
                "AppSchemaVersion table not found. Path: " + db.get_path ()
            );
            return 0;
        }

        auto stmt = db.new_statement (
            "SELECT SQLiteSchemaVersion FROM AppSchemaVersion"
        );

        if (stmt.fetch_row ())
        {
            schema_version = stmt.get_column_int64 (0);

            if (!schema_version)
            {
                log.warning (
                    __LINE__, "Schema version = 0. Path: " + db.get_path ()
                );
            }
        }
        else
        {
            log.warning (
                __LINE__,
                "Schema version not found in AppSchemaVersion table. Path: " +
                    db.get_path ()
            );
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + ". Path: " + db.get_path ()
        );
    }

    return schema_version;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get participants from chatname
// @param chatname Chat name string in the format #participant1/$participant2;hash
// @return Vector of participant identities
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::vector<std::string>
get_participants_from_chatname (const std::string &chatname)
{
    std::vector<std::string> participants;

    if (!chatname.empty () && chatname[0] == '#')
    {
        auto sep_pos = chatname.find ("/$");

        if (sep_pos != std::string::npos)
        {
            participants.push_back (chatname.substr (1, sep_pos - 1));

            auto hash_pos = chatname.find (';', sep_pos + 2);

            if (hash_pos != std::string::npos)
                participants.push_back (
                    chatname.substr (sep_pos + 2, hash_pos - (sep_pos + 2))
                );
        }
    }

    return participants;
}

} // namespace

namespace mobius::extension::app::skype
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_main_db::file_main_db (const mobius::core::io::reader &reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (!reader)
        return;

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Copy reader content to temporary file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::io::tempfile tfile;
        tfile.copy_from (reader);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get schema version
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::database::database db (tfile.get_path ());
        schema_version_ = get_db_schema_version (db);

        if (schema_version_ > LAST_KNOWN_SCHEMA_VERSION ||
            UNKNOWN_SCHEMA_VERSIONS.find (schema_version_) !=
                UNKNOWN_SCHEMA_VERSIONS.end ())
        {
            log.development (
                __LINE__,
                "Unhandled schema version: " + std::to_string (schema_version_)
            );
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load data
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        _load_accounts (db);
        _load_calls (db);
        _load_contacts (db);
        _load_file_transfers (db);
        _load_message_participants (db);
        _load_messages (db);
        _load_sms (db);
        _load_voicemails (db);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Finish decoding
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load accounts
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_main_db::_load_accounts (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        auto stmt = db.new_select_statement (
            "accounts", {"{about",
                         "ad_policy",
                         "added_in_shared_group",
                         "alertstring",
                         "aliases",
                         "assigned_comment",
                         "assigned_speeddial",
                         "authorized_time",
                         "authreq_history",
                         "authreq_timestamp",
                         "authrequest_count",
                         "authrequest_policy",
                         "availability",
                         "avatar_image",
                         "avatar_policy",
                         "avatar_timestamp",
                         "birthday",
                         "buddyblob",
                         "buddycount_policy",
                         "capabilities",
                         "cbl_future",
                         "cbl_profile_blob",
                         "cblsyncstatus",
                         "chat_policy",
                         "city",
                         "cobrand_id",
                         "commitstatus",
                         "contactssyncstatus",
                         "country",
                         "displayname",
                         "emails",
                         "federated_presence_policy",
                         "flamingo_xmpp_status",
                         "forward_starttime",
                         "fullname",
                         "gender",
                         "given_authlevel",
                         "given_displayname",
                         "hidden_expression_tabs",
                         "homepage",
                         "id",
                         "in_shared_group",
                         "ipcountry",
                         "is_permanent",
                         "languages",
                         "lastonline_timestamp",
                         "lastused_timestamp",
                         "liveid_membername",
                         "logoutreason",
                         "mood_text",
                         "mood_timestamp",
                         "msa_pmn",
                         "node_capabilities",
                         "node_capabilities_and",
                         "nr_of_other_instances",
                         "nrof_authed_buddies",
                         "offline_authreq_id",
                         "offline_callforward",
                         "option_ui_color",
                         "options_change_future",
                         "owner_under_legal_age",
                         "partner_channel_status",
                         "partner_optedout",
                         "phone_home",
                         "phone_mobile",
                         "phone_office",
                         "phonenumbers_policy",
                         "profile_attachments",
                         "profile_timestamp",
                         "province",
                         "pstn_call_policy",
                         "pstnnumber",
                         "pwdchangestatus",
                         "read_receipt_optout",
                         "received_authrequest",
                         "refreshing",
                         "registration_timestamp",
                         "revoked_auth",
                         "rich_mood_text",
                         "roaming_history_enabled",
                         "sent_authrequest",
                         "sent_authrequest_serial",
                         "sent_authrequest_time",
                         "service_provider_info",
                         "set_availability",
                         "shortcircuit_sync",
                         "signin_name",
                         "skype_call_policy",
                         "skypein_numbers",
                         "skypename",
                         "skypeout_balance",
                         "skypeout_balance_currency",
                         "skypeout_precision",
                         "stack_version",
                         "status",
                         "subscriptions",
                         "suggested_skypename",
                         "synced_email",
                         "timezone",
                         "timezone_policy",
                         "type",
                         "uses_jcs",
                         "verified_company",
                         "verified_email",
                         "voicemail_policy",
                         "webpresence_policy"}
        );

        // Retrieve rows from query
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            account obj;

            // Set attributes
            obj.idx = idx++;
            obj.about = stmt.get_column_string (0);
            obj.ad_policy = stmt.get_column_int64 (1);
            obj.added_in_shared_group = stmt.get_column_int64 (2);
            obj.alertstring = stmt.get_column_string (3);
            obj.aliases = stmt.get_column_string (4);
            obj.assigned_comment = stmt.get_column_string (5);
            obj.assigned_speeddial = stmt.get_column_string (6);
            obj.authorized_time = get_time (stmt.get_column_int64 (7));
            obj.authreq_history = stmt.get_column_bytearray (8);
            obj.authreq_timestamp = get_datetime (stmt.get_column_int64 (9));
            obj.authrequest_count = stmt.get_column_int64 (10);
            obj.authrequest_policy = stmt.get_column_int64 (11);
            obj.availability = stmt.get_column_int64 (12);
            obj.avatar_image = stmt.get_column_bytearray (13);
            obj.avatar_policy = stmt.get_column_int64 (14);
            obj.avatar_timestamp = get_datetime (stmt.get_column_int64 (15));
            obj.birthday = stmt.get_column_int64 (16);
            obj.buddyblob = stmt.get_column_bytearray (17);
            obj.buddycount_policy = stmt.get_column_int64 (18);
            obj.capabilities = stmt.get_column_bytearray (19);
            obj.cbl_future = stmt.get_column_bytearray (20);
            obj.cbl_profile_blob = stmt.get_column_bytearray (21);
            obj.cblsyncstatus = stmt.get_column_int64 (22);
            obj.chat_policy = stmt.get_column_int64 (23);
            obj.city = stmt.get_column_string (24);
            obj.cobrand_id = stmt.get_column_int64 (25);
            obj.commitstatus = stmt.get_column_int64 (26);
            obj.contactssyncstatus = stmt.get_column_int64 (27);
            obj.country = stmt.get_column_string (28);
            obj.displayname = stmt.get_column_string (29);
            obj.emails = stmt.get_column_string (30);
            obj.federated_presence_policy = stmt.get_column_int64 (31);
            obj.flamingo_xmpp_status = stmt.get_column_int64 (32);
            obj.forward_starttime = get_datetime (stmt.get_column_int64 (33));
            obj.fullname = stmt.get_column_string (34);
            obj.gender = stmt.get_column_int64 (35);
            obj.given_authlevel = stmt.get_column_int64 (36);
            obj.given_displayname = stmt.get_column_string (37);
            obj.hidden_expression_tabs = stmt.get_column_string (38);
            obj.homepage = stmt.get_column_string (39);
            obj.id = stmt.get_column_int64 (40);
            obj.in_shared_group = stmt.get_column_int64 (41);
            obj.ipcountry = stmt.get_column_string (42);
            obj.is_permanent = stmt.get_column_int64 (43);
            obj.languages = stmt.get_column_string (44);
            obj.lastonline_timestamp =
                get_datetime (stmt.get_column_int64 (45));
            obj.lastused_timestamp = get_datetime (stmt.get_column_int64 (46));
            obj.liveid_membername = stmt.get_column_string (47);
            obj.logoutreason = stmt.get_column_int64 (48);
            obj.mood_text = stmt.get_column_string (49);
            obj.mood_timestamp = get_datetime (stmt.get_column_int64 (50));
            obj.msa_pmn = stmt.get_column_string (51);
            obj.node_capabilities = stmt.get_column_int64 (52);
            obj.node_capabilities_and = stmt.get_column_int64 (53);
            obj.nr_of_other_instances = stmt.get_column_int64 (54);
            obj.nrof_authed_buddies = stmt.get_column_int64 (55);
            obj.offline_authreq_id = stmt.get_column_int64 (56);
            obj.offline_callforward = stmt.get_column_string (57);
            obj.option_ui_color = stmt.get_column_string (58);
            obj.options_change_future = stmt.get_column_bytearray (59);
            obj.owner_under_legal_age = stmt.get_column_int64 (60);
            obj.partner_channel_status = stmt.get_column_string (61);
            obj.partner_optedout = stmt.get_column_string (62);
            obj.phone_home = stmt.get_column_string (63);
            obj.phone_mobile = stmt.get_column_string (64);
            obj.phone_office = stmt.get_column_string (65);
            obj.phonenumbers_policy = stmt.get_column_int64 (66);
            obj.profile_attachments = stmt.get_column_bytearray (67);
            obj.profile_timestamp = get_datetime (stmt.get_column_int64 (68));
            obj.province = stmt.get_column_string (69);
            obj.pstn_call_policy = stmt.get_column_int64 (70);
            obj.pstnnumber = stmt.get_column_string (71);
            obj.pwdchangestatus = stmt.get_column_int64 (72);
            obj.read_receipt_optout = stmt.get_column_int64 (73);
            obj.received_authrequest = stmt.get_column_string (74);
            obj.refreshing = stmt.get_column_int64 (75);
            obj.registration_timestamp = get_time (stmt.get_column_int64 (76));
            obj.revoked_auth = stmt.get_column_int64 (77);
            obj.rich_mood_text = stmt.get_column_string (78);
            obj.roaming_history_enabled = stmt.get_column_int64 (79);
            obj.sent_authrequest = stmt.get_column_string (80);
            obj.sent_authrequest_serial = stmt.get_column_int64 (81);
            obj.sent_authrequest_time = get_time (stmt.get_column_int64 (82));
            obj.service_provider_info = stmt.get_column_string (83);
            obj.set_availability = stmt.get_column_int64 (84);
            obj.shortcircuit_sync = stmt.get_column_int64 (85);
            obj.signin_name = stmt.get_column_string (86);
            obj.skype_call_policy = stmt.get_column_int64 (87);
            obj.skypein_numbers = stmt.get_column_string (88);
            obj.skypename = stmt.get_column_string (89);
            obj.skypeout_balance = stmt.get_column_int64 (90);
            obj.skypeout_balance_currency = stmt.get_column_string (91);
            obj.skypeout_precision = stmt.get_column_int64 (92);
            obj.stack_version = stmt.get_column_int64 (93);
            obj.status = stmt.get_column_int64 (94);
            obj.subscriptions = stmt.get_column_string (95);
            obj.suggested_skypename = stmt.get_column_string (96);
            obj.synced_email = stmt.get_column_bytearray (97);
            obj.timezone = stmt.get_column_int64 (98);
            obj.timezone_policy = stmt.get_column_int64 (99);
            obj.type = stmt.get_column_int64 (100);
            obj.uses_jcs = stmt.get_column_int64 (101);
            obj.verified_company = stmt.get_column_bytearray (102);
            obj.verified_email = stmt.get_column_bytearray (103);
            obj.voicemail_policy = stmt.get_column_int64 (104);
            obj.webpresence_policy = stmt.get_column_int64 (105);

            // Add to accounts vector
            accounts_.emplace_back (std::move (obj));
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load Calls
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_main_db::_load_calls (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        std::unordered_multimap<std::int64_t, call_member> call_members;

        // Prepare SQL statement for table CallMembers
        auto stmt_cm = db.new_select_statement (
            "CallMembers", {"accepted_by",
                            "admit_failure_reason",
                            "balance_update",
                            "call_db_id",
                            "call_duration",
                            "call_end_diagnostics_code",
                            "call_name",
                            "call_session_guid",
                            "capabilities",
                            "content_sharing_role",
                            "country",
                            "creation_timestamp",
                            "debuginfo",
                            "dispname",
                            "dominant_speaker_rank",
                            "endpoint_details",
                            "endpoint_type",
                            "failurereason",
                            "fallback_in_progress",
                            "forward_targets",
                            "forwarded_by",
                            "group_calling_capabilities",
                            "guid",
                            "id",
                            "identity",
                            "identity_type",
                            "ip_address",
                            "is_active_speaker",
                            "is_conference",
                            "is_multiparty_video_capable",
                            "is_permanent",
                            "is_premium_video_sponsor",
                            "is_read_only",
                            "is_seamlessly_upgraded_call",
                            "is_server_muted",
                            "is_video_codec_compatible",
                            "languages",
                            "light_weight_meeting_role",
                            "limiting_factor",
                            "mike_status",
                            "mri_identity",
                            "next_redial_time",
                            "nonse_word",
                            "nr_of_delivered_push_notifications",
                            "nrof_redials_done",
                            "nrof_redials_left",
                            "participant_sponsor",
                            "payment_category",
                            "pk_status",
                            "price_currency",
                            "price_per_minute",
                            "price_precision",
                            "prime_status",
                            "pstn_feedback",
                            "pstn_statustext",
                            "quality_problems",
                            "quality_status",
                            "real_identity",
                            "recovery_in_progress",
                            "role",
                            "seconds_left",
                            "sounderror_code",
                            "soundlevel",
                            "start_timestamp",
                            "stats_xml",
                            "status",
                            "target_identity",
                            "tenant_id",
                            "transfer_active",
                            "transfer_status",
                            "transfer_topic",
                            "transferred_by",
                            "transferred_to",
                            "type",
                            "version_string",
                            "video_count_changed",
                            "videostatus",
                            "voicechannel"}
        );

        // Retrieve records from CallMembers table
        std::uint64_t idx = 0;

        while (stmt_cm.fetch_row ())
        {
            call_member obj;

            obj.idx = idx++;
            obj.accepted_by = stmt_cm.get_column_string (0);
            obj.admit_failure_reason = stmt_cm.get_column_int64 (1);
            obj.balance_update = stmt_cm.get_column_string (2);
            obj.call_db_id = stmt_cm.get_column_int64 (3);
            obj.call_duration = stmt_cm.get_column_int64 (4);
            obj.call_end_diagnostics_code = stmt_cm.get_column_string (5);
            obj.call_name = stmt_cm.get_column_string (6);
            obj.call_session_guid = stmt_cm.get_column_string (7);
            obj.capabilities = stmt_cm.get_column_int64 (8);
            obj.content_sharing_role = stmt_cm.get_column_int64 (9);
            obj.country = stmt_cm.get_column_string (10);
            obj.creation_timestamp =
                get_datetime (stmt_cm.get_column_int64 (11));
            obj.debuginfo = stmt_cm.get_column_string (12);
            obj.dispname = stmt_cm.get_column_string (13);
            obj.dominant_speaker_rank = stmt_cm.get_column_int64 (14);
            obj.endpoint_details = stmt_cm.get_column_string (15);
            obj.endpoint_type = stmt_cm.get_column_int64 (16);
            obj.failurereason = stmt_cm.get_column_int64 (17);
            obj.fallback_in_progress = stmt_cm.get_column_int64 (18);
            obj.forward_targets = stmt_cm.get_column_string (19);
            obj.forwarded_by = stmt_cm.get_column_string (20);
            obj.group_calling_capabilities = stmt_cm.get_column_int64 (21);
            obj.guid = stmt_cm.get_column_string (22);
            obj.id = stmt_cm.get_column_int64 (23);
            obj.identity = stmt_cm.get_column_string (24);
            obj.identity_type = stmt_cm.get_column_int64 (25);
            obj.ip_address = stmt_cm.get_column_string (26);
            obj.is_active_speaker = stmt_cm.get_column_bool (27);
            obj.is_conference = stmt_cm.get_column_bool (28);
            obj.is_multiparty_video_capable = stmt_cm.get_column_bool (29);
            obj.is_permanent = stmt_cm.get_column_bool (30);
            obj.is_premium_video_sponsor = stmt_cm.get_column_bool (31);
            obj.is_read_only = stmt_cm.get_column_bool (32);
            obj.is_seamlessly_upgraded_call = stmt_cm.get_column_bool (33);
            obj.is_server_muted = stmt_cm.get_column_bool (34);
            obj.is_video_codec_compatible = stmt_cm.get_column_bool (35);
            obj.languages = stmt_cm.get_column_string (36);
            obj.light_weight_meeting_role = stmt_cm.get_column_int64 (37);
            obj.limiting_factor = stmt_cm.get_column_int64 (38);
            obj.mike_status = stmt_cm.get_column_int64 (39);
            obj.mri_identity = stmt_cm.get_column_string (40);
            obj.next_redial_time = stmt_cm.get_column_int64 (41);
            obj.nonse_word = stmt_cm.get_column_string (42);
            obj.nr_of_delivered_push_notifications =
                stmt_cm.get_column_int64 (43);
            obj.nrof_redials_done = stmt_cm.get_column_int64 (44);
            obj.nrof_redials_left = stmt_cm.get_column_int64 (45);
            obj.participant_sponsor = stmt_cm.get_column_string (46);
            obj.payment_category = stmt_cm.get_column_string (47);
            obj.pk_status = stmt_cm.get_column_int64 (48);
            obj.price_currency = stmt_cm.get_column_string (49);
            obj.price_per_minute = stmt_cm.get_column_int64 (50);
            obj.price_precision = stmt_cm.get_column_int64 (51);
            obj.prime_status = stmt_cm.get_column_int64 (52);
            obj.pstn_feedback = stmt_cm.get_column_string (53);
            obj.pstn_statustext = stmt_cm.get_column_string (54);
            obj.quality_problems = stmt_cm.get_column_string (55);
            obj.quality_status = stmt_cm.get_column_int64 (56);
            obj.real_identity = stmt_cm.get_column_string (57);
            obj.recovery_in_progress = stmt_cm.get_column_int64 (58);
            obj.role = stmt_cm.get_column_string (59);
            obj.seconds_left = stmt_cm.get_column_int64 (60);
            obj.sounderror_code = stmt_cm.get_column_int64 (61);
            obj.soundlevel = stmt_cm.get_column_int64 (62);
            obj.start_timestamp = get_datetime (stmt_cm.get_column_int64 (63));
            obj.stats_xml = stmt_cm.get_column_string (64);
            obj.status = stmt_cm.get_column_int64 (65);
            obj.target_identity = stmt_cm.get_column_string (66);
            obj.tenant_id = stmt_cm.get_column_string (67);
            obj.transfer_active = stmt_cm.get_column_int64 (68);
            obj.transfer_status = stmt_cm.get_column_int64 (69);
            obj.transfer_topic = stmt_cm.get_column_string (70);
            obj.transferred_by = stmt_cm.get_column_string (71);
            obj.transferred_to = stmt_cm.get_column_string (72);
            obj.type = stmt_cm.get_column_int64 (73);
            obj.version_string = stmt_cm.get_column_string (74);
            obj.video_count_changed = stmt_cm.get_column_int64 (75);
            obj.videostatus = stmt_cm.get_column_int64 (76);
            obj.voicechannel = stmt_cm.get_column_int64 (77);

            // Add callmembers to the list
            call_members.emplace (obj.call_db_id, std::move (obj));
        }

        // Prepare SQL statement for table Calls
        auto stmt = db.new_select_statement (
            "Calls", {"access_token",
                      "active_members",
                      "begin_timestamp",
                      "broadcast_metadata",
                      "caller_mri_identity",
                      "conf_participants",
                      "content_sharing_session_count_changed",
                      "conv_dbid",
                      "conversation_type",
                      "current_video_audience",
                      "datachannel_object_id",
                      "duration",
                      "endpoint_details",
                      "failurecode",
                      "failurereason",
                      "forwarding_destination_type",
                      "host_identity",
                      "id",
                      "incoming_type",
                      "is_active",
                      "is_conference",
                      "is_hostless",
                      "is_incoming",
                      "is_incoming_one_on_one_video_call",
                      "is_muted",
                      "is_muted_speaker",
                      "is_on_hold",
                      "is_permanent",
                      "is_premium_video_sponsor",
                      "is_server_muted",
                      "is_unseen_missed",
                      "joined_existing",
                      "leg_id",
                      "light_weight_meeting_count_changed",
                      "max_videoconfcall_participants",
                      "meeting_details",
                      "member_count_changed",
                      "members",
                      "message_id",
                      "mike_status",
                      "name",
                      "old_duration",
                      "old_members",
                      "onbehalfof_mri",
                      "optimal_remote_videos_in_conference",
                      "partner_dispname",
                      "partner_handle",
                      "premium_video_is_grace_period",
                      "premium_video_sponsor_list",
                      "premium_video_status",
                      "pstn_number",
                      "pstn_status",
                      "quality_problems",
                      "queue_info",
                      "role",
                      "server_identity",
                      "soundlevel",
                      "start_timestamp",
                      "status",
                      "technology",
                      "tenant_id",
                      "thread_id",
                      "topic",
                      "transfer_failure_reason",
                      "transfer_status",
                      "transferor_displayname",
                      "transferor_mri",
                      "transferor_type",
                      "type",
                      "vaa_input_status",
                      "video_disabled"}
        );

        // Retrieve records from Calls table
        idx = 0;

        while (stmt.fetch_row ())
        {
            call obj;

            obj.idx = idx++;
            obj.access_token = stmt.get_column_string (0);
            obj.active_members = stmt.get_column_int64 (1);
            obj.begin_timestamp = get_datetime (stmt.get_column_int64 (2));
            obj.broadcast_metadata = stmt.get_column_string (3);
            obj.caller_mri_identity = stmt.get_column_string (4);
            obj.conf_participants = stmt.get_column_bytearray (5);
            obj.content_sharing_session_count_changed =
                stmt.get_column_int64 (6);
            obj.conv_dbid = stmt.get_column_int64 (7);
            obj.conversation_type = stmt.get_column_string (8);
            obj.current_video_audience = stmt.get_column_string (9);
            obj.datachannel_object_id = stmt.get_column_int64 (10);
            obj.duration = stmt.get_column_int64 (11);
            obj.endpoint_details = stmt.get_column_string (12);
            obj.failurecode = stmt.get_column_int64 (13);
            obj.failurereason = stmt.get_column_int64 (14);
            obj.forwarding_destination_type = stmt.get_column_string (15);
            obj.host_identity = stmt.get_column_string (16);
            obj.id = stmt.get_column_int64 (17);
            obj.incoming_type = stmt.get_column_string (18);
            obj.is_active = stmt.get_column_bool (19);
            obj.is_conference = stmt.get_column_bool (20);
            obj.is_hostless = stmt.get_column_bool (21);
            obj.is_incoming = stmt.get_column_bool (22);
            obj.is_incoming_one_on_one_video_call = stmt.get_column_bool (23);
            obj.is_muted = stmt.get_column_bool (24);
            obj.is_muted_speaker = stmt.get_column_bool (25);
            obj.is_on_hold = stmt.get_column_bool (26);
            obj.is_permanent = stmt.get_column_bool (27);
            obj.is_premium_video_sponsor = stmt.get_column_bool (28);
            obj.is_server_muted = stmt.get_column_bool (29);
            obj.is_unseen_missed = stmt.get_column_bool (30);
            obj.joined_existing = stmt.get_column_int64 (31);
            obj.leg_id = stmt.get_column_string (32);
            obj.light_weight_meeting_count_changed = stmt.get_column_int64 (33);
            obj.max_videoconfcall_participants = stmt.get_column_int64 (34);
            obj.meeting_details = stmt.get_column_string (35);
            obj.member_count_changed = stmt.get_column_int64 (36);
            obj.members = stmt.get_column_bytearray (37);
            obj.message_id = stmt.get_column_string (38);
            obj.mike_status = stmt.get_column_int64 (39);
            obj.name = stmt.get_column_string (40);
            obj.old_duration = stmt.get_column_int64 (41);
            obj.old_members = stmt.get_column_bytearray (42);
            obj.onbehalfof_mri = stmt.get_column_string (43);
            obj.optimal_remote_videos_in_conference =
                stmt.get_column_int64 (44);
            obj.partner_dispname = stmt.get_column_string (45);
            obj.partner_handle = stmt.get_column_string (46);
            obj.premium_video_is_grace_period = stmt.get_column_int64 (47);
            obj.premium_video_sponsor_list = stmt.get_column_string (48);
            obj.premium_video_status = stmt.get_column_int64 (49);
            obj.pstn_number = stmt.get_column_string (50);
            obj.pstn_status = stmt.get_column_string (51);
            obj.quality_problems = stmt.get_column_string (52);
            obj.queue_info = stmt.get_column_string (53);
            obj.role = stmt.get_column_string (54);
            obj.server_identity = stmt.get_column_string (55);
            obj.soundlevel = stmt.get_column_int64 (56);
            obj.start_timestamp = get_datetime (stmt.get_column_int64 (57));
            obj.status = stmt.get_column_int64 (58);
            obj.technology = stmt.get_column_int64 (59);
            obj.tenant_id = stmt.get_column_string (60);
            obj.thread_id = stmt.get_column_string (61);
            obj.topic = stmt.get_column_string (62);
            obj.transfer_failure_reason = stmt.get_column_int64 (63);
            obj.transfer_status = stmt.get_column_int64 (64);
            obj.transferor_displayname = stmt.get_column_string (65);
            obj.transferor_mri = stmt.get_column_string (66);
            obj.transferor_type = stmt.get_column_string (67);
            obj.type = stmt.get_column_int64 (68);
            obj.vaa_input_status = stmt.get_column_int64 (69);
            obj.video_disabled = stmt.get_column_int64 (70);

            // Add call members to the call object
            auto range = call_members.equal_range (obj.id);
            std::transform (
                range.first, range.second,
                std::back_inserter (obj.call_members),
                [] (auto &pair) { return pair.second; }
            );

            // Add call to the list
            calls_.emplace_back (std::move (obj));
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load Contacts
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_main_db::_load_contacts (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Contacts table was dropped between schema version 259 and 308
    if (schema_version_ > 307)
        return;

    try
    {
        // Prepare SQL statement for table Contacts
        auto stmt = db.new_select_statement (
            "Contacts", {"about",
                         "account_modification_serial_nr",
                         "added_in_shared_group",
                         "alertstring",
                         "aliases",
                         "assigned_comment",
                         "assigned_phone1",
                         "assigned_phone1_label",
                         "assigned_phone2",
                         "assigned_phone2_label",
                         "assigned_phone3",
                         "assigned_phone3_label",
                         "assigned_speeddial",
                         "authorization_certificate",
                         "authorized_time",
                         "authreq_crc",
                         "authreq_history",
                         "authreq_initmethod",
                         "authreq_nodeinfo",
                         "authreq_src",
                         "authreq_timestamp",
                         "authrequest_count",
                         "availability",
                         "avatar_hiresurl",
                         "avatar_hiresurl_new",
                         "avatar_image",
                         "avatar_timestamp",
                         "avatar_url",
                         "avatar_url_new",
                         "birthday",
                         "buddyblob",
                         "buddystatus",
                         "capabilities",
                         "cbl_future",
                         "certificate_send_count",
                         "city",
                         "contactlist_track",
                         "country",
                         "dirblob_last_search_time",
                         "displayname",
                         "emails",
                         "external_id",
                         "external_system_id",
                         "extprop_can_show_avatar",
                         "extprop_contact_ab_uuid",
                         "extprop_external_data",
                         "extprop_last_sms_number",
                         "extprop_must_hide_avatar",
                         "extprop_seen_birthday",
                         "extprop_sms_pstn_contact_created",
                         "extprop_sms_target",
                         "extprop_viral_upgrade_campaign_id",
                         "firstname",
                         "fullname",
                         "gender",
                         "given_authlevel",
                         "given_displayname",
                         "group_membership",
                         "hashed_emails",
                         "homepage",
                         "id",
                         "in_shared_group",
                         "ipcountry",
                         "is_auto_buddy",
                         "is_mobile",
                         "is_permanent",
                         "is_trusted",
                         "isauthorized",
                         "isblocked",
                         "languages",
                         "last_used_networktime",
                         "lastname",
                         "lastonline_timestamp",
                         "lastused_timestamp",
                         "liveid_cid",
                         "main_phone",
                         "mood_text",
                         "mood_timestamp",
                         "mutual_friend_count",
                         "network_availability",
                         "node_capabilities",
                         "node_capabilities_and",
                         "nr_of_buddies",
                         "nrof_authed_buddies",
                         "offline_authreq_id",
                         "phone_home",
                         "phone_home_normalized",
                         "phone_mobile",
                         "phone_mobile_normalized",
                         "phone_office",
                         "phone_office_normalized",
                         "pop_score",
                         "popularity_ord",
                         "profile_attachments",
                         "profile_etag",
                         "profile_json",
                         "profile_timestamp",
                         "province",
                         "pstnnumber",
                         "received_authrequest",
                         "refreshing",
                         "revoked_auth",
                         "rich_mood_text",
                         "saved_directory_blob",
                         "sent_authrequest",
                         "sent_authrequest_extrasbitmask",
                         "sent_authrequest_initmethod",
                         "sent_authrequest_serial",
                         "sent_authrequest_time",
                         "server_synced",
                         "skypename",
                         "stack_version",
                         "timezone",
                         "type",
                         "unified_servants",
                         "verified_company",
                         "verified_email"}
        );

        // Retrieve records from Contacts table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            contact obj;

            obj.idx = idx++;
            obj.about = stmt.get_column_string (0);
            obj.account_modification_serial_nr = stmt.get_column_int64 (1);
            obj.added_in_shared_group = stmt.get_column_int64 (2);
            obj.alertstring = stmt.get_column_string (3);
            obj.aliases = stmt.get_column_string (4);
            obj.assigned_comment = stmt.get_column_string (5);
            obj.assigned_phone1 = stmt.get_column_string (6);
            obj.assigned_phone1_label = stmt.get_column_string (7);
            obj.assigned_phone2 = stmt.get_column_string (8);
            obj.assigned_phone2_label = stmt.get_column_string (9);
            obj.assigned_phone3 = stmt.get_column_string (10);
            obj.assigned_phone3_label = stmt.get_column_string (11);
            obj.assigned_speeddial = stmt.get_column_string (12);
            obj.authorization_certificate = stmt.get_column_bytearray (13);
            obj.authorized_time = stmt.get_column_int64 (14);
            obj.authreq_crc = stmt.get_column_int64 (15);
            obj.authreq_history = stmt.get_column_bytearray (16);
            obj.authreq_initmethod = stmt.get_column_int64 (17);
            obj.authreq_nodeinfo = stmt.get_column_bytearray (18);
            obj.authreq_src = stmt.get_column_int64 (19);
            obj.authreq_timestamp = stmt.get_column_int64 (20);
            obj.authrequest_count = stmt.get_column_int64 (21);
            obj.availability = stmt.get_column_int64 (22);
            obj.avatar_hiresurl = stmt.get_column_string (23);
            obj.avatar_hiresurl_new = stmt.get_column_string (24);
            obj.avatar_image = stmt.get_column_bytearray (25);
            obj.avatar_timestamp = get_datetime (stmt.get_column_int64 (26));
            obj.avatar_url = stmt.get_column_string (27);
            obj.avatar_url_new = stmt.get_column_string (28);
            obj.birthday = stmt.get_column_int64 (29);
            obj.buddyblob = stmt.get_column_bytearray (30);
            obj.buddystatus = stmt.get_column_int64 (31);
            obj.capabilities = stmt.get_column_bytearray (32);
            obj.cbl_future = stmt.get_column_bytearray (33);
            obj.certificate_send_count = stmt.get_column_int64 (34);
            obj.city = stmt.get_column_string (35);
            obj.contactlist_track = stmt.get_column_int64 (36);
            obj.country = stmt.get_column_string (37);
            obj.dirblob_last_search_time =
                get_datetime (stmt.get_column_int64 (38));
            obj.displayname = stmt.get_column_string (39);
            obj.emails = stmt.get_column_string (40);
            obj.external_id = stmt.get_column_string (41);
            obj.external_system_id = stmt.get_column_string (42);
            obj.extprop_can_show_avatar = stmt.get_column_int64 (43);
            obj.extprop_contact_ab_uuid = stmt.get_column_string (44);
            obj.extprop_external_data = stmt.get_column_string (45);
            obj.extprop_last_sms_number = stmt.get_column_string (46);
            obj.extprop_must_hide_avatar = stmt.get_column_int64 (47);
            obj.extprop_seen_birthday = stmt.get_column_int64 (48);
            obj.extprop_sms_pstn_contact_created = stmt.get_column_int64 (49);
            obj.extprop_sms_target = stmt.get_column_int64 (50);
            obj.extprop_viral_upgrade_campaign_id = stmt.get_column_int64 (51);
            obj.firstname = stmt.get_column_string (52);
            obj.fullname = stmt.get_column_string (53);
            obj.gender = stmt.get_column_int64 (54);
            obj.given_authlevel = stmt.get_column_int64 (55);
            obj.given_displayname = stmt.get_column_string (56);
            obj.group_membership = stmt.get_column_int64 (57);
            obj.hashed_emails = stmt.get_column_string (58);
            obj.homepage = stmt.get_column_string (59);
            obj.id = stmt.get_column_int64 (60);
            obj.in_shared_group = stmt.get_column_int64 (61);
            obj.ipcountry = stmt.get_column_string (62);
            obj.is_auto_buddy = stmt.get_column_int64 (63);
            obj.is_mobile = stmt.get_column_int64 (64);
            obj.is_permanent = stmt.get_column_int64 (65);
            obj.is_trusted = stmt.get_column_int64 (66);
            obj.isauthorized = stmt.get_column_int64 (67);
            obj.isblocked = stmt.get_column_int64 (68);
            obj.languages = stmt.get_column_string (69);
            obj.last_used_networktime = get_time (stmt.get_column_int64 (70));
            obj.lastname = stmt.get_column_string (71);
            obj.lastonline_timestamp =
                get_datetime (stmt.get_column_int64 (72));
            obj.lastused_timestamp = get_datetime (stmt.get_column_int64 (73));
            obj.liveid_cid = stmt.get_column_string (74);
            obj.main_phone = stmt.get_column_string (75);
            obj.mood_text = stmt.get_column_string (76);
            obj.mood_timestamp = get_datetime (stmt.get_column_int64 (77));
            obj.mutual_friend_count = stmt.get_column_int64 (78);
            obj.network_availability = stmt.get_column_int64 (79);
            obj.node_capabilities = stmt.get_column_int64 (80);
            obj.node_capabilities_and = stmt.get_column_int64 (81);
            obj.nr_of_buddies = stmt.get_column_int64 (82);
            obj.nrof_authed_buddies = stmt.get_column_int64 (83);
            obj.offline_authreq_id = stmt.get_column_int64 (84);
            obj.phone_home = stmt.get_column_string (85);
            obj.phone_home_normalized = stmt.get_column_string (86);
            obj.phone_mobile = stmt.get_column_string (87);
            obj.phone_mobile_normalized = stmt.get_column_string (88);
            obj.phone_office = stmt.get_column_string (89);
            obj.phone_office_normalized = stmt.get_column_string (90);
            obj.pop_score = stmt.get_column_int64 (91);
            obj.popularity_ord = stmt.get_column_int64 (92);
            obj.profile_attachments = stmt.get_column_bytearray (93);
            obj.profile_etag = stmt.get_column_string (94);
            obj.profile_json = stmt.get_column_string (95);
            obj.profile_timestamp = get_datetime (stmt.get_column_int64 (96));
            obj.province = stmt.get_column_string (97);
            obj.pstnnumber = stmt.get_column_string (98);
            obj.received_authrequest = stmt.get_column_string (99);
            obj.refreshing = stmt.get_column_int64 (100);
            obj.revoked_auth = stmt.get_column_int64 (101);
            obj.rich_mood_text = stmt.get_column_string (102);
            obj.saved_directory_blob = stmt.get_column_bytearray (103);
            obj.sent_authrequest = stmt.get_column_string (104);
            obj.sent_authrequest_extrasbitmask = stmt.get_column_int64 (105);
            obj.sent_authrequest_initmethod = stmt.get_column_int64 (106);
            obj.sent_authrequest_serial = stmt.get_column_int64 (107);
            obj.sent_authrequest_time =
                get_datetime (stmt.get_column_int64 (108));
            obj.server_synced = stmt.get_column_int64 (109);
            obj.skypename = stmt.get_column_string (110);
            obj.stack_version = stmt.get_column_int64 (111);
            obj.timezone = get_timezone (stmt.get_column_int64 (112));
            obj.type = stmt.get_column_int64 (113);
            obj.unified_servants = stmt.get_column_string (114);
            obj.verified_company = stmt.get_column_bytearray (115);
            obj.verified_email = stmt.get_column_bytearray (116);

            // Add contacts to the list
            contacts_.emplace_back (std::move (obj));
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load File Transfers
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_main_db::_load_file_transfers (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Transfers table was dropped between schema version 259 and 308
    if (schema_version_ > 307)
        return;

    try
    {
        // Prepare SQL statement for table Transfers
        auto stmt = db.new_select_statement (
            "Transfers", {"accepttime",
                          "bytespersecond",
                          "bytestransferred",
                          "chatmsg_guid",
                          "chatmsg_index",
                          "convo_id",
                          "extprop_handled_by_chat",
                          "extprop_hide_from_history",
                          "extprop_localfilename",
                          "extprop_transfer_alias",
                          "extprop_window_visible",
                          "failurereason",
                          "filename",
                          "filepath",
                          "filesize",
                          "finishtime",
                          "flags",
                          "id",
                          "is_permanent",
                          "last_activity",
                          "nodeid",
                          "offer_send_list",
                          "old_filepath",
                          "old_status",
                          "parent_id",
                          "partner_dispname",
                          "partner_handle",
                          "pk_id",
                          "starttime",
                          "status",
                          "type"}
        );

        // Retrieve records from Transfers table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            file_transfer obj;

            obj.idx = idx++;
            obj.accepttime = get_datetime (stmt.get_column_int64 (0));
            obj.bytespersecond = stmt.get_column_int64 (1);
            obj.bytestransferred = stmt.get_column_string (2);
            obj.chatmsg_guid = stmt.get_column_bytearray (3).to_guid ();
            obj.chatmsg_index = stmt.get_column_int64 (4);
            obj.convo_id = stmt.get_column_int64 (5);
            obj.extprop_handled_by_chat = stmt.get_column_int64 (6);
            obj.extprop_hide_from_history = stmt.get_column_int64 (7);
            obj.extprop_localfilename = stmt.get_column_string (8);
            obj.extprop_transfer_alias = stmt.get_column_bytearray (9);
            obj.extprop_window_visible = stmt.get_column_int64 (10);
            obj.failurereason = stmt.get_column_int64 (11);
            obj.filename = stmt.get_column_string (12);
            obj.filepath = stmt.get_column_string (13);
            obj.filesize = stmt.get_column_string (14);
            obj.finishtime = get_datetime (stmt.get_column_int64 (15));
            obj.flags = stmt.get_column_int64 (16);
            obj.id = stmt.get_column_int64 (17);
            obj.is_permanent = stmt.get_column_int64 (18);
            obj.last_activity = stmt.get_column_int64 (19);
            obj.nodeid = stmt.get_column_bytearray (20);
            obj.offer_send_list = stmt.get_column_string (21);
            obj.old_filepath = stmt.get_column_int64 (22);
            obj.old_status = stmt.get_column_int64 (23);
            obj.parent_id = stmt.get_column_int64 (24);
            obj.partner_dispname = stmt.get_column_string (25);
            obj.partner_handle = stmt.get_column_string (26);
            obj.pk_id = stmt.get_column_int64 (27);
            obj.starttime = get_datetime (stmt.get_column_int64 (28));
            obj.status = stmt.get_column_int64 (29);
            obj.type = stmt.get_column_int64 (30);

            // Add transfers to the list
            file_transfers_.emplace_back (std::move (obj));
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load Message Participants
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_main_db::_load_message_participants (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Messages table was dropped between schema version 196 and 209
    if (schema_version_ > 196)
        return;

    try
    {
        // Prepare SQL statement for table Participants
        auto stmt_part = db.new_select_statement (
            "Participants", {"adder",
                             "adding_in_progress_since",
                             "convo_id",
                             "debuginfo",
                             "dominant_speaker_rank",
                             "endpoint_details",
                             "extprop_default_identity",
                             "extprop_identity_to_use",
                             "group_calling_capabilities",
                             "id",
                             "identity",
                             "is_active_speaker",
                             "is_multiparty_video_capable",
                             "is_multiparty_video_updatable",
                             "is_permanent",
                             "is_premium_video_sponsor",
                             "is_seamlessly_upgraded_call",
                             "is_video_codec_compatible",
                             "last_leavereason",
                             "last_voice_error",
                             "live_country",
                             "live_fwd_identities",
                             "live_identity",
                             "live_identity_to_use",
                             "live_ip_address",
                             "live_price_for_me",
                             "live_start_timestamp",
                             "live_type",
                             "live_voicechannel",
                             "livesession_fallback_in_progress",
                             "livesession_recovery_in_progress",
                             "messaging_mode",
                             "next_redial_time",
                             "nrof_redials_left",
                             "quality_problems",
                             "rank",
                             "read_horizon",
                             "real_identity",
                             "requested_rank",
                             "sound_level",
                             "sponsor",
                             "text_status",
                             "transferred_by",
                             "transferred_to",
                             "video_status",
                             "voice_status"}
        );

        // Retrieve records from Participants table
        std::uint64_t idx = 0;

        while (stmt_part.fetch_row ())
        {
            message_participant obj;

            obj.idx = idx++;
            obj.adder = stmt_part.get_column_string (0);
            obj.adding_in_progress_since = stmt_part.get_column_int64 (1);
            obj.convo_id = stmt_part.get_column_int64 (2);
            obj.debuginfo = stmt_part.get_column_string (3);
            obj.dominant_speaker_rank = stmt_part.get_column_int64 (4);
            obj.endpoint_details = stmt_part.get_column_string (5);
            obj.extprop_default_identity = stmt_part.get_column_int64 (6);
            obj.extprop_identity_to_use = stmt_part.get_column_int64 (7);
            obj.group_calling_capabilities = stmt_part.get_column_int64 (8);
            obj.id = stmt_part.get_column_int64 (9);
            obj.identity = stmt_part.get_column_string (10);
            obj.is_active_speaker = stmt_part.get_column_bool (11);
            obj.is_multiparty_video_capable = stmt_part.get_column_bool (12);
            obj.is_multiparty_video_updatable = stmt_part.get_column_bool (13);
            obj.is_permanent = stmt_part.get_column_bool (14);
            obj.is_premium_video_sponsor = stmt_part.get_column_bool (15);
            obj.is_seamlessly_upgraded_call = stmt_part.get_column_bool (16);
            obj.is_video_codec_compatible = stmt_part.get_column_bool (17);
            obj.last_leavereason = stmt_part.get_column_int64 (18);
            obj.last_voice_error = stmt_part.get_column_string (19);
            obj.live_country = stmt_part.get_column_string (20);
            obj.live_fwd_identities = stmt_part.get_column_string (21);
            obj.live_identity = stmt_part.get_column_string (22);
            obj.live_identity_to_use = stmt_part.get_column_string (23);
            obj.live_ip_address = stmt_part.get_column_string (24);
            obj.live_price_for_me = stmt_part.get_column_string (25);
            obj.live_start_timestamp = stmt_part.get_column_int64 (26);
            obj.live_type = stmt_part.get_column_int64 (27);
            obj.live_voicechannel = stmt_part.get_column_int64 (28);
            obj.livesession_fallback_in_progress =
                stmt_part.get_column_int64 (29);
            obj.livesession_recovery_in_progress =
                stmt_part.get_column_int64 (30);
            obj.messaging_mode = stmt_part.get_column_int64 (31);
            obj.next_redial_time = stmt_part.get_column_int64 (32);
            obj.nrof_redials_left = stmt_part.get_column_int64 (33);
            obj.quality_problems = stmt_part.get_column_string (34);
            obj.rank = stmt_part.get_column_int64 (35);
            obj.read_horizon = stmt_part.get_column_int64 (36);
            obj.real_identity = stmt_part.get_column_string (37);
            obj.requested_rank = stmt_part.get_column_int64 (38);
            obj.sound_level = stmt_part.get_column_int64 (39);
            obj.sponsor = stmt_part.get_column_string (40);
            obj.text_status = stmt_part.get_column_int64 (41);
            obj.transferred_by = stmt_part.get_column_string (42);
            obj.transferred_to = stmt_part.get_column_string (43);
            obj.video_status = stmt_part.get_column_int64 (44);
            obj.voice_status = stmt_part.get_column_int64 (45);

            // Add participants to the list
            message_participants_.emplace (obj.convo_id, std::move (obj));
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load Messages
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_main_db::_load_messages (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Messages table was dropped between schema version 196 and 209
    if (schema_version_ > 196)
        return;

    try
    {
        // Prepare SQL statement for table Messages
        auto stmt = db.new_select_statement (
            "Messages", {"annotation_version",
                         "author",
                         "author_was_live",
                         "body_is_rawxml",
                         "body_xml",
                         "bots_settings",
                         "call_guid",
                         "chatmsg_status",
                         "chatmsg_type",
                         "chatname",
                         "consumption_status",
                         "content_flags",
                         "convo_id",
                         "crc",
                         "dialog_partner",
                         "edited_by",
                         "edited_timestamp",
                         "error_code",
                         "extprop_chatmsg_ft_index_timestamp",
                         "extprop_chatmsg_is_pending",
                         "extprop_contact_received_stamp",
                         "extprop_contact_review_date",
                         "extprop_contact_reviewed",
                         "extprop_mms_msg_metadata",
                         "extprop_sms_server_id",
                         "extprop_sms_src_msg_id",
                         "extprop_sms_sync_global_id",
                         "from_dispname",
                         "guid",
                         "id",
                         "identities",
                         "is_permanent",
                         "language",
                         "leavereason",
                         "newoptions",
                         "newrole",
                         "oldoptions",
                         "option_bits",
                         "param_key",
                         "param_value",
                         "participant_count",
                         "pk_id",
                         "reaction_thread",
                         "reason",
                         "remote_id",
                         "sending_status",
                         "server_id",
                         "timestamp",
                         "timestamp__ms",
                         "type"}
        );

        // Retrieve records from Messages table
        std::int64_t idx = 0;

        while (stmt.fetch_row ())
        {
            message obj;

            obj.idx = idx++;
            obj.annotation_version = stmt.get_column_int64 (0);
            obj.author = stmt.get_column_string (1);
            obj.author_was_live = stmt.get_column_int64 (2);
            obj.body_is_rawxml = stmt.get_column_int64 (3);
            obj.body_xml =
                mobius::core::string::strip (stmt.get_column_string (4));
            obj.bots_settings = stmt.get_column_string (5);
            obj.call_guid = stmt.get_column_string (6);
            obj.chatmsg_status = stmt.get_column_int64 (7);
            obj.chatmsg_type = stmt.get_column_int64 (8);
            obj.chatname = stmt.get_column_string (9);
            obj.consumption_status = stmt.get_column_int64 (10);
            obj.content_flags = stmt.get_column_int64 (11);
            obj.convo_id = stmt.get_column_int64 (12);
            obj.crc = stmt.get_column_int64 (13);
            obj.dialog_partner = stmt.get_column_string (14);
            obj.edited_by = stmt.get_column_string (15);
            obj.edited_timestamp = get_datetime (stmt.get_column_int64 (16));
            obj.error_code = stmt.get_column_int64 (17);
            obj.extprop_chatmsg_ft_index_timestamp =
                get_datetime (stmt.get_column_int64 (18));
            obj.extprop_chatmsg_is_pending = stmt.get_column_int64 (19);
            obj.extprop_contact_received_stamp = stmt.get_column_int64 (20);
            obj.extprop_contact_review_date = stmt.get_column_string (21);
            obj.extprop_contact_reviewed = stmt.get_column_int64 (22);
            obj.extprop_mms_msg_metadata = stmt.get_column_string (23);
            obj.extprop_sms_server_id = stmt.get_column_string (24);
            obj.extprop_sms_src_msg_id = stmt.get_column_string (25);
            obj.extprop_sms_sync_global_id = stmt.get_column_string (26);
            obj.from_dispname = stmt.get_column_string (27);
            obj.guid = stmt.get_column_bytearray (28);
            obj.id = stmt.get_column_int64 (29);
            obj.identities = stmt.get_column_string (30);
            obj.is_permanent = stmt.get_column_bool (31);
            obj.language = stmt.get_column_string (32);
            obj.leavereason = stmt.get_column_int64 (33);
            obj.newoptions = stmt.get_column_int64 (34);
            obj.newrole = stmt.get_column_int64 (35);
            obj.oldoptions = stmt.get_column_int64 (36);
            obj.option_bits = stmt.get_column_int64 (37);
            obj.param_key = stmt.get_column_int64 (38);
            obj.param_value = stmt.get_column_int64 (39);
            obj.participant_count = stmt.get_column_int64 (40);
            obj.pk_id = stmt.get_column_int64 (41);
            obj.reaction_thread = stmt.get_column_string (42);
            obj.reason = stmt.get_column_string (43);
            obj.remote_id = stmt.get_column_int64 (44);
            obj.sending_status = stmt.get_column_int64 (45);
            obj.server_id = stmt.get_column_int64 (46);
            obj.timestamp = get_datetime (stmt.get_column_int64 (47));
            obj.timestamp__ms = stmt.get_column_int64 (48);
            obj.type = stmt.get_column_int64 (49);

            // Add system message if applicable
            message_parser parser (obj.body_xml);
            auto sys_message = SYSTEM_MESSAGES.find (obj.type);

            if (sys_message != SYSTEM_MESSAGES.end ())
                parser.add_system_element (sys_message->second);

            // Parse message content
            parser.parse ();
            obj.content = parser.get_content ();

            if (obj.content.empty ())
            {
                obj.content = {mobius::core::pod::map {
                    {"type", "text"},
                    {"text", obj.body_xml}
                }};
            }

            // Add messages to the list
            messages_.emplace_back (std::move (obj));
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load SMSes
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_main_db::_load_sms (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // SMSes table was dropped between schema version 259 and 308
    if (schema_version_ > 259)
        return;

    try
    {
        // Prepare SQL statement for table SMSes
        auto stmt = db.new_select_statement (
            "SMSes", {"body",
                      "chatmsg_id",
                      "convo_name",
                      "error_category",
                      "event_flags",
                      "extprop_extended",
                      "extprop_hide_from_history",
                      "failurereason",
                      "id",
                      "identity",
                      "is_failed_unseen",
                      "is_permanent",
                      "notification_id",
                      "outgoing_reply_type",
                      "price",
                      "price_currency",
                      "price_precision",
                      "reply_id_number",
                      "reply_to_number",
                      "status",
                      "target_numbers",
                      "target_statuses",
                      "timestamp",
                      "type"}
        );

        // Retrieve records from SMSes table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            sms obj;

            obj.idx = idx++;
            obj.body = stmt.get_column_string (0);
            obj.chatmsg_id = stmt.get_column_int64 (1);
            obj.convo_name = stmt.get_column_string (2);
            obj.error_category = stmt.get_column_int64 (3);
            obj.event_flags = stmt.get_column_int64 (4);
            obj.extprop_extended = stmt.get_column_int64 (5);
            obj.extprop_hide_from_history = stmt.get_column_int64 (6);
            obj.failurereason = stmt.get_column_int64 (7);
            obj.id = stmt.get_column_int64 (8);
            obj.identity = stmt.get_column_string (9);
            obj.is_failed_unseen = stmt.get_column_bool (10);
            obj.is_permanent = stmt.get_column_bool (11);
            obj.notification_id = stmt.get_column_int64 (12);
            obj.outgoing_reply_type = stmt.get_column_int64 (13);
            obj.price = stmt.get_column_int64 (14);
            obj.price_currency = stmt.get_column_string (15);
            obj.price_precision = stmt.get_column_int64 (16);
            obj.reply_id_number = stmt.get_column_string (17);
            obj.reply_to_number = stmt.get_column_string (18);
            obj.status = stmt.get_column_int64 (19);
            obj.target_numbers = stmt.get_column_string (20);
            obj.target_statuses = stmt.get_column_bytearray (21);
            obj.timestamp = get_datetime (stmt.get_column_int64 (22));
            obj.type = stmt.get_column_int64 (23);

            // Add smses to the list
            sms_.emplace_back (std::move (obj));
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load Voicemails
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_main_db::_load_voicemails (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Voicemails table was dropped between schema version 259 and 308
    if (schema_version_ > 307)
        return;

    try
    {
        // Prepare SQL statement for table Voicemails
        auto stmt = db.new_select_statement (
            "Voicemails", {"allowed_duration",
                           "chatmsg_guid",
                           "convo_id",
                           "duration",
                           "extprop_hide_from_history",
                           "failurereason",
                           "failures",
                           "flags",
                           "id",
                           "is_permanent",
                           "notification_id",
                           "partner_dispname",
                           "partner_handle",
                           "path",
                           "playback_progress",
                           "size",
                           "status",
                           "subject",
                           "timestamp",
                           "type",
                           "vflags",
                           "xmsg"}
        );

        // Retrieve records from Voicemails table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            voicemail obj;

            obj.idx = idx++;
            obj.allowed_duration = stmt.get_column_int64 (0);
            obj.chatmsg_guid = stmt.get_column_bytearray (1).to_guid ();
            obj.convo_id = stmt.get_column_int64 (2);
            obj.duration = stmt.get_column_int64 (3);
            obj.extprop_hide_from_history = stmt.get_column_int64 (4);
            obj.failurereason = stmt.get_column_int64 (5);
            obj.failures = stmt.get_column_int64 (6);
            obj.flags = stmt.get_column_int64 (7);
            obj.id = stmt.get_column_int64 (8);
            obj.is_permanent = stmt.get_column_int64 (9);
            obj.notification_id = stmt.get_column_int64 (10);
            obj.partner_dispname = stmt.get_column_string (11);
            obj.partner_handle = stmt.get_column_string (12);
            obj.path = stmt.get_column_string (13);
            obj.playback_progress = stmt.get_column_int64 (14);
            obj.size = stmt.get_column_int64 (15);
            obj.status = stmt.get_column_int64 (16);
            obj.subject = stmt.get_column_string (17);
            obj.timestamp = get_datetime (stmt.get_column_int64 (18));
            obj.type = stmt.get_column_int64 (19);
            obj.vflags = stmt.get_column_int64 (20);
            obj.xmsg = stmt.get_column_string (21);

            // Add voicemails to the list
            voicemails_.emplace_back (std::move (obj));
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get Message Participants
// @param msg Message object
// @return Vector of Message Participants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<file_main_db::message_participant>
file_main_db::get_message_participants (const message &msg) const
{
    std::vector<message_participant> participants;

    // Get participants from conversation, using convo_id
    if (msg.convo_id)
    {
        auto range = message_participants_.equal_range (msg.convo_id);

        std::transform (
            range.first, range.second, std::back_inserter (participants),
            [] (auto &pair) { return pair.second; }
        );
    }

    // Get participants from identities if convo_id is 0
    else if (!msg.identities.empty ())
    {
        auto identities = mobius::core::string::split (msg.identities);

        std::transform (
            identities.begin (), identities.end (),
            std::back_inserter (participants),
            [] (auto &identity)
            {
                message_participant p;
                p.identity = identity;
                return p;
            }
        );
    }

    // Get participants from chatname if convo_id is 0
    else
    {
        auto chat_participants = get_participants_from_chatname (msg.chatname);

        std::transform (
            chat_participants.begin (), chat_participants.end (),
            std::back_inserter (participants),
            [] (auto &identity)
            {
                message_participant p;
                p.identity = identity;
                return p;
            }
        );
    }

    return participants;
}

} // namespace mobius::extension::app::skype
