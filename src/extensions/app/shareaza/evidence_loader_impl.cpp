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
// All Date/Times are stored in Coordinated Universal Time (UTC).
// !\see https://msdn.microsoft.com/pt-br/library/windows/desktop/ms724397(v=vs.85).aspx
//
// References:
//   . Shareaza 2.7.10.2 source code
//
// Shareaza main files:
//   . Library1.dat, Library2.dat, Library.dat - Local folders and local files file
//
//   . Profile.xml - User accounts configuration file
//
//   . Searches.dat - Search history and search results file
//
//   . Shareaza.db3 - Thumb cache file
//
//   . *.sd files - Download control files (one per downloading file)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "evidence_loader_impl.hpp"
#include "CDownload.hpp"
#include "CLibrary.hpp"
#include "CThumbCache.hpp"
#include "file_searches_dat.hpp"
#include <mobius/io/walker.h>
#include <mobius/core/log.h>
#include <mobius/datasource/datasource_vfs.h>
#include <mobius/decoder/mfc.h>
#include <mobius/decoder/xml/dom.h>
#include <mobius/exception.inc>
#include <mobius/io/folder.h>
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
static const std::string APP_ID = "shareaza";
static const std::string APP_NAME = "Shareaza";
static const std::string ANT_ID = "evidence.app-shareaza";
static const std::string ANT_NAME = APP_NAME;
static const std::string ANT_VERSION = "1.2";

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get vector of hashes for a given file
// @param f File structure
// @return Vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T> std::vector <mobius::pod::data>
get_file_hashes (const T& f)
{
  std::vector <std::pair <std::string, std::string>> values =
  {
    {"sha1", f.get_hash_sha1 ()},
    {"tiger", f.get_hash_tiger ()},
    {"md5", f.get_hash_md5 ()},
    {"ed2k", f.get_hash_ed2k ()},
    {"bth", f.get_hash_bth ()},
  };

  std::vector <mobius::pod::data> hashes;

  for (const auto& [k, v] : values)
    {
      if (!v.empty ())
        hashes.push_back ({k, v});
    }

  return hashes;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get library timestamp
// @param f File object
// @return Timestamp
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
get_library_timestamp (const mobius::io::file& f)
{
  mobius::datetime::datetime timestamp;

  if (f)
    {
      mobius::decoder::mfc decoder (f.new_reader ());
      timestamp = decoder.get_nt_time ();
    }

  return timestamp;
}

} // namespace

namespace mobius::extension::app::shareaza
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
  log.info (__LINE__, "Evidence loader <app-" + APP_ID + "> started");
  log.info (__LINE__, "Item UID: " + std::to_string (item_.get_uid ()));
  log.info (__LINE__, "Scan mode: " + std::to_string (static_cast <int> (scan_type_)));

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Check if loader has already run for item
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (item_.has_ant (ANT_ID))
    {
      log.info (__LINE__, "Evidence loader <app-" + APP_ID + "> has already run");
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
        //_scan_all_folders ();
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

  log.info (__LINE__, "Evidence loader <app-" + APP_ID + "> ended");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan canonical folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_folders ()
{
  auto vfs_datasource = mobius::datasource::datasource_vfs (item_.get_datasource ());
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

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Scan evidence files
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& f : w.get_files_by_name ("ntuser.dat"))
    _decode_ntuser_dat_file (f);

  for (const auto& f : w.get_folders_by_path ("appdata/roaming/shareaza/data"))
    _scan_canonical_shareaza_data_folder (f);

  for (const auto& f : w.get_files_by_pattern ("appdata/local/shareaza/incomplete/*.sd"))
    _decode_sd_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan AppData/Roaming/Shareaza/Data folder for evidences
// @param folder Data folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_shareaza_data_folder (const mobius::io::folder& folder)
{
  mobius::io::walker w (folder);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get account info first
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& f : w.get_files_by_name ("profile.xml"))
    _decode_profile_xml_file (f);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Process Shareaza.db3 files
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& f : w.get_files_by_name ("shareaza.db3"))
     _decode_shareaza_db3_file (f);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Process current library file
  // The current library is the library with the higher last_modification_time.
  // Active library (non deleted) is preferred.
  // @see CLibrary::Load function
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto lib_f = w.get_file_by_name ("library1.dat");
  auto lib2_f = w.get_file_by_name ("library2.dat");

  if (lib_f && lib2_f && get_library_timestamp (lib2_f) > get_library_timestamp (lib_f))
    lib_f = lib2_f;

  if (!lib_f)
    lib_f = w.get_file_by_name ("library.dat");

  if (lib_f)
    _decode_library_dat_file (lib_f);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Process searches.dat files
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& f : w.get_files_by_name ("searches.dat"))
     _decode_searches_dat_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode LibraryN.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_library_dat_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Decode file
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      auto decoder = mobius::decoder::mfc (f.new_reader ());
      CLibrary clib (decoder);

      if (!clib)
        {
          log.info (__LINE__, "File is not an instance of Library.dat. Path: " + f.get_path ());
          return;
        }

      log.info (__LINE__, "Library.dat file decoded. Path: " + f.get_path ());

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Add library files
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      for (const auto& cfile : clib.get_all_files ())
        {
          local_file lf;

          // Attributes
          lf.filename = cfile.get_name ();
          lf.path = cfile.get_path ();
          lf.username = username_;
          lf.hashes = get_file_hashes (cfile);
          lf.flag_uploaded = cfile.get_uploads_started () > 0;
          lf.flag_shared = cfile.is_shared ();
          lf.f = f;

          // Thumbnail data
          auto thumbnail = account_.thumbcache.get (cfile.get_path ());
          if (thumbnail)
            {
              lf.thumbnail_data = thumbnail->image_data;
              lf.shareaza_db3_f = account_.shareaza_db3_f;
            }

          // Metadata
          lf.metadata.set ("flag_downloaded", "unknown");
          lf.metadata.set ("flag_uploaded", lf.flag_uploaded ? "true" : "false");
          lf.metadata.set ("flag_shared", lf.flag_shared ? "true" : "false");
          lf.metadata.set ("flag_corrupted", "unknown");
          lf.metadata.set ("flag_completed", "true");

          lf.metadata.set ("size", cfile.get_size ());
          lf.metadata.set ("index", cfile.get_index ());
          lf.metadata.set ("virtual_size", cfile.get_virtual_size ());
          lf.metadata.set ("virtual_base", cfile.get_virtual_base ());
          lf.metadata.set ("uri", cfile.get_uri ());

          auto rating = cfile.get_rating ();
          if (rating != -1)
            lf.metadata.set ("rating", rating);

          lf.metadata.set ("comments", cfile.get_comments ());
          lf.metadata.set ("share_tags", cfile.get_share_tags ());
          lf.metadata.set ("hits_total", cfile.get_hits_total ());
          lf.metadata.set ("uploads_started", cfile.get_uploads_started ());
          lf.metadata.set ("last_modification_time", cfile.get_last_modification_time ());
          lf.metadata.set ("metadata_time", cfile.get_metadata_time ());

          if (thumbnail)
            lf.metadata.set ("thumbnail_last_write_time", thumbnail->last_write_time);

          for (const auto& [k, v] : cfile.get_pxml ().get_metadata ())
            lf.metadata.set (k, v);

          local_files_.push_back (lf);

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Add remote files
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          for (const auto& source : cfile.get_sources ())
            {
              remote_file rf;

              rf.timestamp = source.get_timestamp ();
              rf.ip = source.get_ip ();
              rf.port = source.get_port ();
              rf.filename = cfile.get_name ();
              rf.username = username_;
              rf.hashes = get_file_hashes (cfile);
              rf.f = f;

              // metadata
              rf.metadata.set ("size", cfile.get_size ());
              rf.metadata.set ("index", cfile.get_index ());
              rf.metadata.set ("virtual_size", cfile.get_virtual_size ());
              rf.metadata.set ("virtual_base", cfile.get_virtual_base ());
              rf.metadata.set ("url", source.get_url ());
              rf.metadata.set ("schema_uri", cfile.get_uri ());

              auto rating = cfile.get_rating ();
              if (rating != -1)
                rf.metadata.set ("rating", rating);

              rf.metadata.set ("comments", cfile.get_comments ());
              rf.metadata.set ("share_tags", cfile.get_share_tags ());
              rf.metadata.set ("hits_total", cfile.get_hits_total ());
              rf.metadata.set ("uploads_started", cfile.get_uploads_started ());
              rf.metadata.set ("last_modification_time", cfile.get_last_modification_time ());
              rf.metadata.set ("metadata_time", cfile.get_metadata_time ());

              if (thumbnail)
                {
                  rf.metadata.set ("thumbnail_last_write_time", thumbnail->last_write_time);
                  rf.thumbnail_data = thumbnail->image_data;
                  rf.shareaza_db3_f = account_.shareaza_db3_f;
                }

              for (const auto& [k, v] : cfile.get_pxml ().get_metadata ())
                rf.metadata.set (k, v);

              remote_files_.push_back (rf);
            }
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode Profile.xml file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_profile_xml_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      if (!account_.f || (account_.f.is_deleted () && !f.is_deleted ()))
        {
          mobius::decoder::xml::dom dom (f.new_reader ());
          auto root = dom.get_root_element ();

          account acc;
          acc.username = username_;
          acc.gnutella_guid = mobius::string::toupper (root.get_property_by_path ("gnutella/guid"));
          acc.bittorrent_guid = mobius::string::toupper (root.get_property_by_path ("bittorrent/guid"));
          acc.identity = root.get_property_by_path ("identity/handle/primary");
          acc.f = f;

          account_ = acc;
          accounts_.push_back (acc);
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode Searches.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_searches_dat_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Decode file
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      file_searches_dat searches_dat (f.new_reader ());

      if (!searches_dat)
        {
          log.info (__LINE__, "File is not an instance of Searches.dat. Path: " + f.get_path ());
          return;
        }

      log.info (__LINE__, "Searches.dat file decoded. Path: " + f.get_path ());

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Add searches
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      for (const auto& s : searches_dat.get_searches ())
        {
          auto cmanagedsearch = s.obj;
          auto qs = cmanagedsearch.get_query_search ();

          search st;

          // Attributes
          st.timestamp = s.timestamp;
          st.text = s.text;
          st.username = username_;
          st.f = f;

          // Metadata
          st.metadata.set ("search_id", qs.get_guid ());
          st.metadata.set ("first_hit_time", s.first_hit_time);
          st.metadata.set ("last_hit_time", s.last_hit_time);
          st.metadata.set ("file_count", s.file_count);
          st.metadata.set ("schema_uri", qs.get_uri ());
          st.metadata.set ("priority", cmanagedsearch.get_priority ());
          st.metadata.set ("flag_active", cmanagedsearch.get_flag_active ());
          st.metadata.set ("flag_receive", cmanagedsearch.get_flag_receive ());
          st.metadata.set ("flag_allow_g1", cmanagedsearch.get_allow_g1 ());
          st.metadata.set ("flag_allow_g2", cmanagedsearch.get_allow_g2 ());
          st.metadata.set ("flag_allow_ed2k", cmanagedsearch.get_allow_ed2k ());
          st.metadata.set ("flag_allow_dc", cmanagedsearch.get_allow_dc ());
          st.metadata.set ("flag_want_url", qs.get_flag_want_url ());
          st.metadata.set ("flag_want_dn", qs.get_flag_want_dn ());
          st.metadata.set ("flag_want_xml", qs.get_flag_want_xml ());
          st.metadata.set ("flag_want_com", qs.get_flag_want_com ());
          st.metadata.set ("flag_want_pfs", qs.get_flag_want_pfs ());
          st.metadata.set ("min_size", qs.get_min_size ());
          st.metadata.set ("max_size", qs.get_max_size ());
          st.metadata.set ("cmanagedsearch_version", cmanagedsearch.get_version ());
          st.metadata.set ("cquerysearch_version", qs.get_version ());

          searches_.push_back (st);
        }

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Add remote files
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      for (const auto& search_rf : searches_dat.get_remote_files ())
        {
          const auto& mf = search_rf.match_file;
          const auto& q = search_rf.query_hit;

          remote_file rf;

          // attributes
          rf.timestamp = mf.get_found_time ();
          rf.ip = q.get_ip ();
          rf.port = q.get_port ();
          rf.filename = q.get_name ();
          rf.username = username_;

          // metadata
          rf.metadata.set ("size", mf.get_size ());
          rf.metadata.set ("peer_nickname", q.get_nick ());
          rf.metadata.set ("url", q.get_url ());
          rf.metadata.set ("schema_uri", q.get_schema_uri ());
          rf.metadata.set ("rating", q.get_rating ());
          rf.metadata.set ("comments", q.get_comments ());
          rf.metadata.set ("protocol_id", q.get_protocol_id ());
          rf.metadata.set ("protocol_name", q.get_protocol_name ());
          rf.metadata.set ("search_id",  q.get_search_id ());
          rf.metadata.set ("client_id", q.get_client_id ());
          rf.metadata.set ("speed", q.get_speed ());
          rf.metadata.set ("str_speed", q.get_str_speed ());
          rf.metadata.set ("vendor_code", q.get_vendor_code ());
          rf.metadata.set ("vendor_name", q.get_vendor_name ());
          rf.metadata.set ("tri_push", q.get_tri_push ());
          rf.metadata.set ("tri_busy", q.get_tri_busy ());
          rf.metadata.set ("tri_stable", q.get_tri_stable ());
          rf.metadata.set ("tri_measured", q.get_tri_measured ());
          rf.metadata.set ("up_slots", q.get_up_slots ());
          rf.metadata.set ("up_queue", q.get_up_queue ());
          rf.metadata.set ("b_chat", q.get_b_chat ());
          rf.metadata.set ("b_browse_host", q.get_b_browse_host ());
          rf.metadata.set ("b_matched", q.get_b_matched ());
          rf.metadata.set ("b_size", q.get_b_size ());
          rf.metadata.set ("b_preview", q.get_b_preview ());
          rf.metadata.set ("b_collection", q.get_b_collection ());
          rf.metadata.set ("b_bogus", q.get_b_bogus ());
          rf.metadata.set ("b_download", q.get_b_download ());
          rf.metadata.set ("b_exact_match", q.get_b_exact_match ());
          rf.metadata.set ("index", q.get_index ());
          rf.metadata.set ("hit_sources", q.get_hit_sources ());
          rf.metadata.set ("partial", q.get_partial ());
          rf.metadata.set ("schema_plural", q.get_schema_plural ());

          for (const auto& [k, v] : q.get_pxml ().get_metadata ())
            rf.metadata.set (k, v);

          // other data
          rf.thumbnail_data = mf.get_preview ();
          rf.hashes = get_file_hashes (mf);
          rf.f = f;

          // add remote file
          remote_files_.push_back (rf);
        }

    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode Shareaza.db3 file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_shareaza_db3_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Decode file
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      auto thumbcache = CThumbCache (f.new_reader ());

      if (!thumbcache)
        {
          log.info (__LINE__, "File is not an instance of Shareaza.db3. Path: " + f.get_path ());
          return;
        }

      log.info (__LINE__, "Shareaza.db3 file decoded. Path: " + f.get_path ());

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Update account cache, if necessary
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      if (!account_.shareaza_db3_f || (account_.shareaza_db3_f.is_deleted () && !f.is_deleted ()))
        {
          account_.shareaza_db3_f = f;
          account_.thumbcache = thumbcache;
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode Incomplete/*.sd file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_sd_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Decode file
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      auto sd = CDownload (f.new_reader ());

      if (!sd)
        {
          log.info (__LINE__, "File is not an instance of CDownload. Path: " + f.get_path ());
          return;
        }

      log.info (__LINE__, "CDownload file decoded. Path: " + f.get_path ());

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Add local file
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      auto btinfo = sd.get_btinfo ();

      local_file lf;

      // Attributes
      lf.filename = sd.get_name ();
      lf.username = username_;
      lf.hashes = get_file_hashes (sd);
      lf.flag_downloaded = true;
      lf.flag_uploaded = btinfo.get_total_uploaded () > 0;
      lf.flag_shared = sd.is_shared ();
      lf.flag_completed = sd.get_downloaded_size () == sd.get_size ();
      lf.f = f;

      // Metadata
      lf.metadata.set ("flag_downloaded", "true");
      lf.metadata.set ("flag_uploaded", lf.flag_uploaded ? "true" : "false");
      lf.metadata.set ("flag_shared", lf.flag_shared ? "true" : "false");
      lf.metadata.set ("flag_corrupted", "unknown");
      lf.metadata.set ("flag_completed", "true");

      lf.metadata.set ("local_name", sd.get_local_name ());
      lf.metadata.set ("size", sd.get_size ());
      lf.metadata.set ("estimated_size", sd.get_size ());
      lf.metadata.set ("downloaded_size", sd.get_downloaded_size ());
      lf.metadata.set ("remaining_size", sd.get_remaining_size ());
      lf.metadata.set ("is_expanded", sd.is_expanded ());
      lf.metadata.set ("is_paused", sd.is_paused ());
      lf.metadata.set ("is_boosted", sd.is_boosted ());
      lf.metadata.set ("is_seeding", sd.is_seeding ());

      lf.metadata.set ("block_size", btinfo.get_block_size ());
      lf.metadata.set ("block_count", btinfo.get_block_count ());
      lf.metadata.set ("total_uploaded", btinfo.get_total_uploaded ());
      lf.metadata.set ("total_downloaded", btinfo.get_total_downloaded ());
      lf.metadata.set ("creation_time", btinfo.get_creation_time ());
      lf.metadata.set ("created_by", btinfo.get_created_by ());
      lf.metadata.set ("comments", btinfo.get_comments ());

      for (const auto& [k, v] : btinfo.get_metadata ())
        lf.metadata.set (k, v);

      for (const auto& [k, v] : sd.get_pxml ().get_metadata ())
        lf.metadata.set (k, v);

      lf.metadata.set ("sd_file_signature", sd.get_signature ());
      lf.metadata.set ("cdownload_version", sd.get_version ());
      lf.metadata.set ("cbtinfo_version", btinfo.get_version ());

      local_files_.push_back (lf);

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Add remote files
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      for (const auto& source : sd.get_sources ())
        {
          remote_file rf;

          rf.timestamp = source.get_last_seen_time ();
          rf.ip = source.get_ip ();
          rf.port = source.get_port ();
          rf.filename = sd.get_name ();
          rf.username = username_;
          rf.hashes = get_file_hashes (sd);
          rf.f = f;

          // metadata
          rf.metadata.set ("url", source.get_url ());
          rf.metadata.set ("local_name", sd.get_local_name ());
          rf.metadata.set ("size", sd.get_size ());
          rf.metadata.set ("estimated_size", sd.get_size ());
          rf.metadata.set ("is_expanded", sd.is_expanded ());
          rf.metadata.set ("is_paused", sd.is_paused ());
          rf.metadata.set ("is_boosted", sd.is_boosted ());
          rf.metadata.set ("is_seeding", sd.is_seeding ());

          rf.metadata.set ("block_size", btinfo.get_block_size ());
          rf.metadata.set ("block_count", btinfo.get_block_count ());
          rf.metadata.set ("total_uploaded", btinfo.get_total_uploaded ());
          rf.metadata.set ("total_downloaded", btinfo.get_total_downloaded ());
          rf.metadata.set ("creation_time", btinfo.get_creation_time ());
          rf.metadata.set ("created_by", btinfo.get_created_by ());
          rf.metadata.set ("comments", btinfo.get_comments ());

          for (const auto& [k, v] : sd.get_pxml ().get_metadata ())
            rf.metadata.set (k, v);

          rf.metadata.set ("sd_file_signature", sd.get_signature ());
          rf.metadata.set ("cdownload_version", sd.get_version ());
          rf.metadata.set ("cbtinfo_version", btinfo.get_version ());

          remote_files_.push_back (rf);
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
      // Get evidences from Shareaza key
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      const auto& root_key = decoder.get_root_key ();
      const auto& shareaza_key = root_key.get_key_by_path ("Software\\Shareaza\\Shareaza");

      if (shareaza_key)
        {
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Set account
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          account acc;
          acc.username = username_;
          acc.is_deleted = f.is_deleted ();
          acc.f = f;

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Load values from key
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          acc.install_path = shareaza_key.get_data_by_name ("Path").get_data_as_string ("utf-16le");
          acc.user_path = shareaza_key.get_data_by_name ("UserPath").get_data_as_string ("utf-16le");

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Load values from Downloads key
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          const auto& download_key = shareaza_key.get_key_by_name ("Downloads");

          if (download_key)
            {
              acc.collection_path = download_key.get_data_by_name ("CollectionPath").get_data_as_string ("utf-16le");
              acc.complete_path = download_key.get_data_by_name ("CompletePath").get_data_as_string ("utf-16le");
              acc.incomplete_path = download_key.get_data_by_name ("IncompletePath").get_data_as_string ("utf-16le");
              acc.torrent_path = download_key.get_data_by_name ("TorrentPath").get_data_as_string ("utf-16le");
            }

          /*if (account_.guid.empty () ||
              (account_.is_deleted && !acc.is_deleted))
            account_ = acc;*/

          accounts_.push_back (acc);

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Load autofill values
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          for (const auto& value: shareaza_key.get_values_by_mask ("Search\\Search.*"))
            {
              autofill af;

              af.value = mobius::string::word (value.get_data ().get_data_as_string ("utf-16le"), 0, "\n");
              af.username = username_;
              af.id = value.get_name ().substr(7);
              af.is_deleted = acc.is_deleted;
              af.f = f;

              autofills_.push_back (af);
            }
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save evidences
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_evidences ()
{
  auto transaction = item_.new_transaction ();

  _save_accounts ();
  _save_autofills ();
  _save_local_files ();
  _save_p2p_remote_files ();
  _save_received_files ();
  _save_searched_texts ();
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
      metadata.set ("gnutella_guid", a.gnutella_guid);
      metadata.set ("bittorrent_guid", a.bittorrent_guid);
      metadata.set ("identity_primary", a.identity);

      if (!a.gnutella_guid.empty ())
        {
          auto e = item_.new_evidence ("user-account");

          e.set_attribute ("account_type", "p2p.gnutella");
          e.set_attribute ("id", a.gnutella_guid);
          e.set_attribute ("password", {});
          e.set_attribute ("password_found", "no");
          e.set_attribute ("is_deleted", a.is_deleted);
          e.set_attribute ("metadata", metadata.clone ());
          e.set_tag ("p2p");
          e.add_source (a.f);
        }

      if (!a.bittorrent_guid.empty ())
        {
          auto e = item_.new_evidence ("user-account");

          e.set_attribute ("account_type", "p2p.bittorrent");
          e.set_attribute ("id", a.bittorrent_guid);
          e.set_attribute ("password", {});
          e.set_attribute ("password_found", "no");
          e.set_attribute ("is_deleted", a.is_deleted);
          e.set_attribute ("metadata", metadata.clone ());
          e.set_tag ("p2p");
          e.add_source (a.f);
        }
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
      metadata.set ("id", a.id);

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
  for (const auto& lf : local_files_)
    {
      auto e = item_.new_evidence ("local-file");

      e.set_attribute ("username", lf.username);
      e.set_attribute ("filename", lf.filename);
      e.set_attribute ("path", lf.path);
      e.set_attribute ("app_id", APP_ID);
      e.set_attribute ("app_name", APP_NAME);
      e.set_attribute ("hashes", lf.hashes);
      e.set_attribute ("metadata", lf.metadata);

      e.set_tag ("p2p");
      e.add_source (lf.f);
      e.add_source (lf.shareaza_db3_f);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save received files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_received_files ()
{
  for (const auto& lf : local_files_)
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

          e.set_tag ("p2p");
          e.add_source (lf.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save remote files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_p2p_remote_files ()
{
  for (const auto& rf : remote_files_)
    {
      auto e = item_.new_evidence ("p2p-remote-file");

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

      e.set_tag ("p2p");
      e.add_source (rf.f);
      e.add_source (rf.shareaza_db3_f);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save searched texts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_searched_texts ()
{
  for (const auto& search : searches_)
    {
      if (search.timestamp)
        {
          auto e = item_.new_evidence ("searched-text");

          e.set_attribute ("timestamp", search.timestamp);
          e.set_attribute ("search_type", "p2p.shareaza");
          e.set_attribute ("text", search.text);
          e.set_attribute ("username", search.username);
          e.set_attribute ("metadata", search.metadata);

          e.set_tag ("p2p");
          e.add_source (search.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save sent files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_sent_files ()
{
  for (const auto& lf : local_files_)
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

          e.set_tag ("p2p");
          e.add_source (lf.f);
          e.add_source (lf.shareaza_db3_f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save shared files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_shared_files ()
{
  for (const auto& lf : local_files_)
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

          e.set_tag ("p2p");
          e.add_source (lf.f);
          e.add_source (lf.shareaza_db3_f);
        }
    }
}

} // namespace mobius::extension::app::shareaza



