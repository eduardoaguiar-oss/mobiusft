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
//#include "file_key_index_dat.hpp"
//#include "file_known_met.hpp"
//#include "file_part_met.hpp"
//#include "file_part_met_txtsrc.hpp"
//#include "file_stored_searches_met.hpp"
#include <mobius/core/log.hpp>
#include <mobius/core/datasource/datasource_vfs.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/decoder/inifile.hpp>
#include <mobius/framework/model/evidence.hpp>
#include <mobius/core/io/line_reader.hpp>
#include <mobius/core/io/walker.hpp>
#include <mobius/core/string_functions.hpp>

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
// According to eMule Homepage: "Your Incoming and Temporary directory are always shared"
// @see https://www.emule-project.net/home/perl/help.cgi?l=1&topic_id=112&rm=show_topic
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string APP_ID = "utorrent";
static const std::string APP_NAME = "µTorrent";
static const std::string ANT_ID = "evidence.app-utorrent";
static const std::string ANT_NAME = APP_NAME;
static const std::string ANT_VERSION = "1.0";

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get vector of hashes for a given file
// @param f File structure
// @return Vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T> std::vector <mobius::core::pod::data>
get_file_hashes (const T& f)
{
    std::vector <mobius::core::pod::data> hashes = {
        {"ed2k", mobius::core::string::toupper (f.hash_ed2k)}
    };

    auto iter = std::find_if (
        f.tags.begin (),
        f.tags.end (),
        [](const auto& t){ return t.get_id () == 0x27; }    // FT_AICH_HASH
    );

    if (iter != f.tags.end ())
      {
        auto hash_value = iter->get_value ();
        hashes.push_back ({"aich", static_cast <std::string> (hash_value)});
      }

    return hashes;
}


} // namespace

namespace mobius::extension::app::utorrent
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
evidence_loader_impl::_scan_canonical_root_folder (const mobius::core::io::folder& folder)
{
/*
    username_ = {};
    auto w = mobius::core::io::walker (folder);

    // Users folders
    for (const auto& f : w.get_folders_by_pattern ("users/*"))
        _scan_canonical_user_folder (f);

    // Win XP folders
    for (const auto& f : w.get_folders_by_path ("program files/emule"))
        _scan_canonical_emule_xp_folder (f);

    for (const auto& f : w.get_folders_by_path ("program files/dreamule"))
        _scan_canonical_emule_xp_folder (f);

    for (const auto& f : w.get_folders_by_path ("arquivos de programas/emule"))
        _scan_canonical_emule_xp_folder (f);

    for (const auto& f : w.get_folders_by_path ("arquivos de programas/dreamule"))
        _scan_canonical_emule_xp_folder (f);
*/
}

/*
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan user folder for evidences
// @param folder User folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_user_folder (const mobius::core::io::folder& folder)
{
    username_ = folder.get_name ();
    auto w = mobius::core::io::walker (folder);

    for (const auto& f : w.get_folders_by_path ("appdata/local/emule/config"))
        _scan_canonical_emule_config_folder (f);

    for (const auto& f : w.get_folders_by_path ("downloads/emule/incoming"))
        _scan_canonical_emule_download_folder (f);

    for (const auto& f : w.get_folders_by_path ("downloads/emule/temp"))
        _scan_canonical_emule_download_folder (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan emule/dreamule folder for evidences
// @param folder emule/dreamule folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_emule_xp_folder (const mobius::core::io::folder& folder)
{
    auto w = mobius::core::io::walker (folder);
    
    for (const auto& f : w.get_folders_by_name ("config"))
        _scan_canonical_emule_config_folder (f);

    for (const auto& f : w.get_folders_by_name ("incoming"))
        _scan_canonical_emule_download_folder (f);

    for (const auto& f : w.get_folders_by_name ("temp"))
        _scan_canonical_emule_download_folder (f);
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan AppData/Local/eMule/config folder for evidences
// @param folder Config folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_emule_config_folder (const mobius::core::io::folder& folder)
{
    account_ = {};
    account_.username = username_;

    mobius::core::io::walker w (folder);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode account files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto& f : w.get_files ())
      {
        const std::string lname = mobius::core::string::tolower (f.get_name ());

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
        const std::string lname = mobius::core::string::tolower (f.get_name ());

        if (lname == "ac_searchstrings.dat")
            _decode_ac_searchstrings_dat_file (f);
            
        else if (lname == "key_index.dat")
            _decode_key_index_dat_file (f);

        else if (lname == "known.met")
            _decode_known_met_file (f);

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
// @brief Scan Download/emule folder
// @param folder Download/emule folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_emule_download_folder (const mobius::core::io::folder& folder)
{
    mobius::core::io::walker w (folder);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decoder .part.met files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto& f : w.get_files_by_pattern ("*.part.met"))
        _decode_part_met_file (f);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decoder .part.met.txtsrc files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto& f : w.get_files_by_pattern ("*.part.met.txtsrc"))
        _decode_part_met_txtsrc_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode Preferences.dat file
// @param f File object
// @see CPreferences::Init@srchybrid/Preferences.cpp
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_preferences_dat_file (const mobius::core::io::file& f)
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
          mobius::core::decoder::data_decoder decoder (reader);

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
evidence_loader_impl::_decode_preferences_ini_file (const mobius::core::io::file& f)
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
          mobius::core::decoder::inifile ini (reader);

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
evidence_loader_impl::_decode_statistics_ini_file (const mobius::core::io::file& f)
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
          mobius::core::decoder::inifile ini (reader);

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
evidence_loader_impl::_decode_preferenceskad_dat_file (const mobius::core::io::file& f)
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
          mobius::core::decoder::data_decoder decoder (reader);

          account_.preferenceskad_dat_f = f;
          account_.kamdelia_ip = decoder.get_ipv4_le ();

          decoder.skip (2);

          auto c1 = decoder.get_uint32_le ();
          auto c2 = decoder.get_uint32_le ();
          auto c3 = decoder.get_uint32_le ();
          auto c4 = decoder.get_uint32_le ();

          account_.kamdelia_guid =
                mobius::core::string::to_hex (c1, 8) +
                mobius::core::string::to_hex (c2, 8) +
                mobius::core::string::to_hex (c3, 8) +
                mobius::core::string::to_hex (c4, 8);
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
evidence_loader_impl::_decode_ac_searchstrings_dat_file (const mobius::core::io::file& f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
      {
        // Get reader
        auto reader = f.new_reader ();
        if (!reader)
            return;

        // Decode file
        mobius::core::io::line_reader lr (reader, "utf-16", "\r\n");
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
// @brief Decode KeyIndex.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_key_index_dat_file (const mobius::core::io::file& f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
      {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_key_index_dat key_index (f.new_reader ());

        if (!key_index)
          {
            log.info (__LINE__, "File is not an instance of KeyIndex.dat. Path: " + f.get_path ());
            return;
          }

        log.info (__LINE__, "KeyIndex.dat file decoded. Path: " + f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add local files
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto& k : key_index.get_keys ())
          {
            for (const auto& source : k.sources)
              {
                auto hash_ed2k = mobius::core::string::toupper (source.id);

                for (const auto& name : source.names)
                  {
                    auto metadata = get_metadata_from_tags (name.tags);
                    metadata.set ("network", "Kamdelia");
                    metadata.set ("key_id", k.id);
                    metadata.set ("lifetime", name.lifetime);

                    for (const auto& ip : name.ips)
                      {
                        remote_file rf;
                        rf.timestamp = ip.last_published;
                        rf.ip = ip.value;
                        rf.username = username_;
                        rf.key_index_dat_f = f;
                        rf.metadata = metadata.clone ();
                        rf.filename = metadata.get <std::string> ("name");
  
                        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
                        // Content hashes
                        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
                        std::vector <mobius::core::pod::data> hashes = {
                            {"ed2k", hash_ed2k}
                        };

                        auto aich_hash = metadata.get <std::string> ("hash_aich");

                        if (!aich_hash.empty ())
                            hashes.push_back ({"aich", aich_hash});

                        rf.hashes = hashes;

                        remote_files_.push_back (rf);
                      }
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
// @brief Decode Known.met file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_known_met_file (const mobius::core::io::file& f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
      {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_known_met known_met (f.new_reader ());

        if (!known_met)
          {
            log.info (__LINE__, "File is not an instance of Known.met. Path: " + f.get_path ());
            return;
          }

        log.info (__LINE__, "Known.met file decoded. Path: " + f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add local files
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto& kf : known_met.get_known_files ())
          {
            auto metadata = get_metadata_from_tags (kf.tags);

            local_file lf;

            lf.username = username_;
            lf.filename = metadata.get <std::string> ("name");
            
            if (!account_.incoming_dir.empty ())
                lf.path = account_.incoming_dir + '\\' + lf.filename;
  
            lf.flag_downloaded = true;
            lf.flag_uploaded = metadata.get <std::int64_t> ("uploaded_bytes") > 0;
            lf.flag_shared = mobius::framework::evidence_flag::always;
            lf.flag_corrupted = metadata.get <bool> ("is_corrupted");
            lf.flag_completed = true; // @see CPartFile::PerformFileCompleteEnd

            metadata.set ("flag_downloaded", to_string (lf.flag_downloaded));
            metadata.set ("flag_uploaded", to_string (lf.flag_uploaded));
            metadata.set ("flag_shared", to_string (lf.flag_shared));
            metadata.set ("flag_corrupted", to_string (lf.flag_corrupted));
            metadata.set ("flag_completed", to_string (lf.flag_completed));
            metadata.set ("last_modification_time", kf.last_modification_time);
            metadata.set ("network", "eDonkey");

            lf.metadata = metadata;
            lf.hashes = get_file_hashes (kf);
            lf.f = f;

            local_files_.push_back (lf);
          }
      }
    catch (const std::exception& e)
      {
        log.warning (__LINE__, e.what ());
      }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode .part.met file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_part_met_file (const mobius::core::io::file& f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
      {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_part_met part_met (f.new_reader ());

        if (!part_met)
          {
            log.info (__LINE__, "File is not an instance of .part.met. Path: " + f.get_path ());
            return;
          }

        log.info (__LINE__, ".part.met file decoded. Path: " + f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Create local file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto metadata = get_metadata_from_tags (part_met.get_tags ());

        local_file lf;

        lf.username = username_;
        lf.path = f.get_path ();
        lf.path.erase (lf.path.size () - 4);
        lf.filename = metadata.get <std::string> ("name");
        lf.is_deleted = f.is_deleted ();
        lf.f = f;

        lf.flag_downloaded = true;
        lf.flag_uploaded = metadata.get <std::int64_t> ("uploaded_bytes") > 0;
        lf.flag_shared = mobius::framework::evidence_flag::always;
        lf.flag_corrupted = metadata.get <bool> ("is_corrupted");
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
        std::vector <mobius::core::pod::data> hashes = {
            {"ed2k", mobius::core::string::toupper (part_met.get_hash_ed2k ())}
        };

        auto aich_hash = metadata.get <std::string> ("hash_aich");

        if (!aich_hash.empty ())
            hashes.push_back ({"aich", aich_hash});

        lf.hashes = hashes;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add local file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        local_files_.push_back (lf);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add part.met file to the list of part.met files
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto iter = part_met_files_.find (f.get_name ());

        if (iter == part_met_files_.end () || (iter->second.is_deleted && !f.is_deleted ()))
            part_met_files_[f.get_name ()] = lf;
      }
    catch (const std::exception& e)
      {
        log.warning (__LINE__, e.what ());
      }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode .part.met.txtsrc file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_part_met_txtsrc_file (const mobius::core::io::file& f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
      {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_part_met_txtsrc txtsrc (f.new_reader ());

        if (!txtsrc)
          {
            log.info (__LINE__, "File is not an instance of .part.met.txtsrc. Path: " + f.get_path ());
            return;
          }

        log.info (__LINE__, ".part.met.txtsrc file decoded. Path: " + f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get corresponding part.met file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto part_met_txtsrc_name = f.get_name ();
        auto part_met_name = part_met_txtsrc_name.substr (0, part_met_txtsrc_name.size () - 7);
        auto iter = part_met_files_.find (part_met_name);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Create remote file if part.met file is found
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        if (iter != part_met_files_.end ())
          {
            auto lf = iter->second;

            for (const auto& source : txtsrc.get_sources())
              {
                auto rf = remote_file();

                rf.username = username_;
                rf.timestamp = f.get_modification_time ();
                rf.ip = source.ip;
                rf.port = source.port;
                rf.filename = lf.filename;
                rf.part_met_f = lf.f;
                rf.part_met_txtsrc_f = f;
                rf.hashes = lf.hashes.clone();
                rf.metadata = lf.metadata.clone();

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
// @brief Decode StoredSearches.met file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_storedsearches_met_file (const mobius::core::io::file& f)
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
            
            af.metadata = mobius::core::pod::map ();
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
*/

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save evidences
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_evidences ()
{
  auto transaction = item_.new_transaction ();
/*
  _save_accounts ();
  _save_autofills ();
  _save_local_files ();
  _save_p2p_remote_files ();
  _save_received_files ();
  _save_sent_files ();
  _save_shared_files ();
*/

  item_.set_ant (ANT_ID, ANT_NAME, ANT_VERSION);
  transaction.commit ();
}

/*
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save accounts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_accounts ()
{
  for (const auto& a : accounts_)
    {
      mobius::core::pod::map metadata;
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
      mobius::core::pod::map metadata = af.metadata.clone ();
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
      if (lf.flag_downloaded.is_yes ())
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
      e.set_attribute ("metadata", rf.metadata);

      e.set_tag ("p2p");

      e.add_source (rf.key_index_dat_f);
      e.add_source (rf.part_met_f);
      e.add_source (rf.part_met_txtsrc_f);
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
      if (lf.flag_uploaded.is_yes ())
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
      if (lf.flag_shared.is_yes () || lf.flag_shared.is_always ())
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
        }
    }
}

*/

} // namespace mobius::extension::app::utorrent
