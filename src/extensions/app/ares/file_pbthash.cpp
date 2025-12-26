// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008-2026 Eduardo Aguiar
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
#include "file_pbthash.hpp"
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/log.hpp>

namespace mobius::extension::app::ares
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @see read_details_DB_Download@helper_download_disk.pas (line 722)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_pbthash::file_pbthash (const mobius::core::io::reader &reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Check reader
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (!reader || reader.get_size () < 54)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto decoder = mobius::core::decoder::data_decoder (reader);
    decoder.seek (0);

    section_ = mobius::core::file_decoder::section (reader, "File");

    _decode_header (decoder);

    if (version_ != 1)
        return;

    is_instance_ = true;

    _decode_pieces (decoder);
    _decode_metadata (decoder);
    _decode_files (decoder);
    _decode_tags (decoder);

    section_.end ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode header section
// @see BitTorrentDb_load@BitTorrent/BitTorrentDlDb.pas (line 69)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_pbthash::_decode_header (mobius::core::decoder::data_decoder &decoder)
{
    auto header_section = section_.new_child ("header");
    version_ = decoder.get_uint8 ();

    if (version_ != 1)
        return;

    hash_sha1_ = decoder.get_hex_string_by_size (20);

    // Decode state
    // @see BytetoBittorrentState@BitTorrent/BitTorrentUtils (line 76)
    auto state = decoder.get_uint8 ();
    is_paused_ = (state == 1);
    is_seeding_ = (state == 2);

    file_size_ = decoder.get_uint64_le ();
    piece_size_ = decoder.get_uint32_le ();
    bytes_downloaded_ = decoder.get_uint64_le ();
    bytes_uploaded_ = decoder.get_uint64_le ();

    header_section.end ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode pieces section
// @see BitTorrentDb_load - BitTorrent/BitTorrentDlDb.pas (line 163)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_pbthash::_decode_pieces (mobius::core::decoder::data_decoder &decoder)
{
    pieces_count_ = decoder.get_uint32_le ();
    auto pieces_section = section_.new_child ("pieces");

    for (std::uint32_t i = 1; i <= pieces_count_; i++)
    {
        piece p;
        p.idx = i;
        p.hash_sha1 = decoder.get_hex_string_by_size (20);
        p.is_checked = (decoder.get_uint8 () == 1);

        pieces_.push_back (p);
    }

    pieces_section.end ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode torrent name and path
// @param decoder Decoder object
// @see BitTorrentDb_load - BitTorrent/BitTorrentDlDb.pas (line 193)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_pbthash::_decode_metadata (mobius::core::decoder::data_decoder &decoder)
{
    auto metadata_section = section_.new_child ("metadata");

    files_count_ = decoder.get_uint32_le ();
    auto siz = decoder.get_uint16_le ();
    torrent_path_ = decoder.get_string_by_size (siz);

    auto pos = torrent_path_.rfind ('\\');

    if (pos != std::string::npos)
        torrent_name_ = torrent_path_.substr (pos + 1);
    else
        torrent_name_ = torrent_path_;

    metadata_section.end ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode files section
// @param decoder Decoder object
// @see BitTorrentDb_load - BitTorrent/BitTorrentDlDb.pas (line 211)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_pbthash::_decode_files (mobius::core::decoder::data_decoder &decoder)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    auto files_section = section_.new_child ("files");

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (std::uint32_t i = 1; i <= files_count_; i++)
    {
        file f;
        f.idx = i;
        f.size = decoder.get_uint64_le ();

        auto siz = decoder.get_uint16_le ();
        if (!siz)
        {
            f.last_modification_time = decoder.get_unix_datetime ();
            siz = decoder.get_uint16_le ();
        }

        f.name = decoder.get_string_by_size (siz);

        // @see BitTorrentDb_load - BitTorrent/BitTorrentDlDb.pas (line 241)
        if (files_count_ == 1)
            f.path = torrent_path_;

        else
            f.path = torrent_path_ + '\\' + f.name;

        files_.push_back (f);
    }

    files_section.end ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode tags section
// @param decoder Decoder object
// @see BitTorrentDb_load - BitTorrent/BitTorrentDlDb.pas (line 296)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_pbthash::_decode_tags (mobius::core::decoder::data_decoder &decoder)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    auto tags_section = section_.new_child ("tags");
    std::string tracker;

    while (decoder)
    {
        auto tag_id = decoder.get_uint8 ();
        auto tag_size = decoder.get_uint16_le ();

        switch (tag_id)
        {
        case 1:
            name_ = decoder.get_string_by_size (tag_size, "UTF-8");
            break;

        case 2:
            tracker = decoder.get_string_by_size (tag_size, "UTF-8");
            trackers_.push_back (tracker);
            break;

        case 3:
            comment_ = decoder.get_string_by_size (tag_size, "UTF-8");
            break;

        case 4:
            db_time_ = decoder.get_unix_datetime ();
            break;

        case 5:
            download_started_time_ = decoder.get_unix_datetime ();
            break;

        case 6:
            tracker = decoder.get_string_by_size (tag_size, "UTF-8");
            trackers_.push_back (tracker);
            break;

        case 7:
            elapsed_time_ = decoder.get_uint32_le ();
            break;

        default:
            log.development (__LINE__,
                             "unhandled tag ID: " + std::to_string (tag_id));
        }
    }

    tags_section.end ();
}

} // namespace mobius::extension::app::ares
