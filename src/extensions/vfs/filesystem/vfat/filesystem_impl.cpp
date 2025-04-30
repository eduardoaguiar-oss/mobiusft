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
#include "filesystem_impl.hpp"
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/string_functions.hpp>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
constexpr std::uint32_t SECTOR_SIZE = 512;
constexpr std::uint32_t MAX_CLUSTERS_FAT12 = 0xff4;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if reader contains a VFAT boot sector
// @param reader Reader object
// @param offset Offset from the beginning of the stream
// @return True/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static bool
check_vfat_boot_sector (mobius::core::io::reader reader, std::uint64_t offset)
{
  reader.seek (offset);
  auto data = reader.read (512);

  constexpr int SECTOR_SIZE_OFFSET = 11;
  constexpr int RESERVED_SECTORS_OFFSET = 14;
  constexpr int FATS_OFFSET = 16;
  constexpr int MEDIA_TYPE_OFFSET = 21;
  constexpr int VFAT_SIGNATURE_OFFSET = 0x1fe;

  return
    data.size () >= 512 &&

    // Signature
    data[VFAT_SIGNATURE_OFFSET] == 0x55 &&
    data[VFAT_SIGNATURE_OFFSET + 1] == 0xaa &&

    // Sector size must be multiple of 512 bytes
    (data[SECTOR_SIZE_OFFSET] == 0x00 &&
     !(data[SECTOR_SIZE_OFFSET + 1] & 0x01) &&
      (data[SECTOR_SIZE_OFFSET + 1] & 0xfe)) &&

    // Reserved sectors must be != 0
    (data[RESERVED_SECTORS_OFFSET] ||
     data[RESERVED_SECTORS_OFFSET + 1]) &&

    // Number of FATs
    data[FATS_OFFSET] == 2 &&

    // Media type
    (data[MEDIA_TYPE_OFFSET] == 0xf0 ||
     data[MEDIA_TYPE_OFFSET] >= 0xf8);
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if stream contains an instance of vfat filesystem
// @param reader Reader object
// @param offset Offset from the beginning of the stream
// @return True/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
filesystem_impl::is_instance (mobius::core::io::reader reader, std::uint64_t offset)
{
  // check sector 0
  bool rc = check_vfat_boot_sector (reader, offset);

  // check sector 6 (FAT-32 usually has a backup boot sector there)
  if (!rc)
    rc = check_vfat_boot_sector (reader, offset + 6 * 512);

  // return result
  return rc;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param offset Offset from the beginning of volume
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
filesystem_impl::filesystem_impl (
  const mobius::core::io::reader& reader,
  size_type offset
)
 : reader_ (reader),
   offset_ (offset),
   tsk_adaptor_ (reader, offset)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get metadata item
// @param name Item name
// @return Data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
filesystem_impl::get_metadata (const std::string& name) const
{
  _load_data ();
  return metadata_.get (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get root folder
// @return Root folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::folder
filesystem_impl::get_root_folder () const
{
  return tsk_adaptor_.get_root_folder ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load data on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
filesystem_impl::_load_data () const
{
  if (data_loaded_)
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get boot sector offset
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::core::io::reader reader = reader_;
  std::uint64_t boot_offset = 0;

  if (check_vfat_boot_sector (reader, offset_))
    boot_offset = offset_;

  else if (check_vfat_boot_sector (reader, offset_ + 6 * SECTOR_SIZE))
    boot_offset = offset_ + 6 * SECTOR_SIZE;

  else          // force data decoding anyway
    boot_offset = offset_;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode FDC descriptor (ECMA 9.1)
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  reader.seek (boot_offset);
  mobius::core::decoder::data_decoder decoder (reader);

  decoder.skip (3);
  auto oem_name = mobius::core::string::rstrip (decoder.get_string_by_size (8));
  auto sector_size = decoder.get_uint16_le ();
  auto sectors_per_cluster = decoder.get_uint8 ();
  auto reserved_sectors = decoder.get_uint16_le ();
  auto fats = decoder.get_uint8 ();
  auto root_dir_entries = decoder.get_uint16_le ();
  std::uint32_t sectors = decoder.get_uint16_le ();
  auto media_descriptor = decoder.get_uint8 ();
  std::uint32_t sectors_per_fat = decoder.get_uint16_le ();
  auto sectors_per_track = decoder.get_uint16_le ();
  auto heads = decoder.get_uint16_le ();
  auto hidden_sectors = decoder.get_uint32_le ();
  auto sectors_32bit = decoder.get_uint32_le ();

  if (sectors == 0)
    sectors = sectors_32bit;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Calculate FAT entry size in bits
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint16_t fat_entry_size = 0;
  std::uint32_t total_clusters = (sectors - reserved_sectors) / sectors_per_cluster;

  if (sectors_per_fat)		// Either FAT-12 or FAT-16
    fat_entry_size = total_clusters > MAX_CLUSTERS_FAT12 ? 16 : 12;

  else				// FAT-32
    fat_entry_size = 32;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get FAT-32 info
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint16_t flags = 0;
  std::uint16_t version = 0;
  std::uint32_t root_dir_cluster = 0;
  std::uint16_t fs_info_sector = 0;
  std::uint16_t backup_boot_sector = 0;

  if (fat_entry_size == 32)
    {
      sectors_per_fat = decoder.get_uint32_le ();
      flags = decoder.get_uint16_le ();
      version = decoder.get_uint16_le ();
      root_dir_cluster = decoder.get_uint32_le ();
      fs_info_sector = decoder.get_uint16_le ();
      backup_boot_sector = decoder.get_uint16_le ();
      decoder.skip (12);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode Extended BPB fields
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string volume_id;
  std::string volume_label;
  std::string filesystem_type;

  auto logical_drive_number = decoder.get_uint8 ();
  bool is_dirty = (decoder.get_uint8 () == 0x01);
  auto extended_signature = decoder.get_uint8 ();

  if (extended_signature)
    {
      volume_id = "0x" + mobius::core::string::to_hex (decoder.get_uint32_le (), 8);
      volume_label = mobius::core::string::rstrip (decoder.get_string_by_size (11));
      filesystem_type = mobius::core::string::rstrip (decoder.get_string_by_size (8));
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Set metadata
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  metadata_.set ("oem_name", oem_name);
  metadata_.set ("volume_id", volume_id);
  metadata_.set ("volume_label", volume_label);
  metadata_.set ("sector_size", sector_size);
  metadata_.set ("sectors_per_cluster", sectors_per_cluster);
  metadata_.set ("reserved_sectors", reserved_sectors);
  metadata_.set ("total_clusters", total_clusters);
  metadata_.set ("fats", fats);
  metadata_.set ("fat_entry_size", fat_entry_size);
  metadata_.set ("root_dir_entries", root_dir_entries);
  metadata_.set ("sectors", sectors);
  metadata_.set ("media_descriptor", media_descriptor);
  metadata_.set ("sectors_per_fat", sectors_per_fat);
  metadata_.set ("sectors_per_track", sectors_per_track);
  metadata_.set ("heads", heads);
  metadata_.set ("hidden_sectors", hidden_sectors);
  metadata_.set ("logical_drive_number", logical_drive_number);
  metadata_.set ("extended_signature", extended_signature);
  metadata_.set ("filesystem_type", filesystem_type);
  metadata_.set ("is_dirty", is_dirty);

  if (fat_entry_size == 32)
    {
      metadata_.set ("flags", flags);
      metadata_.set ("version", version);
      metadata_.set ("root_dir_cluster", root_dir_cluster);
      metadata_.set ("fs_info_sector", fs_info_sector);
      metadata_.set ("backup_boot_sector", backup_boot_sector);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Set derived information
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  type_ = "fat" + std::to_string (fat_entry_size);
  size_ = static_cast <std::uint64_t> (sectors) * sector_size;

  name_ = "VFAT-" + std::to_string (fat_entry_size);

  if (!volume_label.empty ())
    name_ += " (" + volume_label + ")";

  else if (!oem_name.empty ())
    name_ += " (" + oem_name + ")";

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode FSINFO
  // @see https://thestarman.pcministry.com/asm/mbr/MSWIN41.htm#FSINFO
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (fs_info_sector)
    {
      reader.seek (fs_info_sector * SECTOR_SIZE);
      auto signature_1 = decoder.get_bytearray_by_size (4);

      if (signature_1 == "RRaA")
        {
          reader.skip (0x1e4);
          metadata_.set ("free_clusters", decoder.get_uint32_le ());
          metadata_.set ("next_available_cluster", decoder.get_uint32_le ());
        }
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Set data loaded
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  data_loaded_ = true;
}


