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
//
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "evidence_loader_impl.hpp"
#include "file_arestra.hpp"
#include "file_pbthash.hpp"
#include "file_phash.hpp"
#include "file_shareh.hpp"
#include "file_sharel.hpp"
#include "file_torrenth.hpp"
#include <mobius/io/walker.h>
#include <mobius/core/log.hpp>
#include <mobius/decoder/hexstring.h>
#include <mobius/datasource/datasource_vfs.h>
#include <mobius/exception.inc>
#include <mobius/io/folder.h>
#include <mobius/io/path.h>
#include <mobius/model/evidence.h>
#include <mobius/os/win/registry/hive_file.h>
#include <mobius/os/win/registry/hive_data.h>
#include <mobius/pod/map.h>
#include <mobius/string_functions.h>
#include <algorithm>
#include <stdexcept>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string APP_ID = "ares";
static const std::string APP_NAME = "Ares Galaxy";
static const std::string ANT_ID = "evidence.app-ares";
static const std::string ANT_NAME = APP_NAME;
static const std::string ANT_VERSION = "1.3";

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert registry data into string
// @param data Registry data
// @param encoding Char encoding
// @return String
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
to_string_from_hexstring (const mobius::os::win::registry::hive_data& data, const std::string& encoding = "utf-16le")
{
  std::string value;

  if (data)
    {
      mobius::bytearray d;
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
to_hex_string (const mobius::os::win::registry::hive_data& data)
{
  std::string value;

  if (data)
    value = mobius::string::toupper (data.get_data ().to_hexstring ());

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update metadata map, preferring non null values
// @param metadata Metadata map
// @param other Other metadata map
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
update_metadata (mobius::pod::map& metadata, const mobius::pod::map& other)
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
get_file_hashes (const mobius::extension::app::ares::file& f)
{
  std::vector <mobius::pod::data> hashes;

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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
evidence_loader_impl::evidence_loader_impl (const mobius::model::item& item, scan_type type)
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
  log.debug (__LINE__, "Starting scan");

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
  mobius::core::log log (__FILE__, __FUNCTION__);
  log.debug (__LINE__, "Scan canonical folders started");

  auto vfs_datasource = mobius::datasource::datasource_vfs (item_.get_datasource ());
  auto vfs = vfs_datasource.get_vfs ();

  for (const auto& entry : vfs.get_root_entries ())
    {
      if (entry.is_folder ())
        _scan_canonical_root_folder (entry.get_folder ());
    }

  log.debug (__LINE__, "Scan canonical folders ended");
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

  for (const auto& f : w.get_files_by_name ("ntuser.dat"))
    _decode_ntuser_dat_file (f);

  for (const auto& f : w.get_folders_by_path ("appdata/local/ares"))
    _scan_canonical_ares_folder (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan AppData/Local/Ares folder for evidences
// @param folder Ares folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_ares_folder (const mobius::io::folder& folder)
{
  account_files_.clear ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Scan folders
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto w = mobius::io::walker (folder);

  for (const auto& f : w.get_folders_by_name ("data"))
    _scan_canonical_ares_data_folder (f);

  for (const auto& f : w.get_folders_by_name ("my shared folder"))
    _scan_canonical_ares_my_shared_folder (f);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Copy consolidated files to the list of all files found
  // If file is a torrent file, copy its own component files instead
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& [hash_sha1, af] : account_files_)
    {
      std::ignore = hash_sha1;

      if (af.torrent_files.size () == 0)
        files_.push_back (af);

      else
        {
          for (const auto& tf : af.torrent_files)
            {
              file f = af;

              f.size = tf.size;
              f.filename = tf.name;
              f.path = tf.path;
              f.hash_sha1.clear ();

              f.metadata = af.metadata.clone ();
              f.metadata.set ("torrent_file_idx", tf.idx);
              f.metadata.set ("torrent_last_modification_time", tf.last_modification_time);

              files_.push_back (f);
            }
        }
    }

  account_files_.clear ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan Ares Data folder for evidences
// @param folder Ares/Data folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_ares_data_folder (const mobius::io::folder& folder)
{
  mobius::io::walker w (folder);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Scan files
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& f : w.get_files ())
    {
      const std::string lname = mobius::string::tolower (f.get_name ());

      if (lname == "shareh.dat")
        _decode_shareh_dat_file (f);

      else if (lname == "sharel.dat")
        _decode_sharel_dat_file (f);

      else if (lname == "torrenth.dat")
        _decode_torrenth_dat_file (f);

      else if (lname == "phashidx.dat" || lname == "phashidxtemp.dat" || lname == "tempphash.dat")
        _decode_phashidx_dat_file (f);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Scan subfolders
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& f : w.get_folders_by_name ("tempdl"))
    _scan_canonical_ares_data_tempdl_folder (f);

  for (const auto& f : w.get_folders_by_name ("tempul"))
    _scan_canonical_ares_data_tempul_folder (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan Ares Data/TempDL folder for evidences
// @param folder Ares/Data/TempDL folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_ares_data_tempdl_folder (const mobius::io::folder& folder)
{
  mobius::core::log log (__FILE__, __FUNCTION__);
  mobius::io::walker w (folder);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Scan files
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& f : w.get_files ())
    {
      const std::string lname = mobius::string::tolower (f.get_name ());

      if (mobius::string::startswith (lname, "phash_"))
         _scan_canonical_ares_data_tempdl_phash_file (f);

      else if (mobius::string::startswith (lname, "pbthash_"))
         _scan_canonical_ares_data_tempdl_pbthash_file (f);

      else
         log.development (__LINE__, "unhandled Data/TempDL file: " + f.get_name ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan Ares Data/TempUL folder for evidences
// @param folder Ares/Data/TempUL folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_ares_data_tempul_folder (const mobius::io::folder& folder)
{
  mobius::core::log log (__FILE__, __FUNCTION__);
  mobius::io::walker w (folder);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Scan files
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& f : w.get_files ())
    {
      const std::string lname = mobius::string::tolower (f.get_name ());

      if (mobius::string::startswith (lname, "udpphash_"))
          _scan_canonical_ares_data_tempul_udpphash_file (f);

      else
         log.development (__LINE__, "unhandled Data/TempUL file: " + f.get_name ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode TempDL/PBTHash_xxx.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_ares_data_tempdl_pbthash_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Decode file
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      file_pbthash pbthash (f.new_reader ());

      if (!pbthash)
        {
          log.info (__LINE__, "File is not an instance of PBTHash. Path: " + f.get_path ());
          return;
        }

      log.info (__LINE__, "PBTHash file decoded. Path: " + f.get_path ());

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Process file
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      bool is_deleted = f.is_deleted ();

      auto hash_sha1 = pbthash.get_hash_sha1 ();
      auto [iter, success] = account_files_.try_emplace (hash_sha1, file {});
      std::ignore = success;
      auto& fobj = iter->second;

      if (!fobj.tempdl_pbthash_f || (fobj.tempdl_pbthash_f.is_deleted () && !is_deleted))
        {
          fobj.hash_sha1 = hash_sha1;
          fobj.account_guid = account_.guid;
          fobj.username = username_;
          fobj.size = pbthash.get_file_size ();
          fobj.tempdl_pbthash_f = f;

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Set path and name
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          auto path = pbthash.get_torrent_path ();
          if (!path.empty ())
            fobj.path = path;

          auto name = pbthash.get_torrent_name ();
          if (!name.empty ())
            fobj.filename = name;

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Test if every piece is checked
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          auto pieces = pbthash.get_pieces ();
          auto is_checked = std::all_of (
              pieces.begin (),
              pieces.end (),
              [](const auto& p){ return p.is_checked; }
          );

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Set flags
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          fobj.flag_downloaded = true;
          fobj.flag_completed.set_if_unknown (pbthash.is_completed ());
          fobj.flag_uploaded.set_if_unknown (pbthash.get_bytes_uploaded () > 0);
          fobj.flag_shared.set_if_unknown (pbthash.is_seeding ());
          fobj.flag_corrupted.set_if_unknown (!is_checked);

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Set torrent files
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          auto files = pbthash.get_files ();

          std::transform (
              files.begin (),
              files.end (),
              std::back_inserter (fobj.torrent_files),
              [](const auto& tf){ return torrent_file {tf.idx, tf.size, tf.last_modification_time, tf.name, tf.path}; }
          );

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Set metadata
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          fobj.metadata.set ("torrent_url", "magnet:?xt=urn:btih:" + hash_sha1);
          fobj.metadata.set ("torrent_pieces_count", pbthash.get_pieces_count ());
          fobj.metadata.set ("torrent_piece_size", pbthash.get_piece_size ());
          fobj.metadata.set ("torrent_files_count", pbthash.get_files_count ());
          fobj.metadata.set ("torrent_bytes_downloaded", pbthash.get_bytes_downloaded ());
          fobj.metadata.set ("torrent_bytes_uploaded", pbthash.get_bytes_uploaded ());
          fobj.metadata.set ("torrent_path", pbthash.get_torrent_path ());
          fobj.metadata.set ("torrent_name", pbthash.get_torrent_name ());
          fobj.metadata.set ("torrent_download_started_time", pbthash.get_download_started_time ());
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode TempDL/PHash_xxx.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_ares_data_tempdl_phash_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Decode file
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      file_phash phash (f.new_reader ());

      if (!phash)
        {
          log.info (__LINE__, "File is not an instance of PHash. Path: " + f.get_path ());
          return;
        }

      log.info (__LINE__, "PHash file decoded. Path: " + f.get_path ());

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Process entries
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      bool is_deleted = f.is_deleted ();

      for (const auto& entry : phash)
        {
          auto hash_sha1 = entry.hash_sha1;
          auto [iter, success] = account_files_.try_emplace (hash_sha1, file {});
          std::ignore = success;
          auto& fobj = iter->second;

          if (!fobj.tempdl_phash_f || (fobj.tempdl_phash_f.is_deleted () && !is_deleted))
            {
              fobj.hash_sha1 = hash_sha1;
              fobj.account_guid = account_.guid;
              fobj.username = username_;
              fobj.flag_downloaded = true;
              fobj.tempdl_phash_f = f;
              fobj.metadata.set ("pieces_count", entry.pieces_count);

              if (entry.is_completed != 2)  // STATE_UNKNOWN
                {
                  fobj.flag_completed = bool (entry.is_completed);
                  fobj.metadata.set ("downloaded_bytes", entry.progress);
                  fobj.metadata.set ("pieces_completed", entry.pieces_completed);
                  fobj.metadata.set ("pieces_to_go", entry.pieces_to_go);
                  fobj.metadata.set ("piece_size", entry.piece_size);
                }
            }
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode TempUL/UDPPHash_xxx.dat file
// @param f File object
// @see ICH_ExtractDataForUpload@helper_ICH
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_ares_data_tempul_udpphash_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Decode file
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      file_phash phash (f.new_reader ());

      if (!phash)
        {
          log.info (__LINE__, "File is not an instance of PHash. Path: " + f.get_path ());
          return;
        }

      log.info (__LINE__, "PHash file decoded. Path: " + f.get_path ());

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Process entries
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      bool is_deleted = f.is_deleted ();

      for (const auto& entry : phash)
        {
          auto hash_sha1 = entry.hash_sha1;
          auto [iter, success] = account_files_.try_emplace (hash_sha1, file {});
          std::ignore = success;
          auto& fobj = iter->second;

          if (!fobj.tempul_udpphash_f || (fobj.tempul_udpphash_f.is_deleted () && !is_deleted))
            {
              fobj.hash_sha1 = hash_sha1;
              fobj.account_guid = account_.guid;
              fobj.username = username_;
              fobj.flag_uploaded = true;
              fobj.tempul_udpphash_f = f;
              fobj.metadata.set ("pieces_count", entry.pieces_count);
            }
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode PHashIdx.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_phashidx_dat_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Decode file
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      file_phash phash (f.new_reader ());

      if (!phash)
        {
          log.info (__LINE__, "File is not an instance of PHash. Path: " + f.get_path ());
          return;
        }

      log.info (__LINE__, "PHash file decoded. Path: " + f.get_path ());

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Process entries
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      bool is_deleted = f.is_deleted ();

      for (const auto& entry : phash)
        {
          auto [iter, success] = account_files_.try_emplace (entry.hash_sha1, file {});
          std::ignore = success;
          auto& fobj = iter->second;

          if (!fobj.phashidx_f || (fobj.phashidx_f.is_deleted () && !is_deleted))
            {
              fobj.hash_sha1 = entry.hash_sha1;
              fobj.account_guid = account_.guid;
              fobj.username = username_;
              fobj.phashidx_idx = entry.idx;
              fobj.flag_completed = true;  // PHashIdx.dat entries are always completed
              fobj.flag_downloaded = true;
              fobj.phashidx_f = f;

              if (fobj.size)
                fobj.metadata.set ("downloaded_bytes", fobj.size);
            }
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode ShareH.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_shareh_dat_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Decode file
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      file_shareh shareh (f.new_reader ());

      if (!shareh)
        {
          log.info (__LINE__, "File is not an instance of ShareH.dat. Path: " + f.get_path ());
          return;
        }

      log.info (__LINE__, "ShareH.dat file decoded. Path: " + f.get_path ());

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Get entries
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      bool is_deleted = f.is_deleted ();

      for (const auto& entry : shareh)
        {
          auto [iter, success] = account_files_.try_emplace (entry.hash_sha1, file {});
          std::ignore = success;
          auto& fobj = iter->second;

          if (!fobj.shareh_f || (fobj.shareh_f.is_deleted () && !is_deleted))
            {
              fobj.hash_sha1 = entry.hash_sha1;
              fobj.account_guid = account_.guid;
              fobj.username = username_;
              fobj.download_completed_time = entry.download_completed_time;
              fobj.shareh_idx = entry.idx;
              fobj.shareh_f = f;

              fobj.flag_shared = entry.is_shared;
              fobj.flag_completed = true;  // ShareH entries are always completed
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
              fobj.metadata.set ("download_completed_time", entry.download_completed_time);
            }
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode ShareL.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_sharel_dat_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Decode file
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      file_sharel sharel (f.new_reader ());

      if (!sharel)
        {
          log.info (__LINE__, "File is not an instance of ShareL.dat. Path: " + f.get_path ());
          return;
        }

      log.info (__LINE__, "ShareL.dat file decoded. Path: " + f.get_path ());

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Get entries
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      bool is_deleted = f.is_deleted ();

      for (const auto& entry : sharel)
        {
          auto [iter, success] = account_files_.try_emplace (entry.hash_sha1, file {});
          std::ignore = success;
          auto& fobj = iter->second;

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
                  auto cpath = mobius::string::replace (fobj.path, "\\", "/");
                  fobj.filename = mobius::io::path (cpath).get_filename ();
                }

              // flags
              fobj.flag_corrupted.set_if_unknown (entry.is_corrupted);
              fobj.flag_shared.set_if_unknown (true);   // ShareL is shared by default, unless it is flagged
                                                        // "no" in the corresponding ShareH entry.

              fobj.flag_completed = true;  // ShareL entries are always completed

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
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode TorrentH.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_torrenth_dat_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Decode file
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      file_torrenth torrenth (f.new_reader ());

      if (!torrenth)
        {
          log.info (__LINE__, "File is not an instance of TorrentH.dat. Path: " + f.get_path ());
          return;
        }

      log.info (__LINE__, "TorrentH.dat file decoded. Path: " + f.get_path ());

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Get entries
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      bool is_deleted = f.is_deleted ();

      for (const auto& entry : torrenth)
        {
          auto [iter, success] = account_files_.try_emplace (entry.hash_sha1, file {});
          std::ignore = success;
          auto& fobj = iter->second;

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

              fobj.flag_shared = true;          // @see DHT/thread_dht.pas (line 412)
              fobj.flag_downloaded = true;      // @see DHT/dhtkeywords.pas (line 355)
              fobj.flag_completed = true;       // @see DHT/dhtkeywords.pas (line 355)
              fobj.flag_corrupted = false;      // @see DHT/dhtkeywords.pas (line 355)

              fobj.metadata.set ("seeds", entry.seeds);
              fobj.metadata.set ("media_type", entry.media_type);
              fobj.metadata.set ("evaluated_hash_sha1", entry.evaluated_hash_sha1);
              fobj.metadata.set ("torrent_name", entry.name);
              fobj.metadata.set ("torrent_url", entry.url);
            }
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan My Shared Folder folder
// @param folder Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_ares_my_shared_folder (const mobius::io::folder& folder)
{
  mobius::io::walker w (folder);

  auto filter = [](const auto& f){
     return mobius::string::startswith (f.get_name (), "___ARESTRA___");
  };

  for (const auto& f : w.find_files (filter))
    _decode_arestra_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode ___ARESTRA___ file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_arestra_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Decode file
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      file_arestra arestra (f.new_reader ());

      if (!arestra)
        {
          log.info (__LINE__, "File is not an instance of ___ARESTRA___. Path: " + f.get_path ());
          return;
        }

      log.info (__LINE__, "___ARESTRA___ file decoded. Path: " + f.get_path ());

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Get metadata
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      bool is_deleted = f.is_deleted ();

      auto [iter, success] = account_files_.try_emplace (arestra.get_hash_sha1 (), file {});
      std::ignore = success;
      auto& fobj = iter->second;

      if (!fobj.arestra_f || (fobj.arestra_f.is_deleted () && !is_deleted))
        {
          // set attributes
          fobj.hash_sha1 = arestra.get_hash_sha1 ();
          fobj.account_guid = account_.guid;
          fobj.username = username_;
          fobj.download_started_time = arestra.get_download_started_time ();
          fobj.size = arestra.get_file_size ();
          fobj.arestra_f = f;

          // set filename
          fobj.filename = mobius::io::path (f.get_path ()).get_filename ();
          fobj.filename.erase (0, 13);  // remove "___ARESTRA___"

          // set flags
          fobj.flag_downloaded = true;
          fobj.flag_corrupted.set_if_unknown (arestra.is_corrupted ());
          fobj.flag_shared.set_if_unknown (false);   // @see thread_share.pas (line 1065)
          fobj.flag_completed = arestra.is_completed ();

          // add remote_sources
          for (const auto& [ip, port] : arestra.get_alt_sources ())
            {
              remote_source r_source;
              r_source.timestamp = f.get_modification_time ();
              r_source.ip = ip;
              r_source.port = port;

              fobj.remote_sources.push_back (r_source);
            }

          // set metadata
          fobj.metadata.set ("arestra_signature", arestra.get_signature ());
          fobj.metadata.set ("arestra_file_version", arestra.get_version ());
          fobj.metadata.set ("download_started_time", arestra.get_download_started_time ());
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
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode data from NTUSER.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_ntuser_dat_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Create decoder
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      auto decoder = mobius::os::win::registry::hive_file (f.new_reader ());

      if (!decoder.is_instance ())
        {
          log.info (__LINE__, "File " + f.get_path () + " ignored.");
          return;
        }

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Get evidences from Ares key
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      const auto& root_key = decoder.get_root_key ();
      const auto& ares_key = root_key.get_key_by_path ("Software\\Ares");

      if (ares_key)
        {
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Load account
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          account acc;

          acc.guid = ares_key.get_data_by_name ("Personal.GUID").get_data_as_string ("utf-16le");
          acc.nickname = to_string_from_hexstring (ares_key.get_data_by_name ("Personal.Nickname"));
          acc.dht_id = to_hex_string (ares_key.get_data_by_name ("Network.DHTID"));
          acc.mdht_id = to_hex_string (ares_key.get_data_by_name ("Network.MDHTID"));
          acc.username = username_;
          acc.is_deleted = f.is_deleted ();
          acc.f = f;

          if (account_.guid.empty () ||
              (account_.is_deleted && !acc.is_deleted))
            account_ = acc;

          accounts_.push_back (acc);

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Load autofill values
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          for (const auto& key : ares_key.get_keys_by_mask ("Search.History\\*"))
            {
              std::string category = key.get_name ();

              for (const auto& value : key.get_values ())
                {
                  autofill af;

                  af.value = mobius::decoder::hexstring (value.get_name ()).to_string ();
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
  auto vfs_datasource = mobius::datasource::datasource_vfs (item_.get_datasource ());
  auto vfs = vfs_datasource.get_vfs ();

  for (const auto& entry : vfs.get_root_entries ())
    {
      if (entry.is_folder ())
        _scan_generic_folder (entry.get_folder ());
    }
}

void
evidence_loader_impl::_scan_generic_folder (const mobius::io::folder& folder)
{
  mobius::io::walker w (folder);

  for (const auto& f : w.get_files ())
    {
      const std::string lname = mobius::string::tolower (f.get_name ());

      if (lname == "shareh.dat")
        _decode_shareh_dat_file (f);

      else if (lname == "sharel.dat")
        _decode_sharel_dat_file (f);

      else if (lname == "phashidx.dat")
        _decode_phashidx_dat_file (f);

      else if (mobius::string::startswith (lname, "___arestra___"))
         _decode_arestra_file (f);
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
  mobius::core::log log (__FILE__, __FUNCTION__);
  log.debug (__LINE__, "Saving evidences");

  auto transaction = item_.new_transaction ();

  log.debug (__LINE__, "Saving accounts");
  _save_accounts ();

  log.debug (__LINE__, "Saving autofills");
  _save_autofills ();

  log.debug (__LINE__, "Saving local files");
  _save_local_files ();

  log.debug (__LINE__, "Saving p2p remote files");
  _save_p2p_remote_files ();

  log.debug (__LINE__, "Saving received files");
  _save_received_files ();

  log.debug (__LINE__, "Saving sent files");
  _save_sent_files ();

  log.debug (__LINE__, "Saving shared files");
  _save_shared_files ();

  item_.set_ant (ANT_ID, ANT_NAME, ANT_VERSION);
  transaction.commit ();

  log.debug (__LINE__, "Evidences saved");
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save autofill entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_autofills ()
{
  for (const auto& a : autofills_)
    {
      mobius::pod::map metadata;
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
          e.set_attribute ("hashes", get_file_hashes (f));

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

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Metadata
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          mobius::pod::map metadata;
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
          e.set_attribute ("hashes", get_file_hashes (f));

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Metadata
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          mobius::pod::map metadata;

          metadata.set ("size", f.size);
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
          mobius::pod::map metadata;
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

          std::vector <mobius::pod::data> hashes = {{"sha1", f.hash_sha1}};
          e.set_attribute ("hashes", hashes);

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

} // namespace mobius::extension::app::ares




