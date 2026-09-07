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
#include "file_msiecf.hpp"
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/string_functions.hpp>

#include <iostream>
#include <map>
#include <tuple>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief MSIE Cache File block size
static constexpr std::size_t BLOCK_SIZE = 128;

// @brief Hash PAD table
// @see https://www.geoffchappell.com/studies/windows/ie/wininet/api/urlcache/hashkey.htm
static constexpr std::uint8_t HASH_PAD [0x0100] = {
    0x01, 0x0e, 0x6e, 0x19, 0x61, 0xae, 0x84, 0x77, 0x8a, 0xaa, 0x7d, 0x76, 0x1b, 0xe9, 0x8c, 0x33,
    0x57, 0xc5, 0xb1, 0x6b, 0xea, 0xa9, 0x38, 0x44, 0x1e, 0x07, 0xad, 0x49, 0xbc, 0x28, 0x24, 0x41,
    0x31, 0xd5, 0x68, 0xbe, 0x39, 0xd3, 0x94, 0xdf, 0x30, 0x73, 0x0f, 0x02, 0x43, 0xba, 0xd2, 0x1c,
    0x0c, 0xb5, 0x67, 0x46, 0x16, 0x3a, 0x4b, 0x4e, 0xb7, 0xa7, 0xee, 0x9d, 0x7c, 0x93, 0xac, 0x90,
    0xb0, 0xa1, 0x8d, 0x56, 0x3c, 0x42, 0x80, 0x53, 0x9c, 0xf1, 0x4f, 0x2e, 0xa8, 0xc6, 0x29, 0xfe,
    0xb2, 0x55, 0xfd, 0xed, 0xfa, 0x9a, 0x85, 0x58, 0x23, 0xce, 0x5f, 0x74, 0xfc, 0xc0, 0x36, 0xdd,
    0x66, 0xda, 0xff, 0xf0, 0x52, 0x6a, 0x9e, 0xc9, 0x3d, 0x03, 0x59, 0x09, 0x2a, 0x9b, 0x9f, 0x5d,
    0xa6, 0x50, 0x32, 0x22, 0xaf, 0xc3, 0x64, 0x63, 0x1a, 0x96, 0x10, 0x91, 0x04, 0x21, 0x08, 0xbd,
    0x79, 0x40, 0x4d, 0x48, 0xd0, 0xf5, 0x82, 0x7a, 0x8f, 0x37, 0x69, 0x86, 0x1d, 0xa4, 0xb9, 0xc2,
    0xc1, 0xef, 0x65, 0xf2, 0x05, 0xab, 0x7e, 0x0b, 0x4a, 0x3b, 0x89, 0xe4, 0x6c, 0xbf, 0xe8, 0x8b,
    0x06, 0x18, 0x51, 0x14, 0x7f, 0x11, 0x5b, 0x5c, 0xfb, 0x97, 0xe1, 0xcf, 0x15, 0x62, 0x71, 0x70,
    0x54, 0xe2, 0x12, 0xd6, 0xc7, 0xbb, 0x0d, 0x20, 0x5e, 0xdc, 0xe0, 0xd4, 0xf7, 0xcc, 0xc4, 0x2b,
    0xf9, 0xec, 0x2d, 0xf4, 0x6f, 0xb6, 0x99, 0x88, 0x81, 0x5a, 0xd9, 0xca, 0x13, 0xa5, 0xe7, 0x47,
    0xe6, 0x8e, 0x60, 0xe3, 0x3e, 0xb3, 0xf6, 0x72, 0xa2, 0x35, 0xa0, 0xd7, 0xcd, 0xb4, 0x2f, 0x6d,
    0x2c, 0x26, 0x1f, 0x95, 0x87, 0x00, 0xd8, 0x34, 0x3f, 0x17, 0x25, 0x45, 0x27, 0x75, 0x92, 0xb8,
    0xa3, 0xc8, 0xde, 0xeb, 0xf8, 0xf3, 0xdb, 0x0a, 0x98, 0x83, 0x7b, 0xe5, 0xcb, 0x4c, 0x78, 0xd1
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode tags
// @param decoder data decoder
// @param tags_size size of the tags
// @return Map of tag ID to tag data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::map<std::uint8_t, mobius::core::pod::data>
_decode_tags (mobius::core::decoder::data_decoder &decoder, std::uint32_t tags_size)
{
    std::cerr << "Decoding tags. Size = " << tags_size << ", offset = 0x"
              << mobius::core::string::to_hex (decoder.tell (), 8) << std::endl;
    std::map<std::uint8_t, mobius::core::pod::data> tags;

    auto end_pos = decoder.tell () + tags_size;
    std::uint16_t tag_size = decoder.get_uint16_le ();

    while (decoder.tell () < end_pos && tag_size != 0)
    {
        auto tag_type = static_cast<int> (decoder.get_uint8 ());
        auto value_type = decoder.get_uint8 ();

        mobius::core::pod::data value;

        if (value_type == 0x01 || value_type == 0x1f)
        {
            value = decoder.get_string_by_size (tag_size - 8, "utf-16");
            auto v1 = decoder.get_uint32_le ();
        }

        else if (value_type == 0x03) // dword
        {
            value = static_cast<std::int64_t> (decoder.get_uint32_le ());
            auto v1 = decoder.get_uint32_le ();
        }

        else if (value_type == 0x40) // datetime
        {
            value = decoder.get_nt_datetime ();
            auto v1 = decoder.get_uint32_le ();
        }

        else if (value_type == 0x42) // strings array
        {
            value = decoder.get_bytearray_by_size (tag_size - 8);
            auto v1 = decoder.get_uint32_le ();
        }

        else if (tag_size >= 4)
        {
            auto value_data = decoder.get_bytearray_by_size (tag_size - 4);
            value = value_data;
        }

        tags[tag_type] = value;

        tag_size = decoder.get_uint16_le ();
    }

    return tags;
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Calculate hash value for a given URL
// @param url URL
// @return Hash value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::uint32_t
calculate_hash (const std::string& url)
{
    if (url.length () < 4)
        return 0;

    std::uint8_t h0 = HASH_PAD[url[0]];
    std::uint8_t h1 = HASH_PAD[url[1]];
    std::uint8_t h2 = HASH_PAD[url[2]];
    std::uint8_t h3 = HASH_PAD[url[3]];

    std::size_t size = url.length ();

    for (std::size_t i = 0; i < size; i++)
    {
        std::uint8_t c = url[i];

        if (c != '/' || i + 1 < size)
        {
            h0 = HASH_PAD[h0 ^ c];
            h1 = HASH_PAD[h1 ^ c];
            h2 = HASH_PAD[h2 ^ c];
            h3 = HASH_PAD[h3 ^ c];
        }
    }

    return static_cast<std::uint32_t>(h0) |
    (static_cast<std::uint32_t>(h1) << 8) |
    (static_cast<std::uint32_t>(h2) << 16) |
    (static_cast<std::uint32_t>(h3) << 24);
}

} // namespace

namespace mobius::extension::app::internet_explorer
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @see MSIE Cache File (index.dat) format specification v0.0.18. By Joachim Metz
// @see http://www.stevebunting.org/udpd4n6/forensics/index_dat2.htm
// @see https://www.geoffchappell.com/studies/windows/ie/wininet/api/urlcache/indexdat.htm
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_msiecf::file_msiecf (const mobius::core::io::reader &reader)
{
    if (!reader || reader.get_size () < 72)
        return;

    mobius::core::log log (__FILE__, __func__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode header signature
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto decoder = mobius::core::decoder::data_decoder (reader);
    auto signature = decoder.get_bytearray_by_size (24);

    if (signature != "Client UrlCache MMF Ver ")
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode header data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    signature_ = signature.to_string ();
    version_ = decoder.get_string_by_size (4);
    size_ = decoder.get_uint32_le ();
    hash_table_offset_ = decoder.get_uint32_le ();
    total_blocks_ = decoder.get_uint32_le ();
    allocated_blocks_ = decoder.get_uint32_le ();

    auto u1 = decoder.get_uint32_le ();
    if (u1)
        log.development (__LINE__, "msiecf (index.dat) u1: " + std::to_string (u1));

    cache_size_limit_ = decoder.get_uint64_le ();
    cache_size_ = decoder.get_uint64_le ();
    cache_size_non_releasable_ = decoder.get_uint64_le ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode directory table
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto size = decoder.get_uint32_le ();

    for (std::size_t i = 0; i < size; ++i)
    {
        auto dir_entry = cache_directory_entry ();
        dir_entry.file_count = decoder.get_uint32_le ();
        dir_entry.dirname = decoder.get_string_by_size (8);

        directories_.push_back (dir_entry);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode hash table
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t offset = hash_table_offset_;

    while (offset != 0)
    {
        decoder.seek (offset);
        offset = _decode_hash_table (decoder);
    }

    is_instance_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode hash table
// @param decoder Data decoder
// @return Next hash table offset
//
// Each HASH record has 4096 bytes and is organized as 64 buckets of 7 entries
// each, (448 addressable slots total; any remaining slots in the block are
// padding, seen filled with the 0xdeadbeef uninitialized sentinel).
//
// Invalid entries have the following offsets: 0x00000000, 0x00000003, 0x0badf00d, 0xdeadbeef
//
// Each entry is composed of a 32-bit hash and a 32-bit offset. Each hash value
// is split as:
//   - top 26 bits: The location hash value top 26 bits
//   - low  6 bits: The record type
//
// Because the low 6 bits of the location hash are therefore redundant on disk,
// WinINet reuses that space in the stored record.hash field for the 6-bit
// per-entry type, instead of storing the bucket bits directly:
//   record.hash = (location_hash & ~0x3F) | type
//
// To recover the original 32-bit hash value of an active entry:
//   hash_value = (record.hash & ~0x3F) | (entry_idx / 7)
//
// Types found:
// 0 - URL
// 1 - Deleted
// 2 - URL (locked)
// 3 - Free
// 4 - URL
// 5 - REDR
// 6 - URL
// 8 - URL
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
file_msiecf::_decode_hash_table (mobius::core::decoder::data_decoder &decoder)
{
    // Check if hash table signature is valid
    auto signature = decoder.get_bytearray_by_size (4);

    if (signature != "HASH")
        return 0;

    // Decode hash table data
    auto block_count = decoder.get_uint32_le ();
    auto next_offset = decoder.get_uint32_le ();
    auto idx = decoder.get_uint32_le ();

    // Decode entries
    auto count = (block_count * BLOCK_SIZE - 16) / 8;
    std::vector<hash_table_record> records;

    for (std::size_t i = 0; i < count; ++i)
    {
        auto record_hash = decoder.get_uint32_le ();
        auto record_offset = decoder.get_uint32_le ();
        std::uint8_t record_type = record_hash & 0x3f;
                
        if (record_offset != 0 && record_offset != 3 && record_offset != 0x0badf00d && record_offset != 0xdeadbeef)
        {
            auto record = hash_table_record ();

            record.hash = record_hash;
            record.offset = record_offset;
            record.hash_table_idx = idx;
            record.entry_idx = i;
            record.type = record_type;

            if (record.type == 1)
                record.is_active = false;
            
            else
                record.location_hash = (record.hash & ~0x3f) | (i / 7);
            
            records.push_back (record);
        }
    }

    // Decode records
    for (const auto &record : records)
    {
        auto iter = offsets_.find (record.offset);

        if (iter != offsets_.end ())
            std::cerr << "DEV Duplicate offset: " << mobius::core::string::to_hex (record.hash, 8) << " - 0x" << mobius::core::string::to_hex (record.offset, 8) << std::endl;

        else
        {
            offsets_[record.offset] = record.hash;
            _decode_record (decoder, record);
        }
    }

    std::cerr << "hash table decoded" << std::endl;

    return next_offset;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode record
// @param decoder data decoder
// @param record hash table record
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_msiecf::_decode_record (mobius::core::decoder::data_decoder &decoder, const hash_table_record &record)
{
    // Get record signature
    decoder.seek (record.offset);

    auto signature = decoder.get_bytearray_by_size (4);
    auto size = decoder.get_uint32_le () * BLOCK_SIZE - 8;

    if (record.type == 5)
        std::cerr << "DEV REC5: " << mobius::core::string::to_hex (record.offset, 8) << ", " << signature.dump () << ", " << size << std::endl;

    if (size == 0xfffffffffffffff8)
        return;

    // Process record according to its signature
    if (signature == "URL ")
        _decode_url_record (decoder);

    std::cout << "\t" << record.entry_idx << " [" << signature.to_string () << "] (hash=0x" << std::hex << record.hash
              << ", size=" << std::dec << size << ") at " << record.offset << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode URL record
// @param decoder data decoder
// @param record hash table record
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_msiecf::_decode_url_record (mobius::core::decoder::data_decoder &decoder)
{
    mobius::core::log log (__FILE__, __func__);

    std::uint64_t offset = decoder.tell () - 8;
    auto modification_time = decoder.get_nt_datetime ();
    auto access_time = decoder.get_nt_datetime ();

    mobius::core::datetime::datetime expiration_time;

    if (version_ == "4.7")
        expiration_time = decoder.get_nt_datetime ();

    else if (version_ == "5.2")
    {
        expiration_time = decoder.get_fat_datetime ();
        decoder.skip (4);
    }
    
    else
    {
        log.development (__LINE__, "Unsupported version: " + version_);
        return;
    }
    
    auto cached_file_size = decoder.get_uint64_le ();
    auto group_offset = decoder.get_uint32_le ();
    auto release_seconds = decoder.get_uint32_le ();
    auto d1 = decoder.get_uint32_le ();

    auto location_offset = decoder.get_uint32_le ();
    auto cache_dir_index = decoder.get_uint32_le ();
    auto filename_offset = decoder.get_uint32_le ();
    auto flags = decoder.get_uint32_le ();
    auto data_offset = decoder.get_uint32_le ();
    auto data_size = decoder.get_uint32_le ();
    auto extension_offset = decoder.get_uint32_le ();
    auto last_sync_time = decoder.get_fat_datetime ();
    auto hits = decoder.get_uint32_le ();
    auto d4 = decoder.get_uint32_le ();
    auto creation_time = decoder.get_fat_datetime ();

    if (d1)
        log.development (
            __LINE__,
            "D1: " + std::to_string (d1) + " (0x)" + mobius::core::string::to_hex (d1, 8)
        );

    // Show URL information
    std::cout << "\tURL:" << std::endl;
    std::cout << "\t\tModification Time: " << modification_time << std::endl;
    std::cout << "\t\tAccess Time: " << access_time << std::endl;
    std::cout << "\t\tExpiration Time: " << expiration_time << std::endl;
    std::cout << "\t\tCached File Size: " << cached_file_size << std::endl;
    std::cout << "\t\tLocation Offset: " << location_offset << " (0x"
              << mobius::core::string::to_hex (offset + location_offset, 8) << ")" << std::endl;
    std::cout << "\t\tCache Dir Index: " << cache_dir_index << std::endl;
    std::cout << "\t\tD1: " << d1 << std::endl;
    std::cout << "\t\tFilename Offset: " << filename_offset << " (0x"
              << mobius::core::string::to_hex (offset + filename_offset, 8) << ")" << std::endl;
    std::cout << "\t\tFlags: 0x" << mobius::core::string::to_hex (flags, 8) << std::endl;
    std::cout << "\t\tData Offset: " << data_offset << " (0x" << mobius::core::string::to_hex (offset + data_offset, 8)
              << ")" << std::endl;
    std::cout << "\t\tData Size: " << data_size << std::endl;
    std::cout << "\t\tLast Sync Time: " << last_sync_time << std::endl;
    std::cout << "\t\tHits: " << hits << std::endl;
    std::cout << "\t\tD4: " << d4 << std::endl;
    std::cout << "\t\tCreation Time: " << creation_time << std::endl;

    // Get location
    std::string location;

    if (location_offset)
    {
        decoder.seek (offset + location_offset);
        location = decoder.get_c_string ();
    }

    // Get filename
    std::string filename;

    if (filename_offset)
    {
        decoder.seek (offset + filename_offset);
        filename = decoder.get_c_string ();
    }

    // Get dirname
    std::string dirname;

    if (cache_dir_index < directories_.size ())
        dirname = directories_[cache_dir_index].dirname;

    std::cout << "\t\tLocation: " << location << std::endl;
    std::cout << "\t\tFilename: " << filename << std::endl;
    std::cout << "\t\tDirname: " << dirname << std::endl;

    // Get data
    std::map<std::uint8_t, mobius::core::pod::data> tags;
    std::string http_response;

    if (data_offset && data_size)
    {
        decoder.seek (offset + data_offset);

        if (flags & 0x00200000)
            tags = _decode_tags (decoder, data_size);

        else
            http_response = decoder.get_string_by_size (data_size);
    }

    // Show data
    std::cout << "\t\tHTTP Response: " << http_response << std::endl;
    std::cout << "\t\tTags: " << tags.size () << " entries" << std::endl;

    for (const auto &[k, v] : tags)
        std::cout << "\t\t\t[0x" << mobius::core::string::to_hex (k, 2) << "]: " << v.to_string () << std::endl;

    // Scheme
    auto pos = location.find (':', 1);
    std::string scheme;

    if (pos != std::string::npos)
        scheme = location.substr (0, pos);

    mobius::core::bytearray scheme_data;

    if (data_offset)
    {
        decoder.seek (offset + data_offset);
        scheme_data = decoder.get_bytearray_by_size (data_size);
    }

    std::cout << "SCHEME\t" << scheme << "\t" << data_size << '\t' << scheme_data.dump () << std::endl;
}

} // namespace mobius::extension::app::internet_explorer
