#ifndef MOBIUS_EXTENSION_APP_EMULETORRENT_EVIDENCE_LOADER_IMPL_HPP
#define MOBIUS_EXTENSION_APP_EMULETORRENT_EVIDENCE_LOADER_IMPL_HPP

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
#include <mobius/framework/evidence_flag.hpp>
#include <mobius/framework/evidence_loader_impl_base.hpp>
#include <mobius/framework/model/item.hpp>
#include <mobius/io/file.h>
#include <map>
#include <vector>

namespace mobius::extension::app::emuletorrent
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief EmuleTorrent account
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct account
{
  std::string username;
  std::string edonkey_guid;
  std::string edonkey_nickname;
  std::string download_temp_path;
  std::string download_temp_path_mule;
  std::string download_save_path;
  std::string download_save_path_mule;
  bool autorun = false;
  mobius::io::file f;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remote source (another users sharing file)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct remote_source
{
  mobius::datetime::datetime timestamp;
  std::string ip;
  std::uint16_t port;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief File
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct file
{
  // basic attributes
  std::string hash_ed2k;
  std::string hash_sha1;
  std::string hash_sha2_256;
  std::string account_guid;
  std::string username;
  std::string filename;
  std::string path;
  std::uint64_t size = 0;

  // flags
  mobius::framework::evidence_flag flag_downloaded;
  mobius::framework::evidence_flag flag_uploaded;
  mobius::framework::evidence_flag flag_shared;
  mobius::framework::evidence_flag flag_completed;
  mobius::framework::evidence_flag flag_corrupted;

  // transfer info
  mobius::datetime::datetime download_started_time;
  mobius::datetime::datetime download_completed_time;

  // metadata
  mobius::pod::map metadata;

  // remote sources
  std::vector <remote_source> remote_sources;

  // evidence sources
  mobius::io::file ed2k_fastresume_f;
  mobius::io::file bt_fastresume_f;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>EmuleTorrent evidence_loader</i> implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class evidence_loader_impl : public mobius::framework::evidence_loader_impl_base
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  explicit evidence_loader_impl (const mobius::framework::model::item&, scan_type);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Check if object is valid
  // @return true/false
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  explicit operator bool () const noexcept final
  {
    return true;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get evidence_loader type
  // @return Type as string
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_type () const final
  {
    return "app.emuletorrent";
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Function prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void run () final;

private:
  // @brief Case item
  mobius::framework::model::item item_;

  // @brief Scan type
  scan_type scan_type_;

  // @brief User name
  std::string username_;

  // @brief Account data
  account account_;

  // @brief All accounts found
  std::vector <account> accounts_;

  // @brief File catalog
  std::vector <file> files_;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Helper functions
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void _scan_canonical_folders ();
  void _scan_canonical_root_folder (const mobius::io::folder&);
  void _scan_canonical_user_folder (const mobius::io::folder&);
  void _scan_canonical_emuletorrent_local_folder (const mobius::io::folder&);
  void _scan_canonical_emuletorrent_roaming_folder (const mobius::io::folder&);

  void _scan_all_folders ();
  void _scan_generic_folder (const mobius::io::folder&);

  void _decode_emuletorrent_ini_file (const mobius::io::file&);
  void _decode_bt_fastresume_file (const mobius::io::file&);
  void _decode_ed2k_fastresume_file (const mobius::io::file&);

  void _save_evidences ();
  void _save_accounts ();
  void _save_local_files ();
  void _save_p2p_remote_files ();
  void _save_received_files ();
  void _save_sent_files ();
  void _save_shared_files ();
};

} // namespace mobius::extension::app::emuletorrent

#endif




