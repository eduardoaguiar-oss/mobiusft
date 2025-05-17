#ifndef MOBIUS_EXTENSION_APP_SHAREAZA_EVIDENCE_LOADER_IMPL_HPP
#define MOBIUS_EXTENSION_APP_SHAREAZA_EVIDENCE_LOADER_IMPL_HPP

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
#include "CThumbCache.hpp"
#include <map>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/pod/map.hpp>
#include <mobius/framework/evidence_flag.hpp>
#include <mobius/framework/evidence_loader_impl_base.hpp>
#include <mobius/framework/model/item.hpp>
#include <vector>

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Shareaza account
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct account
{
    bool is_deleted = false;
    std::string gnutella_guid;
    std::string bittorrent_guid;
    std::string username;
    std::string identity;
    std::string install_path;
    std::string user_path;
    std::string collection_path;
    std::string complete_path;
    std::string incomplete_path;
    std::string torrent_path;

    CThumbCache thumbcache;

    mobius::core::io::file f;
    mobius::core::io::file shareaza_db3_f;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Autofill
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct autofill
{
    bool is_deleted = false;
    std::string username;
    std::string value;
    std::string id;
    mobius::core::io::file f;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Searched text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct search
{
    // attributes
    mobius::core::datetime::datetime timestamp;
    std::string text;
    std::string username;

    // metadata
    mobius::core::pod::map metadata;

    // files
    mobius::core::io::file f;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Local file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct local_file
{
    // attributes
    std::string path;
    std::string filename;
    std::string username;

    // metadata
    mobius::core::pod::map metadata;

    // hashes
    mobius::core::pod::data hashes;

    // thumbnail_data
    mobius::core::bytearray thumbnail_data;

    // flags
    bool flag_downloaded = false;
    bool flag_uploaded = false;
    bool flag_shared = false;
    bool flag_completed = false;

    // files
    mobius::core::io::file f;
    mobius::core::io::file shareaza_db3_f;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remote file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct remote_file
{
    // attributes
    mobius::core::datetime::datetime timestamp;
    std::string ip;
    std::uint16_t port = 0;
    std::string filename;
    std::string username;

    // metadata
    mobius::core::pod::map metadata;

    // hashes
    mobius::core::pod::data hashes;

    // thumbnail_data
    mobius::core::bytearray thumbnail_data;

    // files
    mobius::core::io::file f;
    mobius::core::io::file shareaza_db3_f;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>Shareaza evidence_loader</i> implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class evidence_loader_impl : public mobius::framework::evidence_loader_impl_base
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit evidence_loader_impl (const mobius::framework::model::item &,
                                   scan_type);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Function prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void run () final;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if object is valid
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit
    operator bool () const noexcept final
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
        return "app-shareaza";
    }

  private:
    // @brief Case item
    mobius::framework::model::item item_;

    // @brief Scan type
    scan_type scan_type_;

    // @brief User name
    std::string username_;

    // @brief Account data
    account account_;

    // @brief Accounts found
    std::vector<account> accounts_;

    // @brief Autofills found
    std::vector<autofill> autofills_;

    // @brief Searched texts
    std::vector<search> searches_;

    // @brief Library files
    std::vector<local_file> local_files_;

    // @brief Remote files
    std::vector<remote_file> remote_files_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _scan_canonical_folders ();
    void _scan_canonical_root_folder (const mobius::core::io::folder &);
    void _scan_canonical_user_folder (const mobius::core::io::folder &);
    void
    _scan_canonical_shareaza_data_folder (const mobius::core::io::folder &);

    void _decode_library_dat_file (const mobius::core::io::file &);
    void _decode_ntuser_dat_file (const mobius::core::io::file &);
    void _decode_profile_xml_file (const mobius::core::io::file &);
    void _decode_sd_file (const mobius::core::io::file &);
    void _decode_searches_dat_file (const mobius::core::io::file &);
    void _decode_shareaza_db3_file (const mobius::core::io::file &);

    void _save_evidences ();
    void _save_accounts ();
    void _save_autofills ();
    void _save_searched_texts ();
    void _save_local_files ();
    void _save_p2p_remote_files ();
    void _save_received_files ();
    void _save_sent_files ();
    void _save_shared_files ();
};

} // namespace mobius::extension::app::shareaza

#endif
