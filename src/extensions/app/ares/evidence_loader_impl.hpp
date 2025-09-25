#ifndef MOBIUS_EXTENSION_APP_ARES_EVIDENCE_LOADER_IMPL_HPP
#define MOBIUS_EXTENSION_APP_ARES_EVIDENCE_LOADER_IMPL_HPP

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
#include <map>
#include <mobius/core/io/file.hpp>
#include <mobius/framework/evidence_flag.hpp>
#include <mobius/framework/evidence_loader_impl_base.hpp>
#include <mobius/framework/model/item.hpp>
#include <vector>

namespace mobius::extension::app::ares
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Ares account
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct account
{
    bool is_deleted = false;
    std::string guid;
    std::string dht_id;
    std::string mdht_id;
    std::string nickname;
    std::string username;
    mobius::core::io::file f;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remote source (another users sharing file)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct remote_source
{
    mobius::core::datetime::datetime timestamp;
    std::string ip;
    std::uint16_t port;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Torrent file (file description containing in a torrent stream)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct torrent_file
{
    std::uint64_t idx;
    std::uint64_t size;
    mobius::core::datetime::datetime last_modification_time;
    std::string name;
    std::string path;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Ares file catalog
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct file
{
    // basic attributes
    std::string hash_sha1;
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
    mobius::core::datetime::datetime download_started_time;
    mobius::core::datetime::datetime download_completed_time;
    mobius::core::datetime::datetime upload_started_time;

    // metadata
    mobius::core::pod::map metadata;

    // torrent files
    std::vector<torrent_file> torrent_files;

    // alt-sources (remote sources)
    std::vector<remote_source> remote_sources;

    // evidence sources
    mobius::core::io::file shareh_f;
    mobius::core::io::file sharel_f;
    mobius::core::io::file torrenth_f;
    mobius::core::io::file phashidx_f;
    mobius::core::io::file arestra_f;
    mobius::core::io::file tempdl_phash_f;
    mobius::core::io::file tempdl_pbthash_f;
    mobius::core::io::file tempul_udpphash_f;

    std::uint64_t shareh_idx = 0;
    std::uint64_t sharel_idx = 0;
    std::uint64_t torrenth_idx = 0;
    std::uint64_t phashidx_idx = 0;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>ares evidence_loader</i> implementation class
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
        return "app-ares";
    }

  private:
    // @brief Case item
    mobius::framework::model::item item_;

    // @brief Scan type
    scan_type scan_type_;

    // @brief User name
    std::string username_;

    // @brief Account files
    std::map<std::string, file> account_files_;

    // @brief File catalog
    std::vector<file> files_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _scan_canonical_folders ();
    void _scan_canonical_root_folder (const mobius::core::io::folder &);
    void _scan_canonical_user_folder (const mobius::core::io::folder &);
    void _scan_canonical_ares_folder (const mobius::core::io::folder &);
    void
    _scan_canonical_ares_my_shared_folder (const mobius::core::io::folder &);
    void _scan_canonical_ares_data_folder (const mobius::core::io::folder &);
    void _scan_canonical_ares_data_file (const mobius::core::io::file &);
    void
    _scan_canonical_ares_data_tempdl_folder (const mobius::core::io::folder &);
    void _scan_canonical_ares_data_tempdl_phash_file (
        const mobius::core::io::file &);
    void _scan_canonical_ares_data_tempdl_pbthash_file (
        const mobius::core::io::file &);
    void
    _scan_canonical_ares_data_tempul_folder (const mobius::core::io::folder &);
    void _scan_canonical_ares_data_tempul_udpphash_file (
        const mobius::core::io::file &);

    void _scan_all_folders ();
    void _scan_generic_folder (const mobius::core::io::folder &);

    void _decode_arestra_file (const mobius::core::io::file &);
    void _decode_phashidx_dat_file (const mobius::core::io::file &);
    void _decode_shareh_dat_file (const mobius::core::io::file &);
    void _decode_sharel_dat_file (const mobius::core::io::file &);
    void _decode_torrenth_dat_file (const mobius::core::io::file &);

    void _save_evidences ();
    void _save_local_files ();
    void _save_p2p_remote_files ();
    void _save_received_files ();
    void _save_sent_files ();
    void _save_shared_files ();
};

} // namespace mobius::extension::app::ares

#endif
