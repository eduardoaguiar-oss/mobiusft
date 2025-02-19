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
#include "filesystem_impl.h"
#include <mobius/core/log.h>
#include <mobius/decoder/data_decoder.h>
#include <mobius/string_functions.h>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief sector size
static constexpr std::uint32_t SECTOR_SIZE = 512;

//! \brief signature
static const mobius::bytearray SIGNATURE = "EXFAT   ";	// Exfat (section 3.1.2)

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Check if reader contains an EXFAT boot sector
//! \param reader stream reader
//! \return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static bool
check_boot_sector (mobius::io::reader reader, std::uint64_t offset)
{
  bool rc = false;

  try
    {
      reader.seek (offset);
      auto data = reader.read (SECTOR_SIZE);
      rc = data.compare (3, SIGNATURE);
    }
  catch (const std::exception& e)
    {
      mobius::core::log log (__FILE__, __FUNCTION__);
      log.warning (__LINE__, e.what ());
    }

  return rc;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Check if stream contains an instance of exfat filesystem
//! \param reader Reader object
//! \param offset Offset from the beginning of the stream
//! \return True/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
filesystem_impl::is_instance (
  mobius::io::reader reader,
  std::uint64_t offset
)
{
  // Test Main Boot Sector
  bool rc = check_boot_sector (reader, offset);
  
  // Test Backup Boot Sector
  if (!rc)
    rc = check_boot_sector (reader, offset + SECTOR_SIZE * 12);

  return rc;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Constructor
//! \param reader Reader object
//! \param offset Offset from the beginning of volume
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
filesystem_impl::filesystem_impl (
  const mobius::io::reader& reader,
  size_type offset
)
 : reader_ (reader),
   offset_ (offset),
   tsk_adaptor_ (reader, offset)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get metadata item
//! \param name Item name
//! \return Data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::pod::data
filesystem_impl::get_metadata (const std::string& name) const
{
  _load_data ();
  return metadata_.get (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get root folder
//! \return Root folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::io::folder
filesystem_impl::get_root_folder () const
{
  return tsk_adaptor_.get_root_folder ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Load data on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
filesystem_impl::_load_data () const
{
  if (data_loaded_)
    return;

  // define boot sector offset
  mobius::io::reader reader = reader_;

  std::uint64_t boot_offset = 0;

  if (check_boot_sector (reader, offset_))
    boot_offset = offset_;

  else if (check_boot_sector (reader, offset_ + SECTOR_SIZE * 12))
    boot_offset = offset_ + SECTOR_SIZE * 512;

  else          // force data decoding anyway
    boot_offset = offset_;
  
  // decode data - EXFAT (section 3.1)
  reader.seek (boot_offset);
  mobius::decoder::data_decoder decoder (reader);

  decoder.skip (64);
  auto partition_offset = decoder.get_uint64_le ();
  auto sectors = decoder.get_uint64_le ();
  auto fat_offset = decoder.get_uint32_le ();
  auto fat_length = decoder.get_uint32_le ();
  auto cluster_heap_offset = decoder.get_uint32_le ();
  auto cluster_count = decoder.get_uint32_le ();
  auto root_dir_cluster = decoder.get_uint32_le ();
  auto serial_number = "0x" + mobius::string::to_hex (decoder.get_uint32_le (), 8);
  auto revision_minor = decoder.get_uint8 (); 
  auto revision_major = decoder.get_uint8 (); 
  auto flags = decoder.get_uint16_le ();
  auto bytes_per_sector_shift = decoder.get_uint8 ();
  std::uint64_t sector_size = 1 << bytes_per_sector_shift;
  auto sectors_per_cluster_shift = decoder.get_uint8 ();
  std::uint64_t cluster_size = 1LL << sectors_per_cluster_shift << bytes_per_sector_shift;
  auto number_of_fats = decoder.get_uint8 ();
  auto drive_select = decoder.get_uint8 ();
  auto percent_in_use = decoder.get_uint8 ();

  // set metadata
  metadata_.set ("revision", std::to_string (revision_major) + '.' + std::to_string (revision_minor));
  metadata_.set ("volume_serial_number", serial_number);
  metadata_.set ("sectors", sectors);
  metadata_.set ("sector_size", sector_size);
  metadata_.set ("sectors_per_cluster", 1 << sectors_per_cluster_shift);
  metadata_.set ("cluster_count", cluster_count);
  metadata_.set ("cluster_size", cluster_size);
  metadata_.set ("cluster_heap_offset", cluster_heap_offset);
  metadata_.set ("partition_offset", partition_offset);
  metadata_.set ("fat_offset", fat_offset);
  metadata_.set ("fat_length", fat_length);
  metadata_.set ("root_dir_cluster", root_dir_cluster);
  metadata_.set ("active_fat", (flags & 0x01) ? "second" : "first");
  metadata_.set ("volume_dirty", (flags & 0x02) ? "dirty" : "ok");
  metadata_.set ("media_failure", (flags & 0x04) ? "yes" : "no");
  metadata_.set ("number_of_fats", number_of_fats);
  metadata_.set ("drive_select", drive_select);
  
  if (percent_in_use != 0xff)
    metadata_.set ("percent_in_use", std::to_string (percent_in_use) + '%');
  
  type_ = "exfat";
  size_ = sectors * sector_size;
  name_ = "EXFAT (s/n: " + serial_number + ")";

  // Set data loaded
  data_loaded_ = true;
}
