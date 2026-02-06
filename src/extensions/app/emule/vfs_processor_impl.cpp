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
#include <mobius/framework/utils.hpp>
#include "common.hpp"
#include "file_part_met.hpp"
#include "file_part_met_txtsrc.hpp"

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
static const std::string APP_ID = "emule";
static const std::string APP_NAME = "Emule";

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
    _scan_part_met_files (folder);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Called when processing is complete
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::on_complete ()
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Consolidate local files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &p : profiles_)
    {
        auto incoming_dir = p.get_incoming_dir ();

        for (auto &lf : p.get_local_files ())
        {
            if (!incoming_dir.empty ())
                lf.path = incoming_dir + '\\' + lf.filename;
            local_files_.push_back (lf);
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Consolidate remote files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &p : profiles_)
    {
        auto remote_files = p.get_remote_files ();
        std::copy (
            remote_files.begin (), remote_files.end (),
            std::back_inserter (remote_files_)
        );
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Save evidences
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    _save_app_profiles ();
    _save_autofills ();
    _save_local_files ();
    _save_received_files ();
    _save_remote_party_shared_files ();
    _save_sent_files ();
    _save_shared_files ();
    _save_user_accounts ();
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

            else if (name == "preferences.ini" || name == "amule.conf")
                p.add_preferences_ini_file (f);

            else if (name == "statistics.ini" || name == "statbkup.ini")
                p.add_statistics_ini_file (f);

            else if (name == "preferenceskad.dat")
                p.add_preferenceskad_dat_file (f);

            else if (name == "ac_searchstrings.dat")
                p.add_ac_searchstrings_dat_file (f);

            else if (name == "key_index.dat")
                p.add_key_index_dat_file (f);

            else if (name == "known.met")
                p.add_known_met_file (f);

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
// @brief Scan .part.met files in folder
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_scan_part_met_files (
    const mobius::core::io::folder &folder
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);
    auto w = mobius::core::io::walker (folder);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode .part.met.txtsrc files first
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        txtsrc_files_.clear ();

        for (const auto &f : w.get_files_by_pattern ("*.part.met.txtsrc"))
            _decode_part_met_txtsrc_file (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode .part.met files
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &f : w.get_files_by_pattern ("*.part.met"))
            _decode_part_met_file (f);

        txtsrc_files_.clear ();
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__,
            std::string (e.what ()) + " (folder: " + folder.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode .part.met file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_decode_part_met_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_part_met part_met (f.new_reader ());

        if (!part_met)
            return;

        log.info (__LINE__, "File decoded [.part.met]: " + f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Create local file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto metadata = get_metadata_from_tags (part_met.get_tags ());

        profile::local_file lf;

        lf.username = mobius::framework::get_username_from_path (f.get_path ());
        lf.path = f.get_path ();
        lf.path.erase (lf.path.size () - 4);
        lf.filename = metadata.get<std::string> ("name");
        lf.f = f;
        std::tie (lf.app_id, lf.app_name) = get_app_from_path (f.get_path ());

        lf.flag_downloaded = true;
        lf.flag_uploaded = metadata.get<std::int64_t> ("uploaded_bytes") > 0;
        lf.flag_shared = mobius::framework::evidence_flag::always;
        lf.flag_corrupted = metadata.get<bool> ("is_corrupted");
        lf.flag_completed = part_met.get_total_gap_size () == 0;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Metadata
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        metadata.set ("file_version", part_met.get_version ());
        metadata.set ("flag_downloaded", to_string (lf.flag_downloaded));
        metadata.set ("flag_uploaded", to_string (lf.flag_uploaded));
        metadata.set ("flag_shared", to_string (lf.flag_shared));
        metadata.set ("flag_corrupted", to_string (lf.flag_corrupted));
        metadata.set ("flag_completed", to_string (lf.flag_completed));
        metadata.set ("timestamp", part_met.get_timestamp ());
        metadata.set ("total_gap_size", part_met.get_total_gap_size ());
        metadata.set ("network", "eDonkey");

        lf.metadata = metadata;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Content hashes
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        std::vector<mobius::core::pod::data> hashes = {
            {"ed2k", mobius::core::string::toupper (part_met.get_hash_ed2k ())}
        };

        auto aich_hash = metadata.get<std::string> ("hash_aich");

        if (!aich_hash.empty ())
            hashes.push_back ({"aich", aich_hash});

        lf.hashes = hashes;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add local file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        local_files_.push_back (lf);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Create remote file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto iter = txtsrc_files_.find (f.get_name () + ".txtsrc");
        if (iter == txtsrc_files_.end ())
            return;

        for (const auto &source : iter->second.sources)
        {
            auto rf = profile::remote_file ();

            rf.username = username_;
            rf.timestamp = f.get_modification_time ();
            rf.ip = source.ip;
            rf.port = source.port;
            rf.filename = lf.filename;
            rf.source_files.push_back (lf.f);
            rf.source_files.push_back (iter->second.f);
            rf.hashes = lf.hashes.clone ();
            rf.metadata = lf.metadata.clone ();

            remote_files_.push_back (rf);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode .part.met.txtsrc file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_decode_part_met_txtsrc_file (
    const mobius::core::io::file &f
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_part_met_txtsrc txtsrc (f.new_reader ());

        if (!txtsrc)
            return;

        log.info (
            __LINE__, "File decoded [.part.met.txtsrc]: " + f.get_path ()
        );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Store .part.met.txtsrc data in map, keyed by file name
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto [iter, _] = txtsrc_files_.try_emplace (f.get_name ());
        iter->second.f = f;

        for (const auto &source : txtsrc.get_sources ())
            iter->second.sources.emplace_back (source.ip, source.port);
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
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
        e.set_attribute ("app_id", p.get_app_id ());
        e.set_attribute ("app_name", p.get_app_name ());
        e.set_attribute ("username", p.get_username ());
        e.set_attribute ("creation_time", p.get_creation_time ());
        e.set_attribute ("last_modified_time", p.get_last_modified_time ());
        e.set_attribute ("path", p.get_path ());

        // Metadata
        auto metadata = mobius::core::pod::map ();
        metadata.set ("num_autofills", p.get_num_autofills ());
        metadata.set ("num_local_files", p.get_num_local_files ());
        metadata.set ("num_remote_files", p.get_num_remote_files ());
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
        auto app_id = p.get_app_id ();
        auto app_name = p.get_app_name ();

        for (const auto &af : p.get_autofills ())
        {
            mobius::core::pod::map metadata = af.metadata.clone ();
            metadata.set ("id", af.id);

            auto e = item_.new_evidence ("autofill");

            e.set_attribute ("field_name", "search");
            e.set_attribute ("value", af.value);
            e.set_attribute ("app_id", app_id);
            e.set_attribute ("app_name", app_name);
            e.set_attribute ("username", username);
            e.set_attribute ("is_deleted", af.is_deleted);
            e.set_attribute ("metadata", metadata);

            e.set_tag ("app.p2p");
            e.add_source (af.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save local files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_local_files ()
{
    for (const auto &lf : local_files_)
    {
        auto e = item_.new_evidence ("local-file");

        e.set_attribute ("username", lf.username);
        e.set_attribute ("filename", lf.filename);
        e.set_attribute ("path", lf.path);
        e.set_attribute ("app_id", lf.app_id);
        e.set_attribute ("app_name", lf.app_name);
        e.set_attribute ("hashes", lf.hashes);
        e.set_attribute ("metadata", lf.metadata);

        e.set_tag ("app.p2p");
        e.add_source (lf.f);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save received files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_received_files ()
{
    for (const auto &lf : local_files_)
    {
        if (lf.flag_downloaded.is_yes ())
        {
            auto e = item_.new_evidence ("received-file");

            e.set_attribute ("username", lf.username);
            e.set_attribute ("filename", lf.filename);
            e.set_attribute ("path", lf.path);
            e.set_attribute ("app_id", lf.app_id);
            e.set_attribute ("app_name", lf.app_name);
            e.set_attribute ("hashes", lf.hashes);
            e.set_attribute ("metadata", lf.metadata);

            e.set_tag ("app.p2p");
            e.add_source (lf.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save remote files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_remote_party_shared_files ()
{
    for (const auto &rf : remote_files_)
    {
        auto e = item_.new_evidence ("remote-party-shared-file");

        e.set_attribute ("timestamp", rf.timestamp);
        e.set_attribute ("ip", rf.ip);
        e.set_attribute ("port", rf.port);
        e.set_attribute ("filename", rf.filename);
        e.set_attribute ("username", rf.username);
        e.set_attribute ("app_id", APP_ID);
        e.set_attribute ("app_name", APP_NAME);
        e.set_attribute ("hashes", rf.hashes);
        e.set_attribute ("metadata", rf.metadata);

        e.set_tag ("app.p2p");

        for (const auto &sf : rf.source_files)
            e.add_source (sf);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save sent files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_sent_files ()
{
    for (const auto &lf : local_files_)
    {
        if (lf.flag_uploaded.is_yes ())
        {
            auto e = item_.new_evidence ("sent-file");

            e.set_attribute ("username", lf.username);
            e.set_attribute ("filename", lf.filename);
            e.set_attribute ("path", lf.path);
            e.set_attribute ("app_id", lf.app_id);
            e.set_attribute ("app_name", lf.app_name);
            e.set_attribute ("hashes", lf.hashes);
            e.set_attribute ("metadata", lf.metadata);

            e.set_tag ("app.p2p");
            e.add_source (lf.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save shared files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_shared_files ()
{
    for (const auto &lf : local_files_)
    {
        if (lf.flag_shared.is_yes () || lf.flag_shared.is_always ())
        {
            auto e = item_.new_evidence ("shared-file");

            e.set_attribute ("username", lf.username);
            e.set_attribute ("filename", lf.filename);
            e.set_attribute ("path", lf.path);
            e.set_attribute ("app_id", lf.app_id);
            e.set_attribute ("app_name", lf.app_name);
            e.set_attribute ("hashes", lf.hashes);
            e.set_attribute ("metadata", lf.metadata);

            e.set_tag ("app.p2p");
            e.add_source (lf.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save user accounts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_user_accounts ()
{
    for (const auto &p : profiles_)
    {
        auto emule_guid = p.get_emule_guid ();
        auto kamdelia_guid = p.get_kamdelia_guid ();

        mobius::core::pod::map metadata;
        metadata.set ("app_id", p.get_app_id ());
        metadata.set ("app_name", p.get_app_name ());
        metadata.set ("username", p.get_username ());
        metadata.set ("emule_guid", emule_guid);
        metadata.set ("kamdelia_guid", kamdelia_guid);
        metadata.set ("kamdelia_ip", p.get_kamdelia_ip ());
        metadata.set ("incoming_dir", p.get_incoming_dir ());
        metadata.set ("temp_dir", p.get_temp_dir ());
        metadata.set ("nickname", p.get_nick ());
        metadata.set ("app_version", p.get_app_version ());
        metadata.set ("auto_start", p.get_auto_start ());
        metadata.set (
            "total_downloaded_bytes", p.get_total_downloaded_bytes ()
        );
        metadata.set ("total_uploaded_bytes", p.get_total_uploaded_bytes ());
        metadata.set (
            "download_completed_files", p.get_download_completed_files ()
        );

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
