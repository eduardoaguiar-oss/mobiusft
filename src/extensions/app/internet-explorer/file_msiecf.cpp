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

#include <iostream>
#include <map>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief MSIE Cache File block size
static constexpr std::size_t BLOCK_SIZE = 128;

} // namespace

namespace mobius::extension::app::internet_explorer
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @see MSIE Cache File (index.dat) format specification v0.0.18. By Joachim Metz
// @see http://www.stevebunting.org/udpd4n6/forensics/index_dat2.htm
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
        offset = __decode_hash_table (decoder);
    }

    is_instance_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode hash table
// @param decoder data decoder
// @return Next hash table offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
file_msiecf::__decode_hash_table (mobius::core::decoder::data_decoder &decoder)
{
    // Check if hash table signature is valid
    auto signature = decoder.get_bytearray_by_size (4);

    if (signature != "HASH")
        return 0;

    // Decode hash table data
    auto block_count = decoder.get_uint32_le ();
    auto next_offset = decoder.get_uint32_le ();
    auto idx = decoder.get_uint32_le ();

    // Decode blocks
    auto count = (block_count * BLOCK_SIZE - 16) / 8;
    std::vector<hash_table_record> records;

    for (std::size_t i = 0; i < count; ++i)
    {
        auto record = hash_table_record ();
        record.hash = decoder.get_uint32_le ();
        record.offset = decoder.get_uint32_le ();
        record.hash_table_idx = idx;
        record.entry_idx = i;

        if (record.offset && record.offset != 3 && record.hash != record.offset && record.hash != 0x0badf00d &&
            record.hash != 0xdeadbeef)
            records.push_back (record);
    }

    // Decode records
    std::cerr << "hash table (idx=" << idx << ", block_count=" << block_count << ", next_offset=" << next_offset
              << ", records=" << records.size () << ")" << std::endl;

    for (const auto &record : records)
    {
        auto iter = offsets_.find (record.offset);

        if (iter != offsets_.end ())
            std::cerr << "\tDuplicate record offset: " << record.offset << " - 0x" << std::hex << iter->second << "/"
                      << record.hash << std::dec << std::endl;

        else
        {
            offsets_[record.offset] = record.hash;
            __decode_record (decoder, record);
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
file_msiecf::__decode_record (mobius::core::decoder::data_decoder &decoder, const hash_table_record &record)
{
    // Decode record according to its signature
    decoder.seek (record.offset);

    auto signature = decoder.get_bytearray_by_size (4);
    auto size = decoder.get_uint32_le () * BLOCK_SIZE - 8;

    if (size != 0xfffffffffffffff8)
        std::cout << "\t" << record.entry_idx << " [" << signature.to_string () << "] (hash=0x" << std::hex
                  << record.hash << ", size=" << std::dec << size << ") at " << record.offset << std::endl;
}

} // namespace mobius::extension::app::internet_explorer
