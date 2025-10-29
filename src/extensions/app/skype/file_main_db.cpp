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
#include "file_main_db.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <limits>
#include <unordered_set>

#include <iostream>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// main.db file tables
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// - Accounts
//      - about: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - ad_policy: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - added_in_shared_group: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - alertstring: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - aliases: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - assigned_comment: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - assigned_speeddial: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - authorized_time: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - authreq_history: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - authreq_timestamp: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - authrequest_count: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - authrequest_policy: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - availability: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - avatar_image: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - avatar_policy: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - avatar_timestamp: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - birthday: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - buddyblob: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - buddycount_policy: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - capabilities: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - cbl_future: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - cblsyncstatus: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - chat_policy: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - city: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - cobrand_id: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - commitstatus: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - country: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - displayname: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - emails: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - federated_presence_policy: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - flamingo_xmpp_status: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - fullname: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - gender: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - given_authlevel: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - given_displayname: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - homepage: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - id: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - in_shared_group: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - ipcountry: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - is_permanent: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - languages: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - lastonline_timestamp: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - lastused_timestamp: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - liveid_membername: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - logoutreason: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - mood_text: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - mood_timestamp: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - node_capabilities: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - node_capabilities_and: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - nr_of_other_instances: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - nrof_authed_buddies: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - offline_authreq_id: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - offline_callforward: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - option_ui_color: 97, 100, 141, 168, 308
//      - options_change_future: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - owner_under_legal_age: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - partner_channel_status: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - partner_optedout: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - phone_home: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - phone_mobile: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - phone_office: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - phonenumbers_policy: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - profile_attachments: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - profile_timestamp: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - province: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - pstn_call_policy: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - pstnnumber: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - pwdchangestatus: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - received_authrequest: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - refreshing: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - registration_timestamp: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - revoked_auth: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - rich_mood_text: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - roaming_history_enabled: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - sent_authrequest: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - sent_authrequest_serial: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - sent_authrequest_time: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - service_provider_info: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - set_availability: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - shortcircuit_sync: 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - skype_call_policy: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - skypein_numbers: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - skypename: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - skypeout_balance: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - skypeout_balance_currency: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - skypeout_precision: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - stack_version: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - status: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - subscriptions: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - suggested_skypename: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - synced_email: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - timezone: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - timezone_policy: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - type: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - uses_jcs: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - verified_company: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - verified_email: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - voicemail_policy: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
//      - webpresence_policy: 66-67, 72, 77, 81, 84-85, 97, 100, 107, 114, 133, 141, 149, 152-153, 162, 164, 168, 196, 209, 235, 259, 308
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
    61,  62,  63,  64,  65,  68,  69,  70,  71,  73,  74,  75,  76,  78,  79,
    80,  82,  83,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  98,
    99,  101, 102, 103, 104, 105, 106, 108, 109, 110, 111, 112, 113, 115, 116,
    117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131,
    132, 134, 135, 136, 137, 138, 139, 140, 142, 143, 144, 145, 146, 147, 148,
    150, 151, 154, 155, 156, 157, 158, 159, 160, 161, 163, 165, 166, 167, 169,
    170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184,
    185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 197, 198, 199, 200,
    201, 202, 203, 204, 205, 206, 207, 208, 210, 211, 212, 213, 214, 215, 216,
    217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231,
    232, 233, 234, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247,
    248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 260, 261, 262, 263,
    264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278,
    279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293,
    294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307,
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
// @brief Convert Skype timestamp to date/time
// @param timestamp Numerical value representing the timestamp
// @return Date/time object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
get_datetime (std::int64_t timestamp)
{
    return mobius::core::datetime::new_datetime_from_unix_timestamp (timestamp);
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
        auto stmt = db.new_statement (
            "SELECT about, "
            "ad_policy, "
            "added_in_shared_group, "
            "alertstring, "
            "aliases, "
            "assigned_comment, "
            "assigned_speeddial, "
            "authorized_time, "
            "authreq_history, "
            "authreq_timestamp, "
            "authrequest_count, "
            "authrequest_policy, "
            "availability, "
            "avatar_image, "
            "avatar_policy, "
            "avatar_timestamp, "
            "birthday, "
            "buddyblob, "
            "buddycount_policy, "
            "capabilities, "
            "cbl_future, "
            "cblsyncstatus, "
            "chat_policy, "
            "city, "
            "${cobrand_id:1}, "
            "commitstatus, "
            "country, "
            "displayname, "
            "emails, "
            "${federated_presence_policy:1}, "
            "${flamingo_xmpp_status:1}, "
            "fullname, "
            "gender, "
            "given_authlevel, "
            "given_displayname, "
            "homepage, "
            "id, "
            "in_shared_group, "
            "ipcountry, "
            "is_permanent, "
            "languages, "
            "lastonline_timestamp, "
            "lastused_timestamp, "
            "${liveid_membername:1}, "
            "logoutreason, "
            "mood_text, "
            "mood_timestamp, "
            "node_capabilities, "
            "node_capabilities_and, "
            "nr_of_other_instances, "
            "nrof_authed_buddies, "
            "offline_authreq_id, "
            "offline_callforward, "
            "${options_change_future:1}, "
            "owner_under_legal_age, "
            "${partner_channel_status:1}, "
            "partner_optedout, "
            "phone_home, "
            "phone_mobile, "
            "phone_office, "
            "phonenumbers_policy, "
            "profile_attachments, "
            "profile_timestamp, "
            "province, "
            "pstn_call_policy, "
            "pstnnumber, "
            "pwdchangestatus, "
            "received_authrequest, "
            "refreshing, "
            "registration_timestamp, "
            "revoked_auth, "
            "rich_mood_text, "
            "${roaming_history_enabled:1}, "
            "sent_authrequest, "
            "sent_authrequest_serial, "
            "sent_authrequest_time, "
            "service_provider_info, "
            "set_availability, "
            "${shortcircuit_sync:72}, "
            "skype_call_policy, "
            "skypein_numbers, "
            "skypename, "
            "skypeout_balance, "
            "skypeout_balance_currency, "
            "skypeout_precision, "
            "stack_version, "
            "status, "
            "subscriptions, "
            "suggested_skypename, "
            "synced_email, "
            "timezone, "
            "timezone_policy, "
            "type, "
            "${uses_jcs:1}, "
            "${verified_company:1}, "
            "${verified_email:1}, "
            "voicemail_policy, "
            "webpresence_policy "
            "FROM accounts",
            schema_version_
        );

        // Retrieve rows from query
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            account a;

            // Set attributes
            a.idx = idx++;
            a.schema_version = schema_version_;
            a.about = stmt.get_column_string (0);
            a.ad_policy = stmt.get_column_int64 (1);
            a.added_in_shared_group = stmt.get_column_int64 (2);
            a.alertstring = stmt.get_column_string (3);
            a.aliases = stmt.get_column_string (4);
            a.assigned_comment = stmt.get_column_string (5);
            a.assigned_speeddial = stmt.get_column_string (6);
            a.authorized_time = stmt.get_column_int64 (7);
            a.authreq_history = stmt.get_column_bytearray (8);
            a.authreq_timestamp = get_datetime (stmt.get_column_int64 (9));
            a.authrequest_count = stmt.get_column_int64 (10);
            a.authrequest_policy = stmt.get_column_int64 (11);
            a.availability = stmt.get_column_int64 (12);
            a.avatar_image = stmt.get_column_bytearray (13);
            a.avatar_policy = stmt.get_column_int64 (14);
            a.avatar_timestamp = get_datetime (stmt.get_column_int64 (15));
            a.birthday = stmt.get_column_int64 (16);
            a.buddyblob = stmt.get_column_bytearray (17);
            a.buddycount_policy = stmt.get_column_int64 (18);
            a.capabilities = stmt.get_column_bytearray (19);
            a.cbl_future = stmt.get_column_bytearray (20);
            a.cblsyncstatus = stmt.get_column_int64 (21);
            a.chat_policy = stmt.get_column_int64 (22);
            a.city = stmt.get_column_string (23);
            a.cobrand_id = stmt.get_column_int64 (24);
            a.commitstatus = stmt.get_column_int64 (25);
            a.country = stmt.get_column_string (26);
            a.displayname = stmt.get_column_string (27);
            a.emails = stmt.get_column_string (28);
            a.federated_presence_policy = stmt.get_column_int64 (29);
            a.flamingo_xmpp_status = stmt.get_column_int64 (30);
            a.fullname = stmt.get_column_string (31);
            a.gender = stmt.get_column_int64 (32);
            a.given_authlevel = stmt.get_column_int64 (33);
            a.given_displayname = stmt.get_column_string (34);
            a.homepage = stmt.get_column_string (35);
            a.id = stmt.get_column_int64 (36);
            a.in_shared_group = stmt.get_column_int64 (37);
            a.ipcountry = stmt.get_column_string (38);
            a.is_permanent = stmt.get_column_bool (39);
            a.languages = stmt.get_column_string (40);
            a.lastonline_timestamp = get_datetime (stmt.get_column_int64 (41));
            a.lastused_timestamp = get_datetime (stmt.get_column_int64 (42));
            a.liveid_membername = stmt.get_column_string (43);
            a.logoutreason = stmt.get_column_int64 (44);
            a.mood_text = stmt.get_column_string (45);
            a.mood_timestamp = get_datetime (stmt.get_column_int64 (46));
            a.node_capabilities = stmt.get_column_int64 (47);
            a.node_capabilities_and = stmt.get_column_int64 (48);
            a.nr_of_other_instances = stmt.get_column_int64 (49);
            a.nrof_authed_buddies = stmt.get_column_int64 (50);
            a.offline_authreq_id = stmt.get_column_int64 (51);
            a.offline_callforward = stmt.get_column_string (52);
            a.options_change_future = stmt.get_column_bytearray (53);
            a.owner_under_legal_age = stmt.get_column_int64 (54);
            a.partner_channel_status = stmt.get_column_string (55);
            a.partner_optedout = stmt.get_column_string (56);
            a.phone_home = stmt.get_column_string (57);
            a.phone_mobile = stmt.get_column_string (58);
            a.phone_office = stmt.get_column_string (59);
            a.phonenumbers_policy = stmt.get_column_int64 (60);
            a.profile_attachments = stmt.get_column_bytearray (61);
            a.profile_timestamp = get_datetime (stmt.get_column_int64 (62));
            a.province = stmt.get_column_string (63);
            a.pstn_call_policy = stmt.get_column_int64 (64);
            a.pstnnumber = stmt.get_column_string (65);
            a.pwdchangestatus = stmt.get_column_int64 (66);
            a.received_authrequest = stmt.get_column_string (67);
            a.refreshing = stmt.get_column_int64 (68);
            a.registration_timestamp = get_datetime (stmt.get_column_int64 (69));
            a.revoked_auth = stmt.get_column_int64 (70);
            a.rich_mood_text = stmt.get_column_string (71);
            a.roaming_history_enabled = stmt.get_column_bool (72);
            a.sent_authrequest = stmt.get_column_string (73);
            a.sent_authrequest_serial = stmt.get_column_int64 (74);
            a.sent_authrequest_time = stmt.get_column_int64 (75);
            a.service_provider_info = stmt.get_column_string (76);
            a.set_availability = stmt.get_column_int64 (77);
            a.shortcircuit_sync = stmt.get_column_int64 (78);
            a.skype_call_policy = stmt.get_column_int64 (79);
            a.skypein_numbers = stmt.get_column_string (80);
            a.skypename = stmt.get_column_string (81);
            a.skypeout_balance = stmt.get_column_int64 (82);
            a.skypeout_balance_currency = stmt.get_column_string (83);
            a.skypeout_precision = stmt.get_column_int64 (84);
            a.stack_version = stmt.get_column_int64 (85);
            a.status = stmt.get_column_int64 (86);
            a.subscriptions = stmt.get_column_string (87);
            a.suggested_skypename = stmt.get_column_string (88);
            a.synced_email = stmt.get_column_bytearray (89);
            a.timezone = stmt.get_column_int64 (90);
            a.timezone_policy = stmt.get_column_int64 (91);
            a.type = stmt.get_column_int64 (92);
            a.uses_jcs = stmt.get_column_int64 (93);
            a.verified_company = stmt.get_column_bytearray (94);
            a.verified_email = stmt.get_column_bytearray (95);
            a.voicemail_policy = stmt.get_column_int64 (96);
            a.webpresence_policy = stmt.get_column_int64 (97);

            // Add to accounts vector
            accounts_.emplace_back (std::move (a));
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::skype
