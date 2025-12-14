#ifndef MOBIUS_EXTENSION_APP_ARES_PROFILE_HPP
#define MOBIUS_EXTENSION_APP_ARES_PROFILE_HPP

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
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
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/folder.hpp>
#include <mobius/core/pod/map.hpp>
#include <mobius/framework/evidence_flag.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace mobius::extension::app::ares
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Ares Galaxy profile class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class profile
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Remote source (another users sharing file)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct remote_source
    {
        mobius::core::datetime::datetime timestamp;
        std::string ip;
        std::uint16_t port;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Torrent file (file description containing in a torrent stream)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct torrent_file
    {
        std::uint64_t idx;
        std::uint64_t size;
        mobius::core::datetime::datetime last_modification_time;
        std::string name;
        std::string path;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Ares file catalog
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
        std::vector<mobius::core::io::file> evidence_sources;

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

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    profile ();
    profile (const profile &) noexcept = default;
    profile (profile &&) noexcept = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    profile &operator= (const profile &) noexcept = default;
    profile &operator= (profile &&) noexcept = default;
    operator bool () const noexcept;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string get_username () const;
    mobius::core::io::folder get_folder () const;
    std::string get_path () const;
    mobius::core::datetime::datetime get_creation_time () const;
    mobius::core::datetime::datetime get_last_modified_time () const;

    std::size_t size_files () const;
    std::vector<file> get_files () const;

    void add_phashidx_file (const mobius::core::io::file &);
    void add_shareh_file (const mobius::core::io::file &);
    void add_sharel_file (const mobius::core::io::file &);
    void add_torrenth_file (const mobius::core::io::file &);
    void add_tempdl_phash_file (const mobius::core::io::file &);
    void add_tempdl_pbthash_file (const mobius::core::io::file &);
    void add_tempul_udpphash_file (const mobius::core::io::file &);

  private:
    // @brief Forward declaration
    class impl;

    // @brief Implementation pointer
    std::shared_ptr<impl> impl_;
};

} // namespace mobius::extension::app::ares

#endif
