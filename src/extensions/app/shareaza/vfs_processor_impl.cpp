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
#include <mobius/core/os/win/registry/hive_data.hpp>
#include <mobius/core/os/win/registry/hive_file.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/framework/evidence_flag.hpp>
#include <mobius/framework/model/evidence.hpp>
#include "CDownload.hpp"
#include "common.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// All Date/Times are stored in Coordinated Universal Time (UTC).
// @see https://msdn.microsoft.com/pt-br/library/windows/desktop/ms724397(v=vs.85).aspx
//
// References:
//   . Shareaza 2.7.10.2 source code
//
// Shareaza main files:
//   . Library1.dat, Library2.dat, Library.dat - Local folders and local files
//
//   . Profile.xml - User accounts configuration file
//
//   . Searches.dat - Search history and search results file
//
//   . Shareaza.db3 - Thumb cache file
//
//   . *.sd files - Download control files (one per downloading file)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string SAMPLING_ID = "sampling";
static const std::string APP_ID = "shareaza";
static const std::string APP_NAME = "Shareaza";

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

} // namespace

namespace mobius::extension::app::shareaza
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
    _scan_ntuser_dat_files (folder);
    _scan_profile_folder (folder);
    _scan_sd_files (folder);
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
        auto local_files = p.get_local_files ();
        std::copy (
            local_files.begin (), local_files.end (),
            std::back_inserter (local_files_)
        );
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
    auto transaction = item_.new_transaction ();

    _save_app_profiles ();
    _save_autofills ();
    _save_local_files ();
    _save_received_files ();
    _save_remote_party_shared_files ();
    _save_searched_texts ();
    _save_sent_files ();
    _save_shared_files ();
    _save_user_accounts ();

    transaction.commit ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for NTUSER.DAT files
// @param folder Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_scan_ntuser_dat_files (
    const mobius::core::io::folder &folder
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);
    mobius::core::io::walker w (folder);

    for (const auto &f : w.get_files_by_name ("ntuser.dat"))
        _decode_ntuser_dat_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode NTUSER.DAT file
// @param f NTUSER.DAT file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_decode_ntuser_dat_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Create decoder
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto decoder =
            mobius::core::os::win::registry::hive_file (f.new_reader ());

        if (!decoder.is_instance ())
        {
            log.info (__LINE__, "File " + f.get_path () + " ignored.");
            return;
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get evidences from Shareaza key
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        const auto &root_key = decoder.get_root_key ();
        const auto &shareaza_key =
            root_key.get_key_by_path ("Software\\Shareaza\\Shareaza");

        if (shareaza_key)
        {
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Load autofill values
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            for (const auto &value :
                 shareaza_key.get_values_by_mask ("Search\\Search.*"))
            {
                autofill af;

                af.value = mobius::core::string::word (
                    value.get_data ().get_data_as_string ("utf-16le"), 0, "\n"
                );
                af.username = get_username_from_path (f.get_path ());
                af.id = value.get_name ().substr (7);
                af.is_deleted = f.is_deleted ();
                af.f = f;

                autofills_.push_back (af);
            }
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for Shareaza profiles
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
            if (name == "profile.xml")
                p.add_profile_xml_file (f);

            else if (name == "shareaza.db3")
                p.add_shareaza_db3_file (f);

            else if (name == "library1.dat" || name == "library2.dat" ||
                     name == "library.dat")
                p.add_library_dat_file (f);

            else if (name == "searches.dat")
                p.add_searches_dat_file (f);
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
// @brief Scan folder for .sd files
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_scan_sd_files (const mobius::core::io::folder &folder)
{
    mobius::core::log log (__FILE__, __FUNCTION__);
    mobius::core::io::walker w (folder);

    try
    {
        for (const auto &f : w.get_files_by_pattern ("*.sd"))
            _decode_sd_file (f);
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, std::string (e.what ()));
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode .sd file
// @param f .sd file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_decode_sd_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto sd = CDownload (f.new_reader ());

        if (!sd)
        {
            log.info (
                __LINE__,
                "File is not a valid CDownload file. Path: " + f.get_path ()
            );
            return;
        }

        log.info (__LINE__, "File decoded [.sd]: " + f.get_path ());

        auto btinfo = sd.get_btinfo ();
        auto username = get_username_from_path (f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get path, if available
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto parts = sd.get_parts ();
        std::string path;

        if (parts.size () == 1)
            path = parts[0].path;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add local file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        profile::local_file lf;

        // Attributes
        lf.filename = sd.get_name ();
        lf.username = username;
        lf.hashes = get_file_hashes (sd);
        lf.path = path;
        lf.flag_downloaded = true;
        lf.flag_uploaded = btinfo.get_total_uploaded () > 0;
        lf.flag_shared = sd.is_shared ();
        lf.flag_completed = sd.get_downloaded_size () == sd.get_size ();
        lf.f = f;

        // Metadata
        lf.metadata.set ("block_count", btinfo.get_block_count ());
        lf.metadata.set ("block_size", btinfo.get_block_size ());
        lf.metadata.set ("cbtinfo_version", btinfo.get_version ());
        lf.metadata.set ("cdownload_version", sd.get_version ());
        lf.metadata.set ("created_by", btinfo.get_created_by ());
        lf.metadata.set ("creation_time", btinfo.get_creation_time ());
        lf.metadata.set ("comments", btinfo.get_comments ());
        lf.metadata.set ("downloaded_size", sd.get_downloaded_size ());
        lf.metadata.set ("estimated_size", sd.get_size ());
        lf.metadata.set ("flag_downloaded", "true");
        lf.metadata.set ("flag_uploaded", lf.flag_uploaded ? "true" : "false");
        lf.metadata.set ("flag_shared", lf.flag_shared ? "true" : "false");
        lf.metadata.set ("flag_corrupted", "unknown");
        lf.metadata.set ("flag_completed", "true");
        lf.metadata.set ("is_boosted", sd.is_boosted ());
        lf.metadata.set ("is_expanded", sd.is_expanded ());
        lf.metadata.set ("is_paused", sd.is_paused ());
        lf.metadata.set ("is_seeding", sd.is_seeding ());
        lf.metadata.set ("local_name", sd.get_local_name ());
        lf.metadata.set ("remaining_size", sd.get_remaining_size ());
        lf.metadata.set ("sd_file_signature", sd.get_signature ());
        lf.metadata.set ("ser_id", sd.get_ser_id ());
        lf.metadata.set ("search_keyword", sd.get_search_keyword ());
        lf.metadata.set ("serving_file_name", sd.get_serving_file_name ());
        lf.metadata.set ("remaining_size", sd.get_remaining_size ());
        lf.metadata.set ("size", sd.get_size ());
        lf.metadata.set ("torrent_success", sd.get_torrent_success ());
        lf.metadata.set ("total_downloaded", btinfo.get_total_downloaded ());
        lf.metadata.set ("total_uploaded", btinfo.get_total_uploaded ());

        for (const auto &[k, v] : btinfo.get_metadata ())
            lf.metadata.set (k, v);

        for (const auto &[k, v] : sd.get_pxml ().get_metadata ())
            lf.metadata.set (k, v);

        local_files_.push_back (lf);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add remote files
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &source : sd.get_sources ())
        {
            profile::remote_file rf;

            rf.timestamp = source.get_last_seen_time ();
            rf.ip = source.get_ip ();
            rf.port = source.get_port ();
            rf.filename = sd.get_name ();
            rf.username = username;
            rf.hashes = get_file_hashes (sd);
            rf.f = f;

            // metadata
            rf.metadata.set ("block_count", btinfo.get_block_count ());
            rf.metadata.set ("block_size", btinfo.get_block_size ());
            rf.metadata.set ("cbtinfo_version", btinfo.get_version ());
            rf.metadata.set ("cdownload_version", sd.get_version ());
            rf.metadata.set ("comments", btinfo.get_comments ());
            rf.metadata.set ("created_by", btinfo.get_created_by ());
            rf.metadata.set ("creation_time", btinfo.get_creation_time ());
            rf.metadata.set ("estimated_size", sd.get_size ());
            rf.metadata.set ("is_boosted", sd.is_boosted ());
            rf.metadata.set ("is_expanded", sd.is_expanded ());
            rf.metadata.set ("is_paused", sd.is_paused ());
            rf.metadata.set ("is_seeding", sd.is_seeding ());
            rf.metadata.set ("local_name", sd.get_local_name ());
            rf.metadata.set ("sd_file_signature", sd.get_signature ());
            rf.metadata.set ("ser_id", sd.get_ser_id ());
            rf.metadata.set ("search_keyword", sd.get_search_keyword ());
            rf.metadata.set ("serving_file_name", sd.get_serving_file_name ());
            rf.metadata.set ("size", sd.get_size ());
            rf.metadata.set ("torrent_success", sd.get_torrent_success ());
            rf.metadata.set (
                "total_downloaded", btinfo.get_total_downloaded ()
            );
            rf.metadata.set ("total_uploaded", btinfo.get_total_uploaded ());
            rf.metadata.set ("url", source.get_url ());

            for (const auto &[k, v] : sd.get_pxml ().get_metadata ())
                rf.metadata.set (k, v);

            remote_files_.push_back (rf);
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.shareaza.sd"), f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, std::string (e.what ()));
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
        e.set_attribute ("app_id", APP_ID);
        e.set_attribute ("app_name", APP_NAME);
        e.set_attribute ("username", p.get_username ());
        e.set_attribute ("creation_time", p.get_creation_time ());
        e.set_attribute ("last_modified_time", p.get_last_modified_time ());
        e.set_attribute ("path", p.get_path ());

        // Metadata
        auto metadata = mobius::core::pod::map ();

        metadata.set ("gnutella_guid", p.get_gnutella_guid ());
        metadata.set ("bittorrent_guid", p.get_bittorrent_guid ());
        metadata.set ("identity", p.get_identity ());
        metadata.set ("num_local_files", p.num_local_files ());
        metadata.set ("num_remote_files", p.num_remote_files ());
        metadata.set ("num_searched_texts", p.num_searched_texts ());

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
    for (const auto &a : autofills_)
    {
        auto e = item_.new_evidence ("autofill");

        e.set_attribute ("field_name", "search");
        e.set_attribute ("value", a.value);
        e.set_attribute ("app_id", APP_ID);
        e.set_attribute ("app_name", APP_NAME);
        e.set_attribute ("username", a.username);
        e.set_attribute ("is_deleted", a.is_deleted);

        mobius::core::pod::map metadata;
        metadata.set ("id", a.id);
        e.set_attribute ("metadata", metadata);

        e.set_tag ("app.p2p");
        e.add_source (a.f);
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
        e.set_attribute ("app_id", APP_ID);
        e.set_attribute ("app_name", APP_NAME);
        e.set_attribute ("hashes", lf.hashes);
        e.set_attribute ("metadata", lf.metadata);

        e.set_tag ("app.p2p");
        e.add_source (lf.f);
        e.add_source (lf.shareaza_db3_f);
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
        if (lf.flag_downloaded)
        {
            auto e = item_.new_evidence ("received-file");

            e.set_attribute ("username", lf.username);
            e.set_attribute ("filename", lf.filename);
            e.set_attribute ("path", lf.path);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("hashes", lf.hashes);
            e.set_attribute ("metadata", lf.metadata);

            e.set_tag ("app.p2p");
            e.add_source (lf.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save remote party shared files
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
        e.set_attribute ("thumbnail_data", rf.thumbnail_data);
        e.set_attribute ("metadata", rf.metadata);

        e.set_tag ("app.p2p");
        e.add_source (rf.f);
        e.add_source (rf.shareaza_db3_f);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save searched texts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_searched_texts ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &st : p.get_searched_texts ())
        {
            if (st.timestamp)
            {
                auto e = item_.new_evidence ("searched-text");

                e.set_attribute ("timestamp", st.timestamp);
                e.set_attribute ("search_type", "p2p.shareaza");
                e.set_attribute ("text", st.text);
                e.set_attribute ("username", p.get_username ());
                e.set_attribute ("metadata", st.metadata);

                e.set_tag ("app.p2p");
                e.add_source (st.f);
            }
        }
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
        if (lf.flag_uploaded)
        {
            auto e = item_.new_evidence ("sent-file");

            e.set_attribute ("username", lf.username);
            e.set_attribute ("filename", lf.filename);
            e.set_attribute ("path", lf.path);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("hashes", lf.hashes);
            e.set_attribute ("metadata", lf.metadata);

            e.set_tag ("app.p2p");
            e.add_source (lf.f);
            e.add_source (lf.shareaza_db3_f);
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
        if (lf.flag_shared)
        {
            auto e = item_.new_evidence ("shared-file");

            e.set_attribute ("username", lf.username);
            e.set_attribute ("filename", lf.filename);
            e.set_attribute ("path", lf.path);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("hashes", lf.hashes);
            e.set_attribute ("metadata", lf.metadata);

            e.set_tag ("app.p2p");
            e.add_source (lf.f);
            e.add_source (lf.shareaza_db3_f);
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
        auto gnutella_guid = p.get_gnutella_guid ();
        auto bittorrent_guid = p.get_bittorrent_guid ();

        mobius::core::pod::map metadata;
        metadata.set ("app_id", APP_ID);
        metadata.set ("app_name", APP_NAME);
        metadata.set ("username", p.get_username ());
        metadata.set ("gnutella_guid", gnutella_guid);
        metadata.set ("bittorrent_guid", bittorrent_guid);
        metadata.set ("identity_primary", p.get_identity ());

        if (!gnutella_guid.empty ())
        {
            auto e = item_.new_evidence ("user-account");

            e.set_attribute ("account_type", "p2p.gnutella");
            e.set_attribute ("id", p.get_gnutella_guid ());
            e.set_attribute ("password", {});
            e.set_attribute ("password_found", "no");
            e.set_attribute ("metadata", metadata.clone ());
            e.set_tag ("app.p2p");
            e.add_source (p.get_file ());
        }

        if (!bittorrent_guid.empty ())
        {
            auto e = item_.new_evidence ("user-account");

            e.set_attribute ("account_type", "p2p.bittorrent");
            e.set_attribute ("id", bittorrent_guid);
            e.set_attribute ("password", {});
            e.set_attribute ("password_found", "no");
            e.set_attribute ("metadata", metadata.clone ());
            e.set_tag ("app.p2p");
            e.add_source (p.get_file ());
        }
    }
}

} // namespace mobius::extension::app::shareaza