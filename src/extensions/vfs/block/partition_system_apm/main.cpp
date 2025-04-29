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
#include <mobius/decoder/data_decoder.h>
#include <mobius/string_functions.h>
#include <mobius/core/vfs/block.hpp>
#include <map>

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
constexpr std::uint64_t BLOCK0_SIZE = 512;
constexpr std::uint16_t BLOCK0_SIGNATURE = 0x4552;
constexpr std::uint16_t DPME_SIGNATURE = 0x504D;
constexpr std::uint32_t DPME_FLAGS_VALID = 0x00000001;
constexpr std::uint32_t DPME_FLAGS_ALLOCATED = 0x00000002;
constexpr std::uint32_t DPME_FLAGS_IN_USE = 0x00000004;
constexpr std::uint32_t DPME_FLAGS_BOOTABLE = 0x00000008;
constexpr std::uint32_t DPME_FLAGS_READABLE = 0x00000010;
constexpr std::uint32_t DPME_FLAGS_WRITABLE = 0x00000020;
constexpr std::uint32_t DPME_FLAGS_OS_PIC_CODE = 0x00000040;
constexpr std::uint32_t DPME_FLAGS_OS_SPECIFIC_2 = 0x00000080;
constexpr std::uint32_t DPME_FLAGS_OS_SPECIFIC_1 = 0x00000100;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Partition description by dpme_type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::map <const std::string, const std::string> PARTITION_DESCRIPTION =
{
  {"Apple_Boot", "Boot partition"},
  {"Apple_Free",  "Freespace"},
  {"Apple_HFS",  "HFS partition"},
  {"Apple_HFSX", "HFS+ partition"},
  {"Apple_partition_map",  "Partition Map"},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Try to decode block as APM partition system
// @param block Block object
// @param new_blocks Vector for newly created blocks
// @param pending_blocks Pending blocks
// @return <b>true</b> if block was decoded, <b>false</b> otherwise
// @see https://opensource.apple.com/source/IOStorageFamily/IOStorageFamily-116/IOApplePartitionScheme.h.auto.html
// @see https://formats.kaitai.io/apm_partition_table/
// @see https://github.com/libyal/libvsapm/blob/main/documentation/Apple%20partition%20map%20(APM)%20format.asciidoc
// @see https://en.wikipedia.org/wiki/Apple_Partition_Map
// @see File System Forensic Analysis, "Apple Partitions" section
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static bool
decoder (
  const mobius::core::vfs::block& block,
  std::vector <mobius::core::vfs::block>& new_blocks,
  std::vector <mobius::core::vfs::block>&
)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Try to decode Block0 (Driver Descriptor Map)
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto reader = block.new_reader ();
  auto decoder = mobius::decoder::data_decoder (reader);

  // block size must be at least Block0 size
  if (decoder.get_size () < BLOCK0_SIZE)
    return false;

  // check signature
  auto sb_sig = decoder.get_uint16_be ();

  if (sb_sig != BLOCK0_SIGNATURE)
    return false;

  // block size must be a non-zero multiplier of Block0 size
  auto sb_blk_size = decoder.get_uint16_be ();

  if (sb_blk_size < BLOCK0_SIZE || (sb_blk_size % BLOCK0_SIZE))
    return false;

  // decode Block0 metadata
  auto sb_blk_count = decoder.get_uint32_be ();
  auto sb_dev_type = decoder.get_uint16_be ();
  auto sb_dev_id = decoder.get_uint16_be ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Try to detect DPME (Driver Partition Map Entry #1)
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  decoder.seek (sb_blk_size);

  // check DPME signature
  auto dpme_signature = decoder.get_uint16_be ();

  if (dpme_signature != DPME_SIGNATURE)
    return false;

  // decode DPME
  decoder.skip (2);              // dpme_reserved_1
  auto dpme_map_entries = decoder.get_uint32_be ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create partition system block
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto ps_block = mobius::core::vfs::new_slice_block (block, "partition_system");
  sector_size_type sector_size = sb_blk_size;
  sector_type sectors = (block.get_size () + sector_size - 1) / sector_size;

  ps_block.set_attribute ("sector_size", sector_size);
  ps_block.set_attribute ("sectors", sectors);
  ps_block.set_attribute ("description", "APM partition system");
  ps_block.set_attribute ("ps_type", "APM");
  ps_block.set_handled (true);
  new_blocks.push_back (ps_block);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create Driver Descriptor Map
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto ddm_block = ps_block.new_slice_block ("apm.ddm", 0, BLOCK0_SIZE - 1);
  ddm_block.set_attribute ("signature", "0x" + mobius::string::to_hex (sb_sig, 4));
  ddm_block.set_attribute ("block_size", sb_blk_size);
  ddm_block.set_attribute ("block_count", sb_blk_count);
  ddm_block.set_attribute ("device_type", sb_dev_type);
  ddm_block.set_attribute ("device_id", sb_dev_id);
  ddm_block.set_attribute ("start_address", 0);
  ddm_block.set_attribute ("end_address", BLOCK0_SIZE - 1);
  ddm_block.set_attribute ("size", BLOCK0_SIZE);
  ddm_block.set_attribute ("start_sector", 0);
  ddm_block.set_attribute ("end_sector", 0);
  ddm_block.set_attribute ("sectors", 1);
  ddm_block.set_attribute ("description", "Driver Description Map");
  ddm_block.set_handled (true);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create Partitions
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (std::uint32_t i = 1;i <= dpme_map_entries;i++)
    {
      decoder.seek (i * sector_size);
      decoder.skip (8);
      auto dpme_pblock_start = decoder.get_uint32_be ();
      auto dpme_pblocks = decoder.get_uint32_be ();
      auto dpme_name = decoder.get_string_by_size (32);
      auto dpme_type = decoder.get_string_by_size (32);

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Decode remaining data
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      decoder.skip (8);             // dpme_lblock_start, dpme_lblocks
      auto dpme_flags = decoder.get_uint32_be ();
      decoder.skip (28);
      auto dpme_process_id = decoder.get_string_by_size (16);

      bool is_bootable = dpme_flags & DPME_FLAGS_BOOTABLE;
      bool is_readable = dpme_flags & DPME_FLAGS_READABLE;
      bool is_writeable = dpme_flags & DPME_FLAGS_WRITABLE;

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Create block
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      auto start_address = dpme_pblock_start * sector_size;
      auto end_address = start_address + dpme_pblocks * sector_size - 1;
      const std::string block_type = (dpme_type == "Apple_Free") ? "freespace" : "partition";
      auto pm_block = ps_block.new_slice_block (block_type, start_address, end_address);

      if (dpme_type == "Apple_partition_map")
        pm_block.set_handled (true);

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Type description
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      std::string type_description;
      auto iterator = PARTITION_DESCRIPTION.find (dpme_type);

      if (iterator != PARTITION_DESCRIPTION.end ())
        type_description = iterator->second;

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Description
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      std::string description = "Partition #" + std::to_string (i);

      if (!dpme_name.empty ())
        description += " - " + dpme_name;

      else if (!type_description.empty ())
        description += " - " + type_description;

      else
        description += " - " + dpme_type;

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Flags
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      std::string flags;
      if (is_bootable) flags += 'B';
      if (is_readable) flags += 'R';
      if (is_writeable) flags += 'W';

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Set attributes
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      pm_block.set_attribute ("start_address", start_address);
      pm_block.set_attribute ("end_address", end_address);
      pm_block.set_attribute ("size", dpme_pblocks * sector_size);
      pm_block.set_attribute ("start_sector", dpme_pblock_start);
      pm_block.set_attribute ("end_sector", dpme_pblock_start + dpme_pblocks - 1);
      pm_block.set_attribute ("sectors", dpme_pblocks);
      pm_block.set_attribute ("name", dpme_name);
      pm_block.set_attribute ("type", dpme_type);
      pm_block.set_attribute ("type_description", type_description);
      pm_block.set_attribute ("processor", dpme_process_id);
      pm_block.set_attribute ("description", description);
      pm_block.set_attribute ("is_bootable", is_bootable);
      pm_block.set_attribute ("is_readable", is_readable);
      pm_block.set_attribute ("is_writable", is_writeable);
      pm_block.set_attribute ("flags", flags);
    }

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
const char *EXTENSION_ID = "vfs.block.partition_system_apm";
const char *EXTENSION_NAME = "Partition System: APM";
const char *EXTENSION_VERSION = "1.1";
const char *EXTENSION_AUTHORS = "Eduardo Aguiar";
const char *EXTENSION_DESCRIPTION = "Apple Partition Map partition table support";
} // extern "C"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Start extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
start ()
{
  mobius::core::add_resource (
     "vfs.block.decoder.partition_system_apm",
     "APM partition system block decoder",
     static_cast <mobius::core::vfs::block_decoder_resource_type> (decoder)
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Stop extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
stop ()
{
  mobius::core::remove_resource ("vfs.block.decoder.partition_system_apm");
}


