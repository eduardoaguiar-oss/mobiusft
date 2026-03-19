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
#include "object_map.hpp"
#include "common.hpp"
#include "object.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::uint32_t OBJECT_TYPE_OMAP = 0x0000000B;
static constexpr std::size_t BTREE_INFO_SIZE =
    40; // Size of btree_info_t structure in bytes

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// B-Tree flags
// @see Apple File System Reference - B-Tree Node Flags (pg. 132)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::uint16_t BTNODE_ROOT = 0x0001;
static constexpr std::uint16_t BTNODE_LEAF = 0x0002;
static constexpr std::uint16_t BTNODE_FIXED_KV_SIZE = 0x0004;
static constexpr std::uint16_t BTNODE_HASHED = 0x0008;
static constexpr std::uint16_t BTNODE_NOHEADER = 0x0010;
static constexpr std::uint16_t BTNODE_CHECK_KOFF_INVAL = 0x8000;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Table of Contents (TOC) entry structure
// @see Apple File System Reference - B-Trees (pg. 122)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct toc_entry
{
    std::uint16_t key_offset;
    std::uint16_t key_length;
    std::uint16_t val_offset;
    std::uint16_t val_length;
};

} // namespace

namespace mobius::extension::vfs::block::apfs
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Object Map (OMAP) constructor
// @param decoder Data decoder object
// @param omap_oid OMAP object ID
// @param xid Transaction ID
// @param block_size Block size
// @see Apple File System Reference - Object Maps (pg. 44)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
object_map::object_map (
    const mobius::core::decoder::data_decoder &decoder,
    std::uint64_t omap_oid,
    std::uint64_t xid,
    std::uint64_t block_size
)
    : decoder_ (decoder),
      oid_ (omap_oid),
      xid_ (xid),
      block_size_ (block_size)
{
    // Seek to OMAP object
    decoder_.seek (oid_ * block_size_);
    offset_ = decoder_.tell ();

    // Read object header
    object obj (decoder_);
    checksum_ = obj.get_checksum ();

    // Check for a valid checksum
    if (checksum_ == 0 || checksum_ == 0xffffffffffffffff)
        return;

    // Check if it's an object map
    if (obj.get_type () != OBJECT_TYPE_OMAP)
        return;

    // Decode OMAP fields
    // @see Apple File System Reference - Object Maps (pg. 44)
    flags_ = decoder_.get_uint32_le ();
    snap_count_ = decoder_.get_uint32_le ();
    tree_type_ = decoder_.get_uint32_le ();
    snapshot_tree_type_ = decoder_.get_uint32_le ();
    tree_oid_ = decoder_.get_uint64_le ();
    snapshot_tree_oid_ = decoder_.get_uint64_le ();
    most_recent_snap_ = decoder_.get_uint64_le ();
    pending_revert_min_ = decoder_.get_uint64_le ();
    pending_revert_max_ = decoder_.get_uint64_le ();

    // Test checksum
    decoder_.seek (offset_);
    auto data = decoder_.get_bytearray_by_size (block_size_);

    if (!test_checksum (data))
        return;

    // Decode B-Tree data from root node
    // @see Apple File System Reference - btree_info_t (pg. 126)
    decoder_.seek (tree_oid_ * block_size_ + block_size_ - BTREE_INFO_SIZE);
    btree_flags_ = decoder_.get_uint32_le ();
    btree_node_size_ = decoder_.get_uint32_le ();
    btree_key_size_ = decoder_.get_uint32_le ();
    btree_val_size_ = decoder_.get_uint32_le ();
    longest_key_ = decoder_.get_uint32_le ();
    longest_val_ = decoder_.get_uint32_le ();
    btree_key_count_ = decoder_.get_uint64_le ();
    btree_node_count_ = decoder_.get_uint64_le ();

    // Load nodes
    _load_node (tree_oid_ * block_size_);

    // If we reached this point, the object map is valid
    is_valid_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get physical block address for a given OID
// @param oid Object ID
// @return Physical block address, or 0 if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
object_map::get_physical_block_address (std::uint64_t oid) const
{
    auto it = mappings_.find (oid);

    if (it != mappings_.end ())
        return it->second;

    return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load OID to physical block mappings from B-Tree nodes
// @param node_offset Offset of the B-Tree node to load
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
object_map::_load_node (std::uint64_t node_offset)
{
    decoder_.seek (node_offset);

    // Decode object header
    auto obj = mobius::extension::vfs::block::apfs::object (decoder_);

    // Decode btree_node_phys_t fields
    auto flags = decoder_.get_uint16_le ();
    auto level = decoder_.get_uint16_le ();
    auto num_keys = decoder_.get_uint32_le ();
    auto table_space_offset = decoder_.get_uint16_le ();
    auto table_space_len = decoder_.get_uint16_le ();
    auto free_space_offset = decoder_.get_uint16_le ();
    auto free_space_len = decoder_.get_uint16_le ();
    auto key_free_list_offset = decoder_.get_uint16_le ();
    auto key_free_list_len = decoder_.get_uint16_le ();
    auto val_free_list_offset = decoder_.get_uint16_le ();
    auto val_free_list_len = decoder_.get_uint16_le ();
    auto bnt_data_offset = decoder_.tell ();

    // Decode Table of Contents (TOC)
    // The offset for the table of contents is counted from the beginning of the nodeʼs btn_data,
    // which starts immediately after the btree_node_phys_t structure.
    // So we need to seek to the beginning of the node and then add the offset.
    decoder_.seek (bnt_data_offset + table_space_offset);
    std::vector<toc_entry> entries;

    for (std::uint32_t i = 0; i < num_keys; i++)
    {
        toc_entry entry;

        if (flags & BTNODE_FIXED_KV_SIZE)
        {
            entry.key_offset = decoder_.get_uint16_le ();
            entry.key_length = btree_key_size_;
            entry.val_offset = decoder_.get_uint16_le ();
            entry.val_length = btree_val_size_;
        }

        else
        {
            entry.key_offset = decoder_.get_uint16_le ();
            entry.key_length = decoder_.get_uint16_le ();
            entry.val_offset = decoder_.get_uint16_le ();
            entry.val_length = decoder_.get_uint16_le ();
        }

        entries.push_back (entry);
    }

    // Get keys and values. The keys in the B-tree are instances of omap_key_t
    // and the values are instances of omap_val_t.
    // @see Apple File System Reference - Object Maps (pg. 44)

    // @todo Handle XID and multiple XIDs (snapshots). For now we just load the most recent one.
    std::uint64_t keys_offset =
        bnt_data_offset + table_space_offset + table_space_len;
    std::uint64_t vals_offset = node_offset + block_size_ - BTREE_INFO_SIZE;

    for (const auto &entry : entries)
    {
        decoder_.seek (keys_offset + entry.key_offset);
        auto ok_oid = decoder_.get_uint64_le ();
        auto ok_xid = decoder_.get_uint64_le ();

        decoder_.seek (vals_offset - entry.val_offset);
        auto ov_flags = decoder_.get_uint32_le ();
        auto ov_size = decoder_.get_uint32_le ();
        auto ov_paddr = decoder_.get_uint64_le ();

        mappings_[ok_oid] = ov_paddr;
    }
}

} // namespace mobius::extension::vfs::block::apfs
