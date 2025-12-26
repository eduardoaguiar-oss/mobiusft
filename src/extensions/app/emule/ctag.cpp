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
#include "ctag.hpp"
#include <iomanip>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <sstream>
#include <unordered_map>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Tag types
// @see srchybrid/opcodes.h
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
constexpr std::uint8_t TAGTYPE_HASH = 0x01;
constexpr std::uint8_t TAGTYPE_STRING = 0x02;
constexpr std::uint8_t TAGTYPE_UINT32 = 0x03;
constexpr std::uint8_t TAGTYPE_FLOAT32 = 0x04;
constexpr std::uint8_t TAGTYPE_BOOL = 0x05;
constexpr std::uint8_t TAGTYPE_BOOLARRAY = 0x06;
constexpr std::uint8_t TAGTYPE_BLOB = 0x07;
constexpr std::uint8_t TAGTYPE_UINT16 = 0x08;
constexpr std::uint8_t TAGTYPE_UINT8 = 0x09;
constexpr std::uint8_t TAGTYPE_BSOB = 0x0A;
constexpr std::uint8_t TAGTYPE_UINT64 = 0x0B;
constexpr std::uint8_t TAGTYPE_STR1 = 0x11;
constexpr std::uint8_t TAGTYPE_STR2 = 0x12;
constexpr std::uint8_t TAGTYPE_STR3 = 0x13;
constexpr std::uint8_t TAGTYPE_STR4 = 0x14;
constexpr std::uint8_t TAGTYPE_STR5 = 0x15;
constexpr std::uint8_t TAGTYPE_STR6 = 0x16;
constexpr std::uint8_t TAGTYPE_STR7 = 0x17;
constexpr std::uint8_t TAGTYPE_STR8 = 0x18;
constexpr std::uint8_t TAGTYPE_STR9 = 0x19;
constexpr std::uint8_t TAGTYPE_STR10 = 0x1A;
constexpr std::uint8_t TAGTYPE_STR11 = 0x1B;
constexpr std::uint8_t TAGTYPE_STR12 = 0x1C;
constexpr std::uint8_t TAGTYPE_STR13 = 0x1D;
constexpr std::uint8_t TAGTYPE_STR14 = 0x1E;
constexpr std::uint8_t TAGTYPE_STR15 = 0x1F;
constexpr std::uint8_t TAGTYPE_STR16 = 0x20;
constexpr std::uint8_t TAGTYPE_STR17 = 0x21;
constexpr std::uint8_t TAGTYPE_STR18 = 0x22;
constexpr std::uint8_t TAGTYPE_STR19 = 0x23;
constexpr std::uint8_t TAGTYPE_STR20 = 0x24;
constexpr std::uint8_t TAGTYPE_STR21 = 0x25;
constexpr std::uint8_t TAGTYPE_STR22 = 0x26;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Tag metadata names
// @see CKnownFile::LoadTagsFromFile - srchybrid/KnownFile.cpp
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::unordered_map<std::uint8_t, std::string> TAG_METADATA_NAMES = {
    {0x01, "name"},
    {0x02, "size"},
    {0x03, "filetype"},
    {0x04, "fileformat"},
    {0x05, "last_seen_time"},
    {0x06, "part_path"},
    {0x07, "part_hash"},
    {0x08, "downloaded_bytes"},
    {0x0b, "description"},
    {0x11, "version"},
    {0x12, "part_name"},
    {0x13, "priority"},
    {0x14, "status"},
    {0x15, "sources"},
    {0x18, "dl_priority"},
    {0x19, "ul_priority"},
    {0x1a, "compression_gain"},
    {0x1b, "corrupted_loss"},
    {0x21, "last_kad_published_time"},
    {0x22, "flags"},
    {0x23, "download_active_time"},
    {0x27, "hash_aich"},
    {0x30, "complete_sources"},
    {0x34, "last_shared_time"},
    {0x51, "times_requested"},
    {0x52, "times_accepted"},
    {0x53, "category"},
    {0x55, "max_sources"},
    {0x92, "last_update_time"},
    {0xd0, "media_artist"},
    {0xd1, "media_album"},
    {0xd2, "media_title"},
    {0xd3, "media_length"},
    {0xd4, "media_bitrate"},
    {0xd5, "media_codec"},
    {0xf6, "file_comment"},
    {0xf7, "file_rating"},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get UINT32 tag value, based on Tag ID
// @param id Tag ID
// @param value Tag UINT32 value
// @return Get value formatted
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
_get_tag_uint32_value (std::uint8_t id, std::uint32_t value)
{
    switch (id)
    {
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Datetime
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    case 0x05:
    case 0x21:
    case 0x34:
    case 0x92:
        return mobius::core::datetime::new_datetime_from_unix_timestamp (value);
        break;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Duration (seconds)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    case 0x23:
    case 0xd3:
    {
        unsigned int hours = value / 3600;
        unsigned int minutes = (value % 3600) / 60;
        unsigned int seconds = value % 60;

        std::ostringstream oss;
        oss << std::setw (2) << std::setfill ('0') << hours << ":"
            << std::setw (2) << std::setfill ('0') << minutes << ":"
            << std::setw (2) << std::setfill ('0') << seconds;

        return oss.str ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Others
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    default:
        return static_cast<std::int64_t> (value);
    }
}

} // namespace

namespace mobius::extension::app::emule
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param decoder Decoder object
// @see CTag::CTag (srchybrid/packets.cpp)
// @see ConvertED2KTag (srchybrid/SearchFile.cpp)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ctag::ctag (mobius::core::decoder::data_decoder &decoder)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Read tag id and name
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    type_ = decoder.get_uint8 ();

    if (type_ & 0x80)
    {
        type_ &= 0x7f;
        id_ = decoder.get_uint8 ();
    }

    else
    {
        auto length = decoder.get_uint16_le ();

        if (length == 1)
            id_ = decoder.get_uint8 ();

        else
            name_ = decoder.get_string_by_size (length);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Read tag value
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    switch (type_)
    {
    case TAGTYPE_HASH:
        value_ = decoder.get_hex_string_by_size (16);
        break;

    case TAGTYPE_STRING:
    {
        auto length = decoder.get_uint16_le ();
        value_ = decoder.get_string_by_size (length, "utf-8");
        break;
    }

    case TAGTYPE_UINT32:
        value_ = _get_tag_uint32_value (id_, decoder.get_uint32_le ());
        break;

    case TAGTYPE_FLOAT32:
        log.development (__LINE__, "TAGTYPE_FLOAT32 not implemented");
        break;

    case TAGTYPE_BOOL:
        value_ = decoder.get_uint8 () == 1;
        break;

    case TAGTYPE_BOOLARRAY:
    {
        auto length = decoder.get_uint16_le ();
        decoder.skip (length / 8 + 1);
        // @todo 07-Apr-2004: eMule versions prior to 0.42e.29 used the formula
        // "(len+7)/8"!
        break;
    }

    case TAGTYPE_BLOB:
    {
        // @todo 07-Apr-2004: eMule versions prior to 0.42e.29 handled the "len"
        // as int16!
        auto size = decoder.get_uint32_le ();
        value_ = decoder.get_bytearray_by_size (size);
        break;
    }

    case TAGTYPE_UINT16:
        value_ = decoder.get_uint16_le ();
        break;

    case TAGTYPE_UINT8:
        value_ = decoder.get_uint8 ();
        break;

    case TAGTYPE_BSOB:
        // @todo implement
        log.development (__LINE__, "TAGTYPE_BSOB not implemented");
        break;

    case TAGTYPE_UINT64:
        value_ = static_cast<std::int64_t> (decoder.get_uint64_le ());
        break;

    case TAGTYPE_STR1:
    case TAGTYPE_STR2:
    case TAGTYPE_STR3:
    case TAGTYPE_STR4:
    case TAGTYPE_STR5:
    case TAGTYPE_STR6:
    case TAGTYPE_STR7:
    case TAGTYPE_STR8:
    case TAGTYPE_STR9:
    case TAGTYPE_STR10:
    case TAGTYPE_STR11:
    case TAGTYPE_STR12:
    case TAGTYPE_STR13:
    case TAGTYPE_STR14:
    case TAGTYPE_STR15:
    case TAGTYPE_STR16:
    case TAGTYPE_STR17:
    case TAGTYPE_STR18:
    case TAGTYPE_STR19:
    case TAGTYPE_STR20:
    case TAGTYPE_STR21:
    case TAGTYPE_STR22:
    {
        std::uint32_t length = type_ - TAGTYPE_STR1 + 1;
        value_ = decoder.get_string_by_size (length);
        type_ = TAGTYPE_STRING;
        break;
    }

    default:
        log.development (__LINE__, "Unknown tag type: 0x" +
                                       mobius::core::string::to_hex (type_, 2));
    };
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Generate a metadata map from a ctag list
// @param ctags CTag list
// @return Metadata map
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::map
get_metadata_from_tags (const std::vector<ctag> &ctags)
{
    mobius::core::pod::map metadata;
    mobius::core::log log (__FILE__, __FUNCTION__);

    std::uint64_t uploaded_bytes = 0;
    std::uint64_t not_counted_uploaded_bytes = 0;
    std::uint64_t total_gap_size = 0;
    std::uint64_t gap_start = 0;

    for (const auto &tag : ctags)
    {
        auto id = tag.get_id ();

        // Common IDs
        auto iter = TAG_METADATA_NAMES.find (id);

        if (iter != TAG_METADATA_NAMES.end ())
            metadata.set (iter->second, tag.get_value ());

        // Gap start, Gap end
        else if (id == 0)
        {
            auto tag_name = tag.get_name ();

            if (!tag_name.empty ())
            {
                if (tag_name[0] == 0x09) // FT_GAPSTART
                    gap_start = tag.get_value<std::int64_t> ();

                else if (tag_name[0] == 0x0a) // FT_GAPEND
                    total_gap_size +=
                        (tag.get_value<std::int64_t> () - gap_start);
            }
        }

        // Is corrupted
        else if (id == 0x24)
        {
            auto value = tag.get_value<std::string> ();
            metadata.set ("is_corrupted", !value.empty ());
        }

        // AICH Hashset
        else if (id == 0x35)
            ;

        // Uploaded bytes (low 32-bits)
        else if (id == 0x50)
            uploaded_bytes = (uploaded_bytes & 0xffffffff00000000) |
                             tag.get_value<std::int64_t> ();

        // Uploaded bytes (high 32-bits)
        else if (id == 0x54)
            uploaded_bytes = (uploaded_bytes & 0x00000000ffffffff) |
                             (tag.get_value<std::int64_t> () << 32);

        // Not counted uploaded bytes (low 32-bits)
        else if (id == 0x90)
            not_counted_uploaded_bytes =
                (not_counted_uploaded_bytes & 0xffffffff00000000) |
                tag.get_value<std::int64_t> ();

        // Not counted uploaded bytes (high 32-bits)
        else if (id == 0x91)
            not_counted_uploaded_bytes =
                (not_counted_uploaded_bytes & 0x00000000ffffffff) |
                (tag.get_value<std::int64_t> () << 32);

        // Unknown tag ID
        else
            log.development (__LINE__,
                             "Unhandled tag ID: " + std::to_string (id));
    }

    // Set remaining metadata
    metadata.set ("total_gap_size", total_gap_size);
    metadata.set ("uploaded_bytes", uploaded_bytes);
    metadata.set ("not_counted_uploaded_bytes", not_counted_uploaded_bytes);

    // Return metadata
    return metadata;
}

} // namespace mobius::extension::app::emule
