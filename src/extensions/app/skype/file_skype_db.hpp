#ifndef MOBIUS_EXTENSION_APP_SKYPE_FILE_SKYPE_DB_HPP
#define MOBIUS_EXTENSION_APP_SKYPE_FILE_SKYPE_DB_HPP

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
// @brief skype.db file decoder
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_skype_db
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Account structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct account
    {
        // @brief MRI
        std::string mri;

        // @brief Skype name
        std::string skype_name;

        // @brief Balance precision
        std::int64_t balance_precision;

        // @brief Balance currency
        std::string balance_currency;

        // @brief Full name
        std::string full_name;

        // @brief First name
        std::string first_name;

        // @brief Last name
        std::string last_name;

        // @brief Mood
        std::string mood;

        // @brief Avatar URL
        std::string avatar_url;

        // @brief Avatar File Path
        std::string avatar_file_path;

        // @brief Conversation last sync time
        mobius::core::datetime::datetime conversation_last_sync_time;

        // @brief Last seen inbox timestamp
        mobius::core::datetime::datetime last_seen_inbox_timestamp;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Contact structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct contact
    {
        // @brief Record index number
        std::uint64_t idx = 0;

        // @brief About Me
        std::string about_me;

        // @brief Assigned Phonelabel 1
        std::int64_t assigned_phonelabel_1;

        // @brief Assigned Phonelabel 2
        std::int64_t assigned_phonelabel_2;

        // @brief Assigned Phonelabel 3
        std::int64_t assigned_phonelabel_3;

        // @brief Assigned Phonenumber 1
        std::string assigned_phonenumber_1;

        // @brief Assigned Phonenumber 2
        std::string assigned_phonenumber_2;

        // @brief Assigned Phonenumber 3
        std::string assigned_phonenumber_3;

        // @brief Authorized
        mobius::core::bytearray authorized;

        // @brief Avatar Downloaded From
        std::string avatar_downloaded_from;

        // @brief Avatar File Path
        std::string avatar_file_path;

        // @brief Avatar Url
        std::string avatar_url;

        // @brief Birthday
        std::string birthday;

        // @brief Blocked
        bool blocked;

        // @brief City
        std::string city;

        // @brief Contact Type
        std::int64_t contact_type;

        // @brief Country
        std::string country;

        // @brief Display Name
        std::string display_name;

        // @brief Full Name
        std::string full_name;

        // @brief Gender
        std::int64_t gender;

        // @brief Homepage
        std::string homepage;

        // @brief Is Buddy
        bool is_buddy;

        // @brief Is Favorite
        bool is_favorite;

        // @brief Is Suggested
        bool is_suggested;

        // @brief Mood
        std::string mood;

        // @brief Mri
        std::string mri;

        // @brief Phone Number Home
        std::string phone_number_home;

        // @brief Phone Number Mobile
        std::string phone_number_mobile;

        // @brief Phone Number Office
        std::string phone_number_office;

        // @brief Province
        std::string province;

        // @brief Recommendation Json
        std::string recommendation_json;

        // @brief Recommendation Rank
        std::int64_t recommendation_rank;

        // @brief Unistore Version
        std::int64_t unistore_version;

        // @brief Update Version
        std::int64_t update_version;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Message structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct message
    {
        // @brief Record index number
        std::uint64_t idx = 0;

        // @brief Author
        std::string author;

        // @brief Content
        std::string content;

        // @brief Convdbid
        std::int64_t convdbid;

        // @brief Dbid
        std::int64_t dbid;

        // @brief Editedtime
        mobius::core::datetime::datetime editedtime;

        // @brief Id
        std::int64_t id;

        // @brief Messagetype
        std::int64_t messagetype;

        // @brief Sendingstatus
        std::int64_t sendingstatus;

        // @brief Timestamp
        mobius::core::datetime::datetime timestamp;

        // @brief Conversation type
        std::int64_t conversation_type;

        // @brief Conversation identity
        std::string conversation_identity;

        // @brief Conversation MRI
        std::string conversation_mri;

        // @brief Conversation name
        std::string conversation_name;

        // @brief Metadata
        mobius::core::pod::map metadata;

        // @brief Parsed content
        std::vector<mobius::core::pod::map> parsed_content;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief SMS structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct sms_message
    {
        // @brief Record index number
        std::uint64_t idx = 0;

        // @brief Author
        std::string author;

        // @brief Clientmessageid
        std::int64_t clientmessageid;

        // @brief Content
        std::string content;

        // @brief Convdbid
        std::int64_t convdbid;

        // @brief Dbid
        std::int64_t dbid;

        // @brief Editedtime
        mobius::core::datetime::datetime editedtime;

        // @brief Id
        std::int64_t id;

        // @brief Is Preview
        bool is_preview = false;

        // @brief Json
        std::string json;

        // @brief Messagetype
        std::int64_t messagetype;

        // @brief Originalarrivaltime
        mobius::core::datetime::datetime original_arrival_time;

        // @brief Properties
        std::string properties;

        // @brief Sendingstatus
        std::int64_t sendingstatus;

        // @brief Skypeguid
        std::string skypeguid;

        // @brief Smsmessagedbid
        std::int64_t smsmessagedbid;

        // @brief Version
        std::int64_t version;

        // @brief Mmsdownloadstatus
        std::int64_t mmsdownloadstatus;

        // @brief Smstransportid
        std::string smstransportid;

        // @brief Smstransportname
        std::string smstransportname;

        // @brief Unistoreid
        std::string unistoreid;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_skype_db (const mobius::core::io::reader &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if stream is an instance of skype.db file
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
    // @brief Get account
    // @return Account
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    account
    get_account () const
    {
        return account_;
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
    std::vector<sms_message>
    get_sms_messages () const
    {
        return sms_messages_;
    }

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

    // @brief Schema version
    std::uint32_t schema_version_ = 0;

    // @brief Account
    account account_;

    // @brief Contacts
    std::vector<contact> contacts_;

    // @brief Messages
    std::vector<message> messages_;

    // @brief SMS messages
    std::vector<sms_message> sms_messages_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_account (mobius::core::database::database &);
    void _load_contacts (mobius::core::database::database &);
    void _load_corelib_messages (mobius::core::database::database &);
    void _load_messages (mobius::core::database::database &);
    void _load_sms_messages (mobius::core::database::database &);
};

} // namespace mobius::extension::app::skype

#endif
