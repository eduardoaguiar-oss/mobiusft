// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
//      . libed2k v0.0.1
//
// Emule Torrent main files (* decoded by MobiusFT):
//
// . AppData/Local/Emuletorrent/BT_backup/*.fastresume: metadata for
//   downloading files (BitTorrent network)
//
// . AppData/Local/Emuletorrent/ED2K_backup/*.fastresume: metadata for
//   downloading files (ED2K network)
//
// . AppData/Roaming/mulehome/emuletorrent.ini: configuration file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "evidence_loader_impl.hpp"
#include "file_bt_fastresume.hpp"
#include "file_ed2k_fastresume.hpp"
#include <mobius/io/walker.h>
#include <mobius/core/log.hpp>
#include <mobius/core/datasource/datasource_vfs.hpp>
#include <mobius/decoder/inifile.h>
#include <mobius/exception.inc>
#include <mobius/framework/model/evidence.hpp>
#include <mobius/io/folder.h>
#include <mobius/io/path.h>
#include <mobius/string_functions.h>
#include <algorithm>
#include <stdexcept>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string APP_ID = "emuletorrent";
static const std::string APP_NAME = "Emule Torrent";
static const std::string ANT_ID = "evidence.app-emuletorrent";
static const std::string ANT_NAME = APP_NAME;
static const std::string ANT_VERSION = "1.2";

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update metadata map, preferring non null values
// @param metadata Metadata map
// @param other Other metadata map
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_update_metadata (mobius::pod::map& metadata, const mobius::pod::map& other)
{
  for (const auto& [k, v] : other)
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
std::vector <mobius::pod::data>
_get_file_hashes (const mobius::extension::app::emuletorrent::file& f)
{
  std::vector <mobius::pod::data> hashes;

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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
evidence_loader_impl::evidence_loader_impl (const mobius::framework::model::item& item, scan_type type)
  : item_ (item),
    scan_type_ (type)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan item files for evidences
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::run ()
{
  mobius::core::log log (__FILE__, __FUNCTION__);
  log.info (__LINE__, "Evidence loader <" + APP_ID + "> started");
  log.info (__LINE__, "Item UID: " + std::to_string (item_.get_uid ()));
  log.info (__LINE__, "Scan mode: " + std::to_string (static_cast <int> (scan_type_)));

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Check if loader has already run for item
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (item_.has_ant (ANT_ID))
    {
      log.info (__LINE__, "Evidence loader <" + APP_ID + "> has already run");
      return ;
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Check datasource
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto datasource = item_.get_datasource ();

  if (!datasource)
    throw std::runtime_error ("item has no datasource");

  if (datasource.get_type () != "vfs")
    throw std::runtime_error ("datasource type is not VFS");

  if (!datasource.is_available ())
    throw std::runtime_error ("datasource is not available");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Log starting event
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto transaction = item_.new_transaction ();
  item_.add_event ("app." + APP_ID + " started");
  transaction.commit ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Scan item files, according to scan_type
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  switch (scan_type_)
    {
      case scan_type::canonical_folders:
        _scan_canonical_folders ();
        break;

      case scan_type::all_folders:
        _scan_all_folders ();
        break;

      default:
        log.warning (__LINE__, "invalid scan type: " + std::to_string (static_cast <int> (scan_type_)));
        return;
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Save evidences
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  _save_evidences ();

  transaction = item_.new_transaction ();
  item_.add_event ("app." + APP_ID + " ended");
  transaction.commit ();

  log.info (__LINE__, "Evidence loader <" + APP_ID + "> ended");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan canonical folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_folders ()
{
  auto vfs_datasource = mobius::core::datasource::datasource_vfs (item_.get_datasource ());
  auto vfs = vfs_datasource.get_vfs ();

  for (const auto& entry : vfs.get_root_entries ())
    {
      if (entry.is_folder ())
        _scan_canonical_root_folder (entry.get_folder ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan root folder for evidences
// @param folder Root folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_root_folder (const mobius::io::folder& folder)
{
  auto w = mobius::io::walker (folder);

  for (const auto& f : w.get_folders_by_pattern ("users/*"))
    _scan_canonical_user_folder (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan user folder for evidences
// @param folder User folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_user_folder (const mobius::io::folder& folder)
{
  username_ = folder.get_name ();
  account_ = {};

  auto w = mobius::io::walker (folder);

  for (const auto& f : w.get_folders_by_path ("appdata/roaming/mulehome"))
    _scan_canonical_emuletorrent_roaming_folder (f);

  for (const auto& f : w.get_folders_by_path ("appdata/local/eMuleTorrent"))
    _scan_canonical_emuletorrent_local_folder (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan AppData/Roaming/mulehome folder for evidences
// @param folder <i>mulehome</i> folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_emuletorrent_roaming_folder (const mobius::io::folder& folder)
{
  mobius::io::walker w (folder);

  for (const auto& f : w.get_files_by_name ("emuletorrent.ini"))
    _decode_emuletorrent_ini_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan AppData/Local/eMuleTorrent folder for evidences
// @param folder <i>eMuleTorrent</i> folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_emuletorrent_local_folder (const mobius::io::folder& folder)
{
  mobius::core::log log (__FILE__, __FUNCTION__);
  mobius::io::walker w (folder);

  for (const auto& f : w.get_files_by_pattern ("BT_backup/*.fastresume"))
    _decode_bt_fastresume_file (f);

  for (const auto& f : w.get_files_by_pattern ("BT_backup/*.torrent"))
    log.development (__LINE__, "BT_backup: " + f.get_path ());

  for (const auto& f : w.get_files_by_pattern ("ED2K_backup/*.fastresume"))
    _decode_ed2k_fastresume_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode eMuleTorrent.ini file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_emuletorrent_ini_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      mobius::decoder::inifile inifile (f.new_reader ());

      if (!account_.f || (account_.f.is_deleted () && !f.is_deleted ()))
        {
          account acc;

          acc.username = username_;
          acc.edonkey_guid = mobius::string::toupper (inifile.get_value ("Preferences", "eDonkey\\UserHash"));
          acc.edonkey_nickname = inifile.get_value ("Preferences", "eDonkey\\Nick");
          acc.download_temp_path = inifile.get_value ("Preferences", "Downloads\\TempPath");
          acc.download_temp_path_mule = inifile.get_value ("Preferences", "Downloads\\TempPathMule");
          acc.download_save_path = inifile.get_value ("Preferences", "Downloads\\SavePath");
          acc.download_save_path_mule = inifile.get_value ("Preferences", "Downloads\\SavePathMule");
          acc.autorun = inifile.get_value ("AutoRun", "enabled") == "true";
          acc.f = f;

          account_ = acc;
          accounts_.push_back (acc);

          // Waiting for samples...
          auto shared_dirs = inifile.get_value ("SharedDirectories", "ShareDirs\\size");
          if (!shared_dirs.empty () && shared_dirs != "0")
            log.development (__LINE__, "eMuleTorrent shared dirs found");

          auto friends = inifile.get_value ("ED2KFriends", "Friends\\size");
          if (!friends.empty () && friends != "0")
            log.development (__LINE__, "eMuleTorrent ED2KFriends found");
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode BT_backup/*.fastresume file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_bt_fastresume_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      file_bt_fastresume bt (f.new_reader ());

      for (const auto& tf : bt.get_files ())
        {
          file et_file;
          et_file.account_guid = account_.edonkey_guid;
          et_file.username = username_;
          et_file.filename = tf.name;
          et_file.size = tf.size;

          // flags
          if (bt.get_downloaded_bytes () > 0 || bt.get_pieces_downloaded () > 0)
            et_file.flag_downloaded = true;

          if (bt.get_uploaded_bytes () > 0)
            et_file.flag_uploaded = true;

          et_file.flag_completed = bt.is_completed ();
          et_file.flag_shared = bt.is_seeding ();

          // remote sources
          for (const auto& p : bt.get_peers ())
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
          et_file.metadata.set ("downloaded_bytes", bt.get_downloaded_bytes ());
          et_file.metadata.set ("uploaded_bytes", bt.get_uploaded_bytes ());
          et_file.metadata.set ("pieces_count", bt.get_pieces_count ());
          et_file.metadata.set ("pieces_downloaded", bt.get_pieces_downloaded ());
          et_file.metadata.set ("last_modification_time", tf.last_modification_time);
          _update_metadata (et_file.metadata, bt.get_metadata ());

          // evidence sources
          et_file.bt_fastresume_f = f;

          files_.push_back (et_file);
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode ED2K_backup/*.fastresume file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_ed2k_fastresume_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      file_ed2k_fastresume ed2k (f.new_reader ());

      file et_file;
      et_file.hash_ed2k = ed2k.get_hash_ed2k ();
      et_file.account_guid = account_.edonkey_guid;
      et_file.username = username_;
      et_file.filename = ed2k.get_filename ();
      et_file.path = ed2k.get_path ();
      et_file.size = ed2k.get_file_size ();

      // flags
      if (ed2k.get_downloaded_bytes () > 0 || ed2k.get_pieces_downloaded () > 0)
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
      et_file.metadata.set ("pieces_downloaded", ed2k.get_pieces_downloaded ());
      _update_metadata (et_file.metadata, ed2k.get_metadata ());

      // evidence sources
      et_file.ed2k_fastresume_f = f;

      files_.push_back (et_file);
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan all folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_all_folders ()
{
  auto vfs_datasource = mobius::core::datasource::datasource_vfs (item_.get_datasource ());
  auto vfs = vfs_datasource.get_vfs ();

  for (const auto& entry : vfs.get_root_entries ())
    {
      if (entry.is_folder ())
        _scan_generic_folder (entry.get_folder ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan generic folder
// @param folder Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_generic_folder (const mobius::io::folder& folder)
{
  mobius::io::walker w (folder);

  for (const auto& f : w.get_files ())
    {
      const std::string lname = mobius::string::tolower (f.get_name ());

      if (lname == "emuletorrent.ini")
        _decode_emuletorrent_ini_file (f);
    }

  for (const auto& child : w.get_folders ())
    _scan_generic_folder (child);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save evidences
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_evidences ()
{
  auto transaction = item_.new_transaction ();

  _save_accounts ();
  _save_local_files ();
  _save_p2p_remote_files ();
  _save_received_files ();
  _save_sent_files ();
  _save_shared_files ();

  item_.set_ant (ANT_ID, ANT_NAME, ANT_VERSION);
  transaction.commit ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save accounts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_accounts ()
{
  for (const auto& a : accounts_)
    {
      mobius::pod::map metadata;
      metadata.set ("app_id", APP_ID);
      metadata.set ("app_name", APP_NAME);
      metadata.set ("username", a.username);
      metadata.set ("network", "eDonkey");
      metadata.set ("autorun", a.autorun);
      metadata.set ("edonkey_guid", a.edonkey_guid);
      metadata.set ("edonkey_nickname", a.edonkey_nickname);
      metadata.set ("download_temp_path", a.download_temp_path);
      metadata.set ("download_temp_path_mule", a.download_temp_path_mule);
      metadata.set ("download_save_path", a.download_save_path);
      metadata.set ("download_save_path_mule", a.download_save_path_mule);

      auto e = item_.new_evidence ("user-account");

      e.set_attribute ("account_type", "p2p.edonkey");
      e.set_attribute ("id", a.edonkey_guid);
      e.set_attribute ("name", a.edonkey_nickname);
      e.set_attribute ("password", {});
      e.set_attribute ("password_found", "no");
      e.set_attribute ("is_deleted", a.f.is_deleted ());
      e.set_attribute ("metadata", metadata);
      e.set_tag ("p2p");
      e.add_source (a.f);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save local files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_local_files ()
{
  for (const auto& f : files_)
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
          e.set_attribute ("hashes", _get_file_hashes (f));

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Metadata
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          mobius::pod::map metadata;

          metadata.set ("size", f.size);
          metadata.set ("flag_downloaded", to_string (f.flag_downloaded));
          metadata.set ("flag_uploaded", to_string (f.flag_uploaded));
          metadata.set ("flag_shared", to_string (f.flag_shared));
          metadata.set ("flag_corrupted", to_string (f.flag_corrupted));
          metadata.set ("flag_completed", to_string (f.flag_completed));

          _update_metadata (metadata, f.metadata);
          e.set_attribute ("metadata", metadata);

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Tags
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          e.set_tag ("p2p");

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Sources
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          e.add_source (f.bt_fastresume_f);
          e.add_source (f.ed2k_fastresume_f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save received files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_received_files ()
{
  for (const auto& f : files_)
    {
      if (f.flag_downloaded.is_yes ())
        {
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Create evidence
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          auto e = item_.new_evidence ("received-file");

          e.set_attribute ("filename", f.filename);
          e.set_attribute ("path", f.path);
          e.set_attribute ("username", f.username);
          e.set_attribute ("app_id", APP_ID);
          e.set_attribute ("app_name", APP_NAME);
          e.set_attribute ("hashes", _get_file_hashes (f));

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Metadata
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          mobius::pod::map metadata;
          metadata.set ("flag_downloaded", to_string (f.flag_downloaded));
          metadata.set ("flag_uploaded", to_string (f.flag_uploaded));
          metadata.set ("flag_shared", to_string (f.flag_shared));
          metadata.set ("flag_corrupted", to_string (f.flag_corrupted));
          metadata.set ("flag_completed", to_string (f.flag_completed));
          _update_metadata (metadata, f.metadata);

          e.set_attribute ("metadata", metadata);

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Tags
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          e.set_tag ("p2p");

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Sources
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          e.add_source (f.bt_fastresume_f);
          e.add_source (f.ed2k_fastresume_f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save remote files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_p2p_remote_files ()
{
  for (const auto& f : files_)
    {
      for (const auto& rs : f.remote_sources)
        {
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Create evidence
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          auto e = item_.new_evidence ("p2p-remote-file");
          e.set_attribute ("timestamp", rs.timestamp);
          e.set_attribute ("ip", rs.ip);
          e.set_attribute ("port", rs.port);
          e.set_attribute ("filename", f.filename);
          e.set_attribute ("username", f.username);
          e.set_attribute ("app_id", APP_ID);
          e.set_attribute ("app_name", APP_NAME);
          e.set_attribute ("hashes", _get_file_hashes (f));

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Metadata
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          mobius::pod::map metadata;

          metadata.set ("size", f.size);
          _update_metadata (metadata, f.metadata);

          e.set_attribute ("metadata", metadata);

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Tags
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          e.set_tag ("p2p");

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Sources
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          e.add_source (f.bt_fastresume_f);
          e.add_source (f.ed2k_fastresume_f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save sent files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_sent_files ()
{
  for (const auto& f : files_)
    {
      if (f.flag_uploaded.is_yes ())
        {
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Create evidence
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          auto e = item_.new_evidence ("sent-file");

          e.set_attribute ("filename", f.filename);
          e.set_attribute ("path", f.path);
          e.set_attribute ("username", f.username);
          e.set_attribute ("app_id", APP_ID);
          e.set_attribute ("app_name", APP_NAME);
          e.set_attribute ("hashes", _get_file_hashes (f));

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Metadata
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          mobius::pod::map metadata;

          metadata.set ("flag_downloaded", to_string (f.flag_downloaded));
          metadata.set ("flag_uploaded", to_string (f.flag_uploaded));
          metadata.set ("flag_shared", to_string (f.flag_shared));
          metadata.set ("flag_corrupted", to_string (f.flag_corrupted));
          metadata.set ("flag_completed", to_string (f.flag_completed));
          _update_metadata (metadata, f.metadata);

          e.set_attribute ("metadata", metadata);

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Tags
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          e.set_tag ("p2p");

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Sources
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          e.add_source (f.bt_fastresume_f);
          e.add_source (f.ed2k_fastresume_f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save shared files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_shared_files ()
{
  for (const auto& f : files_)
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
          e.set_attribute ("hashes", _get_file_hashes (f));

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Metadata
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          mobius::pod::map metadata;

          metadata.set ("size", f.size);
          metadata.set ("flag_downloaded", to_string (f.flag_downloaded));
          metadata.set ("flag_uploaded", to_string (f.flag_uploaded));
          metadata.set ("flag_shared", to_string (f.flag_shared));
          metadata.set ("flag_corrupted", to_string (f.flag_corrupted));
          metadata.set ("flag_completed", to_string (f.flag_completed));
          _update_metadata (metadata, f.metadata);

          e.set_attribute ("metadata", metadata);

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Tags
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          e.set_tag ("p2p");

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Sources
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          e.add_source (f.bt_fastresume_f);
          e.add_source (f.ed2k_fastresume_f);
        }
    }
}

} // namespace mobius::extension::app::ares



