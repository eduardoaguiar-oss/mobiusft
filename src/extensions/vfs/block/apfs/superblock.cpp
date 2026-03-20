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
#include "superblock.hpp"
#include "common.hpp"
#include "object.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::uint64_t OID_NX_SUPERBLOCK = 1;
static constexpr std::uint32_t OBJECT_TYPE_NX_SUPERBLOCK = 0x00000001;
static constexpr std::size_t NX_NUM_COUNTERS = 4;
static constexpr std::size_t NX_MAX_FILE_SYSTEMS = 100;
static constexpr std::size_t NX_EPH_INFO_COUNT = 4;

} // namespace

namespace mobius::extension::vfs::block::apfs
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Superblock constructor
// @param decoder Data decoder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
superblock::superblock (mobius::core::decoder::data_decoder &decoder)
{
    offset_ = decoder.tell ();

    // Read object header
    object obj (decoder);

    // Check for a valid checksum
    if (obj.get_checksum () == 0 || obj.get_checksum () == 0xffffffffffffffff)
        return;

    // Check if it's a superblock
    if (obj.get_type () != OBJECT_TYPE_NX_SUPERBLOCK ||
        obj.get_oid () != OID_NX_SUPERBLOCK)
        return;

    // Check signature
    signature_ = decoder.get_bytearray_by_size (4);

    if (signature_ != "NXSB")
        return;

    // Read superblock fields
    block_size_ = decoder.get_uint32_le ();
    block_count_ = decoder.get_uint64_le ();
    features_ = decoder.get_uint64_le ();
    readonly_features_ = decoder.get_uint64_le ();
    incompatible_features_ = decoder.get_uint64_le ();
    uuid_ = decoder.get_guid ();
    next_oid_ = decoder.get_uint64_le ();
    next_xid_ = decoder.get_uint64_le ();
    xp_desc_blocks_ = decoder.get_uint32_le ();
    xp_data_blocks_ = decoder.get_uint32_le ();
    xp_desc_base_ = decoder.get_uint64_le ();
    xp_data_base_ = decoder.get_uint64_le ();
    xp_desc_next_ = decoder.get_uint32_le ();
    xp_data_next_ = decoder.get_uint32_le ();
    xp_desc_index_ = decoder.get_uint32_le ();
    xp_desc_len_ = decoder.get_uint32_le ();
    xp_data_index_ = decoder.get_uint32_le ();
    xp_data_len_ = decoder.get_uint32_le ();
    spaceman_oid_ = decoder.get_uint64_le ();
    omap_oid_ = decoder.get_uint64_le ();
    reaper_oid_ = decoder.get_uint64_le ();
    test_type_ = decoder.get_uint32_le ();
    max_file_systems_ = decoder.get_uint32_le ();

    // Read file system OIDs
    for (std::size_t i = 0; i < max_file_systems_; i++)
    {
        auto oid = decoder.get_uint64_le ();
        if (oid)
            file_system_oids_.push_back (oid);
    }

    // Read counters
    checksum_set_count_ = decoder.get_uint64_le ();
    checksum_fail_count_ = decoder.get_uint64_le ();
    decoder.skip ((NX_NUM_COUNTERS - 2) * sizeof(std::uint64_t));

    // Reader other fields
    blocked_out_addr_ = decoder.get_uint64_le ();
    blocked_out_count_ = decoder.get_uint64_le ();
    evict_mapping_tree_oid_ = decoder.get_uint64_le ();
    flags_ = decoder.get_uint64_le ();
    efi_jumpstart_ = decoder.get_uint64_le ();
    fusion_uuid_ = decoder.get_uuid ();
    keylocker_addr_ = decoder.get_uint64_le ();
    keylocker_count_ = decoder.get_uint64_le ();

    // Read ephemeral info
    for (std::size_t i = 0; i < NX_EPH_INFO_COUNT; i++)
        ephemeral_info_.push_back (decoder.get_uint64_le ());

    // Read other fields
    test_oid_ = decoder.get_uint64_le ();
    fusion_mt_oid_ = decoder.get_uint64_le ();
    fusion_wbc_oid_ = decoder.get_uint64_le ();
    fusion_wbc_addr_ = decoder.get_uint64_le ();
    fusion_wbc_count_ = decoder.get_uint64_le ();
    newest_mounted_version_ = decoder.get_uint64_le ();
    mkb_locker_addr_ = decoder.get_uint64_le ();
    mkb_locker_count_ = decoder.get_uint64_le ();

    // Calculate derived attributes
    size_ = block_count_ * static_cast<std::uint64_t> (block_size_);
    flag_supports_fusion_ = bool (incompatible_features_ & 0x100);
    flag_supports_defrag_ = bool (features_ & 0x1);
    flag_lcfd_ = bool (features_ & 0x2);

    if (incompatible_features_ & 0x1)
        version_ = 1;

    else if (incompatible_features_ & 0x2)
        version_ = 2;

    // Test checksum
    decoder.seek (offset_);
    auto data = decoder.get_bytearray_by_size (block_size_);

    if (!test_checksum (data))
        return;

    // If we reached this point, the superblock is valid
    is_valid_ = true;
}

} // namespace mobius::extension::vfs::block::apfs
