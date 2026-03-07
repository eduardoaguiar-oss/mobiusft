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
#include <mobius/core/bytearray.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/resource.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/core/vfs/block.hpp>
#include <vector>

#include <iostream>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References
// @see https://developer.apple.com/support/downloads/Apple-File-System-Reference.pdf
// @see https://github.com/sgan81/apfs-fuse
// @see https://github.com/memecode/apfs-tools
// @see https://www.ntfs.com/apfs-structure.htm
// @see https://www.mac4n6.com/blog/category/APFS
// @see https://github.com/libyal/libfsapfs/blob/main/documentation/Apple%20File%20System%20(APFS).asciidoc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::uint64_t OID_NX_SUPERBLOCK = 1;
static constexpr std::size_t MAX_CKSUM_SIZE = 8;
static constexpr std::size_t NX_NUM_COUNTERS = 4;
static constexpr std::size_t NX_MAX_FILE_SYSTEMS = 100;
static constexpr std::size_t NX_EPH_INFO_COUNT = 4;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief obj_phys_t structure
// @see Apple File System Reference - obj_phys_t (pg. 10)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct obj_phys_t
{
    std::uint64_t checksum;
    std::uint64_t oid;
    std::uint64_t xid;
    std::uint32_t type;
    std::uint32_t subtype;
    bool is_physical = false;
    bool is_virtual = false;
    bool is_ephemeral = false;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Superblock structure
// @see Apple File System Reference - nx_superblock_t (pg. 27)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct superblock
{
    bool is_valid = false;

    std::uint64_t checksum;
    std::uint64_t offset = 0;
    std::uint64_t oid;
    std::uint64_t xid;
    std::uint32_t type;
    std::uint32_t subtype;

    mobius::core::bytearray signature; // 4 bytes
    std::uint32_t block_size;
    std::uint64_t block_count;
    std::uint64_t features;
    std::uint64_t readonly_features;
    std::uint64_t incompatible_features;
    std::string uuid;
    std::uint64_t next_oid;
    std::uint64_t next_xid;
    std::uint32_t xp_desc_blocks;
    std::uint32_t xp_data_blocks;
    std::uint64_t xp_desc_base;
    std::uint64_t xp_data_base;
    std::uint32_t xp_desc_next;
    std::uint32_t xp_data_next;
    std::uint32_t xp_desc_index;
    std::uint32_t xp_desc_len;
    std::uint32_t xp_data_index;
    std::uint32_t xp_data_len;
    std::uint64_t spaceman_oid;
    std::uint64_t omap_oid;
    std::uint64_t reaper_oid;
    std::uint32_t test_type;
    std::uint32_t max_file_systems;
    std::vector<std::uint64_t> file_system_oids;
    std::uint64_t counters[NX_NUM_COUNTERS];
    std::uint64_t block_out_addr;
    std::uint64_t block_out_count;
    std::uint64_t evict_mapping_tree_oid;
    std::uint64_t flags;
    std::uint64_t efi_jumpstart;
    std::string fusion_uuid;
    std::uint64_t keylocker_addr;
    std::uint64_t keylocker_count;
    std::uint64_t ephemeral_info[NX_EPH_INFO_COUNT];
    std::uint64_t test_oid;
    std::uint64_t fusion_mt_oid;
    std::uint64_t fusion_wbc_oid;
    std::uint64_t fusion_wbc_addr;
    std::uint64_t fusion_wbc_count;
    std::uint64_t newest_mounted_version;
    std::uint64_t mkb_locker_addr;
    std::uint64_t mkb_locker_count;

    // Derived attributes
    std::uint64_t size = 0;
    std::uint32_t version = 0;
    bool supports_fusion = false;
    bool supports_defrag = false;
    bool flag_lcfd = false;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test block checksum
// @param data Block data
// @return True if checksum is valid, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static bool
_test_checksum (const mobius::core::bytearray &data)
{
    mobius::core::decoder::data_decoder decoder (data);

    std::uint64_t checksum = decoder.get_uint64_le ();
    std::size_t siz = data.size () - 8;
    std::uint64_t sum1 = 0;
    std::uint64_t sum2 = 0;

    while (siz)
    {
        sum1 += decoder.get_uint32_le ();
        sum2 += sum1;
        sum1 %= 0xffffffff;
        sum2 %= 0xffffffff;
        siz -= 4;
    }

    uint32_t c0 = static_cast<uint32_t> (checksum);
    uint32_t c1 = static_cast<uint32_t> (checksum >> 32);

    sum1 = (sum1 + c0) % 0xFFFFFFFFULL;
    sum2 = (sum2 + sum1) % 0xFFFFFFFFULL;

    sum1 = (sum1 + c1) % 0xFFFFFFFFULL;
    sum2 = (sum2 + sum1) % 0xFFFFFFFFULL;

    return (sum1 == 0 && sum2 == 0);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode obj_phys_t structure
// @param decoder Data decoder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static obj_phys_t
_decode_obj_phys_t (mobius::core::decoder::data_decoder &decoder)
{
    obj_phys_t obj_phys;

    obj_phys.checksum = decoder.get_uint64_le ();
    obj_phys.oid = decoder.get_uint64_le ();
    obj_phys.xid = decoder.get_uint64_le ();
    obj_phys.subtype = decoder.get_uint32_le ();

    auto type = decoder.get_uint32_le ();
    obj_phys.type = type & 0xffff;
    obj_phys.is_physical = (type & 0xc0000000) == 0x40000000;
    obj_phys.is_virtual = (type & 0xc0000000) == 0x00000000;
    obj_phys.is_ephemeral = (type & 0xc0000000) == 0x80000000;

    return obj_phys;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode superblock structure
// @param decoder Data decoder object
// @return Superblock structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static superblock
_decode_superblock (mobius::core::decoder::data_decoder &decoder)
{
    superblock sb;
    sb.offset = decoder.tell ();

    // Read object header
    auto obj = _decode_obj_phys_t (decoder);
    sb.checksum = obj.checksum;
    sb.oid = obj.oid;
    sb.xid = obj.xid;
    sb.type = obj.type;
    sb.subtype = obj.subtype;

    // Check for a valid checksum
    if (sb.checksum == 0 || sb.checksum == 0xffffffffffffffff)
        return sb;

    // Check if it's a superblock
    if (sb.oid != OID_NX_SUPERBLOCK)
        return sb;

    // Check signature
    sb.signature = decoder.get_bytearray_by_size (4);

    if (sb.signature != "NXSB")
        return sb;

    // Read superblock fields
    sb.block_size = decoder.get_uint32_le ();
    sb.block_count = decoder.get_uint64_le ();
    sb.features = decoder.get_uint64_le ();
    sb.readonly_features = decoder.get_uint64_le ();
    sb.incompatible_features = decoder.get_uint64_le ();
    sb.uuid = decoder.get_guid ();
    sb.next_oid = decoder.get_uint64_le ();
    sb.next_xid = decoder.get_uint64_le ();
    sb.xp_desc_blocks = decoder.get_uint32_le ();
    sb.xp_data_blocks = decoder.get_uint32_le ();
    sb.xp_desc_base = decoder.get_uint64_le ();
    sb.xp_data_base = decoder.get_uint64_le ();
    sb.xp_desc_next = decoder.get_uint32_le ();
    sb.xp_data_next = decoder.get_uint32_le ();
    sb.xp_desc_index = decoder.get_uint32_le ();
    sb.xp_desc_len = decoder.get_uint32_le ();
    sb.xp_data_index = decoder.get_uint32_le ();
    sb.xp_data_len = decoder.get_uint32_le ();
    sb.spaceman_oid = decoder.get_uint64_le ();
    sb.omap_oid = decoder.get_uint64_le ();
    sb.reaper_oid = decoder.get_uint64_le ();
    sb.test_type = decoder.get_uint32_le ();
    sb.max_file_systems = decoder.get_uint32_le ();

    // Read file system OIDs
    for (std::size_t i = 0; i < sb.max_file_systems; i++)
    {
        auto oid = decoder.get_uint64_le ();
        if (oid)
            sb.file_system_oids.push_back (oid);
    }

    // Read counters
    for (std::size_t i = 0; i < NX_NUM_COUNTERS; i++)
        sb.counters[i] = decoder.get_uint64_le ();

    // Reader other fields
    sb.block_out_addr = decoder.get_uint64_le ();
    sb.block_out_count = decoder.get_uint64_le ();
    sb.evict_mapping_tree_oid = decoder.get_uint64_le ();
    sb.flags = decoder.get_uint64_le ();
    sb.efi_jumpstart = decoder.get_uint64_le ();
    sb.fusion_uuid = decoder.get_uuid ();
    sb.keylocker_addr = decoder.get_uint64_le ();
    sb.keylocker_count = decoder.get_uint64_le ();

    // Read ephemeral info
    for (std::size_t i = 0; i < NX_EPH_INFO_COUNT; i++)
        sb.ephemeral_info[i] = decoder.get_uint64_le ();

    // Read other fields
    sb.test_oid = decoder.get_uint64_le ();
    sb.fusion_mt_oid = decoder.get_uint64_le ();
    sb.fusion_wbc_oid = decoder.get_uint64_le ();
    sb.fusion_wbc_addr = decoder.get_uint64_le ();
    sb.fusion_wbc_count = decoder.get_uint64_le ();
    sb.newest_mounted_version = decoder.get_uint64_le ();
    sb.mkb_locker_addr = decoder.get_uint64_le ();
    sb.mkb_locker_count = decoder.get_uint64_le ();

    // Calculate derived attributes
    sb.size = sb.block_count * static_cast<std::uint64_t> (sb.block_size);
    sb.supports_fusion = bool (sb.incompatible_features & 0x100);
    sb.supports_defrag = bool (sb.features & 0x1);
    sb.flag_lcfd = bool (sb.features & 0x2);

    if (sb.incompatible_features & 0x1)
        sb.version = 1;

    else if (sb.incompatible_features & 0x2)
        sb.version = 2;

    // Test checksum
    decoder.seek (sb.offset);
    auto data = decoder.get_bytearray_by_size (sb.block_size);

    if (!_test_checksum (data))
        return sb;

    // If we reached this point, the superblock is valid
    sb.is_valid = true;

    return sb;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Try to decode block
// @author Eduardo Aguiar
// @param block Block object
// @param new_blocks Vector for newly created blocks
// @param pending_blocks Pending blocks
// @return <b>true</b> if block was decoded, <b>false</b> otherwise
// @see Apple File System Reference - Mounting an Apple File System Partition (pg. 26)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static bool
decoder (
    const mobius::core::vfs::block &block,
    std::vector<mobius::core::vfs::block> &new_blocks,
    std::vector<mobius::core::vfs::block> &
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Try to read a superblock structure from the beginning of the block (step 1)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto reader = block.new_reader ();
    auto decoder = mobius::core::decoder::data_decoder (reader);
    decoder.seek (0);

    auto sb = _decode_superblock (decoder);

    if (!sb.is_valid)
        return false;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Try to find the most recent valid superblock on checkpoint descriptor
    // area (steps 2-4)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (std::uint64_t idx = sb.xp_desc_base;
         idx < sb.xp_desc_base + sb.xp_desc_blocks; idx++)
    {
        decoder.seek (idx * sb.block_size);
        auto desc_sb = _decode_superblock (decoder);

        if (desc_sb.is_valid && desc_sb.xid > sb.xid)
            sb = desc_sb;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Create APFS container block
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto apfs_block =
        mobius::core::vfs::new_slice_block (block, "apfs.container");

    apfs_block.set_attribute ("block_size", sb.block_size);
    apfs_block.set_attribute ("block_count", sb.block_count);
    apfs_block.set_attribute ("block_out_addr", sb.block_out_addr);
    apfs_block.set_attribute ("block_out_count", sb.block_out_count);
    apfs_block.set_attribute (
        "description", "APFS Container - UUID: " + sb.uuid
    );
    apfs_block.set_attribute (
        "evict_mapping_tree_oid", sb.evict_mapping_tree_oid
    );
    apfs_block.set_attribute ("efi_jumpstart", sb.efi_jumpstart);
    apfs_block.set_attribute ("features", sb.features);
    apfs_block.set_attribute ("flags", sb.flags);
    apfs_block.set_attribute ("flag_lcfd", sb.flag_lcfd);
    apfs_block.set_attribute ("fusion_mt_oid", sb.fusion_mt_oid);
    apfs_block.set_attribute ("fusion_uuid", sb.fusion_uuid);
    apfs_block.set_attribute ("fusion_wbc_addr", sb.fusion_wbc_addr);
    apfs_block.set_attribute ("fusion_wbc_count", sb.fusion_wbc_count);
    apfs_block.set_attribute ("fusion_wbc_oid", sb.fusion_wbc_oid);
    apfs_block.set_attribute ("fs_count", sb.file_system_oids.size ());
    apfs_block.set_attribute ("keylocker_addr", sb.keylocker_addr);
    apfs_block.set_attribute ("keylocker_count", sb.keylocker_count);
    apfs_block.set_attribute ("max_file_systems", sb.max_file_systems);
    apfs_block.set_attribute ("mkb_locker_addr", sb.mkb_locker_addr);
    apfs_block.set_attribute ("mkb_locker_count", sb.mkb_locker_count);
    apfs_block.set_attribute (
        "newest_mounted_version", sb.newest_mounted_version
    );
    apfs_block.set_attribute ("next_oid", sb.next_oid);
    apfs_block.set_attribute ("next_xid", sb.next_xid);
    apfs_block.set_attribute ("omap_oid", sb.omap_oid);
    apfs_block.set_attribute ("reaper_oid", sb.reaper_oid);
    apfs_block.set_attribute (
        "incompatible_features", sb.incompatible_features
    );
    apfs_block.set_attribute ("readonly_features", sb.readonly_features);
    apfs_block.set_attribute ("signature", sb.signature.to_string ());
    apfs_block.set_attribute ("size", sb.size);
    apfs_block.set_attribute ("spaceman_oid", sb.spaceman_oid);
    apfs_block.set_attribute ("superblock_addr", sb.offset / sb.block_size);
    apfs_block.set_attribute (
        "superblock_checksum",
        "0x" + mobius::core::string::to_hex (sb.checksum, 16)
    );
    apfs_block.set_attribute ("superblock_offset", sb.offset);
    apfs_block.set_attribute ("superblock_xid", sb.xid);
    apfs_block.set_attribute ("supports_defrag", sb.supports_defrag);
    apfs_block.set_attribute ("supports_fusion", sb.supports_fusion);
    apfs_block.set_attribute ("test_oid", sb.test_oid);
    apfs_block.set_attribute ("test_type", sb.test_type);
    apfs_block.set_attribute ("uuid", sb.uuid);
    apfs_block.set_attribute ("version", sb.version);
    apfs_block.set_attribute ("xp_data_base", sb.xp_data_base);
    apfs_block.set_attribute ("xp_data_blocks", sb.xp_data_blocks);
    apfs_block.set_attribute ("xp_data_index", sb.xp_data_index);
    apfs_block.set_attribute ("xp_data_len", sb.xp_data_len);
    apfs_block.set_attribute ("xp_data_next", sb.xp_data_next);
    apfs_block.set_attribute ("xp_desc_base", sb.xp_desc_base);
    apfs_block.set_attribute ("xp_desc_blocks", sb.xp_desc_blocks);
    apfs_block.set_attribute ("xp_desc_index", sb.xp_desc_index);
    apfs_block.set_attribute ("xp_desc_len", sb.xp_desc_len);
    apfs_block.set_attribute ("xp_desc_next", sb.xp_desc_next);

    apfs_block.set_handled (true);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add APFS container block to new blocks
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    new_blocks.push_back (apfs_block);
    return true;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Extension data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C"
{
    const char *EXTENSION_ID = "vfs.block.apfs";
    const char *EXTENSION_NAME = "VFS Block: APFS";
    const char *EXTENSION_VERSION = "1.0";
    const char *EXTENSION_AUTHORS = "Eduardo Aguiar";
    const char *EXTENSION_DESCRIPTION = "APFS Container block support";
} // extern "C"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Start extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
start ()
{
    mobius::core::add_resource (
        "vfs.block.decoder.apfs", "APFS block decoder",
        static_cast<mobius::core::vfs::block_decoder_resource_type> (decoder)
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Stop extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
stop ()
{
    mobius::core::remove_resource ("vfs.block.decoder.apfs");
}
