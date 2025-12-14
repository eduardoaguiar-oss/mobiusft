#ifndef MOBIUS_EXTENSION_APP_UTORRENT_FILE_RESUME_DAT_HPP
#define MOBIUS_EXTENSION_APP_UTORRENT_FILE_RESUME_DAT_HPP

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
#include <mobius/core/io/reader.hpp>
#include <mobius/core/pod/map.hpp>
#include <cstdint>
#include <utility>
#include <string>
#include <vector>

namespace mobius::extension::app::utorrent
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief resume.dat file decoder
// @author Eduardo Aguiar
// @see
// https://robertpearsonblog.wordpress.com/2016/11/10/utorrent-forensic-artifacts/
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_resume_dat
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief File structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct file
    {
        std::string name;
        std::string path;
        std::uint64_t length = 0;
        std::uint64_t offset = 0;
        std::uint64_t piece_length = 0;
        std::uint64_t piece_offset = 0;
        mobius::core::datetime::datetime creation_time;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Peer structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct peer
    {
        std::string ip;
        std::uint16_t port = 0;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Entry structure
    // @see docs/manual.rst@libtorrent source code
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct entry
    {
        // @brief Entry name
        std::string name;

        // @brief Torrent name
        std::string torrent_name;

        // @brief Active time in seconds
        std::uint64_t active_time = 0;

        // @brief Added time
        mobius::core::datetime::datetime added_timestamp;

        // @brief Allocation
        std::string allocation;

        // @brief Block size
        std::uint64_t blocksize = 0;

        // @brief Bytes downloaded
        std::uint64_t bytes_downloaded = 0;

        // @brief Bytes uploaded
        std::uint64_t bytes_uploaded = 0;

        // @brief Caption
        std::string caption;

        // @brief Completed time
        mobius::core::datetime::datetime completed_timestamp;

        // @brief Download URL
        std::string download_url;

        // @brief Downloaded time in seconds
        std::uint64_t downloaded_time = 0;

        // @brief Episode number
        std::uint64_t episode_number = 0;

        // @brief File format
        std::string file_format;

        // @brief File version
        std::uint64_t file_version = 0;

        // @brief Finished time in seconds
        std::uint64_t finished_time = 0;

        // @brief Info hash
        std::string info_hash;

        // @brief Info hash v2
        std::string info_hash_v2;

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

        // @brief Last seen complete timestamp
        mobius::core::datetime::datetime last_seen_complete_timestamp;

        // @brief Last upload timestamp
        mobius::core::datetime::datetime last_upload_timestamp;

        // @brief Metadata timestamp
        mobius::core::datetime::datetime metadata_timestamp;

        // @brief Path
        std::string path;

        // @brief Published on
        mobius::core::datetime::datetime published_on;

        // @brief Root dir
        std::string root_dir;

        // @brief Season number
        std::uint64_t season_number = 0;

        // @brief Seeded time in seconds
        std::uint64_t seeded_time = 0;

        // @brief Tracker mode
        std::uint64_t tracker_mode = 0;

        // @brief Metadata
        mobius::core::pod::map metadata;

        // @brief Peers
        std::vector<peer> peers;

        // @brief Content files
        std::vector<file> content_files;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_resume_dat (const mobius::core::io::reader &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if stream is an instance of resume.dat file
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const noexcept
    {
        return is_instance_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get entries
    // @return Vector of entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<entry>
    get_entries () const
    {
        return entries_;
    }

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

    // @brief Timestamp
    std::vector<entry> entries_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _decode_sqlite (const mobius::core::io::reader &);
    void _decode_btencode (const mobius::core::io::reader &);
    void _add_entry (const std::string &, const mobius::core::pod::map &);
};

} // namespace mobius::extension::app::utorrent

#endif
