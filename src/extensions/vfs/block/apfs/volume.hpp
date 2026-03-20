#ifndef MOBIUS_EXTENSION_VFS_BLOCK_APFS_VOLUME_HPP
#define MOBIUS_EXTENSION_VFS_BLOCK_APFS_VOLUME_HPP

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
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <cstdint>
#include <vector>

namespace mobius::extension::vfs::block::apfs
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Volume class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class volume
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    volume () = default;
    volume (mobius::core::decoder::data_decoder &, std::uint64_t);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if volume is valid
    // @return <b>true</b> if volume is valid, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    constexpr
    operator bool () const noexcept
    {
        return is_valid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume checksum
    // @return Checksum
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_checksum () const noexcept
    {
        return checksum_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume OID
    // @return OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_oid () const noexcept
    {
        return oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume XID
    // @return XID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_xid () const noexcept
    {
        return xid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume offset
    // @return Offset in bytes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_offset () const noexcept
    {
        return offset_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume signature
    // @return Signature
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::bytearray
    get_signature () const noexcept
    {
        return signature_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume index
    // @return Index
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_index () const noexcept
    {
        return index_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume features
    // @return Features
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_features () const noexcept
    {
        return features_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume readonly compatible features
    // @return Readonly compatible features
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_readonly_compatible_features () const noexcept
    {
        return readonly_compatible_features_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume incompatible features
    // @return Incompatible features
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_incompatible_features () const noexcept
    {
        return incompatible_features_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume unmount time
    // @return Unmount time as datetime object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_unmount_time () const
    {
        return unmount_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume reserve block count
    // @return Reserve block count
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_reserve_block_count () const noexcept
    {
        return reserve_block_count_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume quota block count
    // @return Quota block count
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_quota_block_count () const noexcept
    {
        return quota_block_count_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume alloc block count
    // @return Alloc block count
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_alloc_block_count () const noexcept
    {
        return alloc_block_count_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume crypto major version
    // @return Crypto major version
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint16_t
    get_crypto_major_version () const noexcept
    {
        return crypto_major_version_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume crypto minor version
    // @return Crypto minor version
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint16_t
    get_crypto_minor_version () const noexcept
    {
        return crypto_minor_version_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume crypto flags
    // @return Crypto flags
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_crypto_flags () const noexcept
    {
        return crypto_flags_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume crypto persistent class
    // @return Crypto persistent class
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_crypto_persistent_class () const noexcept
    {
        return crypto_persistent_class_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume crypto key OS version
    // @return Crypto key OS version
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_crypto_key_os_version () const
    {
        return crypto_key_os_version_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume crypto key revision
    // @return Crypto key revision
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_crypto_key_revision () const noexcept
    {
        return crypto_key_revision_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume crypto persistent key
    // @return Crypto persistent key
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::bytearray
    get_crypto_persistent_key () const
    {
        return crypto_persistent_key_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume root tree type
    // @return Root tree type
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_root_tree_type () const noexcept
    {
        return root_tree_type_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume extentref tree type
    // @return Extentref tree type
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_extentref_tree_type () const noexcept
    {
        return extentref_tree_type_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume snap meta tree type
    // @return Snap meta tree type
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_snap_meta_tree_type () const noexcept
    {
        return snap_meta_tree_type_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume OMAP OID
    // @return OMAP OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_omap_oid () const noexcept
    {
        return omap_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume root tree OID
    // @return Root tree OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_root_tree_oid () const noexcept
    {
        return root_tree_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume extentref tree OID
    // @return Extentref tree OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_extentref_tree_oid () const noexcept
    {
        return extentref_tree_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume snap meta tree OID
    // @return Snap meta tree OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_snap_meta_tree_oid () const noexcept
    {
        return snap_meta_tree_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume revert to XID
    // @return Revert to XID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_revert_to_xid () const noexcept
    {
        return revert_to_xid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get revert to superblock OID
    // @return Revert to superblock OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_revert_to_sblock_oid () const noexcept
    {
        return revert_to_sblock_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get next object ID
    // @return Next object ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_next_obj_id () const noexcept
    {
        return next_obj_id_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of files
    // @return Number of files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_num_files () const noexcept
    {
        return num_files_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of directories
    // @return Number of directories
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_num_directories () const noexcept
    {
        return num_directories_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of symlinks
    // @return Number of symlinks
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_num_symlinks () const noexcept
    {
        return num_symlinks_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of other fsobjects
    // @return Number of other fsobjects
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_num_other_fsobjects () const noexcept
    {
        return num_other_fsobjects_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of snapshots
    // @return Number of snapshots
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_num_snapshots () const noexcept
    {
        return num_snapshots_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get total blocks allocated
    // @return Total blocks allocated
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_total_blocks_alloced () const noexcept
    {
        return total_blocks_alloced_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get total blocks freed
    // @return Total blocks freed
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_total_blocks_freed () const noexcept
    {
        return total_blocks_freed_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get UUID
    // @return UUID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_uuid () const
    {
        return uuid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get last modification time
    // @return Last modification time as datetime object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_last_modification_time () const
    {
        return last_modification_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume flags
    // @return Volume flags
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_flags () const noexcept
    {
        return flags_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get formatted by ID
    // @return Formatted by ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_formatted_by_id () const
    {
        return formatted_by_id_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get formatted by time
    // @return Formatted by time as datetime object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_formatted_by_time () const
    {
        return formatted_by_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get formatted by XID
    // @return Formatted by XID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_formatted_by_xid () const noexcept
    {
        return formatted_by_xid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get last modified by ID
    // @return Last modified by ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_last_modified_by_id () const
    {
        return last_modified_by_id_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get last modified by time
    // @return Last modified by time as datetime object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_last_modified_by_time () const
    {
        return last_modified_by_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get last modified by XID
    // @return Last modified by XID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_last_modified_by_xid () const noexcept
    {
        return last_modified_by_xid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume name
    // @return Volume name
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_name () const
    {
        return name_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get next document ID
    // @return Next document ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_next_doc_id () const noexcept
    {
        return next_doc_id_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get volume role
    // @return Volume role
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_role () const
    {
        return role_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get reserved field
    // @return Reserved field
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint16_t
    get_reserved () const noexcept
    {
        return reserved_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get root to XID
    // @return Root to XID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_root_to_xid () const noexcept
    {
        return root_to_xid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get ER state OID
    // @return ER state OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_er_state_oid () const noexcept
    {
        return er_state_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get cloneinfo ID epoch
    // @return Cloneinfo ID epoch
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_cloneinfo_id_epoch () const noexcept
    {
        return cloneinfo_id_epoch_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get cloneinfo ID XID
    // @return Cloneinfo ID XID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_cloneinfo_id_xid () const noexcept
    {
        return cloneinfo_id_xid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get snap meta ext OID
    // @return Snap meta ext OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_snap_meta_ext_oid () const noexcept
    {
        return snap_meta_ext_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get group ID
    // @return Group ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_group_id () const
    {
        return group_id_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get integrity meta OID
    // @return Integrity meta OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_integrity_meta_oid () const noexcept
    {
        return integrity_meta_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get file extent tree OID
    // @return File extent tree OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_fext_tree_oid () const noexcept
    {
        return fext_tree_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get file extent tree type
    // @return File extent tree type
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_fext_tree_type () const noexcept
    {
        return fext_tree_type_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get reserved type
    // @return Reserved type
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_reserved_type () const noexcept
    {
        return reserved_type_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get reserved OID
    // @return Reserved OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_reserved_oid () const noexcept
    {
        return reserved_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if volume has hardlink map records
    // @return <b>true</b> if volume has hardlink map records, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    get_flag_hardlink_map_records () const noexcept
    {
        return flag_hardlink_map_records_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if volume supports defrag
    // @return <b>true</b> if volume supports defrag, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    get_flag_supports_defrag () const noexcept
    {
        return flag_supports_defrag_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if volume has strict atime
    // @return <b>true</b> if volume has strict atime, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    get_flag_strict_atime () const noexcept
    {
        return flag_strict_atime_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if volume has volgrp system ino space
    // @return <b>true</b> if volume has volgrp system ino space, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    get_flag_volgrp_system_ino_space () const noexcept
    {
        return flag_volgrp_system_ino_space_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if volume is case insensitive
    // @return <b>true</b> if volume is case insensitive, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    get_flag_case_insensitive () const noexcept
    {
        return flag_case_insensitive_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if volume has dataless snaps
    // @return <b>true</b> if volume has dataless snaps, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    get_flag_dataless_snaps () const noexcept
    {
        return flag_dataless_snaps_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if volume has changed encryption key
    // @return <b>true</b> if volume has changed encryption key, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    get_flag_has_changed_encryption_key () const noexcept
    {
        return flag_has_changed_encryption_key_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if volume is normalization insensitive
    // @return <b>true</b> if volume is normalization insensitive, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    get_flag_normalization_insensitive () const noexcept
    {
        return flag_normalization_insensitive_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if volume has incomplete restore
    // @return <b>true</b> if volume has incomplete restore, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    get_flag_incomplete_restore () const noexcept
    {
        return flag_incomplete_restore_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if volume is sealed
    // @return <b>true</b> if volume is sealed, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_sealed_volume () const noexcept
    {
        return is_sealed_volume_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if volume is encrypted
    // @return <b>true</b> if volume is encrypted, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_encrypted () const noexcept
    {
        return is_encrypted_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if volume is onekey encrypted
    // @return <b>true</b> if volume is onekey encrypted, </b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    get_flag_onekey_encrypted () const noexcept
    {
        return flag_onekey_encrypted_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if volume is spilled over
    // @return <b>true</b> if volume is spilled over, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    get_flag_spilled_over () const noexcept
    {
        return flag_spilled_over_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if volume is run spilled over cleaner
    // @return <b>true</b> if volume is run spilled over cleaner, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    get_flag_run_spilled_over_cleaner () const noexcept
    {
        return flag_run_spilled_over_cleaner_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if volume always check extentref
    // @return <b>true</b> if volume always check extentref, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    get_flag_always_check_extentref () const noexcept
    {
        return flag_always_check_extentref_;
    }

  private:
    // Indicate if superblock is valid
    bool is_valid_ = false;

    // Object attributes
    std::uint64_t checksum_;
    std::uint64_t oid_;
    std::uint64_t xid_;

    // Attributes
    std::uint64_t offset_;
    mobius::core::bytearray signature_; // 4 bytes
    std::uint32_t index_;
    std::uint64_t features_;
    std::uint64_t readonly_compatible_features_;
    std::uint64_t incompatible_features_;
    mobius::core::datetime::datetime unmount_time_;
    std::uint64_t reserve_block_count_;
    std::uint64_t quota_block_count_;
    std::uint64_t alloc_block_count_;

    // From wrapped_crypto_state_t
    std::uint16_t crypto_major_version_;
    std::uint16_t crypto_minor_version_;
    std::uint32_t crypto_flags_ = 0;
    std::uint32_t crypto_persistent_class_;
    std::string crypto_key_os_version_;
    std::uint16_t crypto_key_revision_;
    mobius::core::bytearray crypto_persistent_key_;

    // More attributes
    std::uint32_t root_tree_type_;
    std::uint32_t extentref_tree_type_;
    std::uint32_t snap_meta_tree_type_;
    std::uint64_t omap_oid_;
    std::uint64_t root_tree_oid_;
    std::uint64_t extentref_tree_oid_;
    std::uint64_t snap_meta_tree_oid_;
    std::uint64_t revert_to_xid_;
    std::uint64_t revert_to_sblock_oid_;
    std::uint64_t next_obj_id_;
    std::uint64_t num_files_;
    std::uint64_t num_directories_;
    std::uint64_t num_symlinks_;
    std::uint64_t num_other_fsobjects_;
    std::uint64_t num_snapshots_;
    std::uint64_t total_blocks_alloced_;
    std::uint64_t total_blocks_freed_;
    std::string uuid_;
    mobius::core::datetime::datetime last_modification_time_;
    std::uint64_t flags_;
    std::string formatted_by_id_;
    mobius::core::datetime::datetime formatted_by_time_;
    std::uint64_t formatted_by_xid_;
    std::string last_modified_by_id_;
    mobius::core::datetime::datetime last_modified_by_time_;
    std::uint64_t last_modified_by_xid_;
    std::string name_;
    std::uint32_t next_doc_id_;
    std::string role_;
    std::uint16_t reserved_;
    std::uint64_t root_to_xid_;
    std::uint64_t er_state_oid_;
    std::uint64_t cloneinfo_id_epoch_;
    std::uint64_t cloneinfo_id_xid_;
    std::uint64_t snap_meta_ext_oid_;
    std::string group_id_;
    std::uint64_t integrity_meta_oid_;
    std::uint64_t fext_tree_oid_;
    std::uint32_t fext_tree_type_;
    std::uint32_t reserved_type_;
    std::uint64_t reserved_oid_;

    // Derived attributes from features
    bool flag_hardlink_map_records_ = false;
    bool flag_supports_defrag_ = false;
    bool flag_strict_atime_ = false;
    bool flag_volgrp_system_ino_space_ = false;

    // Derived attributes from incompatible features
    bool flag_case_insensitive_ = false;
    bool flag_dataless_snaps_ = false;
    bool flag_has_changed_encryption_key_ = false;
    bool flag_normalization_insensitive_ = false;
    bool flag_incomplete_restore_ = false;
    bool is_sealed_volume_ = false;

    // Derived attributes from flags
    bool is_encrypted_ = false;
    bool flag_onekey_encrypted_ = false;
    bool flag_spilled_over_ = false;
    bool flag_run_spilled_over_cleaner_ = false;
    bool flag_always_check_extentref_ = false;
};

} // namespace mobius::extension::vfs::block::apfs

#endif // MOBIUS_EXTENSION_VFS_BLOCK_APFS_VOLUME_HPP
