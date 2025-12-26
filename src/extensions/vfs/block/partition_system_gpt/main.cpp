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
#include <cstdint>
#include <map>
#include <mobius/core/bytearray.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/resource.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/core/vfs/block.hpp>
#include <stdexcept>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Datatypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
using sector_type = std::uint64_t;
using sector_size_type = std::uint32_t;
using address_type = std::uint64_t;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string GPT_UNUSED_ENTRY =
    "00000000-0000-0000-0000-000000000000";
static const std::string GPT_MS_BASIC_DATA =
    "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7";
static const mobius::core::bytearray GPT_SIGNATURE = "EFI PART";
constexpr std::size_t MBR_PARTITION_OFFSET = 0x1be;
constexpr std::size_t MBR_SIGNATURE_OFFSET = 0x1fe;
constexpr std::uint8_t EFI_GPT_TYPE = 0xee;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Partition description by type GUID
// @see https://en.wikipedia.org/wiki/GUID_Partition_Table#Partition_type_GUIDs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::map<const std::string, const std::string>
    PARTITION_DESCRIPTION = {
        {"C12A7328-F81F-11D2-BA4B-00A0C93EC93B", "EFI System partition"},
        {"21686148-6449-6E6F-744E-656564454649", "BIOS boot partition"},
        {"D3BFE2DE-3DAF-11DF-BA40-E3A556D89593",
         "Intel Fast Flash (iFFS) partition"},
        {"F4019732-066E-4E12-8273-346C5641494F", "Sony boot partition"},
        {"BFBFAFE7-A34F-448A-9A5B-6213EB736C22", "Lenovo boot partition"},
        {"E3C9E316-0B5C-4DB8-817D-F92DF00215AE",
         "Microsoft Reserved Partition"},
        {"EBD0A0A2-B9E5-4433-87C0-68B6B72699C7", "Basic data partition"},
        {"5808C8AA-7E8F-42E0-85D2-E1E90434CFB3",
         "Logical Disk Manager (LDM) metadata partition"},
        {"AF9B60A0-1431-4F62-BC68-3311714A69AD",
         "Logical Disk Manager data partition"},
        {"DE94BBA4-06D1-4D40-A16A-BFD50179D6AC",
         "Windows Recovery Environment"},
        {"0FC63DAF-8483-4772-8E79-3D69D8477DE4", "Linux filesystem data"},
        {"A19D880F-05FC-4D3B-A006-743F0F84911E", "Linux RAID partition"},
        {"4F68BCE3-E8CD-4DB1-96E7-FBCAF984B709",
         "Linux Root partition (x86-64)"},
        {"44479540-F297-41B2-9AF7-D131D5F0458A", "Linux Root partition (x86)"},
        {"0657FD6D-A4AB-43C4-84E5-0933C84B4F4F", "Linux Swap partition"},
        {"E6D6D379-F507-44C2-A23C-238F2A3DF928",
         "Logical Volume Manager (LVM) partition"},
        {"933AC7E1-2EB4-4F13-B844-0E14E2AEF915", "Linux /home partition"},
        {"3B8F8425-20E0-4F3B-907F-1A25A76F98E8",
         "Linux /srv (server data) partition"},
        {"BC13C2FF-59E6-4262-A352-B275FD6F7172", "Linux /boot partition"},
        {"CA7D7CCB-63ED-4C53-861C-1742536059CC", "LUKS partition"},
        {"48465300-0000-11AA-AA11-00306543ECAC",
         "Hierarchical File System Plus (HFS+) partition"},
        {"7C3457EF-0000-11AA-AA11-00306543ECAC", "APFS container"},
        {"55465300-0000-11AA-AA11-00306543ECAC", "Apple UFS container"},
        {"6A898CC3-1DD2-11B2-99A6-080020736631", "ZFS"},
        {"52414944-0000-11AA-AA11-00306543ECAC", "Apple RAID partition"},
        {"52414944-5F4F-11AA-AA11-00306543ECAC",
         "Apple RAID partition, offline"},
        {"426F6F74-0000-11AA-AA11-00306543ECAC",
         "Apple Boot partition (Recovery HD)"},
        {"4C616265-6C00-11AA-AA11-00306543ECAC", "Apple Label"},
        {"5265636F-7665-11AA-AA11-00306543ECAC", "Apple TV Recovery partition"},
        {"53746F72-6167-11AA-AA11-00306543ECAC",
         "Apple Core Storage Container"},
        {"69646961-6700-11AA-AA11-00306543ECAC",
         "Apple APFS Preboot partition"},
        {"52637672-7900-11AA-AA11-00306543ECAC",
         "Apple APFS Recovery partition"},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode protective MBR
// @param ps_block Partition System block
// @param sector_size Sector size
// @see UEFI 2.9 - section 5.3.2
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_decode_protective_mbr (mobius::core::vfs::block &ps_block,
                        sector_size_type sector_size)
{
    auto reader = ps_block.new_reader ();
    auto data = reader.read (sector_size);

    /*if (data.size () == sector_size &&

        // MBR signature
        data[MBR_SIGNATURE_OFFSET] == 0x55 &&
        data[MBR_SIGNATURE_OFFSET + 1] == 0xaa &&

        // First partition must be protective. The others, must be empty
        data[MBR_PARTITION_OFFSET+4] == EFI_GPT_TYPE &&
        data[MBR_PARTITION_OFFSET+16+4] == 0x00 &&
        data[MBR_PARTITION_OFFSET+32+4] == 0x00 &&
        data[MBR_PARTITION_OFFSET+48+4] == 0x00
       )
      {*/
    auto block = ps_block.new_slice_block ("mbr", 0, sector_size - 1);
    block.set_handled (true);

    block.set_attribute ("start_address", 0);
    block.set_attribute ("end_address", sector_size - 1);
    block.set_attribute ("size", sector_size);
    block.set_attribute ("start_sector", 0);
    block.set_attribute ("end_sector", 0);
    block.set_attribute ("sectors", 1);
    block.set_attribute ("description", "Protective MBR");
    //}
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode GPT header
// @param ps_block Partition System block
// @param sector_size Sector size
// @param sector Header sector
// @return Block object
// @see UEFI 2.9 - section 5.3.2
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::vfs::block
_decode_gpt_header (mobius::core::vfs::block &ps_block,
                    sector_size_type sector_size, sector_type sector)
{
    mobius::core::vfs::block block;

    try
    {
        address_type pos = sector * sector_size;
        auto reader = ps_block.new_reader ();
        reader.seek (pos);
        mobius::core::decoder::data_decoder decoder (reader);
        auto signature = decoder.get_bytearray_by_size (8);

        if (signature == GPT_SIGNATURE)
        {
            block = ps_block.new_slice_block ("gpt.header", pos,
                                              pos + sector_size - 1);
            block.set_handled (true);

            block.set_attribute ("start_sector", sector);
            block.set_attribute ("end_sector", sector);
            block.set_attribute ("sectors", 1);
            block.set_attribute ("start_address", pos);
            block.set_attribute ("end_address", pos + sector_size - 1);
            block.set_attribute ("size", sector_size);

            block.set_attribute ("signature", signature.to_string ());
            auto revision_l = decoder.get_uint16_le ();
            auto revision_h = decoder.get_uint16_le ();
            block.set_attribute ("revision", std::to_string (revision_h) + '.' +
                                                 std::to_string (revision_l));

            block.set_attribute ("header_size", decoder.get_uint32_le ());
            block.set_attribute ("crc32", decoder.get_uint32_le ());
            decoder.skip (4); // reserved
            block.set_attribute ("lba", decoder.get_uint64_le ());
            block.set_attribute ("alternate_lba", decoder.get_uint64_le ());
            block.set_attribute ("first_usable_lba", decoder.get_uint64_le ());
            block.set_attribute ("last_usable_lba", decoder.get_uint64_le ());
            block.set_attribute ("disk_guid", decoder.get_guid ());
            block.set_attribute ("partition_entry_lba",
                                 decoder.get_uint64_le ());
            block.set_attribute ("number_of_partition_entries",
                                 decoder.get_uint32_le ());
            block.set_attribute ("size_of_partition_entry",
                                 decoder.get_uint32_le ());
            block.set_attribute ("partition_entry_array_crc32",
                                 decoder.get_uint32_le ());
        }
    }
    catch (const std::exception &e)
    {
    }

    return block;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create gpt.table block from GPT Header block
// @param ps_block Partition System block
// @param header_block Header block
// @param sector_size Sector size
// @return Block
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::vfs::block
_create_gpt_partition_table (mobius::core::vfs::block &ps_block,
                             const mobius::core::vfs::block &header_block,
                             sector_size_type sector_size)
{
    mobius::core::vfs::block block;

    if (header_block)
    {
        std::uint32_t number_of_partition_entries =
            header_block.get_attribute<std::int64_t> (
                "number_of_partition_entries");
        std::uint32_t size_of_partition_entry =
            header_block.get_attribute<std::int64_t> (
                "size_of_partition_entry");
        std::uint64_t partition_entry_lba =
            header_block.get_attribute<std::int64_t> ("partition_entry_lba");
        sector_type sectors =
            (number_of_partition_entries * size_of_partition_entry +
             sector_size - 1) /
            sector_size;
        address_type start_address = partition_entry_lba * sector_size;
        address_type end_address =
            (partition_entry_lba + sectors) * sector_size - 1;

        block =
            ps_block.new_slice_block ("gpt.table", start_address, end_address);
        block.set_handled (true);

        block.set_attribute ("start_sector", partition_entry_lba);
        block.set_attribute ("end_sector", partition_entry_lba + sectors - 1);
        block.set_attribute ("sectors", sectors);
        block.set_attribute ("start_address", start_address);
        block.set_attribute ("end_address", end_address);
        block.set_attribute ("size", sectors * sector_size);
        block.set_attribute ("entry_size", size_of_partition_entry);
        block.set_attribute ("entries", number_of_partition_entries);
    }

    return block;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode GPT partition table blocks
// @param ps_block Partition System block
// @param partition_entry_lbaheader_block Header block
// @param sector_size Sector size
// @return Block
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::uint32_t
_decode_gpt_partition_table (mobius::core::vfs::block &ps_block,
                             const mobius::core::vfs::block &header_block,
                             sector_size_type sector_size)
{
    std::uint32_t partitions = 0;
    sector_type partition_entry_lba =
        header_block.get_attribute<std::int64_t> ("partition_entry_lba");
    std::uint32_t number_of_partition_entries =
        header_block.get_attribute<std::int64_t> (
            "number_of_partition_entries");

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode partition entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto reader = ps_block.new_reader ();
    reader.seek (partition_entry_lba * sector_size);
    mobius::core::decoder::data_decoder decoder (reader);

    for (std::uint32_t i = 0; i < number_of_partition_entries; i++)
    {
        auto type_guid = decoder.get_guid ();

        if (type_guid == GPT_UNUSED_ENTRY)
            decoder.skip (112);

        else
        {
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Decode partition entry
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            auto unique_guid = decoder.get_guid ();
            auto starting_lba = decoder.get_uint64_le ();
            auto ending_lba = decoder.get_uint64_le ();
            auto attributes = decoder.get_uint64_le ();
            auto name = decoder.get_string_by_size (72, "utf-16");
            address_type start_address = starting_lba * sector_size;
            address_type end_address = (ending_lba + 1) * sector_size - 1;
            bool is_bootable = attributes & 0x00000004;
            bool is_hidden = false;
            bool is_readable = true;
            bool is_writeable = true;

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // If partition is GPT_MS_BASIC_DATA, set attributes
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            if (type_guid == GPT_MS_BASIC_DATA)
            {
                is_writeable = attributes ^ 0x10000000;
                is_hidden = attributes & 0x40000000;
            }

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Create flags
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            std::string flags;
            if (is_bootable)
                flags += 'B';
            if (is_hidden)
                flags += 'H';

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Create type description
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            std::string type_description;
            auto iterator = PARTITION_DESCRIPTION.find (type_guid);

            if (iterator != PARTITION_DESCRIPTION.end ())
                type_description = iterator->second;

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Create description
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            std::string description = "Partition #" + std::to_string (i + 1);

            if (!name.empty ())
                description += " - " + name;

            else if (!type_description.empty ())
                description += " - " + type_description;

            else
                description += " - Type: " + type_guid;

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Create partition block
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            auto block = ps_block.new_slice_block ("partition", start_address,
                                                   end_address);

            block.set_attribute ("start_sector", starting_lba);
            block.set_attribute ("end_sector", ending_lba);
            block.set_attribute ("sectors", ending_lba - starting_lba + 1);
            block.set_attribute ("start_address", start_address);
            block.set_attribute ("end_address", end_address);
            block.set_attribute ("size", end_address - start_address + 1);
            block.set_attribute ("type_guid", type_guid);
            block.set_attribute ("type_description", type_description);
            block.set_attribute ("unique_guid", unique_guid);
            block.set_attribute ("name", name);
            block.set_attribute ("description", description);
            block.set_attribute ("attributes",
                                 mobius::core::string::to_hex (attributes, 8));
            block.set_attribute ("is_bootable", is_bootable);
            block.set_attribute ("is_readable", is_readable);
            block.set_attribute ("is_writable", is_writeable);
            block.set_attribute ("is_hidden", is_hidden);
            block.set_attribute ("flags", flags);

            partitions++;
        }
    }

    return partitions;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode GPT partition system
// @param block Block object
// @param new_blocks Vector for newly created blocks
// @param sector_size Sector size in bytes
// @see UEFI 2.9 - section 5.3.2
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_decode_gpt (const mobius::core::vfs::block &block,
             std::vector<mobius::core::vfs::block> &new_blocks,
             sector_size_type sector_size)
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Create partition system block
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto ps_block =
        mobius::core::vfs::new_slice_block (block, "partition_system");
    std::uint64_t sectors = (block.get_size () + sector_size - 1) / sector_size;

    ps_block.set_attribute ("sector_size", sector_size);
    ps_block.set_attribute ("sectors", sectors);
    ps_block.set_attribute ("ps_type", "GPT");
    ps_block.set_handled (true);
    new_blocks.push_back (ps_block);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Create protective MBR
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    _decode_protective_mbr (ps_block, sector_size);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Create GPT headers
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto primary_header_block = _decode_gpt_header (ps_block, sector_size, 1);
    auto backup_header_block =
        _decode_gpt_header (ps_block, sector_size, sectors - 1);
    mobius::core::vfs::block header_block;

    if (primary_header_block)
    {
        primary_header_block.set_attribute ("description",
                                            "Primary GPT Header");
        header_block = primary_header_block;
    }

    if (backup_header_block)
    {
        backup_header_block.set_attribute ("description", "Backup GPT Header");

        if (!header_block)
            header_block = backup_header_block;
    }

    if (!header_block)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("no valid GPT header found"));

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Set ps_block attributes from GPT Header
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto disk_guid = header_block.get_attribute<std::string> ("disk_guid");

    ps_block.set_attribute (
        "revision", header_block.get_attribute<std::string> ("revision"));
    ps_block.set_attribute ("disk_guid", disk_guid);
    ps_block.set_attribute (
        "first_usable_lba",
        header_block.get_attribute<std::int64_t> ("first_usable_lba"));
    ps_block.set_attribute (
        "last_usable_lba",
        header_block.get_attribute<std::int64_t> ("last_usable_lba"));
    ps_block.set_attribute ("description",
                            "GPT partition system - Disk GUID: " + disk_guid);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Create GPT partition tables
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto primary_partition_table = _create_gpt_partition_table (
        ps_block, primary_header_block, sector_size);
    auto backup_partition_table = _create_gpt_partition_table (
        ps_block, backup_header_block, sector_size);

    if (primary_partition_table)
    {
        primary_partition_table.set_attribute ("description",
                                               "Primary GPT Partition Table");
    }

    if (backup_partition_table)
    {
        backup_partition_table.set_attribute ("description",
                                              "Backup GPT Partition Table");
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Create GPT partitions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t partitions =
        _decode_gpt_partition_table (ps_block, header_block, sector_size);
    ps_block.set_attribute ("number_of_partitions", partitions);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add freespace blocks
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    ps_block.add_freespaces ();

    for (auto &b : ps_block.get_children ())
    {
        if (b.get_type () == "freespace")
        {
            address_type start_address =
                static_cast<std::int64_t> (b.get_attribute ("start_address"));
            address_type end_address =
                static_cast<std::int64_t> (b.get_attribute ("end_address"));
            b.set_attribute ("start_sector", start_address / sector_size);
            b.set_attribute ("end_sector", end_address / sector_size);
            b.set_attribute ("sectors",
                             (end_address - start_address + 1) / sector_size);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Try to decode block as GPT partition system
// @param block Block object
// @param new_blocks Vector for newly created blocks
// @param pending_blocks Pending blocks
// @return <b>true</b> if block was decoded, <b>false</b> otherwise
// @see UEFI 2.9
//
// GPT Disk Structure:
// Sector       Description
// 0            Protective MBR
// 1            Primary GPT Header
// 2+n          Primary GPT Partition table
// ...          Partitions
// -2-n         Backup GPT Partition table
// -1           Backup GPT Header
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static bool
decoder (const mobius::core::vfs::block &block,
         std::vector<mobius::core::vfs::block> &new_blocks,
         std::vector<mobius::core::vfs::block> &)
{
    auto reader = block.new_reader ();
    bool rc = false;

    // Test for both 512 and 4096 bytes sector sizes
    for (sector_size_type sector_size : {512, 4096})
    {
        bool is_detected = false;

        // Try to detect Primary GPT header
        reader.seek (sector_size);
        auto data = reader.read (8);

        if (data == GPT_SIGNATURE)
            is_detected = true;

        // If not found, try to detect Backup GPT header
        else if (reader.get_size () >= sector_size * 2)
        {
            reader.seek (reader.get_size () - sector_size);
            data = reader.read (8);

            if (data == GPT_SIGNATURE)
                is_detected = true;
        }

        // If GPT header is detected, decode GPT Partition System
        if (is_detected)
        {
            _decode_gpt (block, new_blocks, sector_size);
            rc = true;
        }
    }

    return rc;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Extension data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C"
{
    const char *EXTENSION_ID = "vfs.block.partition_system_gpt";
    const char *EXTENSION_NAME = "Partition System: GPT";
    const char *EXTENSION_VERSION = "1.1";
    const char *EXTENSION_AUTHORS = "Eduardo Aguiar";
    const char *EXTENSION_DESCRIPTION = "GPT partition table support";
} // extern "C"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Start extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
start ()
{
    mobius::core::add_resource (
        "vfs.block.decoder.partition_system_gpt",
        "GPT partition system block decoder",
        static_cast<mobius::core::vfs::block_decoder_resource_type> (decoder));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Stop extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
stop ()
{
    mobius::core::remove_resource ("vfs.block.decoder.partition_system_gpt");
}
