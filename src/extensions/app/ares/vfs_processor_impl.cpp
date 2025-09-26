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
#include <mobius/core/decoder/hexstring.hpp>
#include <mobius/core/decoder/inifile.hpp>
#include <mobius/core/decoder/json/parser.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/io/uri.hpp>
#include <mobius/core/io/walker.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/os/win/registry/hive_file.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/framework/evidence_flag.hpp>
#include <mobius/framework/model/evidence.hpp>
#include <iomanip>
#include <sstream>
#include "common.hpp"
#include "file_arestra.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
//    . Ares Galaxy 246 source code
//    . Forensic Analysis of Ares Galaxy Peer-to-Peer Network (Kolenbrander)
//
// Ares Galaxy main files (* decoded by MobiusFT):
//
//  . DHTNodes.dat - DHT nodes
//       @see DHT_readnodeFile - DHT/dhtzones.pas (line 125)
//       (client ID, IP, udp_port, tcp_port, type)
//
//  . MDHTNodes.dat - MDHT nodes
//       @see MDHT_readnodeFile - BitTorrent/dht_zones.pas (line 124)
//       (client ID, IP, udp_port, type)
//
//  * PHashIdx.dat, PhashIdxTemp.dat, TempPHash.dat - PHash table
//       @see ICH_load_phash_indexs - helper_ICH.pas (line 1023)
//       (hash_sha1, Phash table)
//
//  * ShareH.dat - Trusted metadata
//       @see get_trusted_metas - helper_library_db.pas (line 542)
//
//  * ShareL.dat - Cached metadata
//       @see get_cached_metas - helper_library_db.pas (line 367)
//
//  . SNodes.dat
//       @see aresnodes_loadfromdisk - helper_ares_nodes (line 445)
//       (IP, port, reports, attempts, connects, first_seen, last_seen)
//
//  * TorrentH.dat - DHT magnet file history and metadata
//       @see tthread_dht.getMagnetFiles - DHT/thread_dht.pas (line 284)
//
//  * TempDL/PHash_XXX.dat - Downloading file pieces info
//       @see ICH_loadPieces - helper_ICH (line 528)
//       (flag_done, progress, hash_sha1)
//
//  * TempDL/PBTHash_XXX.dat - Downloading file (BitTorrent) metadata
//       @see BitTorrentDb_load - BitTorrent/BitTorrentDlDb.pas (line 88)
//
//  * TempUL/UDPPHash_XXX.dat - Uploading file (BitTorrent) metadata
//       @see ICH_send_Phash@helper_ICH.pas (line 776)
//
//  * ___ARESTRA___*.* - Downloading files, with metadata info
//       @see read_details_DB_Download - helper_download_disk.pas (line 722)
//
//  . __INCOMPLETE__*.* - Downloading files (BitTorrent)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string ANT_ID = "evidence.app-ares";
static const std::string ANT_NAME = "App Ares Galaxy";
static const std::string ANT_VERSION = "1.4";
static const std::string SAMPLING_ID = "sampling";
static const std::string APP_NAME = "Ares Galaxy";
static const std::string APP_ID = "ares";

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
// @brief Convert registry data into string
// @param data Registry data
// @param encoding Char encoding
// @return String
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
to_string_from_hexstring (
    const mobius::core::os::win::registry::hive_data &data,
    const std::string &encoding = "utf-16le"
)
{
    std::string value;

    if (data)
    {
        mobius::core::bytearray d;
        d.from_hexstring (data.get_data ().to_string (encoding));
        value = d.to_string ();
    }

    return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert registry data into hex string
// @param data Registry data
// @return String
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
to_hex_string (const mobius::core::os::win::registry::hive_data &data)
{
    std::string value;

    if (data)
        value =
            mobius::core::string::toupper (data.get_data ().to_hexstring ());

    return value;
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get vector of hashes for a given file
// @param f File structure
// @return Vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<mobius::core::pod::data>
get_file_hashes (const mobius::extension::app::ares::profile::file &f)
{
    std::vector<mobius::core::pod::data> hashes;

    if (!f.hash_sha1.empty ())
        hashes.push_back ({"sha1", f.hash_sha1});

    return hashes;
}

} // namespace

namespace mobius::extension::app::ares
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
    _scan_arestra_folder (folder);
    _scan_ntuser_dat_folder (folder);
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
    _save_local_files ();
    _save_p2p_remote_files ();
    _save_received_files ();
    _save_sent_files ();
    _save_shared_files ();
    _save_user_accounts ();

    item_.set_ant (ANT_ID, ANT_NAME, ANT_VERSION);
    transaction.commit ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for ___ARESTRA___ files
// @param folder Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_scan_arestra_folder (const mobius::core::io::folder &folder)
{
    mobius::core::log log (__FILE__, __FUNCTION__);
    mobius::core::io::walker w (folder);

    for (const auto &[name, f] : w.get_files_with_names ())
    {
        try
        {
            if (mobius::core::string::startswith (name, "___arestra___"))
                _decode_arestra_file (f);
        }
        catch (const std::exception &e)
        {
            log.warning (
                __LINE__,
                std::string (e.what ()) + " (file: " + f.get_path () + ")"
            );
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode ARESTRA file
// @param f ARESTRA file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_decode_arestra_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_arestra arestra (f.new_reader ());

    if (!arestra)
    {
        log.info (
            __LINE__,
            "File " + f.get_path () + " is not a valid PBTHash.dat file"
        );
        return;
    }

    log.info (__LINE__, "File decoded [___ARESTRA___]. Path: " + f.get_path ());

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Create file object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    profile::file fobj;

    // set attributes
    fobj.hash_sha1 = arestra.get_hash_sha1 ();
    fobj.username = get_username_from_path (f.get_path ());
    fobj.download_started_time = arestra.get_download_started_time ();
    fobj.size = arestra.get_file_size ();
    fobj.arestra_f = f;

    // set filename
    fobj.filename = mobius::core::io::path (f.get_path ()).get_filename ();
    fobj.filename.erase (0, 13); // remove "___ARESTRA___"

    // set flags
    fobj.flag_downloaded = true;
    fobj.flag_corrupted = arestra.is_corrupted ();
    fobj.flag_shared = false; // @see thread_share.pas (line 1065)
    fobj.flag_completed = arestra.is_completed ();

    // add remote_sources
    for (const auto &[ip, port] : arestra.get_alt_sources ())
    {
        profile::remote_source r_source;
        r_source.timestamp = f.get_modification_time ();
        r_source.ip = ip;
        r_source.port = port;

        fobj.remote_sources.push_back (r_source);
    }

    // set metadata
    fobj.metadata.set ("arestra_signature", arestra.get_signature ());
    fobj.metadata.set ("arestra_file_version", arestra.get_version ());
    fobj.metadata.set (
        "download_started_time", arestra.get_download_started_time ()
    );
    fobj.metadata.set ("downloaded_bytes", arestra.get_progress ());
    fobj.metadata.set ("verified_bytes", arestra.get_phash_verified ());
    fobj.metadata.set ("is_paused", arestra.is_paused ());
    fobj.metadata.set ("media_type", arestra.get_media_type ());
    fobj.metadata.set ("param1", arestra.get_param1 ());
    fobj.metadata.set ("param2", arestra.get_param2 ());
    fobj.metadata.set ("param3", arestra.get_param3 ());
    fobj.metadata.set ("kwgenre", arestra.get_kw_genre ());
    fobj.metadata.set ("title", arestra.get_title ());
    fobj.metadata.set ("artist", arestra.get_artist ());
    fobj.metadata.set ("album", arestra.get_album ());
    fobj.metadata.set ("category", arestra.get_category ());
    fobj.metadata.set ("year", arestra.get_year ());
    fobj.metadata.set ("language", arestra.get_language ());
    fobj.metadata.set ("url", arestra.get_url ());
    fobj.metadata.set ("comment", arestra.get_comment ());
    fobj.metadata.set ("subfolder", arestra.get_subfolder ());

    files_.push_back (fobj);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for NTUSER.DAT files
// @param folder Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_scan_ntuser_dat_folder (
    const mobius::core::io::folder &folder
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);
    mobius::core::io::walker w (folder);

    for (const auto &f : w.get_files_by_name ("ntuser.dat"))
    {
        try
        {
            _decode_ntuser_dat_file (f);
        }
        catch (const std::exception &e)
        {
            log.warning (
                __LINE__,
                std::string (e.what ()) + " (file: " + f.get_path () + ")"
            );
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode NTUSER.DAT file
// @param f NTUSER.DAT file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_decode_ntuser_dat_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Create decoder
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto decoder = mobius::core::os::win::registry::hive_file (f.new_reader ());

    if (!decoder.is_instance ())
    {
        log.info (__LINE__, "File " + f.get_path () + " ignored.");
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get evidences from Ares key
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    const auto &root_key = decoder.get_root_key ();
    const auto &ares_key = root_key.get_key_by_path ("Software\\Ares");

    if (ares_key)
    {
        auto username = get_username_from_path (f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load account
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        account acc;

        acc.guid = ares_key.get_data_by_name ("Personal.GUID")
                       .get_data_as_string ("utf-16le");
        acc.nickname = to_string_from_hexstring (
            ares_key.get_data_by_name ("Personal.Nickname")
        );
        acc.dht_id =
            to_hex_string (ares_key.get_data_by_name ("Network.DHTID"));
        acc.mdht_id =
            to_hex_string (ares_key.get_data_by_name ("Network.MDHTID"));
        acc.username = username;
        acc.is_deleted = f.is_deleted ();
        acc.f = f;

        accounts_.push_back (acc);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load autofill values
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &key : ares_key.get_keys_by_mask ("Search.History\\*"))
        {
            std::string category = key.get_name ();

            for (const auto &value : key.get_values ())
            {
                autofill af;

                af.value = mobius::core::decoder::hexstring (value.get_name ())
                               .to_string ();
                af.username = username;
                af.category = category;
                af.account_guid = acc.guid;
                af.is_deleted = acc.is_deleted;
                af.f = f;

                autofills_.push_back (af);
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for Ares Galaxy profiles
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_scan_profile_folder (const mobius::core::io::folder &folder)
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
            if (name == "shareh.dat")
                p.add_shareh_file (f);

            else if (name == "sharel.dat")
                p.add_sharel_file (f);

            else if (name == "torrenth.dat")
                p.add_torrenth_file (f);

            else if (name == "phashidx.dat" || name == "phashidxtemp.dat" ||
                     name == "tempphash.dat")
                p.add_phashidx_file (f);
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
    {
        profiles_.push_back (p);

        for (const auto &f : w.get_folders_by_name ("tempdl"))
            _scan_tempdl_folder (p, f);

        for (const auto &f : w.get_folders_by_name ("tempul"))
            _scan_tempul_folder (p, f);

        auto files = p.get_files ();

        std::copy (files.begin (), files.end (), std::back_inserter (files_));
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan Ares Data/TempDL folder for evidences
// @param p Profile
// @param folder Ares/Data/TempDL folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_scan_tempdl_folder (
    profile &p, const mobius::core::io::folder &folder
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);
    mobius::core::io::walker w (folder);

    for (const auto &[name, f] : w.get_files_with_names ())
    {
        if (mobius::core::string::startswith (name, "phash_"))
            p.add_tempdl_phash_file (f);

        else if (mobius::core::string::startswith (name, "pbthash_"))
            p.add_tempdl_pbthash_file (f);

        else
            log.development (
                __LINE__, "unhandled Data/TempDL file: " + f.get_name ()
            );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan Ares Data/TempUL folder for evidences
// @param p Profile
// @param folder Ares/Data/TempUL folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_scan_tempul_folder (
    profile &p, const mobius::core::io::folder &folder
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);
    mobius::core::io::walker w (folder);

    for (const auto &[name, f] : w.get_files_with_names ())
    {
        if (mobius::core::string::startswith (name, "udpphash_"))
            p.add_tempul_udpphash_file (f);

        else
            log.development (
                __LINE__, "unhandled Data/TempUL file: " + f.get_name ()
            );
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

        metadata.set ("num_files", p.size_files ());
        e.set_attribute ("metadata", metadata);

        // Tags and sources
        e.set_tag ("app.browser");
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
        mobius::core::pod::map metadata;
        metadata.set ("category", a.category);
        metadata.set ("network", "Ares");
        metadata.set ("ares_account_guid", a.account_guid);

        auto e = item_.new_evidence ("autofill");

        e.set_attribute ("field_name", "search");
        e.set_attribute ("value", a.value);
        e.set_attribute ("app_id", APP_ID);
        e.set_attribute ("app_name", APP_NAME);
        e.set_attribute ("username", a.username);
        e.set_attribute ("is_deleted", a.is_deleted);
        e.set_attribute ("metadata", metadata);
        e.set_tag ("p2p");
        e.add_source (a.f);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save local files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_local_files ()
{
    for (const auto &f : files_)
    {
        if (!f.path.empty ())
        {
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Create evidence
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            auto e = item_.new_evidence ("local-file");

            e.set_attribute ("username", f.username);
            e.set_attribute ("path", f.path);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("hashes", get_file_hashes (f));

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Metadata
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            mobius::core::pod::map metadata;

            metadata.set ("size", f.size);
            metadata.set ("flag_downloaded", to_string (f.flag_downloaded));
            metadata.set ("flag_uploaded", to_string (f.flag_uploaded));
            metadata.set ("flag_shared", to_string (f.flag_shared));
            metadata.set ("flag_corrupted", to_string (f.flag_corrupted));
            metadata.set ("flag_completed", to_string (f.flag_completed));

            if (f.shareh_idx)
                metadata.set ("shareh_idx", f.shareh_idx);

            if (f.sharel_idx)
                metadata.set ("sharel_idx", f.sharel_idx);

            if (f.torrenth_idx)
                metadata.set ("torrenth_idx", f.torrenth_idx);

            if (f.phashidx_idx)
                metadata.set ("phashidx_idx", f.phashidx_idx);

            metadata.set ("network", "Ares");
            update_metadata (metadata, f.metadata);

            e.set_attribute ("metadata", metadata);

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Tags
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            e.set_tag ("p2p");

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Sources
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            e.add_source (f.shareh_f);
            e.add_source (f.sharel_f);
            e.add_source (f.phashidx_f);
            e.add_source (f.arestra_f);
            e.add_source (f.tempdl_pbthash_f);
            e.add_source (f.tempdl_phash_f);
            e.add_source (f.tempul_udpphash_f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save remote files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_p2p_remote_files ()
{
    for (const auto &f : files_)
    {
        for (const auto &rs : f.remote_sources)
        {
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Create evidence
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            auto e = item_.new_evidence ("p2p-remote-file");
            e.set_attribute ("timestamp", rs.timestamp);
            e.set_attribute ("ip", rs.ip);
            e.set_attribute ("port", rs.port);
            e.set_attribute ("filename", f.filename);
            e.set_attribute ("username", f.username);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("hashes", get_file_hashes (f));

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Metadata
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            mobius::core::pod::map metadata;

            metadata.set ("size", f.size);
            metadata.set ("network", "Ares");
            update_metadata (metadata, f.metadata);

            e.set_attribute ("metadata", metadata);

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Tags
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            e.set_tag ("p2p");

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Sources
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            e.add_source (f.shareh_f);
            e.add_source (f.sharel_f);
            e.add_source (f.torrenth_f);
            e.add_source (f.phashidx_f);
            e.add_source (f.arestra_f);
            e.add_source (f.tempdl_pbthash_f);
            e.add_source (f.tempdl_phash_f);
            e.add_source (f.tempul_udpphash_f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save received files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_received_files ()
{
    for (const auto &f : files_)
    {
        if (f.flag_downloaded.is_yes ())
        {
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Create evidence
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            auto e = item_.new_evidence ("received-file");

            if (f.download_completed_time)
                e.set_attribute ("timestamp", f.download_completed_time);

            else if (f.download_started_time)
                e.set_attribute ("timestamp", f.download_started_time);

            e.set_attribute ("filename", f.filename);
            e.set_attribute ("path", f.path);
            e.set_attribute ("username", f.username);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("hashes", get_file_hashes (f));

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Metadata
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            mobius::core::pod::map metadata;
            metadata.set ("flag_downloaded", to_string (f.flag_downloaded));
            metadata.set ("flag_uploaded", to_string (f.flag_uploaded));
            metadata.set ("flag_shared", to_string (f.flag_shared));
            metadata.set ("flag_corrupted", to_string (f.flag_corrupted));
            metadata.set ("flag_completed", to_string (f.flag_completed));

            if (f.shareh_idx)
                metadata.set ("shareh_idx", f.shareh_idx);

            if (f.sharel_idx)
                metadata.set ("sharel_idx", f.sharel_idx);

            if (f.torrenth_idx)
                metadata.set ("torrenth_idx", f.torrenth_idx);

            if (f.phashidx_idx)
                metadata.set ("phashidx_idx", f.phashidx_idx);

            metadata.set ("network", "Ares");
            update_metadata (metadata, f.metadata);

            e.set_attribute ("metadata", metadata);

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Tags
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            e.set_tag ("p2p");

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Sources
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            e.add_source (f.shareh_f);
            e.add_source (f.sharel_f);
            e.add_source (f.torrenth_f);
            e.add_source (f.phashidx_f);
            e.add_source (f.arestra_f);
            e.add_source (f.tempdl_pbthash_f);
            e.add_source (f.tempdl_phash_f);
            e.add_source (f.tempul_udpphash_f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save sent files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_sent_files ()
{
    for (const auto &f : files_)
    {
        if (f.flag_uploaded.is_yes ())
        {
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Create evidence
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            auto e = item_.new_evidence ("sent-file");

            if (f.upload_started_time)
                e.set_attribute ("timestamp", f.upload_started_time);

            e.set_attribute ("filename", f.filename);
            e.set_attribute ("path", f.path);
            e.set_attribute ("username", f.username);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("hashes", get_file_hashes (f));

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Metadata
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            mobius::core::pod::map metadata;
            metadata.set ("flag_downloaded", to_string (f.flag_downloaded));
            metadata.set ("flag_uploaded", to_string (f.flag_uploaded));
            metadata.set ("flag_shared", to_string (f.flag_shared));
            metadata.set ("flag_corrupted", to_string (f.flag_corrupted));
            metadata.set ("flag_completed", to_string (f.flag_completed));

            if (f.shareh_idx)
                metadata.set ("shareh_idx", f.shareh_idx);

            if (f.sharel_idx)
                metadata.set ("sharel_idx", f.sharel_idx);

            if (f.torrenth_idx)
                metadata.set ("torrenth_idx", f.torrenth_idx);

            if (f.phashidx_idx)
                metadata.set ("phashidx_idx", f.phashidx_idx);

            metadata.set ("network", "Ares");
            update_metadata (metadata, f.metadata);

            e.set_attribute ("metadata", metadata);

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Tags
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            e.set_tag ("p2p");

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Sources
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            e.add_source (f.shareh_f);
            e.add_source (f.sharel_f);
            e.add_source (f.torrenth_f);
            e.add_source (f.phashidx_f);
            e.add_source (f.arestra_f);
            e.add_source (f.tempdl_pbthash_f);
            e.add_source (f.tempdl_phash_f);
            e.add_source (f.tempul_udpphash_f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save shared files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_shared_files ()
{
    for (const auto &f : files_)
    {
        if (f.flag_shared.is_yes () || f.flag_shared.is_always ())
        {
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Create evidence
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            auto e = item_.new_evidence ("shared-file");

            e.set_attribute ("username", f.username);
            e.set_attribute ("filename", f.filename);
            e.set_attribute ("path", f.path);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);

            std::vector<mobius::core::pod::data> hashes = {
                {"sha1", f.hash_sha1}
            };
            e.set_attribute ("hashes", hashes);

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Metadata
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            mobius::core::pod::map metadata;

            metadata.set ("size", f.size);
            metadata.set ("flag_downloaded", to_string (f.flag_downloaded));
            metadata.set ("flag_uploaded", to_string (f.flag_uploaded));
            metadata.set ("flag_shared", to_string (f.flag_shared));
            metadata.set ("flag_corrupted", to_string (f.flag_corrupted));
            metadata.set ("flag_completed", to_string (f.flag_completed));

            if (f.shareh_idx)
                metadata.set ("shareh_idx", f.shareh_idx);

            if (f.sharel_idx)
                metadata.set ("sharel_idx", f.sharel_idx);

            if (f.torrenth_idx)
                metadata.set ("torrenth_idx", f.torrenth_idx);

            if (f.phashidx_idx)
                metadata.set ("phashidx_idx", f.phashidx_idx);

            metadata.set ("network", "Ares");
            update_metadata (metadata, f.metadata);

            e.set_attribute ("metadata", metadata);

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Tags
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            e.set_tag ("p2p");

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Sources
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            e.add_source (f.shareh_f);
            e.add_source (f.sharel_f);
            e.add_source (f.torrenth_f);
            e.add_source (f.phashidx_f);
            e.add_source (f.arestra_f);
            e.add_source (f.tempdl_pbthash_f);
            e.add_source (f.tempdl_phash_f);
            e.add_source (f.tempul_udpphash_f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save accounts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_user_accounts ()
{
    for (const auto &a : accounts_)
    {
        mobius::core::pod::map metadata;
        metadata.set ("app_id", APP_ID);
        metadata.set ("app_name", APP_NAME);
        metadata.set ("username", a.username);
        metadata.set ("network", "Ares");
        metadata.set ("guid", a.guid);
        metadata.set ("dht_id", a.dht_id);
        metadata.set ("mdht_id", a.mdht_id);

        auto e = item_.new_evidence ("user-account");

        e.set_attribute ("account_type", "p2p.ares");
        e.set_attribute ("id", a.guid);
        e.set_attribute ("name", a.nickname);
        e.set_attribute ("password", {});
        e.set_attribute ("password_found", "no");
        e.set_attribute ("is_deleted", a.is_deleted);
        e.set_attribute ("metadata", metadata);
        e.set_tag ("p2p");
        e.add_source (a.f);
    }
}

} // namespace mobius::extension::app::ares
