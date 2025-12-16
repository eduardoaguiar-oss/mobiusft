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
#include "file_bt_fastresume.hpp"
#include "file_ed2k_fastresume.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
//      . libed2k v0.0.1
//
// Emule Torrent main files (* decoded by MobiusFT):
//
// * AppData/Local/Emuletorrent/BT_backup/*.fastresume: metadata for
//   downloading files (BitTorrent network)
//
// * AppData/Local/Emuletorrent/ED2K_backup/*.fastresume: metadata for
//   downloading files (ED2K network)
//
// * AppData/Roaming/mulehome/emuletorrent.ini: configuration file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string SAMPLING_ID = "sampling";
static const std::string APP_ID = "emuletorrent";
static const std::string APP_NAME = "EmuleTorrent";

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update metadata map, preferring non null values
// @param metadata Metadata map
// @param other Other metadata map
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_update_metadata (
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
_get_file_hashes (const auto &f)
{
    std::vector<mobius::core::pod::data> hashes;

    if (!f.hash_ed2k.empty ())
        hashes.push_back ({"ed2k", f.hash_ed2k});

    if (!f.hash_sha1.empty ())
        hashes.push_back ({"sha1", f.hash_sha1});

    if (!f.hash_sha2_256.empty ())
        hashes.push_back ({"sha2-256", f.hash_sha2_256});

    return hashes;
}

} // namespace

namespace mobius::extension::app::emuletorrent
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
    _scan_roaming_folder (folder);
    _scan_local_folder (folder);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Called when processing is complete
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::on_complete ()
{
    auto transaction = item_.new_transaction ();

    _save_app_profiles ();
    _save_local_files ();
    _save_received_files ();
    _save_remote_party_shared_files ();
    _save_sent_files ();
    _save_shared_files ();
    _save_user_accounts ();

    transaction.commit ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan Roaming folder
// @param folder Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_scan_roaming_folder (
    const mobius::core::io::folder &folder
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);
    mobius::core::io::walker w (folder);

    profile p;

    for (const auto &f : w.get_files_by_name ("emuletorrent.ini"))
        p.add_emuletorrent_ini_file (f);

    if (p)
        profiles_.push_back (p);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan Local folder
// @param folder Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_scan_local_folder (const mobius::core::io::folder &folder)
{
    mobius::core::log log (__FILE__, __FUNCTION__);
    mobius::core::io::walker w (folder);

    for (const auto &f : w.get_files_by_pattern ("BT_backup/*.fastresume"))
        _decode_bt_fastresume_file (f);

    for (const auto &f : w.get_files_by_pattern ("BT_backup/*.torrent"))
        log.development (__LINE__, "BT_backup: " + f.get_path ());

    for (const auto &f : w.get_files_by_pattern ("ED2K_backup/*.fastresume"))
        _decode_ed2k_fastresume_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode BT_backup/*.fastresume file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_decode_bt_fastresume_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        file_bt_fastresume bt (f.new_reader ());

        for (const auto &tf : bt.get_files ())
        {
            file et_file;
            et_file.username = mobius::framework::get_username_from_path (f.get_path ());
            et_file.filename = tf.name;
            et_file.size = tf.size;

            // flags
            if (bt.get_downloaded_bytes () > 0 ||
                bt.get_pieces_downloaded () > 0)
                et_file.flag_downloaded = true;

            if (bt.get_uploaded_bytes () > 0)
                et_file.flag_uploaded = true;

            et_file.flag_completed = bt.is_completed ();
            et_file.flag_shared = bt.is_seeding ();

            // remote sources
            for (const auto &p : bt.get_peers ())
            {
                remote_source r;

                r.timestamp = p.timestamp;
                r.ip = p.ip;
                r.port = p.port;

                et_file.remote_sources.push_back (r);
            }

            // metadata
            et_file.metadata.set ("magnet_uri", bt.get_magnet_uri ());
            et_file.metadata.set ("is_paused", bt.is_paused ());
            et_file.metadata.set ("is_completed", bt.is_completed ());
            et_file.metadata.set ("is_seeding", bt.is_seeding ());
            et_file.metadata.set (
                "downloaded_bytes", bt.get_downloaded_bytes ()
            );
            et_file.metadata.set ("uploaded_bytes", bt.get_uploaded_bytes ());
            et_file.metadata.set ("pieces_count", bt.get_pieces_count ());
            et_file.metadata.set (
                "pieces_downloaded", bt.get_pieces_downloaded ()
            );
            et_file.metadata.set (
                "last_modification_time", tf.last_modification_time
            );
            _update_metadata (et_file.metadata, bt.get_metadata ());

            // evidence sources
            et_file.f = f;

            files_.push_back (et_file);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode ED2K_backup/*.fastresume file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_decode_ed2k_fastresume_file (
    const mobius::core::io::file &f
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        file_ed2k_fastresume ed2k (f.new_reader ());

        file et_file;
        et_file.hash_ed2k = ed2k.get_hash_ed2k ();
        et_file.username = mobius::framework::get_username_from_path (f.get_path ());
        et_file.filename = ed2k.get_filename ();
        et_file.path = ed2k.get_path ();
        et_file.size = ed2k.get_file_size ();

        // flags
        if (ed2k.get_downloaded_bytes () > 0 ||
            ed2k.get_pieces_downloaded () > 0)
            et_file.flag_downloaded = true;

        if (ed2k.get_uploaded_bytes () > 0)
            et_file.flag_uploaded = true;

        et_file.flag_completed = ed2k.is_completed ();

        // metadata
        et_file.metadata.set ("url", ed2k.get_url ());
        et_file.metadata.set ("is_paused", ed2k.is_paused ());
        et_file.metadata.set ("downloaded_bytes", ed2k.get_downloaded_bytes ());
        et_file.metadata.set ("uploaded_bytes", ed2k.get_uploaded_bytes ());
        et_file.metadata.set ("pieces_count", ed2k.get_pieces_count ());
        et_file.metadata.set (
            "pieces_downloaded", ed2k.get_pieces_downloaded ()
        );
        _update_metadata (et_file.metadata, ed2k.get_metadata ());

        // evidence sources
        et_file.f = f;

        files_.push_back (et_file);
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
        e.set_attribute ("app_id", APP_ID);
        e.set_attribute ("app_name", APP_NAME);
        e.set_attribute ("username", p.get_username ());
        e.set_attribute ("creation_time", p.get_creation_time ());
        e.set_attribute ("last_modified_time", p.get_last_modified_time ());
        e.set_attribute ("path", p.get_path ());

        // Metadata
        auto metadata = mobius::core::pod::map ();

        metadata.set ("autorun", p.get_autorun ());
        metadata.set ("download_temp_path", p.get_download_temp_path ());
        metadata.set (
            "download_temp_path_mule", p.get_download_temp_path_mule ()
        );
        metadata.set ("download_save_path", p.get_download_save_path ());
        metadata.set (
            "download_save_path_mule", p.get_download_save_path_mule ()
        );
        metadata.set ("edonkey_guid", p.get_edonkey_guid ());
        metadata.set ("edonkey_nickname", p.get_edonkey_nickname ());

        e.set_attribute ("metadata", metadata);

        // Tags and sources
        e.set_tag ("app.p2p");
        e.add_source (p.get_folder ());
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
            // Create evidence
            auto e = item_.new_evidence ("local-file");

            e.set_attribute ("username", f.username);
            e.set_attribute ("path", f.path);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("hashes", _get_file_hashes (f));

            // Metadata
            mobius::core::pod::map metadata;

            metadata.set ("size", f.size);
            metadata.set ("flag_downloaded", to_string (f.flag_downloaded));
            metadata.set ("flag_uploaded", to_string (f.flag_uploaded));
            metadata.set ("flag_shared", to_string (f.flag_shared));
            metadata.set ("flag_corrupted", to_string (f.flag_corrupted));
            metadata.set ("flag_completed", to_string (f.flag_completed));

            _update_metadata (metadata, f.metadata);
            e.set_attribute ("metadata", metadata);

            // Tags
            e.set_tag ("app.p2p");

            // Sources
            e.add_source (f.f);
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
            // Create evidence
            auto e = item_.new_evidence ("received-file");

            e.set_attribute ("filename", f.filename);
            e.set_attribute ("path", f.path);
            e.set_attribute ("username", f.username);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("hashes", _get_file_hashes (f));

            // Metadata
            mobius::core::pod::map metadata;
            metadata.set ("flag_downloaded", to_string (f.flag_downloaded));
            metadata.set ("flag_uploaded", to_string (f.flag_uploaded));
            metadata.set ("flag_shared", to_string (f.flag_shared));
            metadata.set ("flag_corrupted", to_string (f.flag_corrupted));
            metadata.set ("flag_completed", to_string (f.flag_completed));
            _update_metadata (metadata, f.metadata);

            e.set_attribute ("metadata", metadata);

            // Tags
            e.set_tag ("app.p2p");

            // Sources
            e.add_source (f.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save remote files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_remote_party_shared_files ()
{
    for (const auto &f : files_)
    {
        for (const auto &rs : f.remote_sources)
        {
            // Create evidence
            auto e = item_.new_evidence ("remote-party-shared-file");
            e.set_attribute ("timestamp", rs.timestamp);
            e.set_attribute ("ip", rs.ip);
            e.set_attribute ("port", rs.port);
            e.set_attribute ("filename", f.filename);
            e.set_attribute ("username", f.username);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("hashes", _get_file_hashes (f));

            // Metadata
            mobius::core::pod::map metadata;

            metadata.set ("size", f.size);
            _update_metadata (metadata, f.metadata);

            e.set_attribute ("metadata", metadata);

            // Tags
            e.set_tag ("app.p2p");

            // Sources
            e.add_source (f.f);
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
            // Create evidence
            auto e = item_.new_evidence ("sent-file");

            e.set_attribute ("filename", f.filename);
            e.set_attribute ("path", f.path);
            e.set_attribute ("username", f.username);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("hashes", _get_file_hashes (f));

            // Metadata
            mobius::core::pod::map metadata;

            metadata.set ("flag_downloaded", to_string (f.flag_downloaded));
            metadata.set ("flag_uploaded", to_string (f.flag_uploaded));
            metadata.set ("flag_shared", to_string (f.flag_shared));
            metadata.set ("flag_corrupted", to_string (f.flag_corrupted));
            metadata.set ("flag_completed", to_string (f.flag_completed));
            _update_metadata (metadata, f.metadata);

            e.set_attribute ("metadata", metadata);

            // Tags
            e.set_tag ("app.p2p");

            // Sources
            e.add_source (f.f);
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
            // Create evidence
            auto e = item_.new_evidence ("shared-file");

            e.set_attribute ("username", f.username);
            e.set_attribute ("filename", f.filename);
            e.set_attribute ("path", f.path);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("hashes", _get_file_hashes (f));

            // Metadata
            mobius::core::pod::map metadata;

            metadata.set ("size", f.size);
            metadata.set ("flag_downloaded", to_string (f.flag_downloaded));
            metadata.set ("flag_uploaded", to_string (f.flag_uploaded));
            metadata.set ("flag_shared", to_string (f.flag_shared));
            metadata.set ("flag_corrupted", to_string (f.flag_corrupted));
            metadata.set ("flag_completed", to_string (f.flag_completed));
            _update_metadata (metadata, f.metadata);

            e.set_attribute ("metadata", metadata);

            // Tags
            e.set_tag ("app.p2p");

            // Sources
            e.add_source (f.f);
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
        auto f = p.get_file ();

        auto e = item_.new_evidence ("user-account");
        e.set_attribute ("account_type", "p2p.edonkey");
        e.set_attribute ("id", p.get_edonkey_guid ());
        e.set_attribute ("name", p.get_edonkey_nickname ());
        e.set_attribute ("password", {});
        e.set_attribute ("password_found", "no");
        e.set_attribute ("is_deleted", f.is_deleted ());

        mobius::core::pod::map metadata;
        metadata.set ("app_id", APP_ID);
        metadata.set ("app_name", APP_NAME);
        metadata.set ("username", p.get_username ());
        metadata.set ("network", "eDonkey");
        metadata.set ("autorun", p.get_autorun ());
        metadata.set ("edonkey_guid", p.get_edonkey_guid ());
        metadata.set ("edonkey_nickname", p.get_edonkey_nickname ());
        metadata.set ("download_temp_path", p.get_download_temp_path ());
        metadata.set (
            "download_temp_path_mule", p.get_download_temp_path_mule ()
        );
        metadata.set ("download_save_path", p.get_download_save_path ());
        metadata.set (
            "download_save_path_mule", p.get_download_save_path_mule ()
        );
        e.set_attribute ("metadata", metadata);

        e.set_tag ("app.p2p");
        e.add_source (f);
    }
}

} // namespace mobius::extension::app::emuletorrent
