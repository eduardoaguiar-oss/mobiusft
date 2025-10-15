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
// References:
// @see
// https://www.forensicfocus.com/articles/forensic-analysis-of-the-%CE%BCtorrent-peer-to-peer-client-in-windows/
// @see
// https://robertpearsonblog.wordpress.com/2016/11/10/utorrent-forensic-artifacts/
// @see
// https://robertpearsonblog.wordpress.com/2016/11/11/utorrent-and-windows-10-forensic-nuggets-of-info/
// @see libtorrent source code
//
// µTorrent main forensic files:
// - settings.dat: contains the settings of the µTorrent client
// - resume.dat: contains the resume data of the torrents
// - dht.dat: contains the DHT data
// - *.torrent: contains information about torrents
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string SAMPLING_ID = "sampling";
static const std::string APP_ID = "utorrent";
static const std::string APP_NAME = "µTorrent/BitTorrent";

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get metadata from local_file
// @param lf Local file structure
// @return Metadata
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::pod::map
get_metadata (const mobius::extension::app::utorrent::profile::local_file &lf)
{
    auto lf_metadata = mobius::core::pod::map ();

    lf_metadata.set ("app_id", APP_ID);
    lf_metadata.set ("app_name", APP_NAME);
    lf_metadata.set ("download_url", lf.download_url);
    lf_metadata.set ("caption", lf.caption);
    lf_metadata.set ("comment", lf.comment);
    lf_metadata.set ("size", lf.size);
    lf_metadata.set ("seeded_seconds", lf.seeded_seconds);
    lf_metadata.set ("downloaded_seconds", lf.downloaded_seconds);
    lf_metadata.set ("blocksize", lf.blocksize);
    lf_metadata.set ("bytes_downloaded", lf.bytes_downloaded);
    lf_metadata.set ("bytes_uploaded", lf.bytes_uploaded);
    lf_metadata.set ("creation_time", lf.creation_time);
    lf_metadata.set ("metadata_time", lf.metadata_time);
    lf_metadata.set ("added_time", lf.added_time);
    lf_metadata.set ("completed_time", lf.completed_time);
    lf_metadata.set ("last_seen_complete_time", lf.last_seen_complete_time);
    lf_metadata.set ("torrent_name", lf.torrent_name);
    lf_metadata.set ("created_by", lf.created_by);
    lf_metadata.set ("encoding", lf.encoding);
    lf_metadata.set ("info_hash", lf.info_hash);
    lf_metadata.set ("local_file_path", lf.path);

    mobius::framework::evidence_flag flag_downloaded;
    mobius::framework::evidence_flag flag_uploaded;
    mobius::framework::evidence_flag flag_shared;
    mobius::framework::evidence_flag flag_completed;

    if (lf.resume_file)
    {
        flag_downloaded =
            (lf.bytes_downloaded > 0 || lf.downloaded_seconds > 0);
        flag_uploaded = (lf.bytes_uploaded > 0);
        flag_shared = (lf.seeded_seconds > 0);
        flag_completed = bool (lf.completed_time);
    }

    lf_metadata.set ("flag_downloaded", flag_downloaded.to_string ());
    lf_metadata.set ("flag_uploaded", flag_uploaded.to_string ());
    lf_metadata.set ("flag_shared", flag_shared.to_string ());
    lf_metadata.set ("flag_completed", flag_completed.to_string ());

    return lf_metadata;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Join paths
// @param root Root path
// @param rpath Relative path
// @return Joined path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
join_paths (const std::string &root, const std::string &rpath)
{
    auto path = root;

    if (!rpath.empty ())
    {
        if (!path.empty ())
            path += '/';

        path += rpath;
    }

    if (path.find_first_of ("\\") != std::string::npos)
        path = mobius::core::string::replace (path, "/", "\\");

    return path;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get filename from path
// @param path Path
// @return Filename
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
get_filename (const std::string &path)
{
    auto filename = path;

    auto pos = filename.find_last_of ("\\");
    if (pos != std::string::npos)
        filename = filename.substr (pos + 1);

    else
    {
        pos = filename.find_last_of ("/");
        if (pos != std::string::npos)
            filename = filename.substr (pos + 1);
    }

    return filename;
}

} // namespace

namespace mobius::extension::app::utorrent
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
    _save_ip_addresses ();
    _save_local_files ();
    _save_p2p_remote_files ();
    _save_received_files ();
    _save_sent_files ();
    _save_shared_files ();
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
            if (name == "settings.dat" || name == "settings.dat.old")
                p.add_settings_dat_file (f);

            else if (name == "dht.dat" || name == "dht.dat.old")
                p.add_dht_dat_file (f);

            else if (name == "resume.dat" || name == "resume.dat.old")
                p.add_resume_dat_file (f);
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
    // Scan .torrent files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &[name, f] : w.get_files_with_names ())
    {
        try
        {
            if (mobius::core::string::endswith (name, ".torrent"))
            {
                if (p)
                    p.add_torrent_file (f);
            }
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

        metadata.set ("num_files", p.size_local_files ());
        e.set_attribute ("metadata", metadata);

        // Tags and sources
        e.set_tag ("app.browser");
        e.add_source (p.get_folder ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save IP addresses
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_ip_addresses ()
{
    for (const auto &p : profiles_)
    {
        auto settings = p.get_main_settings ();

        mobius::core::pod::map metadata;
        metadata.set ("network", "BitTorrent");
        metadata.set (
            "total_downloaded_bytes", settings.total_bytes_downloaded
        );
        metadata.set ("total_uploaded_bytes", settings.total_bytes_uploaded);
        metadata.set ("execution_count", settings.execution_count);
        metadata.set ("installation_time", settings.installation_time);
        metadata.set ("last_used_time", settings.last_used_time);
        metadata.set ("last_bin_change_time", settings.last_bin_change_time);
        metadata.set ("version", settings.version);
        metadata.set ("installation_version", settings.installation_version);
        metadata.set ("language", settings.language);
        metadata.set ("computer_id", settings.computer_id);
        metadata.set ("auto_start", settings.auto_start ? "yes" : "no");

        for (const auto &account : p.get_accounts ())
        {
            auto e_metadata = metadata.clone ();
            e_metadata.set ("client_id", account.client_id);
            e_metadata.set ("first_dht_timestamp", account.first_dht_timestamp);
            e_metadata.set ("last_dht_timestamp", account.last_dht_timestamp);

            for (const auto &[ip, timestamp] : account.ip_addresses)
            {
                auto e = item_.new_evidence ("ip-address");

                e.set_attribute ("timestamp", timestamp);
                e.set_attribute ("address", ip);
                e.set_attribute ("app_id", APP_ID);
                e.set_attribute ("app_name", APP_NAME);
                e.set_attribute ("username", p.get_username ());
                e.set_attribute ("metadata", e_metadata.clone ());
                e.set_tag ("p2p");

                for (const auto &f : account.files)
                    e.add_source (f);

                e.add_source (settings.f);
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save local files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_local_files ()
{
    for (const auto &profile : profiles_)
    {
        for (const auto &lf : profile.get_local_files ())
        {
            if (!lf.path.empty ())
            {
                auto lf_metadata = get_metadata (lf);
                lf_metadata.set ("username", profile.get_username ());

                for (const auto &tf : lf.content_files)
                {
                    auto path = join_paths (lf.path, tf.path);
                    auto filename = get_filename (path);

                    auto e = item_.new_evidence ("local-file");

                    e.set_attribute ("username", profile.get_username ());
                    e.set_attribute ("filename", filename);
                    e.set_attribute ("path", path);
                    e.set_attribute ("app_id", APP_ID);
                    e.set_attribute ("app_name", APP_NAME);
                    // e.set_attribute ("hashes", get_file_hashes (tf));

                    auto tf_metadata = lf_metadata.clone ();
                    tf_metadata.set ("torrent_path", tf.path);
                    tf_metadata.set ("torrent_offset", tf.offset);
                    tf_metadata.set ("torrent_length", tf.length);
                    tf_metadata.set ("torrent_piece_length", tf.piece_length);
                    tf_metadata.set ("torrent_piece_offset", tf.piece_offset);

                    e.set_attribute ("metadata", tf_metadata);

                    e.set_tag ("p2p");
                    for (const auto &f : lf.sources)
                        e.add_source (f);
                }
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save received files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_received_files ()
{
    for (const auto &profile : profiles_)
    {
        for (const auto &lf : profile.get_local_files ())
        {
            if (lf.bytes_downloaded > 0 || lf.downloaded_seconds > 0)
            {
                auto lf_metadata = get_metadata (lf);
                lf_metadata.set ("username", profile.get_username ());

                for (const auto &tf : lf.content_files)
                {
                    auto path = join_paths (lf.path, tf.path);
                    auto filename = get_filename (path);

                    auto e = item_.new_evidence ("received-file");

                    e.set_attribute ("timestamp", lf.added_time);
                    e.set_attribute ("username", profile.get_username ());
                    e.set_attribute ("filename", filename);
                    e.set_attribute ("path", path);
                    e.set_attribute ("app_id", APP_ID);
                    e.set_attribute ("app_name", APP_NAME);
                    // e.set_attribute ("hashes", get_file_hashes (tf));

                    auto tf_metadata = lf_metadata.clone ();
                    tf_metadata.set ("torrent_path", tf.path);
                    tf_metadata.set ("torrent_offset", tf.offset);
                    tf_metadata.set ("torrent_length", tf.length);
                    tf_metadata.set ("torrent_piece_length", tf.piece_length);
                    tf_metadata.set ("torrent_piece_offset", tf.piece_offset);

                    e.set_attribute ("metadata", tf_metadata);

                    e.set_tag ("p2p");
                    for (const auto &f : lf.sources)
                        e.add_source (f);
                }
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save remote files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_p2p_remote_files ()
{
    for (const auto &profile : profiles_)
    {
        for (const auto &lf : profile.get_local_files ())
        {
            auto username = profile.get_username ();

            if (lf.metadata_time && !lf.peers.empty ())
            {
                auto lf_metadata = get_metadata (lf);

                for (const auto &tf : lf.content_files)
                {
                    auto path = join_paths (lf.path, tf.path);
                    auto filename = get_filename (path);

                    for (const auto &[ip, port] : lf.peers)
                    {
                        auto e = item_.new_evidence ("p2p-remote-file");

                        e.set_attribute ("timestamp", lf.metadata_time);
                        e.set_attribute ("ip", ip);
                        e.set_attribute ("port", port);
                        e.set_attribute ("filename", filename);
                        e.set_attribute ("username", username);
                        e.set_attribute ("app_id", APP_ID);
                        e.set_attribute ("app_name", APP_NAME);
                        e.set_attribute ("path", path);
                        // e.set_attribute ("hashes", get_file_hashes (tf));

                        auto tf_metadata = lf_metadata.clone ();
                        tf_metadata.set ("torrent_path", tf.path);
                        tf_metadata.set ("torrent_offset", tf.offset);
                        tf_metadata.set ("torrent_length", tf.length);
                        tf_metadata.set (
                            "torrent_piece_length", tf.piece_length
                        );
                        tf_metadata.set (
                            "torrent_piece_offset", tf.piece_offset
                        );

                        e.set_attribute ("metadata", tf_metadata);

                        e.set_tag ("p2p");
                        for (const auto &f : lf.sources)
                            e.add_source (f);
                    }
                }
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
    for (const auto &profile : profiles_)
    {
        for (const auto &lf : profile.get_local_files ())
        {
            if (lf.bytes_uploaded > 0)
            {
                auto lf_metadata = get_metadata (lf);
                lf_metadata.set ("username", profile.get_username ());

                for (const auto &tf : lf.content_files)
                {
                    auto path = join_paths (lf.path, tf.path);
                    auto filename = get_filename (path);

                    auto e = item_.new_evidence ("sent-file");

                    e.set_attribute ("timestamp", lf.added_time);
                    e.set_attribute ("username", profile.get_username ());
                    e.set_attribute ("filename", filename);
                    e.set_attribute ("path", path);
                    e.set_attribute ("app_id", APP_ID);
                    e.set_attribute ("app_name", APP_NAME);
                    // e.set_attribute ("hashes", get_file_hashes (tf));

                    auto tf_metadata = lf_metadata.clone ();
                    tf_metadata.set ("torrent_path", tf.path);
                    tf_metadata.set ("torrent_offset", tf.offset);
                    tf_metadata.set ("torrent_length", tf.length);
                    tf_metadata.set ("torrent_piece_length", tf.piece_length);
                    tf_metadata.set ("torrent_piece_offset", tf.piece_offset);

                    e.set_attribute ("metadata", tf_metadata);

                    e.set_tag ("p2p");
                    for (const auto &f : lf.sources)
                        e.add_source (f);
                }
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save shared files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_shared_files ()
{
    for (const auto &profile : profiles_)
    {
        for (const auto &lf : profile.get_local_files ())
        {
            if (lf.seeded_seconds > 0)
            {
                auto lf_metadata = get_metadata (lf);
                lf_metadata.set ("username", profile.get_username ());

                for (const auto &tf : lf.content_files)
                {
                    auto path = join_paths (lf.path, tf.path);
                    auto filename = get_filename (path);

                    auto e = item_.new_evidence ("shared-file");

                    e.set_attribute ("username", profile.get_username ());
                    e.set_attribute ("filename", filename);
                    e.set_attribute ("path", path);
                    e.set_attribute ("app_id", APP_ID);
                    e.set_attribute ("app_name", APP_NAME);
                    // e.set_attribute ("hashes", get_file_hashes (tf));

                    auto tf_metadata = lf_metadata.clone ();
                    tf_metadata.set ("torrent_path", tf.path);
                    tf_metadata.set ("torrent_offset", tf.offset);
                    tf_metadata.set ("torrent_length", tf.length);
                    tf_metadata.set ("torrent_piece_length", tf.piece_length);
                    tf_metadata.set ("torrent_piece_offset", tf.piece_offset);

                    e.set_attribute ("metadata", tf_metadata);

                    e.set_tag ("p2p");
                    for (const auto &f : lf.sources)
                        e.add_source (f);
                }
            }
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
        auto settings = p.get_main_settings ();

        mobius::core::pod::map metadata;
        metadata.set ("app_id", APP_ID);
        metadata.set ("app_name", APP_NAME);
        metadata.set ("network", "BitTorrent");
        metadata.set ("username", p.get_username ());
        metadata.set (
            "total_downloaded_bytes", settings.total_bytes_downloaded
        );
        metadata.set ("total_uploaded_bytes", settings.total_bytes_uploaded);
        metadata.set ("execution_count", settings.execution_count);
        metadata.set ("installation_time", settings.installation_time);
        metadata.set ("last_used_time", settings.last_used_time);
        metadata.set ("last_bin_change_time", settings.last_bin_change_time);
        metadata.set ("version", settings.version);
        metadata.set ("installation_version", settings.installation_version);
        metadata.set ("language", settings.language);
        metadata.set ("computer_id", settings.computer_id);
        metadata.set ("auto_start", settings.auto_start ? "yes" : "no");

        for (const auto &account : p.get_accounts ())
        {
            auto e_metadata = metadata.clone ();
            e_metadata.set ("first_dht_timestamp", account.first_dht_timestamp);
            e_metadata.set ("last_dht_timestamp", account.last_dht_timestamp);

            auto e = item_.new_evidence ("user-account");

            e.set_attribute ("account_type", "p2p.bittorrent");
            e.set_attribute ("id", account.client_id);
            e.set_attribute ("password", {});
            e.set_attribute ("password_found", "no");
            e.set_attribute ("is_deleted", account.f.is_deleted ());
            e.set_attribute ("metadata", e_metadata);
            e.set_tag ("p2p");

            for (const auto &f : account.files)
                e.add_source (f);

            e.add_source (settings.f);
        }
    }
}

} // namespace mobius::extension::app::utorrent
