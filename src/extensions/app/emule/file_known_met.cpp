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
#include "file_known_met.hpp"
#include <mobius/core/log.hpp>
#include <mobius/decoder/data_decoder.h>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
constexpr std::uint8_t MET_HEADER = 0x0e;
constexpr std::uint8_t MET_HEADER_I64TAGS = 0x0f;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode CKnownFile structure
// @param decoder Data decoder object
// @see CKnownFile::LoadFromFile - srchybrid/KnownFile.cpp
// @return New CKnownFile struct
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::extension::app::emule::file_known_met::CKnownFile
_decode_cknownfile (mobius::decoder::data_decoder& decoder)
{
    mobius::extension::app::emule::file_known_met::CKnownFile f;

    // decode data
    f.last_modification_time = decoder.get_unix_datetime ();
    f.hash_ed2k = decoder.get_hex_string_by_size (16);

    // decode chunk hashes
    auto chunk_count = decoder.get_uint16_le ();

    for (std::uint32_t i = 0;i < chunk_count;i++)
        f.chunk_hashes.push_back(decoder.get_hex_string_by_size (16));

    // decoder tags
    auto tag_count = decoder.get_uint32_le ();

    for (std::uint32_t i = 0;i < tag_count;i++)
        f.tags.emplace_back (decoder);

    // return struct
    return f;
}

} // namespace

namespace mobius::extension::app::emule
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_known_met::file_known_met (const mobius::io::reader& reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (!reader || reader.get_size () < 4)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode header
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto decoder = mobius::decoder::data_decoder (reader);
    auto header = decoder.get_uint8 ();

    if (header != MET_HEADER_I64TAGS && header != MET_HEADER)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t count = decoder.get_uint32_le ();

    for (std::uint32_t i = 0;i < count;i++)
        known_files_.push_back (_decode_cknownfile (decoder));

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // End decoding
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    is_instance_ = true;
}

} // namespace mobius::extension::app::emule
