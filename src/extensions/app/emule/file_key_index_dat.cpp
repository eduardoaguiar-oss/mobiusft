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
#include "file_key_index_dat.hpp"
#include <mobius/core/log.hpp>
#include <mobius/decoder/data_decoder.h>
#include <mobius/string_functions.h>
#include <unordered_map>

namespace mobius::extension::app::emule
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode string
// @param decoder Data decoder object
// @return String
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
_decode_string (mobius::decoder::data_decoder& decoder)
{
    std::string s;

    auto l = decoder.get_uint16_le ();
    if (l > 0)
        s = decoder.get_string_by_size (l);

    return s;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode KAD ID
// @param decoder Data decoder object
// @return KAD ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
_decode_kad_id (mobius::decoder::data_decoder& decoder)
{
    auto kad_id = mobius::string::to_hex (decoder.get_uint32_le (), 8);
    kad_id += mobius::string::to_hex (decoder.get_uint32_le (), 8);
    kad_id += mobius::string::to_hex (decoder.get_uint32_le (), 8);
    kad_id += mobius::string::to_hex (decoder.get_uint32_le (), 8);

    return kad_id;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode name structure
// @param decoder Data decoder object
// @param version File version
// @return Name struct
// @see CKeyEntry::ReadPublishTrackingDataFromFile (srchybrid/kademlia/kademlia/Entry.cpp)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static file_key_index_dat::name
_decode_name (mobius::decoder::data_decoder& decoder, std::uint32_t version)
{
    auto name = file_key_index_dat::name ();
    name.lifetime = decoder.get_unix_datetime ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // AICH hashes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (version >= 4)
      {
        auto aich_hash_count = decoder.get_uint16_le ();

        for (std::uint32_t i = 0;i < aich_hash_count;i++)
            name.aich_hashes.push_back (decoder.get_hex_string_by_size (20));
      }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // File names + IPs
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (version >= 3)
      {
        auto name_count = decoder.get_uint32_le ();

        for (std::uint32_t i = 0;i < name_count;i++)
          {
            auto filename = _decode_string (decoder);
            auto popularity_index = decoder.get_uint32_le ();
            name.filenames.emplace_back (filename, popularity_index);
          }

        auto ip_count = decoder.get_uint32_le ();

        for (std::uint32_t i = 0;i < ip_count;i++)
          {
            auto ip = file_key_index_dat::ip ();
            ip.value = decoder.get_ipv4_le ();
            ip.last_published = decoder.get_unix_datetime ();

            if (version >= 4)
                ip.aich_hash_idx = decoder.get_uint16_le ();

            name.ips.push_back (ip);
          }
      }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Tags
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto tag_count = decoder.get_uint8 ();
    for (std::uint32_t i = 0;i < tag_count;i++)
        name.tags.emplace_back (decoder);

    return name;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode source structure
// @param decoder Data decoder object
// @param version File version
// @return Source struct
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static file_key_index_dat::source
_decode_source (mobius::decoder::data_decoder& decoder, std::uint32_t version)
{
    auto source = file_key_index_dat::source ();
    source.id = _decode_kad_id (decoder);      // ED2K HASH !!!!!!

    auto count = decoder.get_uint32_le ();
    for (std::uint32_t i = 0;i < count;i++)
        source.names.push_back (_decode_name (decoder, version));

    return source;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode key structure
// @param decoder Data decoder object
// @param version File version
// @return Key struct
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static file_key_index_dat::key
_decode_key (mobius::decoder::data_decoder& decoder, std::uint32_t version)
{
    auto k = file_key_index_dat::key ();
    k.id = _decode_kad_id (decoder);

    auto count = decoder.get_uint32_le ();
    for (std::uint32_t i = 0;i < count;i++)
        k.sources.push_back (_decode_source (decoder, version));

    return k;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @see CIndexed::CLoadDataThread::Run (srchybrid/kademlia/kademlia/Indexed.cpp)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_key_index_dat::file_key_index_dat (const mobius::io::reader& reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (!reader || reader.get_size () < 28)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto decoder = mobius::decoder::data_decoder (reader);
    version_ = decoder.get_uint32_le ();
    
    if (version_ < 5)
      {
        save_time_ = decoder.get_unix_datetime ();
        client_id_ = _decode_kad_id (decoder);

        auto count = decoder.get_uint32_le ();

        for (std::uint32_t i = 0;i < count; i++)
            keys_.push_back (_decode_key (decoder, version_));
      }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // End decoding
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    is_instance_ = true;
}

} // namespace mobius::extension::app::emule
