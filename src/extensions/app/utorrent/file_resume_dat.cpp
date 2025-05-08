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
#include "file_resume_dat.hpp"
#include <mobius/core/decoder/btencode.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/pod/map.hpp>
#include <mobius/core/string_functions.hpp>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode entry
// @param key Key
// @param c_metadata Metadata map
// @return Entry
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::extension::app::utorrent::file_resume_dat::entry
_decode_entry (const std::string& key, const mobius::core::pod::map& c_metadata)
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Create entry object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::pod::map metadata = c_metadata.clone();
    auto entry = mobius::extension::app::utorrent::file_resume_dat::entry();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get data from metadata dictionary
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    entry.name = mobius::core::string::replace(key, ".torrent", "");
    entry.torrent_name = key;
    entry.download_url = metadata.pop<std::string>("download_url");
    entry.caption = metadata.pop<std::string>("caption");
    entry.path = metadata.pop<std::string>("path");
    entry.seeded_seconds = metadata.pop<std::int64_t>("seedtime", 0);
    entry.downloaded_seconds = metadata.pop<std::int64_t>("runtime", 0) - entry.seeded_seconds;
    entry.blocksize = metadata.pop<std::int64_t>("blocksize", 0);
    entry.bytes_downloaded = metadata.pop<std::int64_t>("downloaded", 0);
    entry.bytes_uploaded = metadata.pop<std::int64_t>("uploaded", 0);
    entry.metadata_time = mobius::core::datetime::new_datetime_from_unix_timestamp(metadata.pop<std::int64_t>("time", 0));
    entry.added_time = mobius::core::datetime::new_datetime_from_unix_timestamp(metadata.pop<std::int64_t>("added_on", 0));
    entry.completed_time = mobius::core::datetime::new_datetime_from_unix_timestamp(metadata.pop<std::int64_t>("completed_on", 0));
    entry.last_seen_complete_time = mobius::core::datetime::new_datetime_from_unix_timestamp(metadata.pop<std::int64_t>("last seen complete", 0));
    
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get peers
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto peers_data = metadata.pop<mobius::core::bytearray>("peers");

    if (peers_data)
    {
        auto decoder = mobius::core::decoder::data_decoder(peers_data);

        while (decoder)
        {
            auto ip = decoder.get_ipv4_be();
            auto port = decoder.get_uint16_le();
            entry.peers.emplace_back(ip, port);
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get peers6 (IPv6)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto peers6_data = metadata.pop<mobius::core::bytearray>("peers6");

    if (peers6_data)
    {
        auto decoder = mobius::core::decoder::data_decoder(peers6_data);

        while (decoder)
        {
            auto ip = decoder.get_ipv4_mapped_ipv6();
            auto port = decoder.get_uint16_le();
            entry.peers.emplace_back(ip, port);
        }
    }

    entry.metadata = metadata;
    return entry;
}

} // namespace

namespace mobius::extension::app::utorrent
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_resume_dat::file_resume_dat (const mobius::core::io::reader& reader)
{
    mobius::core::log log(__FILE__, __FUNCTION__);

    if (!reader)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode btencode data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto data = mobius::core::decoder::btencode(reader);
    if (!data.is_map())
        return;

    auto metadata = mobius::core::pod::map(data);

    for (const auto& [key, value] : metadata)
    {
        try
        {
            if (mobius::core::string::endswith(key, ".torrent") && value.is_map())
                entries_.push_back(_decode_entry(key, mobius::core::pod::map(value)));
        }
        catch(const std::exception& e)
        {
            log.warning(__LINE__, e.what());
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // End decoding
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    is_instance_ = true;
}

} // namespace mobius::extension::app::utorrent
