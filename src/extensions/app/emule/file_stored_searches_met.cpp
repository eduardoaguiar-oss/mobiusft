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
#include "file_stored_searches_met.hpp"
#include <mobius/core/log.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/string_functions.h>
#include <unordered_map>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
constexpr std::uint8_t MET_HEADER = 0x0e;
constexpr std::uint8_t MET_HEADER_I64TAGS = 0x0f;
constexpr std::uint8_t MET_LAST_VERSION = 1;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode string
// @param decoder Data decoder object
// @return String
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
read_string (mobius::core::decoder::data_decoder& decoder)
{
    std::string s;

    auto l = decoder.get_uint16_le ();
    if (l > 0)
        s = decoder.get_string_by_size (l);

    return s;
}

} // namespace

namespace mobius::extension::app::emule
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_stored_searches_met::file_stored_searches_met (const mobius::io::reader& reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (!reader || reader.get_size () < 4)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode header
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto decoder = mobius::core::decoder::data_decoder (reader);
    auto header = decoder.get_uint8 ();

    if (header != MET_HEADER_I64TAGS)
        return;

    version_ = decoder.get_uint8 ();

    if (version_ > MET_LAST_VERSION)
        log.development (__LINE__, "Unhandled version: " + std::to_string (version_));

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    int count = decoder.get_uint16_le ();

    for (int i = 0;i < count;i++)
        searches_.push_back (_decode_ssearch_params (decoder));

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // End decoding
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    is_instance_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode SSearchParams structure
// @param decoder Data decoder object
// @see SSearchParams::SSearchParams - srchybrid/SearchParams.h
// @return New search struct
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_stored_searches_met::search
file_stored_searches_met::_decode_ssearch_params (mobius::core::decoder::data_decoder& decoder)
{
    search s;

    // decoder data
    s.id = mobius::string::to_hex (decoder.get_uint32_le (), 8);
    s.e_type = decoder.get_uint8 ();
    s.b_client_search_files = decoder.get_uint8 () > 0;
    s.special_title = read_string (decoder);
    s.expression = read_string (decoder);
    s.filetype = read_string (decoder);

    // decode files
    auto file_count = decoder.get_uint32_le ();

    for (std::uint32_t i = 0;i < file_count;i++)
        s.files.push_back (_decode_csearch_file (decoder));

    // return struct
    return s;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode data from CSearchFile structure
// @see CSearchFile::CSearchFile - srchybrid/CSearchFile.cpp
// @return New CSearchFile structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_stored_searches_met::CSearchFile
file_stored_searches_met::_decode_csearch_file (mobius::core::decoder::data_decoder& decoder)
{
    CSearchFile f;

    // decode data
    f.hash_ed2k = decoder.get_hex_string_by_size (16);
    f.user_ip = decoder.get_ipv4_le ();
    f.user_port = decoder.get_uint16_le ();

    // decoder tags
    auto tag_count = decoder.get_uint32_le ();

    for (std::uint32_t i = 0;i < tag_count;i++)
        f.tags.emplace_back (decoder);

    return f;
}

} // namespace mobius::extension::app::emule
