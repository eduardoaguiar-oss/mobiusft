#ifndef MOBIUS_EXTENSION_APP_SKYPE_FILE_S4L_DB_HPP
#define MOBIUS_EXTENSION_APP_SKYPE_FILE_S4L_DB_HPP

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
#include <unordered_map>
#include <vector>

namespace mobius::extension::app::skype
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief s4l-*.db file decoder
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_s4l_db
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Account structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct account
    {
        // @brief Skype Name
        std::string skype_name;

        // @brief App Version
        std::string app_version;

        // @brief Birthdate
        std::string birthdate;

        // @brief City
        std::string city;

        // @brief Country
        std::string country;

        // @brief Device ID
        std::string device_id;

        // @brief Emails
        std::vector<std::string> emails;

        // @brief Full Name
        std::string full_name;

        // @brief Gender
        std::int64_t gender = 0;

        // @brief Locale
        std::string locale;

        // @brief Mood text
        std::string mood_text;

        // @brief MS Account ID from Sign In
        std::string msaid_from_signin;

        // @brief MS Account CID
        std::string msa_cid;

        // @brief MS Account CID Hex
        std::string msa_cid_hex;

        // @brief MS Account ID
        std::string msa_id;

        // @brief Phone Numbers
        std::vector<std::string> phone_numbers;

        // @brief Primary Member Name
        std::string primary_member_name;

        // @brief Province
        std::string province;

        // @brief Thumbnail URL
        std::string thumbnail_url;

        // @brief Timezone
        std::string timezone;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Call participant structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct call_participant
    {
        // @brief Skype Name
        std::string skype_name;

        // @brief MRI
        std::string mri;

        // @brief Full Name
        std::string full_name;

        // @brief Type
        std::string type;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Call structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct call
    {
        // @brief Call Attributes
        std::string call_attributes;

        // @brief Call ID
        std::string call_id;

        // @brief Call Direction
        std::string call_direction;

        // @brief Call Type
        std::string call_type;

        // @brief Call State
        std::string call_state;

        // @brief Connect time
        mobius::core::datetime::datetime connect_time;

        // @brief End time
        mobius::core::datetime::datetime end_time;

        // @brief Forwarded info
        std::string forwarded_info;

        // @brief Message ID
        std::string message_id;

        // @brief Message CUID
        std::string message_cuid;

        // @brief NSP PK
        std::string nsp_pk;

        // @brief Originator
        std::string originator;

        // @brief Start time
        mobius::core::datetime::datetime start_time;

        // @brief Session type
        std::string session_type;

        // @brief Target
        std::string target;

        // @brief Originator Participant
        call_participant originator_participant;

        // @brief Participants
        std::vector<call_participant> participants;

        // @brief Target Participant
        call_participant target_participant;

        // @brief Thread ID
        std::string thread_id;

        // @brief Transfer info
        std::string transfer_info;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Contact structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct contact
    {
        // @brief Skype Name
        std::string skype_name;

        // @brief MRI
        std::string mri;

        // @brief Full Name
        std::string full_name;

        // @brief Birthdate
        std::string birthdate;

        // @brief Gender
        std::int64_t gender = 0;

        // @brief Country
        std::string country;

        // @brief Province
        std::string province;

        // @brief City
        std::string city;

        // @brief Emails
        std::vector<std::string> emails;

        // @brief Phone Numbers
        std::vector<std::string> phone_numbers;

        // @brief Mood text
        std::string mood_text;

        // @brief Thumbnail URL
        std::string thumbnail_url;

        // @brief Fetched time
        mobius::core::datetime::datetime fetched_time;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Message structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct message
    {
        // @brief Compose time
        mobius::core::datetime::datetime compose_time;

        // @brief Content
        std::string content;

        // @brief Content type
        std::string content_type;

        // @brief Conversation Identity
        std::string conversation_id;

        // @brief Created time
        mobius::core::datetime::datetime created_time;

        // @brief Creator
        std::string creator;

        // @brief CUID
        std::string cuid;

        // @brief Is ephemeral
        bool is_ephemeral = false;

        // @brief Is my message
        bool is_my_message = false;

        // @brief NSP PK
        std::string nsp_pk;

        // @brief Type
        std::string type;

        // @brief Parsed Content
        std::vector<mobius::core::pod::map> parsed_content;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit file_s4l_db (const mobius::core::io::reader &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if stream is an instance of s4l-*.db file
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
    // @return Account structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    account
    get_account () const
    {
        return acc_;
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
        std::vector<contact> vec;

        for (const auto &pair : contacts_)
            vec.push_back (pair.second);

        return vec;
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

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

    // @brief Schema version
    std::uint32_t schema_version_ = 0;

    // @brief Account
    account acc_;

    // @brief Calls
    std::vector<call> calls_;

    // @brief Contacts
    std::unordered_map<std::string, contact> contacts_;

    // @brief Messages
    std::vector<message> messages_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_account (mobius::core::database::database &);
    void _load_calls (mobius::core::database::database &);
    void _load_contacts (mobius::core::database::database &);
    void _load_messages (mobius::core::database::database &);
};

} // namespace mobius::extension::app::skype

#endif
