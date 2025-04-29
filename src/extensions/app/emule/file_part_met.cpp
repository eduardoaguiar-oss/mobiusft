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
#include "file_part_met.hpp"
#include <mobius/core/log.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <algorithm>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
constexpr std::uint8_t PARTFILE_VERSION = 0xe0;
constexpr std::uint8_t PARTFILE_SPLITTEDVERSION = 0xe1;
constexpr std::uint8_t PARTFILE_VERSION_LARGEFILE = 0xe2;
  
} // namespace

namespace mobius::extension::app::emule
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_part_met::file_part_met (const mobius::io::reader& reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (!reader || reader.get_size () < 16)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode header
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto decoder = mobius::core::decoder::data_decoder (reader);
    version_ = decoder.get_uint8 ();

    if (version_ != PARTFILE_VERSION &&
        version_ != PARTFILE_SPLITTEDVERSION &&
        version_ != PARTFILE_VERSION_LARGEFILE)
        return;

    bool is_new_style = (version_ == PARTFILE_SPLITTEDVERSION);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Check eDonkey "old part style"
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (!is_new_style)
      {
        decoder.seek (24);
        auto value = decoder.get_uint32_le ();
        decoder.seek (1);

        if (value == 0x01020000)
          is_new_style = true;
      }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (is_new_style)
      {
        auto temp = decoder.get_uint32_le ();

        if (!temp)
            _decode_md4_hashset (decoder);

        else
          {
            decoder.seek (2);
            timestamp_ = decoder.get_unix_datetime ();
            hash_ed2k_ = decoder.get_hex_string_by_size (16);
          }
      }

    else
      {
        timestamp_ = decoder.get_unix_datetime ();
        _decode_md4_hashset (decoder);
      }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode tags
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t gap_start = 0;
    std::uint64_t gap_end = 0;
    auto tag_count = decoder.get_uint32_le ();

    for (std::uint32_t i = 0;i < tag_count; i++)
      {
        ctag tag (decoder);

        if (tag.get_id () == 0)
          {
            auto name = tag.get_name ();

            if (!name.empty () && name[0] == 0x09)              // FT_GAPSTART
                gap_start = tag.get_value <std::int64_t> ();
              
            else if (!name.empty () && name[0] == 0x0a)         // FT_GAPEND
              {
                gap_end = tag.get_value <std::int64_t> ();
                gaps_.emplace_back (gap_start, gap_end);
                total_gap_size_ += (gap_end - gap_start);
              }
          }

        else
            tags_.push_back (tag);
      }
      
    std::sort (gaps_.begin (), gaps_.end ());

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // End decoding
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    is_instance_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode MD4 hash set
// @param decoder Decoder object
//
// For now, ignore hash set and return only ED2K hash
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_part_met::_decode_md4_hashset (mobius::core::decoder::data_decoder& decoder)
{
    hash_ed2k_ = decoder.get_hex_string_by_size (16);
    std::uint32_t count = decoder.get_uint16_le ();

    for (std::uint32_t i = 0;i < count;i++)
        chunk_hashes_.push_back (decoder.get_hex_string_by_size (16));
}

} // namespace mobius::extension::app::emule
