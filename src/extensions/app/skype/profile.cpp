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
#include "profile.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/framework/utils.hpp>
#include <format>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "common.hpp"
#include "file_main_db.hpp"
#include "file_s4l_db.hpp"
#include "file_skype_db.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
//
// @see https://sqliteforensictoolkit.com/using-group_concat-to-amalgamate-the-results-of-queries/
// @see https://arxiv.org/pdf/1603.05369.pdf
// @see https://answers.microsoft.com/en-us/skype/forum/all/where-is-the-maindb-file-for-new-skype/b4d3f263-a97e-496e-aa28-e1dbb63e768
// @see https://bebinary4n6.blogspot.com/2019/07/
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Call status domain
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
const std::map<std::int64_t, std::string> CALL_STATUS_DOMAIN = {
    {6, "Accepted"},
    {8, "Rejected at destination"},
    {13, "Cancelled at origin"},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Gender domain
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
const std::map<std::int64_t, std::string> GENDER_DOMAIN = {
    {1, "Male"}, {2, "Female"}
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief SMS status domain
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
const std::map<std::int64_t, std::string> SMS_STATUS_DOMAIN = {
    {1, "Draft"},  {2, "Outbox"},   {3, "Sent"},
    {4, "Failed"}, {5, "Received"}, {6, "Deleted"},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Transfer status domain
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
const std::map<std::int64_t, std::string> TRANSFER_STATUS_DOMAIN = {
    {0, "Not initiated"}, {7, "Cancelled"}, {8, "Completed"}, {9, "Error"}
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get domain value
// @param domain Domain map
// @param code Code to lookup
// @return Domain value string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_domain_value (
    const std::map<std::int64_t, std::string> &domain, std::int64_t code
)
{
    auto it = domain.find (code);

    if (it != domain.end ())
        return it->second;

    else
        return std::format ("Unknown ({})", code);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert duration to string
// @param duration Duration in seconds
// @return Duration string in format HH:MM:SS
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_duration (std::int64_t duration)
{
    auto hh = duration / 3600;
    auto mm = (duration % 3600) / 60;
    auto ss = duration % 60;
    return std::format ("{:02}:{:02}:{:02}", hh, mm, ss);
}

} // namespace

namespace mobius::extension::app::skype
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class profile::impl
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if profile is valid
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_valid () const
    {
        return bool (source_);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get username
    // @return username
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_username () const
    {
        return username_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get source entry
    // @return Source entry
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::io::entry
    get_source () const
    {
        return source_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get path to profile
    // @return Path to profile
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_path () const
    {
        return path_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get creation time
    // @return Creation time
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_creation_time () const
    {
        return creation_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get last modified time
    // @return Last modified time
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_last_modified_time () const
    {
        return last_modified_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get account MRI
    // @return Account MRI
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_account_mri () const
    {
        return account_mri_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get account ID
    // @return Account ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_account_id () const
    {
        return account_id_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get account name
    // @return Account name
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_account_name () const
    {
        return account_name_;
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
    // @brief Get number of accounts
    // @return Number of accounts
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::size_t
    size_accounts () const
    {
        return accounts_.size ();
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
    // @brief Get number of calls
    // @return Number of calls
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::size_t
    size_calls () const
    {
        return calls_.size ();
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
    // @brief Get number of contacts
    // @return Number of contacts
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::size_t
    size_contacts () const
    {
        return contacts_.size ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get transfers
    // @return Vector of file transfers
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<file_transfer>
    get_file_transfers () const
    {
        return file_transfers_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of file transfers
    // @return Number of transfers
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::size_t
    size_file_transfers () const
    {
        return file_transfers_.size ();
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
    // @brief Get number of messages
    // @return Number of messages
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::size_t
    size_messages () const
    {
        return messages_.size ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get remote party IP addresses
    // @return Vector of remote party IP addresses
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<remote_party_ip_address>
    get_remote_party_ip_addresses () const
    {
        return remote_party_ip_addresses_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of remote party IP addresses
    // @return Number of remote party IP addresses
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::size_t
    size_remote_party_ip_addresses () const
    {
        return remote_party_ip_addresses_.size ();
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
    // @brief Get number of SMS messages
    // @return Number of SMS messages
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::size_t
    size_sms_messages () const
    {
        return sms_.size ();
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

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of voicemails
    // @return Number of voicemails
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::size_t
    size_voicemails () const
    {
        return voicemails_.size ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void add_main_db_file (const mobius::core::io::file &);
    void add_skype_db_file (const mobius::core::io::file &);
    void add_s4l_db_file (const mobius::core::io::file &);

  private:
    // @brief Source entry
    mobius::core::io::entry source_;

    // @brief Path to profile
    std::string path_;

    // @brief Username
    std::string username_;

    // @brief Creation time
    mobius::core::datetime::datetime creation_time_;

    // @brief Last modified time
    mobius::core::datetime::datetime last_modified_time_;

    // @brief Account MRI
    std::string account_mri_;

    // @brief Account ID
    std::string account_id_;

    // @brief Account name
    std::string account_name_;

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

    // @brief Remote party IP addresses
    std::vector<remote_party_ip_address> remote_party_ip_addresses_;

    // @brief SMS
    std::vector<sms> sms_;

    // @brief Voicemails
    std::vector<voicemail> voicemails_;

    // @brief Skype usernames cache
    std::unordered_map<std::string, std::string> skypename_cache_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _set_entry (const mobius::core::io::entry &);
    void _update_mtime (const mobius::core::io::file &);
    std::string _get_skypename (const std::string &) const;
    std::string _get_account_name (const std::string &) const;
    void _set_skypename (const std::string &, const std::string &);
    void _normalize_data ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // main.db file helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_main_db_accounts (
        const file_main_db &, const mobius::core::io::file &
    );
    void
    _load_main_db_calls (const file_main_db &, const mobius::core::io::file &);
    void _load_main_db_contacts (
        const file_main_db &, const mobius::core::io::file &
    );
    void _load_main_db_file_transfers (
        const file_main_db &, const mobius::core::io::file &
    );
    void _load_main_db_messages (
        const file_main_db &, const mobius::core::io::file &
    );
    void _load_main_db_sms_messages (
        const file_main_db &, const mobius::core::io::file &
    );
    void _load_main_db_voicemails (
        const file_main_db &, const mobius::core::io::file &
    );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // skype.db file helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_skype_db_account (
        const file_skype_db &, const mobius::core::io::file &
    );
    void _load_skype_db_contacts (
        const file_skype_db &, const mobius::core::io::file &
    );
    void _load_skype_db_sms_messages (
        const file_skype_db &, const mobius::core::io::file &
    );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // s4l-xxx.db file helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    _load_s4l_db_accounts (const file_s4l_db &, const mobius::core::io::file &);
    void
    _load_s4l_db_calls (const file_s4l_db &, const mobius::core::io::file &);
    void
    _load_s4l_db_contacts (const file_s4l_db &, const mobius::core::io::file &);
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set source entry
// @param e Source entry
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_set_entry (const mobius::core::io::entry &e)
{
    if (source_ || !e)
        return;

    source_ = e;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get data from folder
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    path_ = e.get_path ();
    username_ = mobius::framework::get_username_from_path (e.get_path ());

    if (e.is_folder ())
    {
        auto f = e.get_folder ();
        last_modified_time_ = f.get_modification_time ();
        creation_time_ = f.get_creation_time ();

        mobius::core::emit (
            "sampling_folder", std::string ("app.skype.profiles"), f
        );
    }
    else
    {
        auto f = e.get_file ();
        last_modified_time_ = f.get_modification_time ();
        creation_time_ = f.get_creation_time ();
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update last modified time based on file
// @param f File
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_update_mtime (const mobius::core::io::file &f)
{
    if (!f)
        return;

    if (!last_modified_time_ ||
        f.get_modification_time () > last_modified_time_)
        last_modified_time_ = f.get_modification_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get skype user name
// @param Skype name
// @return User name
// If skype name found in cache, return "Full Name (skype_name)". Otherwise,
// return skype_name as is.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
profile::impl::_get_skypename (const std::string &skype_name) const
{
    std::string full_name;

    if (!skype_name.empty ())
    {
        auto it = skypename_cache_.find (skype_name);

        if (it != skypename_cache_.end ())
        {
            full_name = it->second;

            if (!full_name.empty () && full_name != skype_name)
                full_name += " (" + skype_name + ")";
        }
        else
            full_name = skype_name;
    }

    return full_name;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get accout name
// @param Skype name
// @return Account name, if found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
profile::impl::_get_account_name (const std::string &skype_name) const
{
    auto it = skypename_cache_.find (skype_name);

    if (it != skypename_cache_.end ())
        return it->second;

    return {};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set skype user name in cache
// @param Skype name
// @param Full name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_set_skypename (
    const std::string &skype_name, const std::string &full_name
)
{
    if (!skype_name.empty () && !full_name.empty () && skype_name != full_name)
        skypename_cache_[skype_name] = full_name;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add main.db file
// @param f Main.db file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_main_db_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_main_db fm (f.new_reader ());

        if (!fm)
        {
            log.info (__LINE__, "File is not a valid 'main.db' file");
            return;
        }

        log.info (__LINE__, "File decoded [main.db]: " + f.get_path ());

        _set_entry (mobius::core::io::entry (f.get_parent ()));
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load data
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        _load_main_db_accounts (fm, f);
        _load_main_db_calls (fm, f);
        _load_main_db_contacts (fm, f);
        _load_main_db_file_transfers (fm, f);
        _load_main_db_messages (fm, f);
        _load_main_db_sms_messages (fm, f);
        _load_main_db_voicemails (fm, f);
        _normalize_data ();

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file",
            "app.skype.main_db." +
                mobius::core::string::to_string (fm.get_schema_version (), 5),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load main.db accounts
// @param fm Main.db file
// @param f Original file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_main_db_accounts (
    const file_main_db &fm, const mobius::core::io::file &f
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        for (const auto &acc : fm.get_accounts ())
        {
            account a;
            a.id = acc.skypename;
            a.name = acc.fullname;

            if (!acc.phone_home.empty ())
                a.phone_numbers.push_back (acc.phone_home);

            if (!acc.phone_office.empty ())
                a.phone_numbers.push_back (acc.phone_office);

            if (!acc.phone_mobile.empty ())
                a.phone_numbers.push_back (acc.phone_mobile);

            if (!acc.emails.empty ())
                a.emails = mobius::core::string::split (acc.emails, " ");

            // Calculate skypeout balance
            double balance = double (acc.skypeout_balance);
            for (int i = 0; i < acc.skypeout_precision; i++)
                balance /= 10.0;

            a.metadata.set ("record_idx", acc.idx);
            a.metadata.set ("schema_version", fm.get_schema_version ());
            a.metadata.set ("about", acc.about);
            a.metadata.set ("ad_policy", acc.ad_policy);
            a.metadata.set ("added_in_shared_group", acc.added_in_shared_group);
            a.metadata.set ("alertstring", acc.alertstring);
            a.metadata.set ("aliases", acc.aliases);
            a.metadata.set ("assigned_comment", acc.assigned_comment);
            a.metadata.set ("assigned_speeddial", acc.assigned_speeddial);
            a.metadata.set ("authorized_time", acc.authorized_time);
            a.metadata.set ("authreq_timestamp", acc.authreq_timestamp);
            a.metadata.set ("authrequest_count", acc.authrequest_count);
            a.metadata.set ("authrequest_policy", acc.authrequest_policy);
            a.metadata.set ("availability", acc.availability);
            a.metadata.set ("avatar_policy", acc.avatar_policy);
            a.metadata.set ("avatar_timestamp", acc.avatar_timestamp);
            a.metadata.set ("birthday", acc.birthday);
            a.metadata.set ("buddycount_policy", acc.buddycount_policy);
            a.metadata.set ("cblsyncstatus", acc.cblsyncstatus);
            a.metadata.set ("chat_policy", acc.chat_policy);
            a.metadata.set ("city", acc.city);
            a.metadata.set ("cobrand_id", acc.cobrand_id);
            a.metadata.set ("commitstatus", acc.commitstatus);
            a.metadata.set ("contactssyncstatus", acc.contactssyncstatus);
            a.metadata.set ("country", acc.country);
            a.metadata.set ("displayname", acc.displayname);
            a.metadata.set (
                "federated_presence_policy", acc.federated_presence_policy
            );
            a.metadata.set ("forward_starttime", acc.forward_starttime);
            a.metadata.set ("flamingo_xmpp_status", acc.flamingo_xmpp_status);
            a.metadata.set ("fullname", acc.fullname);
            a.metadata.set (
                "gender", get_domain_value (GENDER_DOMAIN, acc.gender)
            );
            a.metadata.set ("given_authlevel", acc.given_authlevel);
            a.metadata.set ("given_displayname", acc.given_displayname);
            a.metadata.set (
                "hidden_expression_tabs", acc.hidden_expression_tabs
            );
            a.metadata.set ("homepage", acc.homepage);
            a.metadata.set ("id", acc.id);
            a.metadata.set ("in_shared_group", acc.in_shared_group);
            a.metadata.set ("ipcountry", acc.ipcountry);
            a.metadata.set ("is_permanent", acc.is_permanent);
            a.metadata.set ("languages", acc.languages);
            a.metadata.set ("lastonline_timestamp", acc.lastonline_timestamp);
            a.metadata.set ("lastused_timestamp", acc.lastused_timestamp);
            a.metadata.set ("liveid_membername", acc.liveid_membername);
            a.metadata.set ("logoutreason", acc.logoutreason);
            a.metadata.set ("mood_text", acc.mood_text);
            a.metadata.set ("mood_timestamp", acc.mood_timestamp);
            a.metadata.set ("msa_pmn", acc.msa_pmn);
            a.metadata.set ("node_capabilities", acc.node_capabilities);
            a.metadata.set ("node_capabilities_and", acc.node_capabilities_and);
            a.metadata.set ("nr_of_other_instances", acc.nr_of_other_instances);
            a.metadata.set ("nrof_authed_buddies", acc.nrof_authed_buddies);
            a.metadata.set ("offline_authreq_id", acc.offline_authreq_id);
            a.metadata.set ("offline_callforward", acc.offline_callforward);
            a.metadata.set ("option_ui_color", acc.option_ui_color);
            a.metadata.set ("owner_under_legal_age", acc.owner_under_legal_age);
            a.metadata.set (
                "partner_channel_status", acc.partner_channel_status
            );
            a.metadata.set ("partner_optedout", acc.partner_optedout);
            a.metadata.set ("phonenumbers_policy", acc.phonenumbers_policy);
            a.metadata.set ("profile_timestamp", acc.profile_timestamp);
            a.metadata.set ("province", acc.province);
            a.metadata.set ("pstn_call_policy", acc.pstn_call_policy);
            a.metadata.set ("pstnnumber", acc.pstnnumber);
            a.metadata.set ("pwdchangestatus", acc.pwdchangestatus);
            a.metadata.set ("read_receipt_optout", acc.read_receipt_optout);
            a.metadata.set ("received_authrequest", acc.received_authrequest);
            a.metadata.set ("refreshing", acc.refreshing);
            a.metadata.set (
                "registration_timestamp", acc.registration_timestamp
            );
            a.metadata.set ("revoked_auth", acc.revoked_auth);
            a.metadata.set ("rich_mood_text", acc.rich_mood_text);
            a.metadata.set (
                "roaming_history_enabled", acc.roaming_history_enabled
            );
            a.metadata.set ("sent_authrequest", acc.sent_authrequest);
            a.metadata.set (
                "sent_authrequest_serial", acc.sent_authrequest_serial
            );
            a.metadata.set ("sent_authrequest_time", acc.sent_authrequest_time);
            a.metadata.set ("service_provider_info", acc.service_provider_info);
            a.metadata.set ("set_availability", acc.set_availability);
            a.metadata.set ("shortcircuit_sync", acc.shortcircuit_sync);
            a.metadata.set ("signin_name", acc.signin_name);
            a.metadata.set ("skype_call_policy", acc.skype_call_policy);
            a.metadata.set ("skypein_numbers", acc.skypein_numbers);
            a.metadata.set ("skypename", acc.skypename);
            a.metadata.set ("skypeout_balance", acc.skypeout_balance);
            a.metadata.set (
                "skypeout_balance_currency", acc.skypeout_balance_currency
            );
            a.metadata.set ("skypeout_precision", acc.skypeout_precision);
            a.metadata.set ("stack_version", acc.stack_version);
            a.metadata.set ("status", acc.status);
            a.metadata.set ("subscriptions", acc.subscriptions);
            a.metadata.set ("suggested_skypename", acc.suggested_skypename);
            a.metadata.set ("timezone", acc.timezone);
            a.metadata.set ("timezone_policy", acc.timezone_policy);
            a.metadata.set ("type", acc.type);
            a.metadata.set ("uses_jcs", acc.uses_jcs);
            a.metadata.set ("voicemail_policy", acc.voicemail_policy);
            a.metadata.set ("webpresence_policy", acc.webpresence_policy);

            a.f = f;
            accounts_.push_back (a);

            account_id_ = acc.skypename;
            account_name_ = acc.fullname;
            _set_skypename (acc.skypename, acc.fullname);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load main.db calls
// @param fm Main.db file
// @param f Original file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_main_db_calls (
    const file_main_db &fm, const mobius::core::io::file &f
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        for (const auto &cl : fm.get_calls ())
        {
            call c;

            // Timestamp
            c.timestamp = cl.begin_timestamp;
            if (!c.timestamp)
                c.timestamp = cl.start_timestamp;

            // Caller and callees
            for (const auto &m : cl.call_members)
            {
                _set_skypename (m.identity, m.dispname);

                if (m.type == 1)
                    c.caller = m.identity;

                else if (m.type == 2)
                    c.callees.push_back (m.identity);
            }

            if (cl.is_incoming)
                c.callees.push_back (get_account_id ());

            else
                c.caller = get_account_id ();

            std::sort (c.callees.begin (), c.callees.end ());

            // Other data
            c.duration = get_duration (cl.duration);

            // Metadata
            c.metadata.set ("record_idx", cl.idx);
            c.metadata.set ("schema_version", fm.get_schema_version ());
            c.metadata.set ("access_token", cl.access_token);
            c.metadata.set ("active_members", cl.active_members);
            c.metadata.set ("begin_timestamp", cl.begin_timestamp);
            c.metadata.set ("broadcast_metadata", cl.broadcast_metadata);
            c.metadata.set ("caller_mri_identity", cl.caller_mri_identity);
            c.metadata.set ("conf_participants", cl.conf_participants);
            c.metadata.set (
                "content_sharing_session_count_changed",
                cl.content_sharing_session_count_changed
            );
            c.metadata.set ("conv_dbid", cl.conv_dbid);
            c.metadata.set ("conversation_type", cl.conversation_type);
            c.metadata.set (
                "current_video_audience", cl.current_video_audience
            );
            c.metadata.set ("datachannel_object_id", cl.datachannel_object_id);
            c.metadata.set ("duration", cl.duration);
            c.metadata.set ("endpoint_details", cl.endpoint_details);
            c.metadata.set ("failurecode", cl.failurecode);
            c.metadata.set ("failurereason", cl.failurereason);
            c.metadata.set (
                "forwarding_destination_type", cl.forwarding_destination_type
            );
            c.metadata.set ("host_identity", cl.host_identity);
            c.metadata.set ("id", cl.id);
            c.metadata.set ("incoming_type", cl.incoming_type);
            c.metadata.set ("is_active", cl.is_active);
            c.metadata.set ("is_conference", cl.is_conference);
            c.metadata.set ("is_hostless", cl.is_hostless);
            c.metadata.set ("is_incoming", cl.is_incoming);
            c.metadata.set (
                "is_incoming_one_on_one_video_call",
                cl.is_incoming_one_on_one_video_call
            );
            c.metadata.set ("is_muted", cl.is_muted);
            c.metadata.set ("is_muted_speaker", cl.is_muted_speaker);
            c.metadata.set ("is_on_hold", cl.is_on_hold);
            c.metadata.set ("is_permanent", cl.is_permanent);
            c.metadata.set (
                "is_premium_video_sponsor", cl.is_premium_video_sponsor
            );
            c.metadata.set ("is_server_muted", cl.is_server_muted);
            c.metadata.set ("is_unseen_missed", cl.is_unseen_missed);
            c.metadata.set ("joined_existing", cl.joined_existing);
            c.metadata.set ("leg_id", cl.leg_id);
            c.metadata.set (
                "light_weight_meeting_count_changed",
                cl.light_weight_meeting_count_changed
            );
            c.metadata.set (
                "max_videoconfcall_participants",
                cl.max_videoconfcall_participants
            );
            c.metadata.set ("meeting_details", cl.meeting_details);
            c.metadata.set ("member_count_changed", cl.member_count_changed);
            c.metadata.set ("mike_status", cl.mike_status);
            c.metadata.set ("name", cl.name);
            c.metadata.set ("old_duration", cl.old_duration);
            c.metadata.set ("onbehalfof_mri", cl.onbehalfof_mri);
            c.metadata.set (
                "optimal_remote_videos_in_conference",
                cl.optimal_remote_videos_in_conference
            );
            c.metadata.set ("partner_dispname", cl.partner_dispname);
            c.metadata.set ("partner_handle", cl.partner_handle);
            c.metadata.set (
                "premium_video_is_grace_period",
                cl.premium_video_is_grace_period
            );
            c.metadata.set (
                "premium_video_sponsor_list", cl.premium_video_sponsor_list
            );
            c.metadata.set ("premium_video_status", cl.premium_video_status);
            c.metadata.set ("pstn_number", cl.pstn_number);
            c.metadata.set ("pstn_status", cl.pstn_status);
            c.metadata.set ("quality_problems", cl.quality_problems);
            c.metadata.set ("server_identity", cl.server_identity);
            c.metadata.set ("soundlevel", cl.soundlevel);
            c.metadata.set ("start_timestamp", cl.start_timestamp);
            c.metadata.set (
                "status", get_domain_value (CALL_STATUS_DOMAIN, cl.status)
            );
            c.metadata.set ("technology", cl.technology);
            c.metadata.set ("tenant_id", cl.tenant_id);
            c.metadata.set ("thread_id", cl.thread_id);
            c.metadata.set ("topic", cl.topic);
            c.metadata.set (
                "transfer_failure_reason", cl.transfer_failure_reason
            );
            c.metadata.set ("transfer_status", cl.transfer_status);
            c.metadata.set (
                "transferor_displayname", cl.transferor_displayname
            );
            c.metadata.set ("transferor_mri", cl.transferor_mri);
            c.metadata.set ("transferor_type", cl.transferor_type);
            c.metadata.set ("type", cl.type);
            c.metadata.set ("vaa_input_status", cl.vaa_input_status);
            c.metadata.set ("video_disabled", cl.video_disabled);
            calls_.push_back (c);

            for (const auto &cm : cl.call_members)
            {
                if (!cm.ip_address.empty () && cm.creation_timestamp)
                {
                    remote_party_ip_address rpia;
                    rpia.timestamp = cm.creation_timestamp;
                    rpia.ip_address = cm.ip_address;
                    rpia.user_id = cm.identity;
                    rpia.metadata = c.metadata.clone ();

                    remote_party_ip_addresses_.push_back (rpia);
                }

                if (!cm.ip_address.empty () && cm.start_timestamp &&
                    cm.start_timestamp != cm.creation_timestamp)
                {
                    remote_party_ip_address rpia;
                    rpia.timestamp = cm.start_timestamp;
                    rpia.ip_address = cm.ip_address;
                    rpia.user_id = cm.identity;
                    rpia.metadata = c.metadata.clone ();

                    remote_party_ip_addresses_.push_back (rpia);
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load main.db contacts
// @param fm Main.db file
// @param f Original file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_main_db_contacts (
    const file_main_db &fm, const mobius::core::io::file &f
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        for (const auto &ct : fm.get_contacts ())
        {
            contact c;
            c.id = ct.skypename;
            c.gender = get_domain_value (GENDER_DOMAIN, ct.gender);
            c.birthday = ct.birthday;

            // Get names
            c.name = ct.fullname;
            if (c.name.empty ())
                c.name = ct.displayname;

            // Get phones
            std::set<std::string> phones;

            if (!ct.phone_home.empty ())
                phones.insert (ct.phone_home);

            if (!ct.phone_home_normalized.empty ())
                phones.insert (ct.phone_home_normalized);

            if (!ct.phone_office.empty ())
                phones.insert (ct.phone_office);

            if (!ct.phone_office_normalized.empty ())
                phones.insert (ct.phone_office_normalized);

            if (!ct.phone_mobile.empty ())
                phones.insert (ct.phone_mobile);

            if (!ct.phone_mobile_normalized.empty ())
                phones.insert (ct.phone_mobile_normalized);

            if (!ct.pstnnumber.empty ())
                phones.insert (ct.pstnnumber);

            std::copy (
                phones.begin (), phones.end (),
                std::back_inserter (c.phone_numbers)
            );

            // Get other fields
            c.accounts.push_back (ct.skypename);

            if (!ct.fullname.empty ())
                c.names.push_back (ct.fullname);

            if (!ct.emails.empty ())
                c.emails = mobius::core::string::split (ct.emails, " ");

            if (!ct.homepage.empty ())
                c.web_addresses.push_back (ct.homepage);

            if (!ct.mood_text.empty ())
                c.notes.push_back (ct.mood_text);

            // Set metadata
            c.metadata.set ("record_idx", ct.idx);
            c.metadata.set ("schema_version", fm.get_schema_version ());
            c.metadata.set ("about", ct.about);
            c.metadata.set (
                "account_modification_serial_nr",
                ct.account_modification_serial_nr
            );
            c.metadata.set ("added_in_shared_group", ct.added_in_shared_group);
            c.metadata.set ("alertstring", ct.alertstring);
            c.metadata.set ("aliases", ct.aliases);
            c.metadata.set ("assigned_comment", ct.assigned_comment);
            c.metadata.set ("assigned_phone1", ct.assigned_phone1);
            c.metadata.set ("assigned_phone1_label", ct.assigned_phone1_label);
            c.metadata.set ("assigned_phone2", ct.assigned_phone2);
            c.metadata.set ("assigned_phone2_label", ct.assigned_phone2_label);
            c.metadata.set ("assigned_phone3", ct.assigned_phone3);
            c.metadata.set ("assigned_phone3_label", ct.assigned_phone3_label);
            c.metadata.set ("assigned_speeddial", ct.assigned_speeddial);
            c.metadata.set ("authorized_time", ct.authorized_time);
            c.metadata.set ("authreq_crc", ct.authreq_crc);
            c.metadata.set ("authreq_initmethod", ct.authreq_initmethod);
            c.metadata.set ("authreq_src", ct.authreq_src);
            c.metadata.set ("authreq_timestamp", ct.authreq_timestamp);
            c.metadata.set ("authrequest_count", ct.authrequest_count);
            c.metadata.set ("availability", ct.availability);
            c.metadata.set ("avatar_hiresurl", ct.avatar_hiresurl);
            c.metadata.set ("avatar_hiresurl_new", ct.avatar_hiresurl_new);
            c.metadata.set ("avatar_timestamp", ct.avatar_timestamp);
            c.metadata.set ("avatar_url", ct.avatar_url);
            c.metadata.set ("avatar_url_new", ct.avatar_url_new);
            c.metadata.set ("birthday", ct.birthday);
            c.metadata.set ("buddystatus", ct.buddystatus);
            c.metadata.set (
                "certificate_send_count", ct.certificate_send_count
            );
            c.metadata.set ("city", ct.city);
            c.metadata.set ("contactlist_track", ct.contactlist_track);
            c.metadata.set ("country", ct.country);
            c.metadata.set (
                "dirblob_last_search_time", ct.dirblob_last_search_time
            );
            c.metadata.set ("displayname", ct.displayname);
            c.metadata.set ("external_id", ct.external_id);
            c.metadata.set ("external_system_id", ct.external_system_id);
            c.metadata.set (
                "extprop_can_show_avatar", ct.extprop_can_show_avatar
            );
            c.metadata.set (
                "extprop_contact_ab_uuid", ct.extprop_contact_ab_uuid
            );
            c.metadata.set ("extprop_external_data", ct.extprop_external_data);
            c.metadata.set (
                "extprop_last_sms_number", ct.extprop_last_sms_number
            );
            c.metadata.set (
                "extprop_must_hide_avatar", ct.extprop_must_hide_avatar
            );
            c.metadata.set ("extprop_seen_birthday", ct.extprop_seen_birthday);
            c.metadata.set (
                "extprop_sms_pstn_contact_created",
                ct.extprop_sms_pstn_contact_created
            );
            c.metadata.set ("extprop_sms_target", ct.extprop_sms_target);
            c.metadata.set (
                "extprop_viral_upgrade_campaign_id",
                ct.extprop_viral_upgrade_campaign_id
            );
            c.metadata.set ("firstname", ct.firstname);
            c.metadata.set ("fullname", ct.fullname);
            c.metadata.set (
                "gender", get_domain_value (GENDER_DOMAIN, ct.gender)
            );
            c.metadata.set ("given_authlevel", ct.given_authlevel);
            c.metadata.set ("given_displayname", ct.given_displayname);
            c.metadata.set ("group_membership", ct.group_membership);
            c.metadata.set ("hashed_emails", ct.hashed_emails);
            c.metadata.set ("homepage", ct.homepage);
            c.metadata.set ("id", ct.id);
            c.metadata.set ("in_shared_group", ct.in_shared_group);
            c.metadata.set ("ipcountry", ct.ipcountry);
            c.metadata.set ("is_auto_buddy", ct.is_auto_buddy);
            c.metadata.set ("is_mobile", ct.is_mobile);
            c.metadata.set ("is_permanent", ct.is_permanent);
            c.metadata.set ("is_trusted", ct.is_trusted);
            c.metadata.set ("isauthorized", ct.isauthorized);
            c.metadata.set ("isblocked", ct.isblocked);
            c.metadata.set ("languages", ct.languages);
            c.metadata.set ("last_used_networktime", ct.last_used_networktime);
            c.metadata.set ("lastname", ct.lastname);
            c.metadata.set ("lastonline_timestamp", ct.lastonline_timestamp);
            c.metadata.set ("lastused_timestamp", ct.lastused_timestamp);
            c.metadata.set ("liveid_cid", ct.liveid_cid);
            c.metadata.set ("main_phone", ct.main_phone);
            c.metadata.set ("mood_text", ct.mood_text);
            c.metadata.set ("mood_timestamp", ct.mood_timestamp);
            c.metadata.set ("mutual_friend_count", ct.mutual_friend_count);
            c.metadata.set ("network_availability", ct.network_availability);
            c.metadata.set ("node_capabilities", ct.node_capabilities);
            c.metadata.set ("node_capabilities_and", ct.node_capabilities_and);
            c.metadata.set ("nr_of_buddies", ct.nr_of_buddies);
            c.metadata.set ("nrof_authed_buddies", ct.nrof_authed_buddies);
            c.metadata.set ("offline_authreq_id", ct.offline_authreq_id);
            c.metadata.set ("phone_home", ct.phone_home);
            c.metadata.set ("phone_home_normalized", ct.phone_home_normalized);
            c.metadata.set ("phone_mobile", ct.phone_mobile);
            c.metadata.set (
                "phone_mobile_normalized", ct.phone_mobile_normalized
            );
            c.metadata.set ("phone_office", ct.phone_office);
            c.metadata.set (
                "phone_office_normalized", ct.phone_office_normalized
            );
            c.metadata.set ("pop_score", ct.pop_score);
            c.metadata.set ("popularity_ord", ct.popularity_ord);
            c.metadata.set ("profile_etag", ct.profile_etag);
            c.metadata.set ("profile_json", ct.profile_json);
            c.metadata.set ("profile_timestamp", ct.profile_timestamp);
            c.metadata.set ("province", ct.province);
            c.metadata.set ("pstnnumber", ct.pstnnumber);
            c.metadata.set ("received_authrequest", ct.received_authrequest);
            c.metadata.set ("refreshing", ct.refreshing);
            c.metadata.set ("revoked_auth", ct.revoked_auth);
            c.metadata.set ("rich_mood_text", ct.rich_mood_text);
            c.metadata.set ("sent_authrequest", ct.sent_authrequest);
            c.metadata.set (
                "sent_authrequest_extrasbitmask",
                ct.sent_authrequest_extrasbitmask
            );
            c.metadata.set (
                "sent_authrequest_initmethod", ct.sent_authrequest_initmethod
            );
            c.metadata.set (
                "sent_authrequest_serial", ct.sent_authrequest_serial
            );
            c.metadata.set ("sent_authrequest_time", ct.sent_authrequest_time);
            c.metadata.set ("server_synced", ct.server_synced);
            c.metadata.set ("skypename", ct.skypename);
            c.metadata.set ("stack_version", ct.stack_version);
            c.metadata.set ("timezone", ct.timezone);
            c.metadata.set ("type", ct.type);
            c.metadata.set ("unified_servants", ct.unified_servants);

            contacts_.push_back (c);
            _set_skypename (c.id, c.name);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load main.db file transfers
// @param fm Main.db file
// @param f Original file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_main_db_file_transfers (
    const file_main_db &fm, const mobius::core::io::file &f
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        for (const auto &ft : fm.get_file_transfers ())
        {
            file_transfer ft_obj;
            ft_obj.timestamp = ft.starttime;
            ft_obj.type = ft.type;
            ft_obj.filename = ft.filename;
            ft_obj.path = ft.filepath;

            ft_obj.metadata.set ("record_idx", ft.idx);
            ft_obj.metadata.set ("schema_version", fm.get_schema_version ());
            ft_obj.metadata.set ("accepttime", ft.accepttime);
            ft_obj.metadata.set ("bytespersecond", ft.bytespersecond);
            ft_obj.metadata.set ("bytestransferred", ft.bytestransferred);
            ft_obj.metadata.set ("chatmsg_guid", ft.chatmsg_guid);
            ft_obj.metadata.set ("chatmsg_index", ft.chatmsg_index);
            ft_obj.metadata.set ("convo_id", ft.convo_id);
            ft_obj.metadata.set (
                "extprop_handled_by_chat", ft.extprop_handled_by_chat
            );
            ft_obj.metadata.set (
                "extprop_hide_from_history", ft.extprop_hide_from_history
            );
            ft_obj.metadata.set (
                "extprop_localfilename", ft.extprop_localfilename
            );
            ft_obj.metadata.set (
                "extprop_transfer_alias", ft.extprop_transfer_alias
            );
            ft_obj.metadata.set (
                "extprop_window_visible", ft.extprop_window_visible
            );
            ft_obj.metadata.set ("failurereason", ft.failurereason);
            ft_obj.metadata.set ("filename", ft.filename);
            ft_obj.metadata.set ("filepath", ft.filepath);
            ft_obj.metadata.set ("filesize", ft.filesize);
            ft_obj.metadata.set ("finishtime", ft.finishtime);
            ft_obj.metadata.set ("flags", ft.flags);
            ft_obj.metadata.set ("id", ft.id);
            ft_obj.metadata.set ("is_permanent", ft.is_permanent);
            ft_obj.metadata.set ("last_activity", ft.last_activity);
            ft_obj.metadata.set ("nodeid", ft.nodeid.to_hexstring ());
            ft_obj.metadata.set ("offer_send_list", ft.offer_send_list);
            ft_obj.metadata.set ("old_filepath", ft.old_filepath);
            ft_obj.metadata.set ("old_status", ft.old_status);
            ft_obj.metadata.set ("parent_id", ft.parent_id);
            ft_obj.metadata.set ("partner_dispname", ft.partner_dispname);
            ft_obj.metadata.set ("partner_handle", ft.partner_handle);
            ft_obj.metadata.set ("pk_id", ft.pk_id);
            ft_obj.metadata.set ("starttime", ft.starttime);
            ft_obj.metadata.set (
                "status", get_domain_value (TRANSFER_STATUS_DOMAIN, ft.status)
            );
            ft_obj.metadata.set ("type", ft.type);

            file_transfers_.push_back (ft_obj);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load main.db messages
// @param fm Main.db file
// @param f Original file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_main_db_messages (
    const file_main_db &fm, const mobius::core::io::file &f
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Load messages
        for (const auto &m : fm.get_messages ())
        {
            _set_skypename (m.author, m.from_dispname);

            message m_obj;
            m_obj.timestamp = m.timestamp;
            m_obj.sender = m.author;
            m_obj.content = m.content;

            for (const auto &p : m.participants)
            {
                if (p.identity != m.author)
                    m_obj.recipients.push_back (p.identity);
            }

            // Metadata
            m_obj.metadata.set ("record_idx", m.idx);
            m_obj.metadata.set ("schema_version", fm.get_schema_version ());
            m_obj.metadata.set ("annotation_version", m.annotation_version);
            m_obj.metadata.set ("author", m.author);
            m_obj.metadata.set ("author_was_live", m.author_was_live);
            m_obj.metadata.set ("body_is_rawxml", m.body_is_rawxml);
            m_obj.metadata.set ("body_xml", m.body_xml);
            m_obj.metadata.set ("bots_settings", m.bots_settings);

            m_obj.metadata.set ("call_guid", m.call_guid);
            m_obj.metadata.set ("chatmsg_status", m.chatmsg_status);
            m_obj.metadata.set ("chatmsg_type", m.chatmsg_type);
            m_obj.metadata.set ("chatname", m.chatname);
            m_obj.metadata.set ("consumption_status", m.consumption_status);
            m_obj.metadata.set ("content_flags", m.content_flags);
            m_obj.metadata.set ("convo_id", m.convo_id);
            m_obj.metadata.set ("crc", m.crc);
            m_obj.metadata.set ("dialog_partner", m.dialog_partner);
            m_obj.metadata.set ("edited_by", m.edited_by);
            m_obj.metadata.set ("edited_timestamp", m.edited_timestamp);
            m_obj.metadata.set ("error_code", m.error_code);
            m_obj.metadata.set (
                "extprop_chatmsg_ft_index_timestamp",
                m.extprop_chatmsg_ft_index_timestamp
            );
            m_obj.metadata.set (
                "extprop_chatmsg_is_pending", m.extprop_chatmsg_is_pending
            );
            m_obj.metadata.set (
                "extprop_contact_received_stamp",
                m.extprop_contact_received_stamp
            );
            m_obj.metadata.set (
                "extprop_contact_review_date", m.extprop_contact_review_date
            );
            m_obj.metadata.set (
                "extprop_contact_reviewed", m.extprop_contact_reviewed
            );
            m_obj.metadata.set (
                "extprop_mms_msg_metadata", m.extprop_mms_msg_metadata
            );
            m_obj.metadata.set (
                "extprop_sms_server_id", m.extprop_sms_server_id
            );
            m_obj.metadata.set (
                "extprop_sms_src_msg_id", m.extprop_sms_src_msg_id
            );
            m_obj.metadata.set (
                "extprop_sms_sync_global_id", m.extprop_sms_sync_global_id
            );
            m_obj.metadata.set ("from_dispname", m.from_dispname);
            m_obj.metadata.set ("guid", m.guid.to_hexstring ());
            m_obj.metadata.set ("id", m.id);
            m_obj.metadata.set ("identities", m.identities);
            m_obj.metadata.set ("is_permanent", m.is_permanent);
            m_obj.metadata.set ("language", m.language);
            m_obj.metadata.set ("leavereason", m.leavereason);
            m_obj.metadata.set ("newoptions", m.newoptions);
            m_obj.metadata.set ("newrole", m.newrole);
            m_obj.metadata.set ("oldoptions", m.oldoptions);
            m_obj.metadata.set ("option_bits", m.option_bits);
            m_obj.metadata.set ("param_key", m.param_key);
            m_obj.metadata.set ("param_value", m.param_value);
            m_obj.metadata.set ("participant_count", m.participant_count);
            m_obj.metadata.set ("pk_id", m.pk_id);
            m_obj.metadata.set ("reaction_thread", m.reaction_thread);
            m_obj.metadata.set ("reason", m.reason);
            m_obj.metadata.set ("remote_id", m.remote_id);
            m_obj.metadata.set ("sending_status", m.sending_status);
            m_obj.metadata.set ("server_id", m.server_id);
            m_obj.metadata.set ("timestamp", m.timestamp);
            m_obj.metadata.set ("timestamp__ms", m.timestamp__ms);
            m_obj.metadata.set ("type", m.type);

            messages_.push_back (m_obj);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load main.db SMS messages
// @param fm Main.db file
// @param f Original file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_main_db_sms_messages (
    const file_main_db &fm, const mobius::core::io::file &f
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        for (const auto &s : fm.get_sms_messages ())
        {
            sms s_obj;
            s_obj.timestamp = s.timestamp;
            s_obj.text = s.body;

            // Sender
            if (s.type == 1)
                s_obj.sender = s.identity;

            else if (s.type == 2)
                s_obj.sender = get_account_id ();

            // Recipients
            auto target_numbers =
                mobius::core::string::split (s.target_numbers);

            std::copy (
                target_numbers.begin (), target_numbers.end (),
                std::back_inserter (s_obj.recipients)
            );

            // Metadata
            s_obj.metadata.set ("record_idx", s.idx);
            s_obj.metadata.set ("schema_version", fm.get_schema_version ());
            s_obj.metadata.set ("body", s.body);
            s_obj.metadata.set ("chatmsg_id", s.chatmsg_id);
            s_obj.metadata.set ("convo_name", s.convo_name);
            s_obj.metadata.set ("error_category", s.error_category);
            s_obj.metadata.set ("event_flags", s.event_flags);
            s_obj.metadata.set ("extprop_extended", s.extprop_extended);
            s_obj.metadata.set (
                "extprop_hide_from_history", s.extprop_hide_from_history
            );
            s_obj.metadata.set ("failurereason", s.failurereason);
            s_obj.metadata.set ("id", s.id);
            s_obj.metadata.set ("identity", s.identity);
            s_obj.metadata.set ("is_failed_unseen", s.is_failed_unseen);
            s_obj.metadata.set ("is_permanent", s.is_permanent);
            s_obj.metadata.set ("notification_id", s.notification_id);
            s_obj.metadata.set ("outgoing_reply_type", s.outgoing_reply_type);
            s_obj.metadata.set ("price", s.price);
            s_obj.metadata.set ("price_currency", s.price_currency);
            s_obj.metadata.set ("price_precision", s.price_precision);
            s_obj.metadata.set ("reply_id_number", s.reply_id_number);
            s_obj.metadata.set ("reply_to_number", s.reply_to_number);
            s_obj.metadata.set (
                "status", get_domain_value (SMS_STATUS_DOMAIN, s.status)
            );
            s_obj.metadata.set ("target_numbers", s.target_numbers);
            s_obj.metadata.set ("type", s.type);

            sms_.push_back (s_obj);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load main.db voicemails
// @param fm Main.db file
// @param f Original file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_main_db_voicemails (
    const file_main_db &fm, const mobius::core::io::file &f
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        for (const auto &vm : fm.get_voicemails ())
        {
            voicemail v;
            v.timestamp = vm.timestamp;
            v.duration = vm.duration;

            v.metadata.set ("record_idx", vm.idx);
            v.metadata.set ("schema_version", fm.get_schema_version ());
            v.metadata.set ("allowed_duration", vm.allowed_duration);
            v.metadata.set ("chatmsg_guid", vm.chatmsg_guid);
            v.metadata.set ("convo_id", vm.convo_id);
            v.metadata.set ("duration", vm.duration);
            v.metadata.set (
                "extprop_hide_from_history", vm.extprop_hide_from_history
            );
            v.metadata.set ("failurereason", vm.failurereason);
            v.metadata.set ("failures", vm.failures);
            v.metadata.set ("flags", vm.flags);
            v.metadata.set ("id", vm.id);
            v.metadata.set ("is_permanent", vm.is_permanent);
            v.metadata.set ("notification_id", vm.notification_id);
            v.metadata.set ("partner_dispname", vm.partner_dispname);
            v.metadata.set ("partner_handle", vm.partner_handle);
            v.metadata.set ("path", vm.path);
            v.metadata.set ("playback_progress", vm.playback_progress);
            v.metadata.set ("size", vm.size);
            v.metadata.set ("status", vm.status);
            v.metadata.set ("subject", vm.subject);
            v.metadata.set ("timestamp", vm.timestamp);
            v.metadata.set ("type", vm.type);
            v.metadata.set ("vflags", vm.vflags);
            v.metadata.set ("xmsg", vm.xmsg);

            voicemails_.push_back (v);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add skype.db file
// @param f Skype.db file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_skype_db_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_skype_db fs (f.new_reader ());

        if (!fs)
        {
            log.info (__LINE__, "File is not a valid 'skype.db' file");
            return;
        }

        log.info (__LINE__, "File decoded [skype.db]: " + f.get_path ());

        _set_entry (mobius::core::io::entry (f.get_parent ()));
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load data
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        _load_skype_db_contacts (fs, f);
        _load_skype_db_sms_messages (fs, f);
        _normalize_data ();

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file",
            "app.skype.skype_db." +
                mobius::core::string::to_string (fs.get_schema_version (), 5),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load skype.db file account
// @param fs Skype.db file
// @param f Original file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_skype_db_account (
    const file_skype_db &fs, const mobius::core::io::file &f
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        const auto &acc = fs.get_account ();

        account_id_ = acc.skype_name;
        account_name_ = acc.full_name;
        account_mri_ = acc.mri;

        account a;
        a.id = acc.skype_name;
        a.name = acc.full_name;

        a.metadata.set ("schema_version", fs.get_schema_version ());
        a.metadata.set ("balance_precision", acc.balance_precision);
        a.metadata.set ("balance_currency", acc.balance_currency);
        a.metadata.set ("mri", acc.mri);
        a.metadata.set ("full_name", acc.full_name);
        a.metadata.set ("first_name", acc.first_name);
        a.metadata.set ("last_name", acc.last_name);
        a.metadata.set ("mood", acc.mood);
        a.metadata.set ("avatar_url", acc.avatar_url);
        a.metadata.set ("avatar_file_path", acc.avatar_file_path);
        a.metadata.set (
            "conversation_last_sync_time", acc.conversation_last_sync_time
        );
        a.metadata.set (
            "last_seen_inbox_timestamp", acc.last_seen_inbox_timestamp
        );

        accounts_.push_back (a);
        _set_skypename (acc.skype_name, acc.full_name);
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load skype.db contacts
// @param fs Skype.db file
// @param f Original file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_skype_db_contacts (
    const file_skype_db &fs, const mobius::core::io::file &f
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        for (const auto &ct : fs.get_contacts ())
        {
            contact c;
            //c.id = ct.skypename;
            c.gender = get_domain_value (GENDER_DOMAIN, ct.gender);
            c.birthday = ct.birthday;

            // Get names
            c.name = ct.full_name;
            if (c.name.empty ())
                c.name = ct.display_name;

            std::set<std::string> names;

            if (!ct.display_name.empty ())
                names.insert (ct.display_name);

            if (!ct.full_name.empty ())
                names.insert (ct.full_name);

            std::copy (
                names.begin (), names.end (), std::back_inserter (c.names)
            );

            // Get phones
            std::set<std::string> phones;

            if (!ct.phone_number_home.empty ())
                phones.insert (ct.phone_number_home);

            if (!ct.phone_number_office.empty ())
                phones.insert (ct.phone_number_office);

            if (!ct.phone_number_mobile.empty ())
                phones.insert (ct.phone_number_mobile);

            if (!ct.assigned_phonenumber_1.empty ())
                phones.insert (ct.assigned_phonenumber_1);

            if (!ct.assigned_phonenumber_2.empty ())
                phones.insert (ct.assigned_phonenumber_2);

            if (!ct.assigned_phonenumber_3.empty ())
                phones.insert (ct.assigned_phonenumber_3);

            std::copy (
                phones.begin (), phones.end (),
                std::back_inserter (c.phone_numbers)
            );

            // Get other fields
            c.accounts.push_back (get_skype_name_from_mri (ct.mri));

            if (!ct.homepage.empty ())
                c.web_addresses.push_back (ct.homepage);

            if (!ct.mood.empty ())
                c.notes.push_back (ct.mood);

            // Set metadata
            c.metadata.set ("record_idx", ct.idx);
            c.metadata.set ("schema_version", fs.get_schema_version ());
            c.metadata.set ("about_me", ct.about_me);
            c.metadata.set ("assigned_phonelabel_1", ct.assigned_phonelabel_1);
            c.metadata.set ("assigned_phonelabel_2", ct.assigned_phonelabel_2);
            c.metadata.set ("assigned_phonelabel_3", ct.assigned_phonelabel_3);
            c.metadata.set (
                "assigned_phonenumber_1", ct.assigned_phonenumber_1
            );
            c.metadata.set (
                "assigned_phonenumber_2", ct.assigned_phonenumber_2
            );
            c.metadata.set (
                "assigned_phonenumber_3", ct.assigned_phonenumber_3
            );
            c.metadata.set ("authorized", ct.authorized);
            c.metadata.set (
                "avatar_downloaded_from", ct.avatar_downloaded_from
            );
            c.metadata.set ("avatar_file_path", ct.avatar_file_path);
            c.metadata.set ("avatar_url", ct.avatar_url);
            c.metadata.set ("birthday", ct.birthday);
            c.metadata.set ("blocked", ct.blocked);
            c.metadata.set ("city", ct.city);
            c.metadata.set ("contact_type", ct.contact_type);
            c.metadata.set ("country", ct.country);
            c.metadata.set ("display_name", ct.display_name);
            c.metadata.set ("full_name", ct.full_name);
            c.metadata.set (
                "gender", get_domain_value (GENDER_DOMAIN, ct.gender)
            );
            c.metadata.set ("homepage", ct.homepage);
            c.metadata.set ("is_buddy", ct.is_buddy);
            c.metadata.set ("is_favorite", ct.is_favorite);
            c.metadata.set ("is_suggested", ct.is_suggested);
            c.metadata.set ("mood", ct.mood);
            c.metadata.set ("mri", ct.mri);
            c.metadata.set ("phone_number_home", ct.phone_number_home);
            c.metadata.set ("phone_number_mobile", ct.phone_number_mobile);
            c.metadata.set ("phone_number_office", ct.phone_number_office);
            c.metadata.set ("province", ct.province);
            c.metadata.set ("recommendation_json", ct.recommendation_json);
            c.metadata.set ("recommendation_rank", ct.recommendation_rank);
            c.metadata.set ("unistore_version", ct.unistore_version);
            c.metadata.set ("update_version", ct.update_version);
            c.f = f;

            contacts_.push_back (c);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load skype.db file SMS messages
// @param fs Skype.db file
// @param f Original file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_skype_db_sms_messages (
    const file_skype_db &fs, const mobius::core::io::file &f
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        for (const auto &s : fs.get_sms_messages ())
        {
            sms s_obj;

            s_obj.timestamp = get_datetime (s.id / 1000);
            s_obj.text = s.content;
            s_obj.sender = s.author;

            // Recipients
            if (s.author == account_mri_)
            {
                log.development (
                    __LINE__, "SMS message sent by account user: " +
                                  _get_skypename (account_id_)
                );
            }
            else
                s_obj.recipients.push_back (account_id_);

            // Metadata
            s_obj.metadata.set ("record_idx", s.idx);
            s_obj.metadata.set ("schema_version", fs.get_schema_version ());
            s_obj.metadata.set ("author", s.author);
            s_obj.metadata.set ("clientmessageid", s.clientmessageid);
            s_obj.metadata.set ("content", s.content);
            s_obj.metadata.set ("convdbid", s.convdbid);
            s_obj.metadata.set ("dbid", s.dbid);
            s_obj.metadata.set ("editedtime", s.editedtime);
            s_obj.metadata.set ("id", s.id);
            s_obj.metadata.set ("is_preview", s.is_preview);
            s_obj.metadata.set ("json", s.json);
            s_obj.metadata.set ("messagetype", s.messagetype);
            s_obj.metadata.set (
                "original_arrival_time", s.original_arrival_time
            );
            s_obj.metadata.set ("properties", s.properties);
            s_obj.metadata.set ("sendingstatus", s.sendingstatus);
            s_obj.metadata.set ("skypeguid", s.skypeguid);
            s_obj.metadata.set ("smsmessagedbid", s.smsmessagedbid);
            s_obj.metadata.set ("smstransportid", s.smstransportid);
            s_obj.metadata.set ("smstransportname", s.smstransportname);
            s_obj.metadata.set ("unistoreid", s.unistoreid);
            sms_.push_back (s_obj);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add s4l-xxx.db file
// @param f s4l-xxx.db file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_s4l_db_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_s4l_db fs (f.new_reader ());

        if (!fs)
        {
            log.info (__LINE__, "File is not a valid 's4l-xxx.db' file");
            return;
        }

        log.info (__LINE__, "File decoded [s4l-xxx.db]: " + f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get data from folder
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        _set_entry (mobius::core::io::entry (f));
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load data
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        _load_s4l_db_accounts (fs, f);
        _load_s4l_db_calls (fs, f);
        _load_s4l_db_contacts (fs, f);
        _normalize_data ();

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file",
            "app.skype.s4l_db." +
                mobius::core::string::to_string (fs.get_schema_version (), 5),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load s4l-xxx.db accounts
// @param fs s4l-xxx.db file
// @param f Original file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_s4l_db_accounts (
    const file_s4l_db &fs, const mobius::core::io::file &f
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        auto acc = fs.get_account ();

        account a;
        a.id = acc.skype_name;
        a.name = acc.full_name;
        a.phone_numbers = acc.phone_numbers;
        a.emails = acc.emails;

        if (!acc.full_name.empty ())
            a.names.push_back (acc.full_name);

        if (!acc.primary_member_name.empty ())
            a.names.push_back (acc.primary_member_name);

        // Metadata
        a.metadata.set ("schema_version", fs.get_schema_version ());
        a.metadata.set ("app_version", acc.app_version);
        a.metadata.set ("birthday", acc.birthdate);
        a.metadata.set ("city", acc.city);
        a.metadata.set ("country", acc.country);
        a.metadata.set ("device_id", acc.device_id);
        a.metadata.set ("full_name", acc.full_name);
        a.metadata.set ("gender", get_domain_value (GENDER_DOMAIN, acc.gender));
        a.metadata.set ("locale", acc.locale);
        a.metadata.set ("mood_text", acc.mood_text);
        a.metadata.set ("ms_account_id", acc.msa_id);
        a.metadata.set ("ms_account_id_from_signin", acc.msaid_from_signin);
        a.metadata.set ("ms_account_cid", acc.msa_cid);
        a.metadata.set ("ms_account_cid_hex", acc.msa_cid_hex);
        a.metadata.set ("primary_member_name", acc.primary_member_name);
        a.metadata.set ("province", acc.province);
        a.metadata.set ("thumbnail_url", acc.thumbnail_url);
        a.metadata.set ("timezone", acc.timezone);

        a.f = f;
        accounts_.push_back (a);
        _set_skypename (acc.skype_name, acc.full_name);
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load s4l-xxx.db calls
// @param fs s4l-xxx.db file
// @param f Original file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_s4l_db_calls (
    const file_s4l_db &fs, const mobius::core::io::file &f
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        for (const auto &cl : fs.get_calls ())
        {
            call c;
            c.timestamp = cl.start_time;

            if (cl.end_time)
                c.duration =
                    get_duration ((cl.end_time - cl.start_time).to_seconds ());

            // Caller and callees
            c.caller = cl.originator_participant.skype_name;
            _set_skypename (
                cl.originator_participant.skype_name,
                cl.originator_participant.full_name
            );

            if (cl.call_type == "twoParty")
            {
                c.callees.push_back (cl.target_participant.skype_name);
                _set_skypename (
                    cl.target_participant.skype_name,
                    cl.target_participant.full_name
                );
            }

            else if (cl.call_type == "multiParty")
            {
                for (const auto &p : cl.participants)
                {
                    if (p.skype_name != cl.originator_participant.skype_name)
                        c.callees.push_back (p.skype_name);
                    _set_skypename (p.skype_name, p.full_name);
                }
            }

            std::sort (c.callees.begin (), c.callees.end ());

            // Metadata
            c.metadata.set ("schema_version", fs.get_schema_version ());
            c.metadata.set ("call_id", cl.call_id);
            c.metadata.set ("call_direction", cl.call_direction);
            c.metadata.set ("call_type", cl.call_type);
            c.metadata.set ("call_state", cl.call_state);
            c.metadata.set ("connect_time", cl.connect_time);
            c.metadata.set ("end_time", cl.end_time);
            c.metadata.set ("message_id", cl.message_id);
            c.metadata.set ("message_cuid", cl.message_cuid);
            c.metadata.set ("nsp_pk", cl.nsp_pk);
            c.metadata.set ("originator", cl.originator);
            c.metadata.set ("session_type", cl.session_type);
            c.metadata.set ("target", cl.target);
            c.metadata.set ("thread_id", cl.thread_id);
            c.f = f;

            calls_.push_back (c);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load s4l-xxx.db contacts
// @param fs s4l-xxx.db file
// @param f Original file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_s4l_db_contacts (
    const file_s4l_db &fs, const mobius::core::io::file &f
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        for (const auto &ct : fs.get_contacts ())
        {
            contact c;
            c.id = ct.skype_name;
            c.name = ct.full_name;
            c.phone_numbers = ct.phone_numbers;
            c.emails = ct.emails;
            c.f = f;

            c.metadata.set ("schema_version", fs.get_schema_version ());
            c.metadata.set ("skype_name", ct.skype_name);
            c.metadata.set ("mri", ct.mri);
            c.metadata.set ("full_name", ct.full_name);
            c.metadata.set ("birthdate", ct.birthdate);
            c.metadata.set (
                "gender", get_domain_value (GENDER_DOMAIN, ct.gender)
            );
            c.metadata.set ("country", ct.country);
            c.metadata.set ("province", ct.province);
            c.metadata.set ("city", ct.city);
            c.metadata.set ("mood_text", ct.mood_text);
            c.metadata.set ("thumbnail_url", ct.thumbnail_url);
            c.metadata.set ("fetched_time", ct.fetched_time);

            contacts_.push_back (c);
            _set_skypename (ct.skype_name, ct.full_name);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Normalize data - Set skypenames where possible, using skype names
// cache already built
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_normalize_data ()
{
    if (account_name_.empty ())
        account_name_ = _get_account_name (account_id_);

    for (auto &cl : calls_)
    {
        // Caller
        cl.caller = _get_skypename (cl.caller);

        // Callees
        for (auto &callee : cl.callees)
            callee = _get_skypename (callee);

        // Sort callees again after normalization
        std::sort (cl.callees.begin (), cl.callees.end ());
    }

    for (auto &c : contacts_)
    {
        if (c.name.empty ())
            c.name = _get_account_name (c.id);
    }

    for (auto &m : messages_)
    {
        // Sender
        m.sender = _get_skypename (m.sender);

        // Recipients
        for (auto &recipient : m.recipients)
            recipient = _get_skypename (recipient);
    }

    for (auto &s : sms_)
    {
        // Sender
        s.sender = _get_skypename (s.sender);

        // Recipients
        for (auto &recipient : s.recipients)
            recipient = _get_skypename (recipient);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
profile::profile ()
    : impl_ (std::make_shared<impl> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if profile is valid
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
profile::
operator bool () const noexcept
{
    return impl_->is_valid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get username
// @return username
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
profile::get_username () const
{
    return impl_->get_username ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get source entry
// @return Source entry
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::entry
profile::get_source () const
{
    return impl_->get_source ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get path to profile
// @return Path to profile
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
profile::get_path () const
{
    return impl_->get_path ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get creation time
// @return Creation time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
profile::get_creation_time () const
{
    return impl_->get_creation_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last modified time
// @return Last modified time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
profile::get_last_modified_time () const
{
    return impl_->get_last_modified_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get account ID
// @return Account ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
profile::get_account_id () const
{
    return impl_->get_account_id ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get account name
// @return Account name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
profile::get_account_name () const
{
    return impl_->get_account_name ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get accounts
// @return Vector of accounts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<profile::account>
profile::get_accounts () const
{
    return impl_->get_accounts ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of accounts
// @return Number of accounts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::size_t
profile::size_accounts () const
{
    return impl_->size_accounts ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get calls
// @return Vector of calls
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<profile::call>
profile::get_calls () const
{
    return impl_->get_calls ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of calls
// @return Number of calls
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::size_t
profile::size_calls () const
{
    return impl_->size_calls ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get contacts
// @return Vector of contacts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<profile::contact>
profile::get_contacts () const
{
    return impl_->get_contacts ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of contacts
// @return Number of contacts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::size_t
profile::size_contacts () const
{
    return impl_->size_contacts ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file transfers
// @return Vector of file transfers
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<profile::file_transfer>
profile::get_file_transfers () const
{
    return impl_->get_file_transfers ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of file transfers
// @return Number of file transfers
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::size_t
profile::size_file_transfers () const
{
    return impl_->size_file_transfers ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get messages
// @return Vector of messages
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<profile::message>
profile::get_messages () const
{
    return impl_->get_messages ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of messages
// @return Number of messages
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::size_t
profile::size_messages () const
{
    return impl_->size_messages ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get remote party IP addresses
// @return Vector of remote party IP addresses
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<profile::remote_party_ip_address>
profile::get_remote_party_ip_addresses () const
{
    return impl_->get_remote_party_ip_addresses ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of remote party IP addresses
// @return Number of remote party IP addresses
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::size_t
profile::size_remote_party_ip_addresses () const
{
    return impl_->size_remote_party_ip_addresses ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get SMS messages
// @return Vector of SMS messages
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<profile::sms>
profile::get_sms_messages () const
{
    return impl_->get_sms_messages ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of SMS messages
// @return Number of SMS messages
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::size_t
profile::size_sms_messages () const
{
    return impl_->size_sms_messages ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get voicemails
// @return Vector of voicemails
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<profile::voicemail>
profile::get_voicemails () const
{
    return impl_->get_voicemails ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of voicemails
// @return Number of voicemails
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::size_t
profile::size_voicemails () const
{
    return impl_->size_voicemails ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add main.db file
// @param f Main.db file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_main_db_file (const mobius::core::io::file &f)
{
    impl_->add_main_db_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add skype.db file
// @param f Skype.db file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_skype_db_file (const mobius::core::io::file &f)
{
    impl_->add_skype_db_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add s4l-xxx.db file
// @param f s4l-xxx.db file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_s4l_db_file (const mobius::core::io::file &f)
{
    impl_->add_s4l_db_file (f);
}

} // namespace mobius::extension::app::skype
