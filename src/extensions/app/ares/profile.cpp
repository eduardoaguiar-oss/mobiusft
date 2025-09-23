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
#include "profile.hpp"
#include <mobius/core/decoder/hexstring.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/os/win/registry/hive_file.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/core/value_selector.hpp>
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include "file_arestra.hpp"
#include "file_pbthash.hpp"
#include "file_phash.hpp"
#include "file_shareh.hpp"
#include "file_sharel.hpp"
#include "file_torrenth.hpp"

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

} // namespace

namespace mobius::extension::app::ares
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
        return is_valid_;
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
    // @brief Set username
    // @param username username
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    set_username (const std::string &username)
    {
        username_ = username;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get folder
    // @return Folder object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::io::folder
    get_folder () const
    {
        return folder_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get path to profile
    // @return Path to profile
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_path () const
    {
        return (folder_) ? folder_.get_path () : "";
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
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void set_folder (const mobius::core::io::folder &);

    void add_arestra_file (const mobius::core::io::file &);
    void add_ntuser_dat_file (const mobius::core::io::file &);
    void add_phashidx_file (const mobius::core::io::file &);
    void add_shareh_file (const mobius::core::io::file &);
    void add_sharel_file (const mobius::core::io::file &);
    void add_torrenth_file (const mobius::core::io::file &);
    void add_tempdl_phash_file (const mobius::core::io::file &);
    void add_tempdl_pbthash_file (const mobius::core::io::file &);
    void add_tempul_udpphash_file (const mobius::core::io::file &);

  private:
    // @brief Check if profile is valid
    bool is_valid_ = false;

    // @brief Folder object
    mobius::core::io::folder folder_;

    // @brief Username
    std::string username_;

    // @brief Creation time
    mobius::core::datetime::datetime creation_time_;

    // @brief Last modified time
    mobius::core::datetime::datetime last_modified_time_;

    // @brief Account data
    account account_;

    // @brief Account files
    std::map<std::string, file> account_files_;

    // @brief All accounts found
    std::vector<account> accounts_;

    // @brief All autofills found
    std::vector<autofill> autofills_;

    // @brief File catalog
    std::vector<file> files_;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set folder
// @param f Folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::set_folder (const mobius::core::io::folder &f)
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get data from folder
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    folder_ = f;
    last_modified_time_ = f.get_modification_time ();
    creation_time_ = f.get_creation_time ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get username, app ID and app name from path
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto path = f.get_path ();
    username_ = get_username_from_path (path);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add ARESTRA file
// @param f ARESTRA file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_arestra_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_arestra fa (f.new_reader ());

    if (!fa)
    {
        log.info (
            __LINE__, "File " + f.get_path () + " is not a valid ARESTRA file."
        );
        return;
    }
    log.info (__LINE__, "File decoded [ARESTRA]. Path: " + f.get_path ());

    if (!last_modified_time_ ||
        f.get_modification_time () > last_modified_time_)
        last_modified_time_ = f.get_modification_time ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get metadata
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_deleted = f.is_deleted ();

    auto [iter, success] =
        account_files_.try_emplace (fa.get_hash_sha1 (), file {});
    std::ignore = success;
    auto &fobj = iter->second;

    if (!fobj.arestra_f || (fobj.arestra_f.is_deleted () && !is_deleted))
    {
        // set attributes
        fobj.hash_sha1 = fa.get_hash_sha1 ();
        fobj.account_guid = account_.guid;
        fobj.username = username_;
        fobj.download_started_time = fa.get_download_started_time ();
        fobj.size = fa.get_file_size ();
        fobj.arestra_f = f;

        // set filename
        fobj.filename = mobius::core::io::path (f.get_path ()).get_filename ();
        fobj.filename.erase (0, 13); // remove "___ARESTRA___"

        // set flags
        fobj.flag_downloaded = true;
        fobj.flag_corrupted.set_if_unknown (fa.is_corrupted ());
        fobj.flag_shared.set_if_unknown (
            false
        ); // @see thread_share.pas (line 1065)
        fobj.flag_completed = fa.is_completed ();

        // add remote_sources
        for (const auto &[ip, port] : fa.get_alt_sources ())
        {
            remote_source r_source;
            r_source.timestamp = f.get_modification_time ();
            r_source.ip = ip;
            r_source.port = port;

            fobj.remote_sources.push_back (r_source);
        }

        // set metadata
        fobj.metadata.set ("arestra_signature", fa.get_signature ());
        fobj.metadata.set ("arestra_file_version", fa.get_version ());
        fobj.metadata.set (
            "download_started_time", fa.get_download_started_time ()
        );
        fobj.metadata.set ("downloaded_bytes", fa.get_progress ());
        fobj.metadata.set ("verified_bytes", fa.get_phash_verified ());
        fobj.metadata.set ("is_paused", fa.is_paused ());
        fobj.metadata.set ("media_type", fa.get_media_type ());
        fobj.metadata.set ("param1", fa.get_param1 ());
        fobj.metadata.set ("param2", fa.get_param2 ());
        fobj.metadata.set ("param3", fa.get_param3 ());
        fobj.metadata.set ("kwgenre", fa.get_kw_genre ());
        fobj.metadata.set ("title", fa.get_title ());
        fobj.metadata.set ("artist", fa.get_artist ());
        fobj.metadata.set ("album", fa.get_album ());
        fobj.metadata.set ("category", fa.get_category ());
        fobj.metadata.set ("year", fa.get_year ());
        fobj.metadata.set ("language", fa.get_language ());
        fobj.metadata.set ("url", fa.get_url ());
        fobj.metadata.set ("comment", fa.get_comment ());
        fobj.metadata.set ("subfolder", fa.get_subfolder ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add NTUSER.DAT file
// @param f NTUSER.DAT file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_ntuser_dat_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Create decoder
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto decoder = mobius::core::os::win::registry::hive_file (f.new_reader ());

    if (!decoder.is_instance ())
    {
        log.info (__LINE__, "File " + f.get_path () + " ignored.");
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get evidences from Ares key
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    const auto &root_key = decoder.get_root_key ();
    const auto &ares_key = root_key.get_key_by_path ("Software\\Ares");

    if (ares_key)
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load account
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
        acc.username = username_;
        acc.is_deleted = f.is_deleted ();
        acc.f = f;

        if (account_.guid.empty () || (account_.is_deleted && !acc.is_deleted))
            account_ = acc;

        accounts_.push_back (acc);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Load autofill values
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &key : ares_key.get_keys_by_mask ("Search.History\\*"))
        {
            std::string category = key.get_name ();

            for (const auto &value : key.get_values ())
            {
                autofill af;

                af.value = mobius::core::decoder::hexstring (value.get_name ())
                               .to_string ();
                af.username = username_;
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
// @brief Add PHashIdx.dat file
// @param f PHashIdx.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_phashidx_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_phash ph (f.new_reader ());

    if (!ph)
    {
        log.info (
            __LINE__,
            "File " + f.get_path () + " is not a valid PHashIdx.dat file."
        );
        return;
    }
    log.info (__LINE__, "File decoded [PHashIdx]. Path: " + f.get_path ());

    if (!last_modified_time_ ||
        f.get_modification_time () > last_modified_time_)
        last_modified_time_ = f.get_modification_time ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add PHashIdx entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_deleted = f.is_deleted ();

    for (const auto &entry : ph)
    {
        auto [iter, success] =
            account_files_.try_emplace (entry.hash_sha1, file {});
        std::ignore = success;
        auto &fobj = iter->second;

        if (!fobj.phashidx_f || (fobj.phashidx_f.is_deleted () && !is_deleted))
        {
            fobj.hash_sha1 = entry.hash_sha1;
            fobj.account_guid = account_.guid;
            fobj.username = username_;
            fobj.phashidx_idx = entry.idx;
            fobj.flag_completed =
                true; // PHashIdx.dat entries are always completed
            fobj.flag_downloaded = true;
            fobj.phashidx_f = f;

            if (fobj.size)
                fobj.metadata.set ("downloaded_bytes", fobj.size);
        }
    }

    is_valid_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add ShareH.dat file
// @param f Share history file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_shareh_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_shareh fh (f.new_reader ());

    if (!fh)
    {
        log.info (
            __LINE__,
            "File " + f.get_path () + " is not a valid ShareH.dat file"
        );
        return;
    }

    log.info (__LINE__, "File decoded [ShareH.dat]. Path: " + f.get_path ());

    if (!last_modified_time_ ||
        f.get_modification_time () > last_modified_time_)
        last_modified_time_ = f.get_modification_time ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add share history entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_deleted = f.is_deleted ();

    for (const auto &entry : fh)
    {
        auto [iter, success] =
            account_files_.try_emplace (entry.hash_sha1, file {});
        std::ignore = success;
        auto &fobj = iter->second;

        if (!fobj.shareh_f || (fobj.shareh_f.is_deleted () && !is_deleted))
        {
            fobj.hash_sha1 = entry.hash_sha1;
            fobj.account_guid = account_.guid;
            fobj.username = username_;
            fobj.download_completed_time = entry.download_completed_time;
            fobj.shareh_idx = entry.idx;
            fobj.shareh_f = f;

            fobj.flag_shared = entry.is_shared;
            fobj.flag_completed = true; // ShareH entries are always completed
            fobj.flag_corrupted = entry.is_corrupted;

            if (fobj.download_completed_time)
                fobj.flag_downloaded = true;

            fobj.metadata.set ("title", entry.title);
            fobj.metadata.set ("artist", entry.artist);
            fobj.metadata.set ("album", entry.album);
            fobj.metadata.set ("category", entry.category);
            fobj.metadata.set ("year", entry.year);
            fobj.metadata.set ("language", entry.language);
            fobj.metadata.set ("url", entry.url);
            fobj.metadata.set ("comment", entry.comment);
            fobj.metadata.set (
                "download_completed_time", entry.download_completed_time
            );
        }
    }

    is_valid_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add ShareL.dat file
// @param f Share library file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_sharel_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_sharel fl (f.new_reader ());

    if (!fl)
    {
        log.info (
            __LINE__,
            "File " + f.get_path () + " is not a valid ShareL.dat file"
        );
        return;
    }

    log.info (__LINE__, "File decoded [ShareL.dat]. Path: " + f.get_path ());

    if (!last_modified_time_ ||
        f.get_modification_time () > last_modified_time_)
        last_modified_time_ = f.get_modification_time ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add share library entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_deleted = f.is_deleted ();

    for (const auto &entry : fl)
    {
        auto [iter, success] =
            account_files_.try_emplace (entry.hash_sha1, file {});
        std::ignore = success;
        auto &fobj = iter->second;

        if (!fobj.sharel_f || (fobj.sharel_f.is_deleted () && !is_deleted))
        {
            // attributes
            fobj.hash_sha1 = entry.hash_sha1;
            fobj.account_guid = account_.guid;
            fobj.username = username_;
            fobj.path = entry.path;
            fobj.size = entry.size;
            fobj.sharel_idx = entry.idx;
            fobj.sharel_f = f;

            if (!fobj.path.empty ())
            {
                auto cpath =
                    mobius::core::string::replace (fobj.path, "\\", "/");
                fobj.filename = mobius::core::io::path (cpath).get_filename ();
            }

            // flags
            fobj.flag_corrupted.set_if_unknown (entry.is_corrupted);
            fobj.flag_shared.set_if_unknown (
                true
            ); // ShareL is shared by default, unless it is flagged
               // "no" in the corresponding ShareH entry.

            fobj.flag_completed = true; // ShareL entries are always completed

            // metadata
            fobj.metadata.set ("media_type", entry.media_type);
            fobj.metadata.set ("param1", entry.param1);
            fobj.metadata.set ("param2", entry.param2);
            fobj.metadata.set ("param3", entry.param3);
            fobj.metadata.set ("path", entry.path);
            fobj.metadata.set ("title", entry.title);
            fobj.metadata.set ("artist", entry.artist);
            fobj.metadata.set ("album", entry.album);
            fobj.metadata.set ("category", entry.category);
            fobj.metadata.set ("year", entry.year);
            fobj.metadata.set ("vidinfo", entry.vidinfo);
            fobj.metadata.set ("language", entry.language);
            fobj.metadata.set ("url", entry.url);
            fobj.metadata.set ("comment", entry.comment);
            fobj.metadata.set ("hash_of_phash", entry.hash_of_phash);
        }
    }

    is_valid_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add TorrentH.dat file
// @param f TorrentH.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_torrenth_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_torrenth th (f.new_reader ());

    if (!th)
    {
        log.info (
            __LINE__,
            "File " + f.get_path () + " is not a valid TorrentH.dat file"
        );
        return;
    }

    log.info (__LINE__, "File decoded [TorrentH.dat]. Path: " + f.get_path ());

    if (!last_modified_time_ ||
        f.get_modification_time () > last_modified_time_)
        last_modified_time_ = f.get_modification_time ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add TorrentH entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_deleted = f.is_deleted ();

    for (const auto &entry : th)
    {
        auto [iter, success] =
            account_files_.try_emplace (entry.hash_sha1, file {});
        std::ignore = success;
        auto &fobj = iter->second;

        if (!fobj.shareh_f || (fobj.shareh_f.is_deleted () && !is_deleted))
        {
            fobj.hash_sha1 = entry.hash_sha1;
            fobj.account_guid = account_.guid;
            fobj.username = username_;
            fobj.torrenth_idx = entry.idx;
            fobj.torrenth_f = f;
            fobj.filename = entry.name;
            fobj.size = entry.size;
            fobj.download_started_time = entry.timestamp;

            fobj.flag_shared = true;     // @see DHT/thread_dht.pas (line 412)
            fobj.flag_downloaded = true; // @see DHT/dhtkeywords.pas (line 355)
            fobj.flag_completed = true;  // @see DHT/dhtkeywords.pas (line 355)
            fobj.flag_corrupted = false; // @see DHT/dhtkeywords.pas (line 355)

            fobj.metadata.set ("seeds", entry.seeds);
            fobj.metadata.set ("media_type", entry.media_type);
            fobj.metadata.set (
                "evaluated_hash_sha1", entry.evaluated_hash_sha1
            );
            fobj.metadata.set ("torrent_name", entry.name);
            fobj.metadata.set ("torrent_url", entry.url);
        }
    }

    is_valid_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add TempDL/PHash_XXX.dat file
// @param f TempDL/PHash_XXX.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_tempdl_phash_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_phash phash (f.new_reader ());

    if (!phash)
    {
        log.info (
            __LINE__, "File " + f.get_path () + " is not a valid PHash.dat file"
        );
        return;
    }

    log.info (__LINE__, "File decoded [PHash.dat]. Path: " + f.get_path ());

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Process entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_deleted = f.is_deleted ();

    for (const auto &entry : phash)
    {
        auto hash_sha1 = entry.hash_sha1;
        auto [iter, success] = account_files_.try_emplace (hash_sha1, file {});
        std::ignore = success;
        auto &fobj = iter->second;

        if (!fobj.tempdl_phash_f ||
            (fobj.tempdl_phash_f.is_deleted () && !is_deleted))
        {
            fobj.hash_sha1 = hash_sha1;
            fobj.account_guid = account_.guid;
            fobj.username = username_;
            fobj.flag_downloaded = true;
            fobj.tempdl_phash_f = f;
            fobj.metadata.set ("pieces_count", entry.pieces_count);

            if (entry.is_completed != 2) // STATE_UNKNOWN
            {
                fobj.flag_completed = bool (entry.is_completed);
                fobj.metadata.set ("downloaded_bytes", entry.progress);
                fobj.metadata.set ("pieces_completed", entry.pieces_completed);
                fobj.metadata.set ("pieces_to_go", entry.pieces_to_go);
                fobj.metadata.set ("piece_size", entry.piece_size);
            }
        }
    }

    is_valid_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add TempDL/PBTHash_XXX.dat file
// @param f TempDL/PBTHash_XXX.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_tempdl_pbthash_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_pbthash pbthash (f.new_reader ());

    if (!pbthash)
    {
        log.info (
            __LINE__,
            "File " + f.get_path () + " is not a valid PBTHash.dat file"
        );
        return;
    }

    log.info (__LINE__, "File decoded [PBTHash.dat]. Path: " + f.get_path ());

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Process file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_deleted = f.is_deleted ();

    auto hash_sha1 = pbthash.get_hash_sha1 ();
    auto [iter, success] = account_files_.try_emplace (hash_sha1, file {});
    std::ignore = success;
    auto &fobj = iter->second;

    if (!fobj.tempdl_pbthash_f ||
        (fobj.tempdl_pbthash_f.is_deleted () && !is_deleted))
    {
        fobj.hash_sha1 = hash_sha1;
        fobj.account_guid = account_.guid;
        fobj.username = username_;
        fobj.size = pbthash.get_file_size ();
        fobj.tempdl_pbthash_f = f;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Set path and name
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto path = pbthash.get_torrent_path ();
        if (!path.empty ())
            fobj.path = path;

        auto name = pbthash.get_torrent_name ();
        if (!name.empty ())
            fobj.filename = name;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Test if every piece is checked
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto pieces = pbthash.get_pieces ();
        auto is_checked = std::all_of (
            pieces.begin (), pieces.end (),
            [] (const auto &p) { return p.is_checked; }
        );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Set flags
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        fobj.flag_downloaded = true;
        fobj.flag_completed.set_if_unknown (pbthash.is_completed ());
        fobj.flag_uploaded.set_if_unknown (pbthash.get_bytes_uploaded () > 0);
        fobj.flag_shared.set_if_unknown (pbthash.is_seeding ());
        fobj.flag_corrupted.set_if_unknown (!is_checked);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Set torrent files
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto files = pbthash.get_files ();

        std::transform (
            files.begin (), files.end (),
            std::back_inserter (fobj.torrent_files),
            [] (const auto &tf)
            {
                return torrent_file {
                    tf.idx, tf.size, tf.last_modification_time, tf.name, tf.path
                };
            }
        );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Set metadata
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        fobj.metadata.set ("torrent_url", "magnet:?xt=urn:btih:" + hash_sha1);
        fobj.metadata.set ("torrent_pieces_count", pbthash.get_pieces_count ());
        fobj.metadata.set ("torrent_piece_size", pbthash.get_piece_size ());
        fobj.metadata.set ("torrent_files_count", pbthash.get_files_count ());
        fobj.metadata.set (
            "torrent_bytes_downloaded", pbthash.get_bytes_downloaded ()
        );
        fobj.metadata.set (
            "torrent_bytes_uploaded", pbthash.get_bytes_uploaded ()
        );
        fobj.metadata.set ("torrent_path", pbthash.get_torrent_path ());
        fobj.metadata.set ("torrent_name", pbthash.get_torrent_name ());
        fobj.metadata.set (
            "torrent_download_started_time",
            pbthash.get_download_started_time ()
        );
    }

    is_valid_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add TempUL/UDPPHash_XXX.dat file
// @param f TempUL/UDPPHash_XXX.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_tempul_udpphash_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_phash phash (f.new_reader ());

    if (!phash)
    {
        log.info (
            __LINE__, "File " + f.get_path () + " is not a valid PHash.dat file"
        );
        return;
    }

    log.info (__LINE__, "File decoded [UDPPHash.dat] Path: " + f.get_path ());

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Process entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_deleted = f.is_deleted ();

    for (const auto &entry : phash)
    {
        auto hash_sha1 = entry.hash_sha1;
        auto [iter, success] = account_files_.try_emplace (hash_sha1, file {});
        std::ignore = success;
        auto &fobj = iter->second;

        if (!fobj.tempul_udpphash_f ||
            (fobj.tempul_udpphash_f.is_deleted () && !is_deleted))
        {
            fobj.hash_sha1 = hash_sha1;
            fobj.account_guid = account_.guid;
            fobj.username = username_;
            fobj.flag_uploaded = true;
            fobj.tempul_udpphash_f = f;
            fobj.metadata.set ("pieces_count", entry.pieces_count);
        }
    }

    is_valid_ = true;
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
// @brief Set username
// @param username username
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::set_username (const std::string &username)
{
    impl_->set_username (username);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folder
// @return Folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::folder
profile::get_folder () const
{
    return impl_->get_folder ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set folder
// @param f Folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::set_folder (const mobius::core::io::folder &f)
{
    impl_->set_folder (f);
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
// @brief Add ARESTRA file
// @param f ARESTRA file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_arestra_file (const mobius::core::io::file &f)
{
    impl_->add_arestra_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add NTUSER.DAT file
// @param f NTUSER.DAT file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_ntuser_dat_file (const mobius::core::io::file &f)
{
    impl_->add_ntuser_dat_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add PHashIdx.dat file
// @param f PHashIdx.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_phashidx_file (const mobius::core::io::file &f)
{
    impl_->add_phashidx_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add ShareH.dat file
// @param f ShareH.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_shareh_file (const mobius::core::io::file &f)
{
    impl_->add_shareh_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add ShareL.dat file
// @param f ShareL.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_sharel_file (const mobius::core::io::file &f)
{
    impl_->add_sharel_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add TorrentH.dat file
// @param f TorrentH.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_torrenth_file (const mobius::core::io::file &f)
{
    impl_->add_torrenth_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add TempDL/PHash_XXX.dat file
// @param f TempDL/PHash_XXX.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_tempdl_phash_file (const mobius::core::io::file &f)
{
    impl_->add_tempdl_phash_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add TempDL/PBTHash_XXX.dat file
// @param f TempDL/PBTHash_XXX.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_tempdl_pbthash_file (const mobius::core::io::file &f)
{
    impl_->add_tempdl_pbthash_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add TempUL/UDPPHash_XXX.dat file
// @param f TempUL/UDPPHash_XXX.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_tempul_udpphash_file (const mobius::core::io::file &f)
{
    impl_->add_tempul_udpphash_file (f);
}

} // namespace mobius::extension::app::ares
