#ifndef MOBIUS_EXTENSION_APP_SKYPE_FILE_MAIN_DB_HPP
#define MOBIUS_EXTENSION_APP_SKYPE_FILE_MAIN_DB_HPP

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
#include <mobius/core/database/database.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/reader.hpp>
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
        std::int64_t federated_presence_policy = 0;

        // @brief Flamingo Xmpp Status
        std::int64_t flamingo_xmpp_status = 0;

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
        mobius::core::bytearray options_change_future;

        // @brief Owner Under Legal Age
        std::int64_t owner_under_legal_age = 0;

        // @brief Partner Channel Status
        std::string partner_channel_status;

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
        bool roaming_history_enabled = false;

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
        std::int64_t shortcircuit_sync = 0;

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
        std::int64_t uses_jcs = 0;

        // @brief Verified Company
        mobius::core::bytearray verified_company;

        // @brief Verified Email
        mobius::core::bytearray verified_email;

        // @brief Voicemail Policy
        std::int64_t voicemail_policy = 0;

        // @brief Webpresence Policy
        std::int64_t webpresence_policy = 0;
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

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

    // @brief Schema version
    std::uint32_t schema_version_ = 0;

    // @brief Accounts
    std::vector<account> accounts_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_accounts (mobius::core::database::database &);
};

} // namespace mobius::extension::app::skype

#endif
