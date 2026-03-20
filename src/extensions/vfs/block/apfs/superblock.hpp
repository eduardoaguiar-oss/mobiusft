#ifndef MOBIUS_EXTENSION_VFS_BLOCK_APFS_SUPERBLOCK_HPP
#define MOBIUS_EXTENSION_VFS_BLOCK_APFS_SUPERBLOCK_HPP

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
#include <mobius/core/decoder/data_decoder.hpp>
#include <cstdint>
#include <vector>

namespace mobius::extension::vfs::block::apfs
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Superblock class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class superblock
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    superblock () = default;
    superblock (mobius::core::decoder::data_decoder &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if superblock is valid
    // @return <b>true</b> if superblock is valid, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    constexpr
    operator bool () const noexcept
    {
        return is_valid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get superblock checksum
    // @return Checksum
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_checksum () const noexcept
    {
        return checksum_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get offset
    // @return Offset in bytes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_offset () const noexcept
    {
        return offset_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get object ID
    // @return Object ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_oid () const noexcept
    {
        return oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get transaction ID
    // @return Transaction ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_xid () const noexcept
    {
        return xid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get signature
    // @return Signature
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::bytearray
    get_signature () const
    {
        return signature_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get block size
    // @return Block size in bytes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_block_size () const noexcept
    {
        return block_size_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get block count
    // @return Block count
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_block_count () const noexcept
    {
        return block_count_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get features
    // @return Features
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_features () const noexcept
    {
        return features_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get readonly features
    // @return Readonly features
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_readonly_features () const noexcept
    {
        return readonly_features_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get incompatible features
    // @return Incompatible features
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_incompatible_features () const noexcept
    {
        return incompatible_features_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get UUID
    // @return UUID string
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_uuid () const
    {
        return uuid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get next object ID
    // @return Next object ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_next_oid () const noexcept
    {
        return next_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get next transaction ID
    // @return Next transaction ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_next_xid () const noexcept
    {
        return next_xid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of blocks used by the checkpoint descriptor area
    // @return Number of blocks used by the checkpoint descriptor area
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_xp_desc_blocks () const noexcept
    {
        return xp_desc_blocks_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of blocks used by the checkpoint data area
    // @return Number of blocks used by the checkpoint data area
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_xp_data_blocks () const noexcept
    {
        return xp_data_blocks_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get checkpoint descriptor area base offset
    // @return Checkpoint descriptor area base offset in bytes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_xp_desc_base () const noexcept
    {
        return xp_desc_base_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get checkpoint data area base offset
    // @return Checkpoint data area base offset in bytes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_xp_data_base () const noexcept
    {
        return xp_data_base_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get checkpoint descriptor area next index
    // @return Checkpoint descriptor area next index
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_xp_desc_next () const noexcept
    {
        return xp_desc_next_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get checkpoint data area next index
    // @return Checkpoint data area next index
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_xp_data_next () const noexcept
    {
        return xp_data_next_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get checkpoint descriptor area next index
    // @return Checkpoint descriptor area next index
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_xp_desc_index () const noexcept
    {
        return xp_desc_index_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get checkpoint descriptor area length
    // @return Checkpoint descriptor area length in bytes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_xp_desc_len () const noexcept
    {
        return xp_desc_len_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get checkpoint data area next index
    // @return Checkpoint data area next index
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_xp_data_index () const noexcept
    {
        return xp_data_index_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get checkpoint data area length
    // @return Checkpoint data area length in bytes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_xp_data_len () const noexcept
    {
        return xp_data_len_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get spaceman OID
    // @return Spaceman OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_spaceman_oid () const noexcept
    {
        return spaceman_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get object map OID
    // @return Object map OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_omap_oid () const noexcept
    {
        return omap_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get reaper OID
    // @return Reaper OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_reaper_oid () const noexcept
    {
        return reaper_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get test type
    // @return Test type
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_test_type () const noexcept
    {
        return test_type_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get max file systems
    // @return Max file systems
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_max_file_systems () const noexcept
    {
        return max_file_systems_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get FS OIDs
    // @return FS OIDs
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<std::uint64_t>
    get_file_system_oids () const
    {
        return file_system_oids_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get checksum set count
    // @return Checksum set count
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_checksum_set_count () const noexcept
    {
        return checksum_set_count_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get checksum fail count
    // @return Checksum fail count
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_checksum_fail_count () const noexcept
    {
        return checksum_fail_count_;
    }
    
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get blocked out address
    // @return Blocked out address in bytes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_blocked_out_addr () const noexcept
    {
        return blocked_out_addr_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get blocked out count
    // @return Blocked out count
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_blocked_out_count () const noexcept
    {
        return blocked_out_count_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get evict mapping tree OID
    // @return Evict mapping tree OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_evict_mapping_tree_oid () const noexcept
    {
        return evict_mapping_tree_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get flags
    // @return Superblock flags
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_flags () const noexcept
    {
        return flags_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get EFI jumpstart
    // @return EFI jumpstart
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_efi_jumpstart () const noexcept
    {
        return efi_jumpstart_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get fusion UUID
    // @return Fusion UUID string
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_fusion_uuid () const
    {
        return fusion_uuid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get keylocker address
    // @return Keylocker address in bytes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_keylocker_addr () const noexcept
    {
        return keylocker_addr_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get keylocker count
    // @return Keylocker count
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_keylocker_count () const noexcept
    {
        return keylocker_count_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get test OID
    // @return Test OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_test_oid () const noexcept
    {
        return test_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get fusion media tree OID
    // @return Fusion media tree OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_fusion_mt_oid () const noexcept
    {
        return fusion_mt_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get fusion WBC OID
    // @return Fusion WBC OID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_fusion_wbc_oid () const noexcept
    {
        return fusion_wbc_oid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get fusion WBC address
    // @return Fusion WBC address in bytes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_fusion_wbc_addr () const noexcept
    {
        return fusion_wbc_addr_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get fusion WBC count
    // @return Fusion WBC count
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_fusion_wbc_count () const noexcept
    {
        return fusion_wbc_count_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get newest mounted version
    // @return Newest mounted version
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_newest_mounted_version () const noexcept
    {
        return newest_mounted_version_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get MKB locker address
    // @return MKB locker address in bytes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_mkb_locker_addr () const noexcept
    {
        return mkb_locker_addr_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get MKB locker count
    // @return MKB locker count
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_mkb_locker_count () const noexcept
    {
        return mkb_locker_count_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get size
    // @return Size in bytes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_size () const noexcept
    {
        return size_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get version
    // @return Version
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_version () const noexcept
    {
        return version_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if superblock supports fusion
    // @return <b>true</b> if superblock supports fusion, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    get_flag_supports_fusion () const noexcept
    {
        return flag_supports_fusion_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if superblock supports defrag
    // @return <b>true</b> if superblock supports defrag, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    get_flag_supports_defrag () const noexcept
    {
        return flag_supports_defrag_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if superblock has LCFD flag set
    // @return <b>true</b> if superblock has LCFD flag set, <b>false</b> otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    get_flag_lcfd () const noexcept
    {
        return flag_lcfd_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of volumes
    // @return Number of volumes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::size_t
    get_volume_count () const noexcept
    {
        return file_system_oids_.size ();
    }

  private:
    // Indicate if superblock is valid
    bool is_valid_ = false;

    // Object attributes
    std::uint64_t checksum_ = 0;
    std::uint64_t oid_ = 0;
    std::uint64_t xid_ = 0;
    std::uint64_t offset_;

    // Superblock attributes
    mobius::core::bytearray signature_; // 4 bytes
    std::uint32_t block_size_;
    std::uint64_t block_count_;
    std::uint64_t features_;
    std::uint64_t readonly_features_;
    std::uint64_t incompatible_features_;
    std::string uuid_;
    std::uint64_t next_oid_;
    std::uint64_t next_xid_;
    std::uint32_t xp_desc_blocks_;
    std::uint32_t xp_data_blocks_;
    std::uint64_t xp_desc_base_;
    std::uint64_t xp_data_base_;
    std::uint32_t xp_desc_next_;
    std::uint32_t xp_data_next_;
    std::uint32_t xp_desc_index_;
    std::uint32_t xp_desc_len_;
    std::uint32_t xp_data_index_;
    std::uint32_t xp_data_len_;
    std::uint64_t spaceman_oid_;
    std::uint64_t omap_oid_;
    std::uint64_t reaper_oid_;
    std::uint32_t test_type_;
    std::uint32_t max_file_systems_;
    std::vector<std::uint64_t> file_system_oids_;
    std::uint64_t checksum_set_count_;
    std::uint64_t checksum_fail_count_;
    std::uint64_t blocked_out_addr_;
    std::uint64_t blocked_out_count_;
    std::uint64_t evict_mapping_tree_oid_;
    std::uint64_t flags_;
    std::uint64_t efi_jumpstart_;
    std::string fusion_uuid_;
    std::uint64_t keylocker_addr_;
    std::uint64_t keylocker_count_;
    std::vector<std::uint64_t> ephemeral_info_;
    std::uint64_t test_oid_;
    std::uint64_t fusion_mt_oid_;
    std::uint64_t fusion_wbc_oid_;
    std::uint64_t fusion_wbc_addr_;
    std::uint64_t fusion_wbc_count_;
    std::uint64_t newest_mounted_version_;
    std::uint64_t mkb_locker_addr_;
    std::uint64_t mkb_locker_count_;

    // Derived attributes
    std::uint64_t size_ = 0;

    // Derived attributes from features
    bool flag_supports_defrag_ = false;
    bool flag_lcfd_ = false;

    // Derived attributes from incompatible features
    std::uint32_t version_ = 0;
    bool flag_supports_fusion_ = false;
};

} // namespace mobius::extension::vfs::block::apfs

#endif // MOBIUS_EXTENSION_VFS_BLOCK_APFS_SUPERBLOCK_HPP
