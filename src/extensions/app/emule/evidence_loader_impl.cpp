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
#include "evidence_loader_impl.hpp"
#include "file_stored_searches_met.hpp"
#include <mobius/core/log.h>
#include <mobius/datasource/datasource_vfs.h>
#include <mobius/decoder/data_decoder.h>
#include <mobius/decoder/inifile.h>
#include <mobius/io/line_reader.h>
#include <mobius/io/walker.h>
#include <mobius/model/evidence.h>
#include <mobius/string_functions.h>

#include <iostream>

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
// According to eMule Homepage: "Your Incoming and Temporary directory are always shared"
// @see https://www.emule-project.net/home/perl/help.cgi?l=1&topic_id=112&rm=show_topic
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string APP_ID = "emule";
static const std::string APP_NAME = "Emule";
static const std::string ANT_ID = "evidence.app-emule";
static const std::string ANT_NAME = APP_NAME;
static const std::string ANT_VERSION = "1.0";

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

} // namespace

namespace mobius::extension::app::emule
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

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Check if loader has already run for item
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (item_.has_ant (ANT_ID))
      {
        log.info (__LINE__, "Evidence loader <app-" + APP_ID + "> has already run");
        return ;
      }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Check datasource
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto datasource = item_.get_datasource ();

    if (!datasource)
        throw std::runtime_error ("item has no datasource");

    if (datasource.get_type () != "vfs")
        throw std::runtime_error ("datasource type is not VFS");

    if (!datasource.is_available ())
        throw std::runtime_error ("datasource is not available");

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Log starting event
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto transaction = item_.new_transaction ();
    item_.add_event ("app." + APP_ID + " started");
    transaction.commit ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Scan item files, according to scan_type
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Save evidences
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
    username_ = {};

    auto w = mobius::io::walker (folder);

    for (const auto& f : w.get_folders_by_pattern ("users/*"))
        _scan_canonical_user_folder (f);

    // Win XP folders
    for (const auto& f : w.get_folders_by_path ("program files/dreamule/config"))
        _scan_canonical_emule_config_folder (f);

    for (const auto& f : w.get_folders_by_path ("arquivos de programas/dreamule/config"))
        _scan_canonical_emule_config_folder (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan user folder for evidences
// @param folder User folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_user_folder (const mobius::io::folder& folder)
{
    username_ = folder.get_name ();

    auto w = mobius::io::walker (folder);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Scan evidence files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  for (const auto& f : w.get_files_by_name ("ntuser.dat"))
//    _decode_ntuser_dat_file (f);

    for (const auto& f : w.get_folders_by_path ("appdata/local/emule/config"))
        _scan_canonical_emule_config_folder (f);
/*
  for (const auto& f : w.get_folders_by_path ("downloads/emule/incoming"))
    _scan_canonical_emule_download_folder (f);

  for (const auto& f : w.get_folders_by_path ("downloads/emule/temp"))
    _scan_canonical_emule_download_folder (f);
*/
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan AppData/Local/eMule/config folder for evidences
// @param folder Config folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_emule_config_folder (const mobius::io::folder& folder)
{
    account_ = {};
    account_.username = username_;

    mobius::io::walker w (folder);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode account files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto& f : w.get_files ())
      {
        const std::string lname = mobius::string::tolower (f.get_name ());

        if (lname == "preferences.dat")
            _decode_preferences_dat_file (f);

        else if (lname == "preferences.ini")
            _decode_preferences_ini_file (f);

        else if (lname == "statistics.ini")
            _decode_statistics_ini_file (f);

        else if (lname == "preferenceskad.dat")
            _decode_preferenceskad_dat_file (f);
    }

    if (!account_.statistics_ini_f)
      {
        for (const auto& f : w.get_files_by_name ("statbkup.ini"))
            _decode_statistics_ini_file (f);
      }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decoder other config files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto& f : w.get_files ())
      {
        const std::string lname = mobius::string::tolower (f.get_name ());

        if (lname == "ac_searchstrings.dat")
            _decode_ac_searchstrings_dat_file (f);
            
        else if (lname == "storedsearches.met")
            _decode_storedsearches_met_file (f);
      }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add account to accounts list
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (!account_.emule_guid.empty () || !account_.kamdelia_guid.empty ())
        accounts_.push_back (account_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode Preferences.dat file
// @param f File object
// @see CPreferences::Init@srchybrid/Preferences.cpp
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_preferences_dat_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      // Create account object
      if (!account_.preferences_dat_f ||
          (account_.preferences_dat_f.is_deleted () && !f.is_deleted ()))
        {
          // Get reader
          auto reader = f.new_reader ();
          if (!reader)
            return;

          // Decode file
          mobius::decoder::data_decoder decoder (reader);

          account_.is_deleted = f.is_deleted ();
          account_.preferences_dat_version = decoder.get_uint8 ();
          account_.emule_guid = decoder.get_hex_string_by_size (16);
          account_.preferences_dat_f = f;
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode Preferences.ini file
// @param f File object
// @see CPreferences::LoadPreferences@srchybrid/Preferences.cpp
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_preferences_ini_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      // Create account object
      if (!account_.preferences_ini_f ||
          (account_.preferences_ini_f.is_deleted () && !f.is_deleted ()))
        {
          // Get reader
          auto reader = f.new_reader ();
          if (!reader)
            return;

          // Decode file
          mobius::decoder::inifile ini (reader);

          account_.incoming_dir = ini.get_value ("emule", "incomingdir");
          account_.temp_dir = ini.get_value ("emule", "tempdir");
          account_.nick = ini.get_value ("emule", "nick");
          account_.app_version = ini.get_value ("emule", "appversion");

          if (ini.has_value ("emule", "autostart"))
            account_.auto_start = ini.get_value ("emule", "autostart") == "1";

          account_.preferences_ini_f = f;
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode statistics.ini file
// @param f File object
// @see CPreferences::LoadStats@srchybrid/Preferences.cpp
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_statistics_ini_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      // Create account object
      if (!account_.statistics_ini_f ||
          (account_.statistics_ini_f.is_deleted () && !f.is_deleted ()))
        {
          // Get reader
          auto reader = f.new_reader ();
          if (!reader)
            return;

          // Decode file
          mobius::decoder::inifile ini (reader);

          if (ini.has_value ("statistics", "TotalDownloadedBytes"))
            account_.total_downloaded_bytes = std::stol (ini.get_value ("statistics", "TotalDownloadedBytes"));

          if (ini.has_value ("statistics", "TotalUploadedBytes"))
            account_.total_uploaded_bytes = std::stol (ini.get_value ("statistics", "TotalUploadedBytes"));

          if (ini.has_value ("statistics", "DownCompletedFiles"))
            account_.download_completed_files = std::stol (ini.get_value ("statistics", "DownCompletedFiles"));

          account_.statistics_ini_f = f;
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode PreferencesKad.dat file
// @param f File object
// @see CPrefs::ReadFile@kademlia/kademlia/Prefs.cpp
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_preferenceskad_dat_file (const mobius::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      if (!account_.preferenceskad_dat_f ||
          (account_.preferenceskad_dat_f.is_deleted () && !f.is_deleted ()))
        {
          // Get reader
          auto reader = f.new_reader ();
          if (!reader)
            return;

          // Decode file
          mobius::decoder::data_decoder decoder (reader);

          account_.preferenceskad_dat_f = f;
          account_.kamdelia_ip = decoder.get_ipv4_le ();

          decoder.skip (2);

          auto c1 = decoder.get_uint32_le ();
          auto c2 = decoder.get_uint32_le ();
          auto c3 = decoder.get_uint32_le ();
          auto c4 = decoder.get_uint32_le ();

          account_.kamdelia_guid =
                mobius::string::to_hex (c1, 8) +
                mobius::string::to_hex (c2, 8) +
                mobius::string::to_hex (c3, 8) +
                mobius::string::to_hex (c4, 8);
        }
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode AC_SearchStrings.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_ac_searchstrings_dat_file (const mobius::io::file& f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
      {
        // Get reader
        auto reader = f.new_reader ();
        if (!reader)
            return;

        // Decode file
        mobius::io::line_reader lr (reader, "utf-16", "\r\n");
        std::string line;
        std::size_t rec_number = 0;

        while (lr.read (line))
          {
            ++rec_number;

            if (!line.empty ())
              {
                autofill af;

                af.is_deleted = f.is_deleted ();
                af.username = username_;
                af.value = line;
                af.id = "search";
                af.f = f;

                af.metadata.set("record_number", rec_number);

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
// @brief Decode StoredSearches.met file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_storedsearches_met_file (const mobius::io::file& f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
      {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_stored_searches_met stored_searches (f.new_reader ());

        if (!stored_searches)
          {
            log.info (__LINE__, "File is not an instance of StoredSearches.met. Path: " + f.get_path ());
            return;
          }

        auto version = stored_searches.get_version ();
        log.info (__LINE__, "StoredSearches.met file decoded. Path: " + f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add searches
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto& s : stored_searches.get_searches ())
          {
            autofill af;

            af.is_deleted = f.is_deleted ();
            af.username = username_;
            af.value = s.expression;
            af.id = "search";
            af.f = f;
            
            af.metadata = mobius::pod::map ();
            af.metadata.set("stored_searches_version", version);
            af.metadata.set("search_id", s.id);
            af.metadata.set("e_type", s.e_type);
            af.metadata.set("special_title", s.special_title);
            af.metadata.set("filetype", s.filetype);
            af.metadata.set("file_count", s.files.size ());

            autofills_.push_back (af);
          }
      }
    catch (const std::exception& e)
      {
        log.warning (__LINE__, e.what ());
      }
}

/*
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
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}*/

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save evidences
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_evidences ()
{
  auto transaction = item_.new_transaction ();

  _save_accounts ();
  _save_autofills ();
  //_save_local_files ();
  //_save_p2p_remote_files ();
  //_save_received_files ();
  //_save_sent_files ();
  //_save_shared_files ();

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
      metadata.set ("emule_guid", a.emule_guid);
      metadata.set ("kamdelia_guid", a.kamdelia_guid);
      metadata.set ("kamdelia_ip", a.kamdelia_ip);
      metadata.set ("incoming_dir", a.incoming_dir);
      metadata.set ("temp_dir", a.temp_dir);
      metadata.set ("nickname", a.nick);
      metadata.set ("app_version", a.app_version);
      metadata.set ("auto_start", to_string (a.auto_start));
      metadata.set ("total_downloaded_bytes", a.total_downloaded_bytes);
      metadata.set ("total_uploaded_bytes", a.total_uploaded_bytes);
      metadata.set ("download_completed_files", a.download_completed_files);

      if (!a.emule_guid.empty ())
        {
          auto e = item_.new_evidence ("user-account");

          e.set_attribute ("account_type", "p2p.edonkey");
          e.set_attribute ("id", a.emule_guid);
          e.set_attribute ("password", {});
          e.set_attribute ("password_found", "no");
          e.set_attribute ("is_deleted", a.is_deleted);
          e.set_attribute ("metadata", metadata.clone ());
          e.set_tag ("p2p");

          e.add_source (a.preferences_dat_f);
          e.add_source (a.preferences_ini_f);
          e.add_source (a.preferenceskad_dat_f);
          e.add_source (a.statistics_ini_f);
        }

      if (!a.kamdelia_guid.empty ())
        {
          auto e = item_.new_evidence ("user-account");

          e.set_attribute ("account_type", "p2p.kamdelia");
          e.set_attribute ("id", a.kamdelia_guid);
          e.set_attribute ("password", {});
          e.set_attribute ("password_found", "no");
          e.set_attribute ("is_deleted", a.is_deleted);
          e.set_attribute ("metadata", metadata.clone ());
          e.set_tag ("p2p");

          e.add_source (a.preferences_dat_f);
          e.add_source (a.preferences_ini_f);
          e.add_source (a.preferenceskad_dat_f);
          e.add_source (a.statistics_ini_f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save autofill entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_autofills ()
{
  for (const auto& af : autofills_)
    {
      mobius::pod::map metadata = af.metadata.clone ();
      metadata.set ("id", af.id);

      auto e = item_.new_evidence ("autofill");

      e.set_attribute ("field_name", "search");
      e.set_attribute ("value", af.value);
      e.set_attribute ("app_id", APP_ID);
      e.set_attribute ("app_name", APP_NAME);
      e.set_attribute ("username", af.username);
      e.set_attribute ("is_deleted", af.is_deleted);
      e.set_attribute ("metadata", metadata);

      e.set_tag ("p2p");
      e.add_source (af.f);
    }
}
/*
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
*/

} // namespace mobius::extension::app::emule

