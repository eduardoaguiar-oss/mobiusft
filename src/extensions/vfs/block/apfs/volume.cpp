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
#include "volume.hpp"
#include <unordered_map>
#include "common.hpp"
#include "object.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::uint32_t OBJECT_TYPE_FS = 0x0000000D;
static constexpr std::size_t APFS_MODIFIED_NAMELEN = 32;
static constexpr std::size_t APFS_VOLNAME_LEN = 256;
static constexpr std::size_t APFS_MAX_HIST = 8;

static constexpr std::uint64_t APFS_FS_UNENCRYPTED = 0x00000001;
static constexpr std::uint64_t APFS_FS_ONEKEY = 0x00000008;
static constexpr std::uint64_t APFS_FS_SPILLEDOVER = 0x00000010;
static constexpr std::uint64_t APFS_FS_RUN_SPILLOVER_CLEANER = 0x00000020;
static constexpr std::uint64_t APFS_FS_ALWAYS_CHECK_EXTENTREF = 0x00000040;

// @see Apple File System Reference - Volume Roles (pg. 63)
static const std::unordered_map<std::uint16_t, std::string> ROLES = {
    {0x0001, "System"},     {0x0002, "User"},     {0x0004, "Recovery"},
    {0x0008, "VM"},         {0x0010, "Preboot"},  {0x0020, "Installer"},
    {0x0040, "Data"},       {0x0080, "Baseband"}, {0x00c0, "Update"},
    {0x0100, "XArt"},       {0x0140, "Hardware"}, {0x0180, "Backup"},
    {0x0200, "Enterprise"}, {0x0280, "Prelogin"}
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get role string from role value
// @param role Role value
// @return Role string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
get_role_string (std::uint16_t role)
{
    auto it = ROLES.find (role);

    if (it != ROLES.end ())
        return it->second;

    return {};
}

} // namespace

namespace mobius::extension::vfs::block::apfs
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Volume constructor
// @param decoder Data decoder object
// @param paddr Physical block address of the volume
// @param block_size Block size of the container
// @see Apple File System Reference - Volumes (pg. 51)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
volume::volume (
    mobius::core::decoder::data_decoder &decoder,
    std::uint64_t paddr,
    std::uint64_t block_size
)
{
    decoder.seek (paddr * block_size);
    offset_ = decoder.tell ();
    block_ = paddr;

    // Read object header
    object obj (decoder);

    // Check for a valid checksum
    if (obj.get_checksum () == 0 || obj.get_checksum () == 0xffffffffffffffff)
        return;

    // Check if it's a volume
    if (obj.get_type () != OBJECT_TYPE_FS)
        return;

    checksum_ = obj.get_checksum ();
    oid_ = obj.get_oid ();
    xid_ = obj.get_xid ();

    // Check signature
    signature_ = decoder.get_bytearray_by_size (4);

    if (signature_ != "APSB")
        return;

    // Decode fields
    index_ = decoder.get_uint32_le ();
    features_ = decoder.get_uint64_le ();
    readonly_compatible_features_ = decoder.get_uint64_le ();
    incompatible_features_ = decoder.get_uint64_le ();
    unmount_time_ = decoder.get_apfs_datetime ();
    reserve_block_count_ = decoder.get_uint64_le ();
    quota_block_count_ = decoder.get_uint64_le ();
    alloc_block_count_ = decoder.get_uint64_le ();

    // Decode wrapped_crypto_state_t fields
    // @see Apple File System Reference - Wrapped Crypto State (pg. 138)
    crypto_major_version_ = decoder.get_uint16_le ();
    crypto_minor_version_ = decoder.get_uint16_le ();
    crypto_flags_ = decoder.get_uint32_le ();
    crypto_persistent_class_ = decoder.get_uint32_le ();

    // Decode key OS version
    // @see Apple File System Reference - cp_key_os_version_t (pg. 141)
    auto key_os_version = decoder.get_uint32_le ();
    crypto_key_os_version_ = std::to_string (key_os_version >> 24) +
                             std::string (1, (key_os_version >> 16) & 0xff) +
                             std::to_string (key_os_version & 0xffff);

    crypto_key_revision_ = decoder.get_uint16_le ();

    auto key_len = decoder.get_uint16_le ();
    crypto_persistent_key_ = decoder.get_bytearray_by_size (key_len);

    // Decode more fields
    root_tree_type_ = decoder.get_uint32_le ();
    extentref_tree_type_ = decoder.get_uint32_le ();
    snap_meta_tree_type_ = decoder.get_uint32_le ();
    omap_oid_ = decoder.get_uint64_le ();
    root_tree_oid_ = decoder.get_uint64_le ();
    extentref_tree_oid_ = decoder.get_uint64_le ();
    snap_meta_tree_oid_ = decoder.get_uint64_le ();
    revert_to_xid_ = decoder.get_uint64_le ();
    revert_to_sblock_oid_ = decoder.get_uint64_le ();
    next_obj_id_ = decoder.get_uint64_le ();
    num_files_ = decoder.get_uint64_le ();
    num_directories_ = decoder.get_uint64_le ();
    num_symlinks_ = decoder.get_uint64_le ();
    num_other_fsobjects_ = decoder.get_uint64_le ();
    num_snapshots_ = decoder.get_uint64_le ();
    total_blocks_alloced_ = decoder.get_uint64_le ();
    total_blocks_freed_ = decoder.get_uint64_le ();
    uuid_ = decoder.get_uuid ();
    last_modification_time_ = decoder.get_apfs_datetime ();
    flags_ = decoder.get_uint64_le ();
    formatted_by_id_ = decoder.get_string_by_size (APFS_MODIFIED_NAMELEN);
    formatted_by_time_ = decoder.get_apfs_datetime ();
    formatted_by_xid_ = decoder.get_uint64_le ();
    last_modified_by_id_ = decoder.get_string_by_size (APFS_MODIFIED_NAMELEN);
    last_modified_by_time_ = decoder.get_apfs_datetime ();
    last_modified_by_xid_ = decoder.get_uint64_le ();

    decoder.skip (
        (APFS_MODIFIED_NAMELEN + 16) * (APFS_MAX_HIST - 1)
    ); // Skip remaining history entries

    name_ = decoder.get_string_by_size (APFS_VOLNAME_LEN);
    next_doc_id_ = decoder.get_uint32_le ();
    role_ = get_role_string (decoder.get_uint16_le ());

    // Decode more fields
    reserved_ = decoder.get_uint16_le ();
    root_to_xid_ = decoder.get_uint64_le ();
    er_state_oid_ = decoder.get_uint64_le ();
    cloneinfo_id_epoch_ = decoder.get_uint64_le ();
    cloneinfo_id_xid_ = decoder.get_uint64_le ();
    snap_meta_ext_oid_ = decoder.get_uint64_le ();
    group_id_ = decoder.get_uuid ();
    integrity_meta_oid_ = decoder.get_uint64_le ();
    fext_tree_oid_ = decoder.get_uint64_le ();
    fext_tree_type_ = decoder.get_uint32_le ();
    reserved_type_ = decoder.get_uint32_le ();
    reserved_oid_ = decoder.get_uint64_le ();

    // Test checksum
    decoder.seek (offset_);
    auto data = decoder.get_bytearray_by_size (block_size);

    if (!test_checksum (data))
        return;

    // Derived attributes from features
    flag_hardlink_map_records_ = (features_ & 0x00000002) != 0;
    flag_supports_defrag_ = (features_ & 0x00000004) != 0;
    flag_strict_atime_ = (features_ & 0x00000008) != 0;
    flag_volgrp_system_ino_space_ = (features_ & 0x00000010) != 0;

    // Derived attributes from incompatible features
    flag_case_insensitive_ = (incompatible_features_ & 0x00000001) != 0;
    flag_dataless_snaps_ = (incompatible_features_ & 0x00000002) != 0;
    flag_has_changed_encryption_key_ =
        (incompatible_features_ & 0x00000004) != 0;
    flag_normalization_insensitive_ =
        (incompatible_features_ & 0x00000008) != 0;
    flag_incomplete_restore_ = (incompatible_features_ & 0x00000010) != 0;
    is_sealed_volume_ = (incompatible_features_ & 0x00000020) != 0;

    // Derived attributes from flags
    is_encrypted_ = (flags_ & APFS_FS_UNENCRYPTED) == 0;
    flag_onekey_encrypted_ = (flags_ & APFS_FS_ONEKEY) != 0;
    flag_spilled_over_ = (flags_ & APFS_FS_SPILLEDOVER) != 0;
    flag_run_spilled_over_cleaner_ =
        (flags_ & APFS_FS_RUN_SPILLOVER_CLEANER) != 0;
    flag_always_check_extentref_ =
        (flags_ & APFS_FS_ALWAYS_CHECK_EXTENTREF) != 0;

    // If we reached this point, the volume is valid
    is_valid_ = true;
}

} // namespace mobius::extension::vfs::block::apfs
