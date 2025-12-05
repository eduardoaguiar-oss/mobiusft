#ifndef MOBIUS_EXTENSION_APP_UTORRENT_PROFILE_HPP
#define MOBIUS_EXTENSION_APP_UTORRENT_PROFILE_HPP

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
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/pod/map.hpp>
#include <cstdint>
#include <memory>
#include <utility>
#include <set>
#include <string>
#include <vector>

namespace mobius::extension::app::utorrent
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief uTorrent profile class
// @author Eduardo Aguiar
// @see
// https://robertpearsonblog.wordpress.com/2016/11/10/utorrent-forensic-artifacts/
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class profile
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Account structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct account
    {
        // @brief Client ID
        std::string client_id;

        // @brief IP Addresses
        std::set<std::pair<std::string, mobius::core::datetime::datetime>>
            ip_addresses;

        // @brief First DHT timestamp
        mobius::core::datetime::datetime first_dht_timestamp;

        // @brief Last DHT timestamp
        mobius::core::datetime::datetime last_dht_timestamp;

        // @brief Source files
        std::vector<mobius::core::io::file> files;

        // @brief Most reliable dht.dat file
        mobius::core::io::file f;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Settings structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct settings
    {
        // @brief CID
        std::string computer_id;

        // @brief Auto start
        bool auto_start = false;

        // @brief Execution count
        std::uint32_t execution_count = 0;

        // @brief Total bytes downloaded
        std::uint64_t total_bytes_downloaded = 0;

        // @brief Total bytes uploaded
        std::uint64_t total_bytes_uploaded = 0;

        // @brief Installation time
        mobius::core::datetime::datetime installation_time;

        // @brief Last used time
        mobius::core::datetime::datetime last_used_time;

        // @brief Last bin change time
        mobius::core::datetime::datetime last_bin_change_time;

        // @brief Current version
        std::string version;

        // @brief Installation version
        std::string installation_version;

        // @brief Language
        std::string language;

        // @brief settings.dat file object
        mobius::core::io::file f;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Torrent content file structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct torrent_content_file
    {
        // @brief File name
        std::string name;

        // @brief File path
        std::string path;

        // @brief File length
        std::uint64_t length = 0;

        // @brief File offset
        std::uint64_t offset = 0;

        // @brief File piece length
        std::uint64_t piece_length = 0;

        // @brief File piece offset
        std::uint64_t piece_offset = 0;

        // @brief Creation time
        mobius::core::datetime::datetime creation_time;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Local file structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct local_file
    {
        // @brief Entry name
        std::string name;

        // @brief Download URL
        std::string download_url;

        // @brief Caption
        std::string caption;

        // @brief Path
        std::string path;

        // @brief Size
        std::uint64_t size = 0;

        // @brief Added timestamp
        mobius::core::datetime::datetime added_timestamp;    

        // @brief Block size
        std::uint64_t blocksize = 0;

        // @brief Bytes downloaded
        std::uint64_t bytes_downloaded = 0;

        // @brief Bytes uploaded
        std::uint64_t bytes_uploaded = 0;

        // @brief Comment
        std::string comment;

        // @brief Completed time
        mobius::core::datetime::datetime completed_timestamp;

        // @brief Created by
        std::string created_by;

        // @brief Creation time
        mobius::core::datetime::datetime creation_time;

        // @brief Downloaded time in seconds
        std::uint64_t downloaded_time = 0;

        // @brief Encoding
        std::string encoding;

        // @brief Info hash
        std::string info_hash;

        // @brief Info hash v2
        std::string info_hash2;

       // @brief Is auto managed
        bool is_auto_managed = false;

        // @brief Is corrupted
        bool is_corrupted = false;

        // @brief Is paused
        bool is_paused = false;

        // @brief Is seeding
        bool is_seeding = false;

        // @brief Is sharing
        bool is_sharing = false;

        // @brief Is super seeding
        bool is_super_seeding = false;

        // @brief Is sequential downloading
        bool is_sequential_downloading = false;

        // @brief Is uploading
        bool is_uploading = false;

        // @brief Is visible
        bool is_visible = true;

        // @brief Last download timestamp
        mobius::core::datetime::datetime last_download_timestamp;

        // @brief Last seen complete time
        mobius::core::datetime::datetime last_seen_complete_timestamp;

        // @brief Last upload timestamp
        mobius::core::datetime::datetime last_upload_timestamp;
        
        // @brief Metadata time
        mobius::core::datetime::datetime metadata_time;

        // @brief Seeded time in seconds
        std::uint64_t seeded_time = 0;

        // @brief Torrent name
        std::string torrent_name;

        // @brief Metadata
        mobius::core::pod::map metadata;

        // @brief Peers
        std::vector<std::pair<std::string, std::uint16_t>> peers;

        // @brief Resume.dat file
        mobius::core::io::file resume_file;

        // @brief Torrent file
        mobius::core::io::file torrent_file;

        // @brief Torrent content files
        std::vector<torrent_content_file> content_files;

        // @brief Source files
        std::vector<mobius::core::io::file> sources;
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
    operator bool () const;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string get_username () const;
    mobius::core::io::folder get_folder () const;
    std::string get_path () const;
    mobius::core::datetime::datetime get_creation_time () const;
    mobius::core::datetime::datetime get_last_modified_time () const;
    std::vector<account> get_accounts () const;
    std::vector<local_file> get_local_files () const;
    std::size_t size_local_files () const;
    settings get_main_settings () const;
    std::vector<settings> get_settings () const;
    void add_dht_dat_file (const mobius::core::io::file &);
    void add_resume_dat_file (const mobius::core::io::file &);
    void add_settings_dat_file (const mobius::core::io::file &);
    void add_torrent_file (const mobius::core::io::file &);

  private:
    // @brief Forward declaration
    class impl;

    // @brief Implementation pointer
    std::shared_ptr<impl> impl_;
};

} // namespace mobius::extension::app::utorrent

#endif
