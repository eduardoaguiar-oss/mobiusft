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
#include <mobius/core/string_functions.hpp>
#include <mobius/core/vfs/block.hpp>
#include <mobius/core/vfs/vfs.hpp>
#include "common.hpp"
#include "filesystem_impl.hpp"
#include "object.hpp"
#include "object_map.hpp"
#include "superblock.hpp"
#include "volume.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
//
// @see https://developer.apple.com/support/downloads/Apple-File-System-Reference.pdf
// @see https://github.com/sgan81/apfs-fuse
// @see https://github.com/memecode/apfs-tools
// @see https://www.ntfs.com/apfs-structure.htm
// @see https://www.mac4n6.com/blog/category/APFS
// @see https://github.com/libyal/libfsapfs/blob/main/documentation/Apple%20File%20System%20(APFS).asciidoc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Rationale:
//
// 1. APFS containers are the main structure in the file system and they contain volumes
//    that share the same storage area. They also have a superblock at the beginning of the
//    container, which contains important information about the container and its volumes.
//
// 2. APFS containers are composed of the following areas:
//    - Superblock: Contains metadata about the container, such as block size,
//      block count, and pointers to other important structures.
//    - Checkpoint Area: Contains multiple checkpoint descriptors, which are
//      used to store the state of the file system at different points in time. Each checkpoint
//      stores the state of the file system at different points in time. Each checkpoint
//      descriptor contains a copy of the superblock and a pointer to the container's object map.
//    - Space Manager Area: Contains the space manager, which is responsible for
//      managing free space within the container.
//    - Storage Area: Contains the actual data blocks for the volumes within the container.
//
// 3. The first block (usually 4096 bytes) of the container has a copy of the
//    superblock. It might be a copy of the latest version or an old version.
//    It should be used only to locate the checkpoint descriptor area.
//
// 4. The checkpoint descriptor area contains multiple checkpoint descriptors, which are
//    used to store the state of the file system at different points in time. Each checkpoint
//    descriptor contains a copy of the superblock and a pointer to the container's object map.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
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
    auto decoder = mobius::core::decoder::data_decoder (block.new_reader ());

    mobius::extension::vfs::block::apfs::superblock sb (decoder);

    if (!sb)
        return false;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Try to find the most recent valid superblock on checkpoint descriptor
    // area (steps 2-4)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (std::uint64_t idx = sb.get_xp_desc_base ();
         idx < sb.get_xp_desc_base () + sb.get_xp_desc_blocks (); idx++)
    {
        decoder.seek (idx * sb.get_block_size ());
        mobius::extension::vfs::block::apfs::superblock desc_sb (decoder);

        if (desc_sb && desc_sb.get_xid () > sb.get_xid ())
            sb = desc_sb;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Read container object map (step 6)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::extension::vfs::block::apfs::object_map omap (
        decoder, sb.get_omap_oid (), sb.get_xid (), sb.get_block_size ()
    );

    if (!omap)
        return false;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Read volumes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<mobius::extension::vfs::block::apfs::volume> volumes;

    for (std::uint64_t fs_oid : sb.get_file_system_oids ())
    {
        auto paddr = omap.get_physical_block_address (fs_oid);

        if (paddr)
        {
            mobius::extension::vfs::block::apfs::volume volume (
                decoder, paddr, sb.get_block_size ()
            );

            if (volume)
                volumes.push_back (volume);

            else
            {
                log.warning (
                    __LINE__,
                    "Failed to decode volume OID: " + std::to_string (paddr)
                );
            }
        }

        else
        {
            log.warning (
                __LINE__, "Failed to get physical block address for volume "
                          "with FS OID = " +
                              std::to_string (fs_oid)
            );
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Create APFS container block
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto apfs_block =
        mobius::core::vfs::new_slice_block (block, "apfs.container");

    apfs_block.set_attribute ("block_count", sb.get_block_count ());
    apfs_block.set_attribute ("block_size", sb.get_block_size ());
    apfs_block.set_attribute ("blocked_out_addr", sb.get_blocked_out_addr ());
    apfs_block.set_attribute ("blocked_out_count", sb.get_blocked_out_count ());
    apfs_block.set_attribute (
        "checksum_set_count", sb.get_checksum_set_count ()
    );
    apfs_block.set_attribute (
        "checksum_fail_count", sb.get_checksum_fail_count ()
    );
    apfs_block.set_attribute (
        "description", "APFS Container - UUID: " + sb.get_uuid ()

    );
    apfs_block.set_attribute ("efi_jumpstart", sb.get_efi_jumpstart ());
    apfs_block.set_attribute (
        "evict_mapping_tree_oid", sb.get_evict_mapping_tree_oid ()
    );
    apfs_block.set_attribute (
        "features", "0x" + mobius::core::string::to_hex (sb.get_features (), 16)
    );
    apfs_block.set_attribute (
        "flags", "0x" + mobius::core::string::to_hex (sb.get_flags (), 8)
    );
    apfs_block.set_attribute ("flag_lcfd", sb.get_flag_lcfd ());
    apfs_block.set_attribute (
        "flag_supports_defrag", sb.get_flag_supports_defrag ()
    );
    apfs_block.set_attribute (
        "flag_supports_fusion", sb.get_flag_supports_fusion ()
    );
    apfs_block.set_attribute ("fusion_mt_oid", sb.get_fusion_mt_oid ());
    apfs_block.set_attribute ("fusion_uuid", sb.get_fusion_uuid ());
    apfs_block.set_attribute ("fusion_wbc_addr", sb.get_fusion_wbc_addr ());
    apfs_block.set_attribute ("fusion_wbc_count", sb.get_fusion_wbc_count ());
    apfs_block.set_attribute ("fusion_wbc_oid", sb.get_fusion_wbc_oid ());
    apfs_block.set_attribute (
        "incompatible_features",
        "0x" +
            mobius::core::string::to_hex (sb.get_incompatible_features (), 16)
    );
    apfs_block.set_attribute ("keylocker_addr", sb.get_keylocker_addr ());
    apfs_block.set_attribute ("keylocker_count", sb.get_keylocker_count ());
    apfs_block.set_attribute ("max_file_systems", sb.get_max_file_systems ());
    apfs_block.set_attribute ("mkb_locker_addr", sb.get_mkb_locker_addr ());
    apfs_block.set_attribute ("mkb_locker_count", sb.get_mkb_locker_count ());
    apfs_block.set_attribute (
        "newest_mounted_version", sb.get_newest_mounted_version ()
    );
    apfs_block.set_attribute ("next_oid", sb.get_next_oid ());
    apfs_block.set_attribute ("next_xid", sb.get_next_xid ());
    apfs_block.set_attribute (
        "omap_btree_flags",
        "0x" + mobius::core::string::to_hex (omap.get_btree_flags (), 8)
    );
    apfs_block.set_attribute (
        "omap_btree_key_count", omap.get_btree_key_count ()
    );
    apfs_block.set_attribute (
        "omap_btree_node_count", omap.get_btree_node_count ()
    );
    apfs_block.set_attribute (
        "omap_checksum",
        "0x" + mobius::core::string::to_hex (omap.get_checksum (), 16)
    );
    apfs_block.set_attribute (
        "omap_flags", "0x" + mobius::core::string::to_hex (omap.get_flags (), 8)
    );

    apfs_block.set_attribute (
        "omap_most_recent_snap", omap.get_most_recent_snap ()
    );
    apfs_block.set_attribute ("omap_offset", omap.get_offset ());
    apfs_block.set_attribute ("omap_oid", sb.get_omap_oid ());
    apfs_block.set_attribute (
        "omap_pending_revert_min", omap.get_pending_revert_min ()
    );
    apfs_block.set_attribute (
        "omap_pending_revert_max", omap.get_pending_revert_max ()
    );
    apfs_block.set_attribute ("omap_snap_count", omap.get_snapshot_count ());
    apfs_block.set_attribute (
        "omap_snapshot_tree_oid", omap.get_snapshot_tree_oid ()
    );
    apfs_block.set_attribute (
        "omap_snapshot_tree_type",
        "0x" + mobius::core::string::to_hex (omap.get_snapshot_tree_type ())
    );
    apfs_block.set_attribute ("omap_tree_oid", omap.get_tree_oid ());
    apfs_block.set_attribute (
        "omap_tree_type",
        "0x" + mobius::core::string::to_hex (omap.get_tree_type ())
    );
    apfs_block.set_attribute ("reaper_oid", sb.get_reaper_oid ());
    apfs_block.set_attribute (
        "readonly_features",
        "0x" + mobius::core::string::to_hex (sb.get_readonly_features (), 16)
    );
    apfs_block.set_attribute ("signature", sb.get_signature ().to_string ());
    apfs_block.set_attribute ("size", sb.get_size ());
    apfs_block.set_attribute ("spaceman_oid", sb.get_spaceman_oid ());
    apfs_block.set_attribute (
        "superblock_addr", sb.get_offset () / sb.get_block_size ()
    );
    apfs_block.set_attribute (
        "superblock_checksum",
        "0x" + mobius::core::string::to_hex (sb.get_checksum (), 16)
    );
    apfs_block.set_attribute ("superblock_offset", sb.get_offset ());
    apfs_block.set_attribute ("superblock_oid", sb.get_oid ());
    apfs_block.set_attribute ("superblock_xid", sb.get_xid ());
    apfs_block.set_attribute ("test_oid", sb.get_test_oid ());
    apfs_block.set_attribute ("test_type", sb.get_test_type ());
    apfs_block.set_attribute ("uuid", sb.get_uuid ());
    apfs_block.set_attribute ("version", sb.get_version ());
    apfs_block.set_attribute ("volume_count", sb.get_volume_count ());
    apfs_block.set_attribute ("xp_data_base", sb.get_xp_data_base ());
    apfs_block.set_attribute ("xp_data_blocks", sb.get_xp_data_blocks ());
    apfs_block.set_attribute ("xp_data_index", sb.get_xp_data_index ());
    apfs_block.set_attribute ("xp_data_len", sb.get_xp_data_len ());
    apfs_block.set_attribute ("xp_data_next", sb.get_xp_data_next ());
    apfs_block.set_attribute ("xp_desc_base", sb.get_xp_desc_base ());
    apfs_block.set_attribute ("xp_desc_blocks", sb.get_xp_desc_blocks ());
    apfs_block.set_attribute ("xp_desc_index", sb.get_xp_desc_index ());
    apfs_block.set_attribute ("xp_desc_len", sb.get_xp_desc_len ());
    apfs_block.set_attribute ("xp_desc_next", sb.get_xp_desc_next ());

    apfs_block.set_handled (true);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Create APFS volume blocks
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &volume : volumes)
    {
        auto volume_block = apfs_block.new_slice_block ("apfs.volume");

        volume_block.set_attribute (
            "allocated_block_count", volume.get_alloc_block_count ()
        );
        volume_block.set_attribute (
            "allocated_size",
            volume.get_alloc_block_count () * sb.get_block_size ()
        );
        volume_block.set_attribute (
            "checksum",
            "0x" + mobius::core::string::to_hex (volume.get_checksum (), 16)
        );
        volume_block.set_attribute (
            "cloneinfo_id_epoch", volume.get_cloneinfo_id_epoch ()
        );
        volume_block.set_attribute (
            "cloneinfo_id_xid", volume.get_cloneinfo_id_xid ()
        );
        volume_block.set_attribute ("crypto_flags", volume.get_crypto_flags ());
        volume_block.set_attribute (
            "crypto_key_os_version", volume.get_crypto_key_os_version ()
        );
        volume_block.set_attribute (
            "crypto_key_revision", volume.get_crypto_key_revision ()
        );
        volume_block.set_attribute (
            "crypto_major_version", volume.get_crypto_major_version ()
        );
        volume_block.set_attribute (
            "crypto_minor_version", volume.get_crypto_minor_version ()
        );
        volume_block.set_attribute (
            "crypto_persistent_class", volume.get_crypto_persistent_class ()
        );

        auto crypto_persistent_key = volume.get_crypto_persistent_key ();
        volume_block.set_attribute (
            "crypto_persistent_key",
            crypto_persistent_key ? "0x" + crypto_persistent_key.to_hexstring ()
                                  : ""
        );

        volume_block.set_attribute (
            "description", "APFS Volume - UUID: " + volume.get_uuid ()
        );

        volume_block.set_attribute ("er_state_oid", volume.get_er_state_oid ());
        volume_block.set_attribute (
            "extentref_tree_oid", volume.get_extentref_tree_oid ()
        );
        volume_block.set_attribute (
            "extentref_tree_type",
            "0x" + mobius::core::string::to_hex (
                       volume.get_extentref_tree_type (), 8
                   )
        );
        volume_block.set_attribute ("features", volume.get_features ());
        volume_block.set_attribute (
            "fext_tree_oid", volume.get_fext_tree_oid ()
        );
        volume_block.set_attribute (
            "fext_tree_type",
            "0x" +
                mobius::core::string::to_hex (volume.get_fext_tree_type (), 8)
        );
        volume_block.set_attribute ("flags", volume.get_flags ());
        volume_block.set_attribute (
            "flag_always_check_extentref",
            volume.get_flag_always_check_extentref ()
        );
        volume_block.set_attribute (
            "flag_case_insensitive", volume.get_flag_case_insensitive ()
        );
        volume_block.set_attribute (
            "flag_dataless_snaps", volume.get_flag_dataless_snaps ()
        );
        volume_block.set_attribute (
            "flag_hardlink_map_records", volume.get_flag_hardlink_map_records ()
        );
        volume_block.set_attribute (
            "flag_has_changed_encryption_key",
            volume.get_flag_has_changed_encryption_key ()
        );
        volume_block.set_attribute (
            "flag_incomplete_restore", volume.get_flag_incomplete_restore ()
        );
        volume_block.set_attribute (
            "flag_normalization_insensitive",
            volume.get_flag_normalization_insensitive ()
        );
        volume_block.set_attribute (
            "flag_onekey_encrypted", volume.get_flag_onekey_encrypted ()
        );
        volume_block.set_attribute (
            "flag_run_spilled_over_cleaner",
            volume.get_flag_run_spilled_over_cleaner ()
        );
        volume_block.set_attribute (
            "flag_spilled_over", volume.get_flag_spilled_over ()
        );
        volume_block.set_attribute (
            "flag_strict_atime", volume.get_flag_strict_atime ()
        );
        volume_block.set_attribute (
            "flag_supports_defrag", volume.get_flag_supports_defrag ()
        );
        volume_block.set_attribute (
            "flag_volgrp_system_ino_space",
            volume.get_flag_volgrp_system_ino_space ()
        );
        volume_block.set_attribute (
            "formatted_by_id", volume.get_formatted_by_id ()
        );
        volume_block.set_attribute (
            "formatted_by_time", volume.get_formatted_by_time ()
        );
        volume_block.set_attribute (
            "formatted_by_xid", volume.get_formatted_by_xid ()
        );
        volume_block.set_attribute ("group_id", volume.get_group_id ());
        volume_block.set_attribute (
            "incompatible_features", volume.get_incompatible_features ()
        );
        volume_block.set_attribute ("index", volume.get_index ());
        volume_block.set_attribute (
            "integrity_meta_oid", volume.get_integrity_meta_oid ()
        );
        volume_block.set_attribute ("is_encrypted", volume.is_encrypted ());
        volume_block.set_attribute (
            "is_sealed_volume", volume.is_sealed_volume ()
        );
        volume_block.set_attribute (
            "last_modification_time", volume.get_last_modification_time ()
        );
        volume_block.set_attribute (
            "last_modified_by_id", volume.get_last_modified_by_id ()
        );
        volume_block.set_attribute (
            "last_modified_by_time", volume.get_last_modified_by_time ()
        );
        volume_block.set_attribute (
            "last_modified_by_xid", volume.get_last_modified_by_xid ()
        );
        volume_block.set_attribute ("name", volume.get_name ());
        volume_block.set_attribute ("next_obj_id", volume.get_next_obj_id ());
        volume_block.set_attribute (
            "num_directories", volume.get_num_directories ()
        );
        volume_block.set_attribute ("num_files", volume.get_num_files ());
        volume_block.set_attribute (
            "num_other_fsobjects", volume.get_num_other_fsobjects ()
        );
        volume_block.set_attribute (
            "num_snapshots", volume.get_num_snapshots ()
        );
        volume_block.set_attribute ("num_symlinks", volume.get_num_symlinks ());
        volume_block.set_attribute ("oid", volume.get_oid ());
        volume_block.set_attribute ("omap_oid", volume.get_omap_oid ());
        volume_block.set_attribute (
            "quota_block_count", volume.get_quota_block_count ()
        );
        volume_block.set_attribute (
            "readonly_compatible_features",
            "0x" + mobius::core::string::to_hex (
                       volume.get_readonly_compatible_features (), 16
                   )
        );
        volume_block.set_attribute (
            "reserve_block_count", volume.get_reserve_block_count ()
        );
        volume_block.set_attribute ("reserved", volume.get_reserved ());
        volume_block.set_attribute ("reserved_oid", volume.get_reserved_oid ());
        volume_block.set_attribute (
            "reserved_type",
            "0x" + mobius::core::string::to_hex (volume.get_reserved_type (), 8)
        );
        volume_block.set_attribute (
            "revert_to_sblock_oid", volume.get_revert_to_sblock_oid ()
        );
        volume_block.set_attribute (
            "revert_to_xid", volume.get_revert_to_xid ()
        );

        volume_block.set_attribute ("role", volume.get_role ());
        volume_block.set_attribute ("root_to_xid", volume.get_root_to_xid ());
        volume_block.set_attribute (
            "root_tree_oid", volume.get_root_tree_oid ()
        );
        volume_block.set_attribute (
            "root_tree_type",
            "0x" +
                mobius::core::string::to_hex (volume.get_root_tree_type (), 8)
        );
        volume_block.set_attribute (
            "signature", volume.get_signature ().to_string ()
        );
        volume_block.set_attribute (
            "snap_meta_ext_oid", volume.get_snap_meta_ext_oid ()
        );
        volume_block.set_attribute (
            "snap_meta_tree_oid", volume.get_snap_meta_tree_oid ()
        );
        volume_block.set_attribute (
            "snap_meta_tree_type",
            "0x" + mobius::core::string::to_hex (
                       volume.get_snap_meta_tree_type (), 8
                   )
        );
        volume_block.set_attribute ("superblock_block", volume.get_block ());
        volume_block.set_attribute (
            "superblock_rel_offset", volume.get_offset ()
        );
        volume_block.set_attribute (
            "superblock_offset",
            block.get_attribute<std::int64_t> ("offset") + volume.get_offset ()
        );
        volume_block.set_attribute (
            "total_blocks_alloced", volume.get_total_blocks_alloced ()
        );
        volume_block.set_attribute (
            "total_blocks_freed", volume.get_total_blocks_freed ()
        );
        volume_block.set_attribute ("unmount_time", volume.get_unmount_time ());
        volume_block.set_attribute ("uuid", volume.get_uuid ());
        volume_block.set_attribute ("xid", volume.get_xid ());

        volume_block.set_handled (true);
    }

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
    mobius::core::vfs::register_block_decoder ("apfs", decoder);
    mobius::core::vfs::register_block_to_filesystems_function (
        "apfs.volume",
        mobius::extension::vfs::block::apfs::apfs_volume_block_to_filesystems
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Stop extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
stop ()
{
    mobius::core::vfs::unregister_block_decoder ("apfs");
    mobius::core::vfs::unregister_block_to_filesystems_function ("apfs.volume");
}
