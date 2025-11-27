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
#include "file_resume_dat.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/core/decoder/btencode.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/io/tempfile.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/pod/map.hpp>
#include <mobius/core/string_functions.hpp>

#include <iostream>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief SQLite file header
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
const mobius::core::bytearray SQLITE_HEADER = {'S', 'Q', 'L', 'i',   't', 'e',
                                               ' ', 'f', 'o', 'r',   'm', 'a',
                                               't', ' ', '3', '\x00'};

} // namespace

namespace mobius::extension::app::utorrent
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_resume_dat::file_resume_dat (const mobius::core::io::reader &reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (!reader)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Check if resume.dat file is a SQLite file or a BTEncode data file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto header_reader = reader;
    auto header = header_reader.read (16);
    header_reader.rewind ();

    if (header == SQLITE_HEADER)
        _decode_sqlite (reader);

    else
        _decode_btencode (reader);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode SQLite resume.dat file
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_resume_dat::_decode_sqlite (const mobius::core::io::reader &reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Copy reader content to temporary file
        mobius::core::io::tempfile tfile;
        tfile.copy_from (reader);

        // Get schema version
        mobius::core::database::database db (tfile.get_path ());

        // Prepare SQL statement for table TORRENTS
        auto stmt = db.new_statement (
            "SELECT infohash,"
            "resume,"
            "save_path "
            "FROM TORRENTS"
        );

        // Retrieve records from TORRENTS table
        while (stmt.fetch_row ())
        {
            auto infohash = stmt.get_column_string (0);
            auto resume_data =
                mobius::core::decoder::btencode (stmt.get_column_bytearray (1));
            auto save_path = stmt.get_column_string (2);

            if (resume_data.is_map ())
                _add_entry (infohash + ".torrent", resume_data.to_map ());
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }

    // End decoding
    is_instance_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode BTEncode resume.dat file
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_resume_dat::_decode_btencode (const mobius::core::io::reader &reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    auto data = mobius::core::decoder::btencode (reader);
    if (!data.is_map ())
        return;

    auto metadata = mobius::core::pod::map (data);

    for (const auto &[key, value] : metadata)
    {
        try
        {
            if (mobius::core::string::endswith (key, ".torrent") &&
                value.is_map ())
                _add_entry (key, value.to_map ());
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, e.what ());
        }
    }

    // End decoding
    is_instance_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode entry
// @param key Key
// @param c_metadata Metadata map
// @return Entry
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_resume_dat::_add_entry (
    const std::string &key, const mobius::core::pod::map &c_metadata
)
{
    mobius::core::pod::map metadata = c_metadata.clone ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get data from metadata dictionary
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto entry = mobius::extension::app::utorrent::file_resume_dat::entry ();

    entry.name = mobius::core::string::replace (key, ".torrent", "");
    entry.torrent_name = key;

    entry.active_seconds = metadata.pop<std::int64_t> ("active_time", 0);
    entry.added_time =
        mobius::core::datetime::new_datetime_from_unix_timestamp (
            metadata.pop<std::int64_t> ("added_time", 0)
        );
    entry.allocation = metadata.pop<std::string> ("allocation");
    entry.auto_managed = metadata.pop<std::int64_t> ("auto_managed", 0) == 1;
    entry.blocksize = metadata.pop<std::int64_t> ("blocksize", 0);
    entry.bytes_downloaded = metadata.pop<std::int64_t> ("downloaded", 0) +
                             metadata.pop<std::int64_t> ("total_downloaded", 0);
    entry.bytes_uploaded = metadata.pop<std::int64_t> ("uploaded", 0) +
                           metadata.pop<std::int64_t> ("total_uploaded", 0);
    entry.caption = metadata.pop<std::string> ("caption", {});
    entry.completed_time =
        mobius::core::datetime::new_datetime_from_unix_timestamp (
            metadata.pop<std::int64_t> ("completed_time", 0)
        );
    entry.download_url = metadata.pop<std::string> ("download_url");
    entry.downloaded_seconds =
        metadata.pop<std::int64_t> ("runtime", 0) - entry.seeded_seconds;
    entry.file_format = metadata.pop<std::string> ("file-format");
    entry.file_version = metadata.pop<std::int64_t> ("file-version", 0);
    entry.finished_seconds = metadata.pop<std::int64_t> ("finished_time", 0);
    entry.last_seen_complete_time =
        mobius::core::datetime::new_datetime_from_unix_timestamp (
            metadata.pop<std::int64_t> ("last_seen_complete", 0)
        );
    entry.metadata_time =
        mobius::core::datetime::new_datetime_from_unix_timestamp (
            metadata.pop<std::int64_t> ("time", 0)
        );
    entry.path = metadata.pop<std::string> ("path");
    if (entry.path.empty ())
        entry.path = metadata.pop<std::string> ("save_path");
    entry.paused = metadata.pop<std::int64_t> ("paused", 0) == 1;
    entry.seeded_seconds = metadata.pop<std::int64_t> ("seedtime", 0) +
                           metadata.pop<std::int64_t> ("seeding_time", 0);
    entry.sequential_download =
        metadata.pop<std::int64_t> ("sequential_download", 0) == 1;
    entry.super_seeding = metadata.pop<std::int64_t> ("super_seeding", 0) == 1;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get peers
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto peers_data = metadata.pop<mobius::core::bytearray> ("peers");

    if (peers_data)
    {
        auto decoder = mobius::core::decoder::data_decoder (peers_data);

        while (decoder)
        {
            auto ip = decoder.get_ipv4_be ();
            auto port = decoder.get_uint16_le ();
            entry.peers.emplace_back (ip, port);
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get peers6 (IPv6)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto peers6_data = metadata.pop<mobius::core::bytearray> ("peers6");

    if (peers6_data)
    {
        auto decoder = mobius::core::decoder::data_decoder (peers6_data);

        while (decoder)
        {
            auto ip = decoder.get_ipv4_mapped_ipv6 ();
            auto port = decoder.get_uint16_le ();
            entry.peers.emplace_back (ip, port);
        }
    }

    entry.metadata = metadata;

    entries_.push_back (entry);
}

} // namespace mobius::extension::app::utorrent
