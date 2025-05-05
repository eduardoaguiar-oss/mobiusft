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
// @see https://www.forensicfocus.com/articles/forensic-analysis-of-the-%CE%BCtorrent-peer-to-peer-client-in-windows/
// @see https://robertpearsonblog.wordpress.com/2016/11/10/utorrent-forensic-artifacts/
// @see https://robertpearsonblog.wordpress.com/2016/11/11/utorrent-and-windows-10-forensic-nuggets-of-info/
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
    username_ = {};
    auto w = mobius::core::io::walker (folder);

    // Users folders
    for (const auto& f : w.get_folders_by_pattern ("users/*"))
        _scan_canonical_user_folder (f);
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan user folder for evidences
// @param folder User folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_user_folder (const mobius::core::io::folder& folder)
{
    username_ = folder.get_name ();
    auto w = mobius::core::io::walker (folder);

    for (const auto& f : w.get_folders_by_path ("appdata/roaming/utorrent"))
        _scan_canonical_utorrent_folder (f);

    for (const auto& f : w.get_folders_by_path ("appdata/roaming/bittorrent"))
        _scan_canonical_utorrent_folder (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan utorrent folder for evidences
// @param folder utorrent folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_utorrent_folder (const mobius::core::io::folder& folder)
{
    profile_ = {};
    profile_.set_username (username_);

    auto w = mobius::core::io::walker (folder);
    
    for (const auto& [name, f] : w.get_files_with_name())
      {
        if (name == "settings.dat")
            _decode_settings_dat_file (f);

        else if (name == "settings.dat.old")
            _decode_settings_dat_file (f);

        else if (name == "settings.dat.bak")
            _decode_settings_dat_file (f);

        else if (name == "settings.dat.tmp")
            _decode_settings_dat_file (f);

        else if (name == "dht.dat")
            _decode_dht_dat_file (f);

        else if (name == "dht.dat.old")
            _decode_dht_dat_file (f);
      }

    if (profile_)
        profiles_.push_back(profile_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode dht.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_dht_dat_file (const mobius::core::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      profile_.add_dht_dat_file(f);
    }
  catch (const std::exception& e)
    {
      log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode Settings.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_settings_dat_file (const mobius::core::io::file& f)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  try
    {
      profile_.add_settings_dat_file(f);
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
evidence_loader_impl::_save_evidences()
{
  auto transaction = item_.new_transaction();

  _save_accounts();
  _save_ip_addresses();
  /*
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save accounts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_accounts ()
{
    for(const auto& p : profiles_)
    {
        auto settings = p.get_main_settings ();

        mobius::core::pod::map metadata;
        metadata.set("app_id", APP_ID);
        metadata.set("app_name", APP_NAME);
        metadata.set("network", "BitTorrent");
        metadata.set("username", p.get_username ());
        metadata.set("total_downloaded_bytes", settings.total_bytes_downloaded);
        metadata.set("total_uploaded_bytes", settings.total_bytes_uploaded);
        metadata.set("execution_count", settings.execution_count);
        metadata.set("installation_time", settings.installation_time);
        metadata.set("last_used_time", settings.last_used_time);
        metadata.set("last_bin_change_time", settings.last_bin_change_time);
        metadata.set("version", settings.version);
        metadata.set("installation_version", settings.installation_version);
        metadata.set("language", settings.language);
        metadata.set("computer_id", settings.computer_id);
        metadata.set("auto_start", settings.auto_start ? "yes" : "no");

        for (const auto& account : p.get_accounts())
        {
            auto e_metadata = metadata.clone();
            e_metadata.set("first_dht_timestamp", account.first_dht_timestamp);
            e_metadata.set("last_dht_timestamp", account.last_dht_timestamp);

            auto e = item_.new_evidence ("user-account");

            e.set_attribute ("account_type", "p2p.bittorrent");
            e.set_attribute ("id", account.client_id);
            e.set_attribute ("password", {});
            e.set_attribute ("password_found", "no");
            e.set_attribute ("is_deleted", account.f.is_deleted());
            e.set_attribute ("metadata", e_metadata);
            e.set_tag ("p2p");
  
            for (const auto& f : account.files)
                e.add_source (account.f);

            e.add_source (settings.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save IP addresses
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_ip_addresses ()
{
    for(const auto& p : profiles_)
    {
        auto settings = p.get_main_settings ();

        mobius::core::pod::map metadata;
        metadata.set("network", "BitTorrent");
        metadata.set("total_downloaded_bytes", settings.total_bytes_downloaded);
        metadata.set("total_uploaded_bytes", settings.total_bytes_uploaded);
        metadata.set("execution_count", settings.execution_count);
        metadata.set("installation_time", settings.installation_time);
        metadata.set("last_used_time", settings.last_used_time);
        metadata.set("last_bin_change_time", settings.last_bin_change_time);
        metadata.set("version", settings.version);
        metadata.set("installation_version", settings.installation_version);
        metadata.set("language", settings.language);
        metadata.set("computer_id", settings.computer_id);
        metadata.set("auto_start", settings.auto_start ? "yes" : "no");

        for (const auto& account : p.get_accounts())
        {
            auto e_metadata = metadata.clone();
            e_metadata.set("client_id", account.client_id);
            e_metadata.set("first_dht_timestamp", account.first_dht_timestamp);
            e_metadata.set("last_dht_timestamp", account.last_dht_timestamp);

            for (const auto& [ip, timestamp] : account.ip_addresses)
            {
                auto e = item_.new_evidence ("ip-address");

                e.set_attribute("timestamp", timestamp);
                e.set_attribute("address", ip);
                e.set_attribute("app_id", APP_ID);
                e.set_attribute("app_name", APP_NAME);
                e.set_attribute("username", p.get_username ());
                e.set_attribute ("metadata", e_metadata.clone());
                e.set_tag ("p2p");

                for (const auto& f : account.files)
                    e.add_source (f);

                e.add_source (settings.f);
            }
        }
    }
}


/*
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
