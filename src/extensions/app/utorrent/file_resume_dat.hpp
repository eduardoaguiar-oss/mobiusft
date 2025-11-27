#ifndef MOBIUS_EXTENSION_APP_UTORRENT_FILE_RESUME_DAT_HPP
#define MOBIUS_EXTENSION_APP_UTORRENT_FILE_RESUME_DAT_HPP

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
    // @brief Entry structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct entry
    {
        // @brief Entry name
        std::string name;

        // @brief Torrent name
        std::string torrent_name;

        // @brief Active seconds
        std::uint64_t active_seconds = 0;

        // @brief Added time
        mobius::core::datetime::datetime added_time;

        // @brief Allocation
        std::string allocation;

        // @brief Auto managed
        bool auto_managed = false;

        // @brief Block size
        std::uint64_t blocksize = 0;

        // @brief Bytes downloaded
        std::uint64_t bytes_downloaded = 0;

        // @brief Bytes uploaded
        std::uint64_t bytes_uploaded = 0;

        // @brief Caption
        std::string caption;

        // @brief Completed time
        mobius::core::datetime::datetime completed_time;

        // @brief Download URL
        std::string download_url;

        // @brief Downloaded seconds
        std::uint64_t downloaded_seconds = 0;

        // @brief File format
        std::string file_format;

        // @brief File version
        std::uint64_t file_version = 0;

        // @brief Finished seconds
        std::uint64_t finished_seconds = 0;

        // @brief Last seen complete time
        mobius::core::datetime::datetime last_seen_complete_time;

        // @brief Metadata time
        mobius::core::datetime::datetime metadata_time;

        // @brief Path
        std::string path;

        // @brief Paused
        bool paused = false;

        // @brief Seeded seconds
        std::uint64_t seeded_seconds = 0;

        // @brief Sequential download
        bool sequential_download = false;

        // @brief Super seeding
        bool super_seeding = false;

        // @brief Metadata
        mobius::core::pod::map metadata;

        // @brief Peers
        std::vector<std::pair<std::string, std::uint16_t>> peers;
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
