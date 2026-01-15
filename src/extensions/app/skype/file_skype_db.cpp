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
#include "file_skype_db.hpp"
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
// skype.db file tables:
//
// - contacts
// - messages
// - sms_messages
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief DEBUG flag
static constexpr bool DEBUG = false;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Last known schema version
static constexpr std::int64_t LAST_KNOWN_SCHEMA_VERSION = 3576;

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

} // namespace

namespace mobius::extension::app::skype
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_skype_db::file_skype_db (const mobius::core::io::reader &reader)
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

        if (schema_version_ > LAST_KNOWN_SCHEMA_VERSION)
        {
            log.development (
                __LINE__,
                "Unhandled schema version: " + std::to_string (schema_version_)
            );
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load data
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        _load_account (db);
        _load_contacts (db);
        _load_corelib_messages (db);
        _load_messages (db);
        _load_sms_messages (db);

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
// @brief Load account
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_skype_db::_load_account (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Load key_value table into a map
        std::unordered_map<std::string, std::string> key_value_map;

        // Prepare SQL statement for table key_value
        auto kv_stmt = db.new_statement ("SELECT key, value FROM key_value");

        // Retrieve records from key_value table
        while (kv_stmt.fetch_row ())
        {
            std::string key = kv_stmt.get_column_string (0);
            std::string value = kv_stmt.get_column_string (1);
            key_value_map[key] = value;
        }

        // Lambda function to get value from key_value_map with default
        auto get_value_or_default = [&key_value_map] (
                                        const std::string &key,
                                        const std::string &default_value = {}
                                    ) -> std::string
        {
            auto it = key_value_map.find (key);
            return (it != key_value_map.end ()) ? it->second : default_value;
        };

        // Set account info
        account_.mri = get_value_or_default ("mePersonMri");
        account_.balance_precision =
            std::stoi (get_value_or_default ("ACCOUNT_BALANCE_PRECISION", "0"));
        account_.balance_currency =
            get_value_or_default ("ACCOUNT_BALANCE_CURRENCY");
        account_.full_name = get_value_or_default ("ACCOUNT_FULLNAME");
        account_.first_name = get_value_or_default ("ACCOUNT_FIRSTNAME");
        account_.last_name = get_value_or_default ("ACCOUNT_LASTNAME");
        account_.mood = get_value_or_default ("ACCOUNT_MOOD");
        account_.avatar_url = get_value_or_default ("ACCOUNT_AVATARURL");
        account_.avatar_file_path =
            get_value_or_default ("ACCOUNT_AVATARFILEPATH");
        account_.conversation_last_sync_time = get_datetime (
            std::stoll (get_value_or_default ("conv_lastsynctime", "0")) / 1000
        );
        account_.last_seen_inbox_timestamp = get_datetime (
            std::stoll (
                get_value_or_default ("last_seen_inbox_timestamp", "0")
            ) /
            1000
        );
        account_.skype_name = get_skype_name_from_mri (account_.mri);
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load contacts
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_skype_db::_load_contacts (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare SQL statement for table contacts
        auto stmt = db.new_select_statement (
            "contacts", {"about_me",
                         "assigned_phonelabel_1",
                         "assigned_phonelabel_2",
                         "assigned_phonelabel_3",
                         "assigned_phonenumber_1",
                         "assigned_phonenumber_2",
                         "assigned_phonenumber_3",
                         "authorized",
                         "avatar_downloaded_from",
                         "avatar_file_path",
                         "avatar_url",
                         "birthday",
                         "blocked",
                         "city",
                         "contact_type",
                         "country",
                         "display_name",
                         "full_name",
                         "gender",
                         "homepage",
                         "is_buddy",
                         "is_favorite",
                         "is_suggested",
                         "mood",
                         "mri",
                         "phone_number_home",
                         "phone_number_mobile",
                         "phone_number_office",
                         "province",
                         "recommendation_json",
                         "recommendation_rank",
                         "unistore_version",
                         "update_version"}
        );

        // Retrieve records from contacts table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            contact obj;

            obj.idx = idx++;
            obj.about_me = stmt.get_column_string (0);
            obj.assigned_phonelabel_1 = stmt.get_column_int64 (1);
            obj.assigned_phonelabel_2 = stmt.get_column_int64 (2);
            obj.assigned_phonelabel_3 = stmt.get_column_int64 (3);
            obj.assigned_phonenumber_1 = stmt.get_column_string (4);
            obj.assigned_phonenumber_2 = stmt.get_column_string (5);
            obj.assigned_phonenumber_3 = stmt.get_column_string (6);
            obj.authorized = stmt.get_column_bytearray (7);
            obj.avatar_downloaded_from = stmt.get_column_string (8);
            obj.avatar_file_path = stmt.get_column_string (9);
            obj.avatar_url = stmt.get_column_string (10);
            obj.birthday = get_birthday (stmt.get_column_int64 (11));
            obj.blocked = stmt.get_column_bool (12);
            obj.city = stmt.get_column_string (13);
            obj.contact_type = stmt.get_column_int64 (14);
            obj.country = stmt.get_column_string (15);
            obj.display_name = stmt.get_column_string (16);
            obj.full_name = stmt.get_column_string (17);
            obj.gender = stmt.get_column_int64 (18);
            obj.homepage = stmt.get_column_string (19);
            obj.is_buddy = stmt.get_column_bool (20);
            obj.is_favorite = stmt.get_column_bool (21);
            obj.is_suggested = stmt.get_column_bool (22);
            obj.mood = stmt.get_column_string (23);
            obj.mri = stmt.get_column_string (24);
            obj.phone_number_home = stmt.get_column_string (25);
            obj.phone_number_mobile = stmt.get_column_string (26);
            obj.phone_number_office = stmt.get_column_string (27);
            obj.province = stmt.get_column_string (28);
            obj.recommendation_json = stmt.get_column_string (29);
            obj.recommendation_rank = stmt.get_column_int64 (30);
            obj.unistore_version = stmt.get_column_int64 (31);
            obj.update_version = stmt.get_column_int64 (32);

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
// @brief Load corelib messages
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_skype_db::_load_corelib_messages (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=
        // Load corelib_conversations table into a multimap
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=
        std::unordered_map<std::int64_t, std::pair<std::string, std::string>>
            conv_identity;

        auto stmt = db.new_statement (
            "SELECT id, identity, given_displayname FROM corelib_conversations"
        );

        // Retrieve records from corelib_conversations table
        while (stmt.fetch_row ())
        {
            std::int64_t id = stmt.get_column_int64 (0);
            std::string identity = stmt.get_column_string (1);
            std::string given_displayname = stmt.get_column_string (2);

            conv_identity.emplace (
                id, std::make_pair (identity, given_displayname)
            );
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=
        // Prepare SQL statement for table corelib_messages
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=
        stmt = db.new_select_statement (
            "corelib_messages", {"annotation_version",
                                 "author",
                                 "author_was_live",
                                 "body_is_rawxml",
                                 "body_xml",
                                 "bots_settings",
                                 "call_guid",
                                 "charmsg_type",
                                 "chatmsg_status",
                                 "chatname",
                                 "consumption_status",
                                 "content_flags",
                                 "convo_id",
                                 "crc",
                                 "dialog_partner",
                                 "edited_by",
                                 "edited_timestamp",
                                 "error_code",
                                 "extprop_mms_msg_metadata",
                                 "extprop_sms_server_id",
                                 "extprop_sms_src_msg_id",
                                 "extprop_sms_sync_global_id",
                                 "from_dispname",
                                 "guid",
                                 "id",
                                 "identities",
                                 "is_parmanent",
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
                                 "timestamp_ms",
                                 "type"}
        );

        // Retrieve records from corelib_messages table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            message obj;

            // Set attributes
            obj.idx = idx++;
            obj.author = stmt.get_column_string (1);
            obj.content = stmt.get_column_string (4);
            obj.convdbid = stmt.get_column_int64 (12);
            obj.editedtime = get_datetime (stmt.get_column_int64 (16));
            obj.id = stmt.get_column_int64 (24);
            obj.sendingstatus = stmt.get_column_int64 (40);
            obj.timestamp = get_datetime (stmt.get_column_int64 (42));
            obj.messagetype = stmt.get_column_int64 (44);

            // Set metadata
            obj.metadata.set ("annotation_version", stmt.get_column_int64 (0));
            obj.metadata.set ("author", stmt.get_column_string (1));
            obj.metadata.set ("author_was_live", stmt.get_column_bool (2));
            obj.metadata.set ("body_is_rawxml", stmt.get_column_int64 (3));
            obj.metadata.set ("body_xml", stmt.get_column_string (4));
            obj.metadata.set ("bots_settings", stmt.get_column_string (5));
            obj.metadata.set ("call_guid", stmt.get_column_string (6));
            obj.metadata.set ("charmsg_type", stmt.get_column_int64 (7));
            obj.metadata.set ("chatmsg_status", stmt.get_column_int64 (8));
            obj.metadata.set ("chatname", stmt.get_column_string (9));
            obj.metadata.set ("consumption_status", stmt.get_column_int64 (10));
            obj.metadata.set ("content_flags", stmt.get_column_int64 (11));
            obj.metadata.set ("convo_id", stmt.get_column_int64 (12));
            obj.metadata.set ("crc", stmt.get_column_int64 (13));
            obj.metadata.set ("dialog_partner", stmt.get_column_string (14));
            obj.metadata.set ("edited_by", stmt.get_column_string (15));
            obj.metadata.set (
                "edited_timestamp", get_datetime (stmt.get_column_int64 (16))
            );
            obj.metadata.set ("error_code", stmt.get_column_int64 (17));
            obj.metadata.set (
                "extprop_mms_msg_metadata", stmt.get_column_string (18)
            );
            obj.metadata.set (
                "extprop_sms_server_id", stmt.get_column_string (19)
            );
            obj.metadata.set (
                "extprop_sms_src_msg_id", stmt.get_column_string (20)
            );
            obj.metadata.set (
                "extprop_sms_sync_global_id", stmt.get_column_string (21)
            );
            obj.metadata.set ("from_dispname", stmt.get_column_string (22));
            obj.metadata.set ("id", stmt.get_column_int64 (24));
            obj.metadata.set ("identities", stmt.get_column_string (25));
            obj.metadata.set ("is_parmanent", stmt.get_column_int64 (26));
            obj.metadata.set ("language", stmt.get_column_string (27));
            obj.metadata.set ("leavereason", stmt.get_column_int64 (28));
            obj.metadata.set ("newoptions", stmt.get_column_int64 (29));
            obj.metadata.set ("newrole", stmt.get_column_int64 (30));
            obj.metadata.set ("oldoptions", stmt.get_column_int64 (31));
            obj.metadata.set ("option_bits", stmt.get_column_int64 (32));
            obj.metadata.set ("param_key", stmt.get_column_int64 (33));
            obj.metadata.set ("param_value", stmt.get_column_int64 (34));
            obj.metadata.set ("participant_count", stmt.get_column_int64 (35));
            obj.metadata.set ("pk_id", stmt.get_column_int64 (36));
            obj.metadata.set ("reaction_thread", stmt.get_column_string (37));
            obj.metadata.set ("reason", stmt.get_column_string (38));
            obj.metadata.set ("remote_id", stmt.get_column_int64 (39));
            obj.metadata.set ("sending_status", stmt.get_column_int64 (40));
            obj.metadata.set ("server_id", stmt.get_column_int64 (41));
            obj.metadata.set ("timestamp", stmt.get_column_int64 (42));
            obj.metadata.set ("timestamp_ms", stmt.get_column_int64 (43));
            obj.metadata.set ("type", stmt.get_column_int64 (44));

            // Parse message content
            try
            {
                message_parser parser (obj.content);
                parser.parse ();

                obj.parsed_content = parser.get_content ();

                if (obj.parsed_content.empty ())
                {
                    obj.content = {mobius::core::pod::map {
                        {"type", "text"},
                        {"text", obj.content}
                    }};
                }
            }
            catch (const std::exception &e)
            {
                log.warning (__LINE__, e.what ());
                log.warning (__LINE__, "Raw message content: " + obj.content);
            }

            // Set conversation data
            auto iter = conv_identity.find (obj.convdbid);

            if (iter != conv_identity.end ())
            {
                obj.conversation_identity = iter->second.first;
                obj.conversation_name = iter->second.second;
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
// @brief Load messages
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_skype_db::_load_messages (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare SQL statement for tables messages and conversations
        auto stmt = db.new_statement (
            "SELECT m.author, "
            "m.clientmessageid, "
            "m.content, "
            "m.convdbid, "
            "m.dbid, "
            "m.editedtime,"
            "m.id, "
            "m.is_preview, "
            "m.json, "
            "m.messagetype, "
            "m.originalarrivaltime, "
            "m.properties, "
            "m.sendingstatus, "
            "m.skypeguid, "
            "m.version, "
            "c.type, "
            "c.id "
            "FROM messages m, conversations c "
            "WHERE c.dbid = m.convdbid "
            "AND m.smsmessagedbid IS NULL"
        );

        // Retrieve records from messages table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            message obj;

            // Set attributes
            obj.idx = idx++;
            obj.author = stmt.get_column_string (0);
            obj.content = stmt.get_column_string (2);
            obj.convdbid = stmt.get_column_int64 (3);
            obj.dbid = stmt.get_column_int64 (4);
            obj.editedtime = get_time (stmt.get_column_int64 (5));
            obj.id = stmt.get_column_int64 (6);
            obj.messagetype = stmt.get_column_int64 (9);
            obj.timestamp = get_datetime (stmt.get_column_int64 (10) / 1000);
            obj.sendingstatus = stmt.get_column_int64 (12);
            obj.conversation_type = stmt.get_column_int64 (15);
            obj.conversation_identity = stmt.get_column_string (16);

            // Set metadata
            obj.metadata.set ("clientmessageid", stmt.get_column_int64 (1));
            obj.metadata.set ("is_preview", stmt.get_column_bool (7));
            obj.metadata.set ("json", stmt.get_column_string (8));
            obj.metadata.set (
                "original_arrival_time",
                get_datetime (stmt.get_column_int64 (10) / 1000)
            );
            obj.metadata.set ("properties", stmt.get_column_string (11));
            obj.metadata.set ("version", stmt.get_column_int64 (14));

            // Set conversation MRI
            obj.conversation_mri = std::format (
                "{}:{}", obj.conversation_type, obj.conversation_identity
            );

            // Parse message content
            try
            {
                message_parser parser (obj.content);
                parser.parse ();

                obj.parsed_content = parser.get_content ();

                if (obj.parsed_content.empty ())
                {
                    obj.content = {mobius::core::pod::map {
                        {"type", "text"},
                        {"text", obj.content}
                    }};
                }
            }
            catch (const std::exception &e)
            {
                log.warning (__LINE__, e.what ());
                log.warning (__LINE__, "Raw message content: " + obj.content);
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
// @brief Load SMS messages
// @param db Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_skype_db::_load_sms_messages (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Prepare SQL statement for tables messages and sms_messages
        auto stmt = db.new_statement (
            "SELECT m.author, "
            "m.clientmessageid, "
            "m.content, "
            "m.convdbid, "
            "m.dbid, "
            "m.editedtime,"
            "m.id, "
            "m.is_preview, "
            "m.json, "
            "m.messagetype, "
            "m.originalarrivaltime, "
            "m.properties, "
            "m.sendingstatus, "
            "m.skypeguid, "
            "m.smsmessagedbid, "
            "m.version, "
            "s.mmsdownloadstatus, "
            "s.smstransportid, "
            "s.smstransportname, "
            "s.unistoreid "
            "FROM messages m, sms_messages s "
            "WHERE m.smsmessagedbid = s.dbid"
        );

        // Retrieve records from messages table
        std::uint64_t idx = 0;

        while (stmt.fetch_row ())
        {
            sms_message obj;

            obj.idx = idx++;
            obj.author = stmt.get_column_string (0);
            obj.clientmessageid = stmt.get_column_int64 (1);
            obj.content = stmt.get_column_string (2);
            obj.convdbid = stmt.get_column_int64 (3);
            obj.dbid = stmt.get_column_int64 (4);
            obj.editedtime = get_time (stmt.get_column_int64 (5));
            obj.id = stmt.get_column_int64 (6);
            obj.is_preview = stmt.get_column_bool (7);
            obj.json = stmt.get_column_string (8);
            obj.messagetype = stmt.get_column_int64 (9);
            obj.original_arrival_time =
                get_datetime (stmt.get_column_int64 (10) / 1000);
            obj.properties = stmt.get_column_string (11);
            obj.sendingstatus = stmt.get_column_int64 (12);
            obj.skypeguid = stmt.get_column_string (13);
            obj.smsmessagedbid = stmt.get_column_int64 (14);
            obj.version = stmt.get_column_int64 (15);
            obj.mmsdownloadstatus = stmt.get_column_int64 (16);
            obj.smstransportid = stmt.get_column_string (17);
            obj.smstransportname = stmt.get_column_string (18);
            obj.unistoreid = stmt.get_column_string (19);

            // Add messages to the list
            sms_messages_.emplace_back (std::move (obj));
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::skype
