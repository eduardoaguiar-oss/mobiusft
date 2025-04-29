// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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
#include <mobius/core/resource.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/string_functions.h>
#include <mobius/core/vfs/block.hpp>
#include <map>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
using sector_type = std::uint64_t;
using sector_size_type = std::uint32_t;
using address_type = std::uint64_t;

constexpr sector_size_type SECTOR_SIZE = 512;
constexpr sector_size_type MBR_SIZE = 512;
constexpr std::size_t MBR_PARTITION_OFFSET = 0x1be;
constexpr std::size_t MBR_SIGNATURE_OFFSET = 0x1fe;
constexpr std::uint8_t EFI_GPT_TYPE = 0xee;
constexpr std::uint16_t DISK_COPY_PROTECTED = 0x5a5a;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Partition description by type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::map <std::uint8_t, const std::string> PARTITION_DESCRIPTION =
{
  {0x01, "FAT-12"},
  {0x04, "FAT-16 (<32 MB)"},
  {0x05, "Extended (CHS)"},
  {0x06, "FAT-16B"},
  {0x07, "NTFS/HPFS"},
  {0x0b, "FAT-32 (CHS)"},
  {0x0c, "FAT-32 (LBA)"},
  {0x0e, "FAT-16 (LBA)"},
  {0x0f, "Extended (LBA)"},
  {0x11, "Hidden FAT-12"},
  {0x12, "Hibernation/firmware"},
  {0x14, "Hidden FAT-16 (<32 MB)"},
  {0x15, "Hidden extended (CHS)"},
  {0x16, "Hidden FAT-16B"},
  {0x17, "Hidden NTFS/HPFS"},
  {0x1b, "Hidden FAT-32 (CHS)"},
  {0x1c, "Hidden FAT-32 (LBA)"},
  {0x1e, "Hidden FAT-16 (LBA)"},
  {0x1f, "Hidden extended (LBA)"},
  {0x27, "Windows Recovery Environment"},
  {0x82, "Linux swap space"},
  {0x83, "Linux"},
  {0x85, "Linux extended"},
  {0x86, "Linux RAID"},
  {0x8e, "Linux LVM"},
  {0xe8, "Luks"},
  {0xee, "GPT protective MBR"},
  {0xef, "EFI system"}
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert CHS information to string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
chs_to_string (const mobius::bytearray& chs)
{
  std::uint16_t h = chs[0];
  std::uint16_t s = (chs[1] & 0x3f) + 1;
  std::uint16_t c = (static_cast <std::uint16_t> (chs[1] & 0xc0) << 2) | chs[2];

  return '(' + std::to_string (c) + ',' + std::to_string (h) + ',' + std::to_string (s) + ')';
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if block has an instance of DOS partition system
// @param block Block object
// @return true/false
// @see Linux source code: block/partitions/msdos.c
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static bool
_is_instance (const mobius::core::vfs::block& block)
{
  auto reader = block.new_reader ();
  auto data = reader.read (SECTOR_SIZE);

  return
    data.size () >= MBR_SIZE &&

    // MBR signature
    data[MBR_SIGNATURE_OFFSET] == 0x55 &&
    data[MBR_SIGNATURE_OFFSET + 1] == 0xaa &&

    // Boot indicators must be either 0x00 or 0x80
    (data[MBR_PARTITION_OFFSET] & 0x7f) == 0x00 &&
    (data[MBR_PARTITION_OFFSET+16] & 0x7f) == 0x00 &&
    (data[MBR_PARTITION_OFFSET+32] & 0x7f) == 0x00 &&
    (data[MBR_PARTITION_OFFSET+48] & 0x7f) == 0x00 &&

    // It must have at least one valid non protective partition
    (
      (data[MBR_PARTITION_OFFSET+4] != EFI_GPT_TYPE &&
       data[MBR_PARTITION_OFFSET+4] != 0x00) ||
      (data[MBR_PARTITION_OFFSET+16+4] != EFI_GPT_TYPE &&
       data[MBR_PARTITION_OFFSET+16+4] != 0x00) ||
      (data[MBR_PARTITION_OFFSET+32+4] != EFI_GPT_TYPE &&
       data[MBR_PARTITION_OFFSET+32+4] != 0x00) ||
      (data[MBR_PARTITION_OFFSET+48+4] != EFI_GPT_TYPE &&
       data[MBR_PARTITION_OFFSET+48+4] != 0x00)
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan partitions
// @param ps_block Partition system block
// @param sector_size Sector size in bytes
// @param sector MBR/EMBR sector
// @see Linux source code: block/partitions/msdos.c
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_scan_partitions (
  mobius::core::vfs::block& ps_block,
  sector_size_type sector_size,
  sector_type sector = 0
)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create MBR entry
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  address_type start_address = sector * sector_size;
  address_type end_address = (sector + 1) * sector_size - 1;

  auto mbr_block = ps_block.new_slice_block ((sector == 0) ? "mbr" : "embr", start_address, end_address);
  mbr_block.set_attribute ("start_sector", sector);
  mbr_block.set_attribute ("end_sector", sector);
  mbr_block.set_attribute ("sectors", 1);
  mbr_block.set_attribute ("start_address", start_address);
  mbr_block.set_attribute ("end_address", end_address);
  mbr_block.set_attribute ("size", sector_size);
  mbr_block.set_attribute ("description",
                            (sector == 0) ? "Master Boot Record (MBR)" : "Extended Master Boot Record"
                          );
  mbr_block.set_handled (true);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Read MBR
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto reader = mbr_block.new_reader ();
  mobius::core::decoder::data_decoder decoder (reader);
  decoder.skip (440);   // bootcode
  bool is_copy_protected = false;

  if (sector == 0)
    {
      auto disk_id = "0x" + mobius::string::to_hex (decoder.get_uint32_le (), 8);
      is_copy_protected = decoder.get_uint16_le () == DISK_COPY_PROTECTED;
      ps_block.set_attribute ("disk_id", disk_id);
      ps_block.set_attribute ("is_copy_protected", is_copy_protected);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create partition blocks
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  decoder.seek (MBR_PARTITION_OFFSET);

  std::vector <sector_type> extended_list;

  for (int i = 0; i < 4; i++)
    {
      auto drive_index = decoder.get_uint8 ();
      auto start_chs = decoder.get_bytearray_by_size (3);
      auto type = decoder.get_uint8 ();
      auto end_chs = decoder.get_bytearray_by_size (3);
      auto starting_sector = decoder.get_uint32_le () + sector;
      auto sectors = decoder.get_uint32_le ();

      if (sectors)
        {
          address_type start_address = static_cast <sector_type> (starting_sector) * sector_size;
          address_type end_address = static_cast <sector_type> (starting_sector + sectors) * sector_size - 1;
          bool is_bootable = (drive_index & 0x80);
          bool is_primary = (sector == 0);
          bool is_extended = (type == 0x05 || type == 0x0f || type == 0x15 || type == 0x1f || type == 0x85);
          bool is_logical = (!is_primary && !is_extended);
          bool is_hidden = bool (type == 0x14 || type == 0x15 || type == 0x16 || type == 0x17 || type == 0x1b || type == 0x1c || type == 0x1e || type == 0x1f);

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Flags
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          std::string flags;
          if (is_bootable) flags += 'B';
          if (is_primary) flags += 'P';
          if (is_extended) flags += 'E';
          if (is_logical) flags += 'L';
          if (is_hidden) flags += 'H';

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Partition type description
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          std::string type_description;

          auto iterator = PARTITION_DESCRIPTION.find (type);

          if (iterator != PARTITION_DESCRIPTION.end ())
            type_description = iterator->second;

          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // If partition is extended, add to extended_list. Otherwise,
          // create partition block.
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          if (is_extended)
            extended_list.push_back (starting_sector);

          else
            {
              auto partition_block = ps_block.new_slice_block ("partition", start_address, end_address);

              partition_block.set_attribute ("start_sector", starting_sector);
              partition_block.set_attribute ("end_sector", starting_sector + sectors - 1);
              partition_block.set_attribute ("sectors", sectors);
              partition_block.set_attribute ("start_address", start_address);
              partition_block.set_attribute ("end_address", end_address);
              partition_block.set_attribute ("size", end_address - start_address + 1);
              partition_block.set_attribute ("is_bootable", is_bootable);
              partition_block.set_attribute ("is_primary", is_primary);
              partition_block.set_attribute ("is_extended", is_extended);
              partition_block.set_attribute ("is_logical", is_logical);
              partition_block.set_attribute ("is_hidden", is_hidden);
              partition_block.set_attribute ("is_readable", true);
              partition_block.set_attribute ("is_writeable", !is_copy_protected);
              partition_block.set_attribute ("type", "0x" + mobius::string::to_hex (type, 2));
              partition_block.set_attribute ("type_description", type_description);
              partition_block.set_attribute ("drive_index", drive_index);
              partition_block.set_attribute ("start_chs", chs_to_string (start_chs));
              partition_block.set_attribute ("end_chs", chs_to_string (end_chs));
              partition_block.set_attribute ("flags", flags);
            }
        }
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Scan extended partitions
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (auto sector : extended_list)
    _scan_partitions (ps_block, sector_size, sector);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Try to decode block as DOS partition system
// @param block Block object
// @param new_blocks Vector for newly created blocks
// @param pending_blocks Pending blocks
// @return <b>true</b> if block was decoded, <b>false</b> otherwise
// @see Linux source code: block/partitions/msdos.c
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static bool
decoder (
  const mobius::core::vfs::block& block,
  std::vector <mobius::core::vfs::block>& new_blocks,
  std::vector <mobius::core::vfs::block>&
)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Check if block is an instance of DOS partition system
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (!_is_instance (block))
    return false;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create partition system block
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto ps_block = mobius::core::vfs::new_slice_block (block, "partition_system");
  std::uint32_t sector_size = SECTOR_SIZE;
  std::uint64_t sectors = (block.get_size () + SECTOR_SIZE - 1) / SECTOR_SIZE;

  ps_block.set_attribute ("sector_size", SECTOR_SIZE);
  ps_block.set_attribute ("sectors", sectors);
  ps_block.set_attribute ("ps_type", "DOS");
  ps_block.set_handled (true);
  new_blocks.push_back (ps_block);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create partitions, freespaces, mbr/embr blocks
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  _scan_partitions (ps_block, sector_size);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Format partitions description
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  int i = 1;

  for (auto& b : ps_block.get_children ())
    {
      if (b.get_type () == "partition")
        {
          std::string type_description = static_cast <std::string> (b.get_attribute ("type_description"));
          std::string type = static_cast <std::string> (b.get_attribute ("type"));
          std::string description = "Partition #" + std::to_string (i) + " - ";

          if (type_description.empty ())
            description += "Type: " + type;

          else
            description += type_description;

          b.set_attribute ("description", description);
          ++i;
        }
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Set ps_block description
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto disk_id = ps_block.get_attribute <std::string> ("disk_id");
  ps_block.set_attribute ("description", "DOS partition system - Disk ID: " + disk_id);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Add freespace blocks
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  ps_block.add_freespaces ();

  for (auto& b : ps_block.get_children ())
    {
      if (b.get_type () == "freespace")
        {
          address_type start_address = static_cast <std::int64_t> (b.get_attribute ("start_address"));
          address_type end_address = static_cast <std::int64_t> (b.get_attribute ("end_address"));
          b.set_attribute ("start_sector", start_address / sector_size);
          b.set_attribute ("end_sector", end_address / sector_size);
          b.set_attribute ("sectors", (end_address - start_address + 1) / sector_size);
        }
    }

  return true;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Extension data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C"
{
const char *EXTENSION_ID = "vfs.block.partition_system_dos";
const char *EXTENSION_NAME = "Partition System: DOS";
const char *EXTENSION_VERSION = "1.1";
const char *EXTENSION_AUTHORS = "Eduardo Aguiar";
const char *EXTENSION_DESCRIPTION = "DOS partition table support";
} // extern "C"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Start extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
start ()
{
  mobius::core::add_resource (
     "vfs.block.decoder.partition_system_dos",
     "DOS partition system block decoder",
     static_cast <mobius::core::vfs::block_decoder_resource_type> (decoder)
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Stop extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
stop ()
{
  mobius::core::remove_resource ("vfs.block.decoder.partition_system_dos");
}


