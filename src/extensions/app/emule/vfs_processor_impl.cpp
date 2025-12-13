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
#include "vfs_processor_impl.hpp"
#include <mobius/core/datasource/datasource_vfs.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/io/uri.hpp>
#include <mobius/core/io/walker.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/framework/evidence_flag.hpp>
#include <mobius/framework/model/evidence.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Versions examined: Emule 0.50a and DreaMule 3.2
//
// Emule main forensic files:
// @see http://www.emule-project.net/home/perl/help.cgi?l=1&rm=show_topic&topic_id=106
//
// . AC_SearchStrings.dat: Stores search strings
//
// . Cancelled.met: Files cancelled before completing downloading
//
// . Clients.met: Credit control file. Control credits of each peer (dl and ul)
//
// . Downloads.txt/bak: Summary of .part.met files (part name and url)
//
// . KeyIndex.dat: Kamdelia search result file, with sources, IPs and filenames
//
// . Known.met: Shared files, downloading files, downloaded files
//
// . Preferences.dat: Stores the user GUID for eDonkey network
//
// . PreferencesKad.dat: Stores Kademlia network client ID
//
// . Sharedir.dat: Stores the paths to all shared directories
//
// . Statistics.ini: Stores statistics about program usage
//
// . StoredSearches.met: Stores open searches (ongoing searches)
//
// . *.part.met: information about a file being downloaded (not in known.met)
//
// DreaMule forensic files:
// . *.part.met.txtsrc: list of sources, with IP and expiration date/time
//
// Kademlia forensic files:
// . key_index.dat: stores a chunk of Kademlia's Distributed Hash Table,
//   including search hash value, hits (files) and peers sharing those files
//
// All Date/Times are stored in Coordinated Universal Time (UTC).
// @see https://msdn.microsoft.com/pt-br/library/windows/desktop/ms724397(v=vs.85).aspx
//
// According to eMule Homepage: "Your Incoming and Temporary directory are
// always shared"
// @see https://www.emule-project.net/home/perl/help.cgi?l=1&topic_id=112&rm=show_topic
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string SAMPLING_ID = "sampling";
static const std::string APP_ID = "emule";
static const std::string APP_NAME = "Emule";

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get username from path
// @param path Path to profile
// @return Username extracted from path
//
// @note Paths are in the following format: /FSxx/Users/username/... or
// /FSxx/home/username/... where FSxx is the filesystem identifier.
// Example: /FS01/Users/johndoe/AppData/Local/Google/Chrome/User Data/
// In this case, the username is "johndoe".
// If the path does not match the expected format, an empty string is returned.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_username_from_path (const std::string &path)
{
    auto dirnames = mobius::core::string::split (path, "/");

    if (dirnames.size () > 3 &&
        (dirnames[2] == "Users" || dirnames[2] == "home"))
        return dirnames[3]; // Username is the fourth directory

    return {}; // No username found
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update metadata map, preferring non null values
// @param metadata Metadata map
// @param other Other metadata map
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
update_metadata (
    mobius::core::pod::map &metadata, const mobius::core::pod::map &other
)
{
    for (const auto &[k, v] : other)
    {
        auto old_v = metadata.get (k);

        if (!metadata.contains (k) || (old_v.is_null () && !v.is_null ()))
            metadata.set (k, v);
    }
}

} // namespace

namespace mobius::extension::app::emule
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param item Item object
// @param case_profile Case profile object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
vfs_processor_impl::vfs_processor_impl (
    const mobius::framework::model::item &item,
    const mobius::framework::case_profile &
)
    : item_ (item)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan all subfolders of a folder
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::on_folder (const mobius::core::io::folder &folder)
{
    _scan_profile_folder (folder);
    //_scan_arestra_folder (folder);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Called when processing is complete
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::on_complete ()
{
    auto transaction = item_.new_transaction ();

    _save_app_profiles ();
    _save_autofills ();
    _save_user_accounts ();

    transaction.commit ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for µTorrent/BitTorrent profiles
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_scan_profile_folder (
    const mobius::core::io::folder &folder
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Scan folder
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto w = mobius::core::io::walker (folder);
    profile p;

    for (const auto &[name, f] : w.get_files_with_names ())
    {
        try
        {
            if (name == "preferences.dat")
                p.add_preferences_dat_file (f);

            else if (name == "preferences.ini")
                p.add_preferences_ini_file (f);

            else if (name == "statistics.ini" || name == "statbkup.ini")
                p.add_statistics_ini_file(f);

            else if (name == "preferenceskad.dat")
                p.add_preferenceskad_dat_file (f);

            else if (name == "ac_searchstrings.dat")
                p.add_ac_searchstrings_dat_file (f);

            else if (name == "key_index.dat")
                ; //_decode_key_index_dat_file (f);

            else if (name == "known.met")
                ; //_decode_known_met_file (f);

            else if (name == "storedsearches.met")
                p.add_storedsearches_met_file (f);
        }
        catch (const std::exception &e)
        {
            log.warning (
                __LINE__,
                std::string (e.what ()) + " (file: " + f.get_path () + ")"
            );
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // If we have a new profile, add it to the profiles list
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (p)
        profiles_.push_back (p);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save app profiles
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_app_profiles ()
{
    for (const auto &p : profiles_)
    {
        auto e = item_.new_evidence ("app-profile");

        // Attributes
        e.set_attribute ("app_id", APP_ID);
        e.set_attribute ("app_name", APP_NAME);
        e.set_attribute ("username", p.get_username ());
        e.set_attribute ("creation_time", p.get_creation_time ());
        e.set_attribute ("last_modified_time", p.get_last_modified_time ());
        e.set_attribute ("path", p.get_path ());

        // Metadata
        auto metadata = mobius::core::pod::map ();
        metadata.set ("app_version", p.get_app_version ());
        metadata.set ("auto_start", p.get_auto_start ());
        metadata.set (
            "download_completed_files", p.get_download_completed_files ()
        );
        metadata.set ("emule_guid", p.get_emule_guid ());
        metadata.set ("incoming_dir", p.get_incoming_dir ());
        metadata.set ("kamdelia_guid", p.get_kamdelia_guid ());
        metadata.set ("nick", p.get_nick ());
        metadata.set (
            "preferences_dat_version", p.get_preferences_dat_version ()
        );
        metadata.set ("temp_dir", p.get_temp_dir ());
        metadata.set (
            "total_downloaded_bytes", p.get_total_downloaded_bytes ()
        );
        metadata.set ("total_uploaded_bytes", p.get_total_uploaded_bytes ());

        e.set_attribute ("metadata", metadata);

        // Tags and sources
        e.set_tag ("app.p2p");
        e.add_source (p.get_folder ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save autofill entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_autofills ()
{
    for (const auto &p : profiles_)
    {
        auto username = p.get_username ();

        for (const auto &af : p.get_autofills ())
        {
            mobius::core::pod::map metadata = af.metadata.clone ();
            metadata.set ("id", af.id);

            auto e = item_.new_evidence ("autofill");

            e.set_attribute ("field_name", "search");
            e.set_attribute ("value", af.value);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("username", username);
            e.set_attribute ("is_deleted", af.is_deleted);
            e.set_attribute ("metadata", metadata);

            e.set_tag ("app.p2p");
            e.add_source (af.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save accounts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_user_accounts ()
{
    for (const auto &p : profiles_)
    {
        auto emule_guid = p.get_emule_guid ();
        auto kamdelia_guid = p.get_kamdelia_guid ();

        mobius::core::pod::map metadata;
        metadata.set ("app_id", APP_ID);
        metadata.set ("app_name", APP_NAME);
        metadata.set ("username", p.get_username ());
        metadata.set ("emule_guid", emule_guid);
        metadata.set ("kamdelia_guid", kamdelia_guid);
        metadata.set ("kamdelia_ip", p.get_kamdelia_ip ());
        metadata.set ("incoming_dir", p.get_incoming_dir ());
        metadata.set ("temp_dir", p.get_temp_dir ());
        metadata.set ("nickname", p.get_nick ());
        metadata.set ("app_version", p.get_app_version ());
        metadata.set ("auto_start", p.get_auto_start ());
        metadata.set ("total_downloaded_bytes", p.get_total_downloaded_bytes ());
        metadata.set ("total_uploaded_bytes", p.get_total_uploaded_bytes ());
        metadata.set ("download_completed_files", p.get_download_completed_files ());

        if (!emule_guid.empty ())
        {
            auto e = item_.new_evidence ("user-account");

            e.set_attribute ("account_type", "p2p.edonkey");
            e.set_attribute ("id", emule_guid);
            e.set_attribute ("password", {});
            e.set_attribute ("password_found", "no");
            e.set_attribute ("metadata", metadata.clone ());
            e.set_tag ("app.p2p");

            for (const auto &sf : p.get_source_files ())
                e.add_source (sf);
        }

        if (!kamdelia_guid.empty ())
        {
            auto e = item_.new_evidence ("user-account");

            e.set_attribute ("account_type", "p2p.kamdelia");
            e.set_attribute ("id", kamdelia_guid);
            e.set_attribute ("password", {});
            e.set_attribute ("password_found", "no");
            e.set_attribute ("metadata", metadata.clone ());
            e.set_tag ("app.p2p");

            for (const auto &sf : p.get_source_files ())
                e.add_source (sf);
        }
    }
}

} // namespace mobius::extension::app::emule
