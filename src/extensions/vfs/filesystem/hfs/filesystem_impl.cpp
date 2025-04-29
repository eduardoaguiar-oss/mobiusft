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
#include <mobius/decoder/data_decoder.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if stream contains an instance of vfat filesystem
// @param reader Reader object
// @param offset Offset from the beginning of the stream
// @return True/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
filesystem_impl::is_instance (mobius::io::reader reader, std::uint64_t offset)
{
  reader.seek (offset + 1024);
  auto data = reader.read (2);

  return data == "HX" || data == "H+";
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param offset Offset from the beginning of volume
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
filesystem_impl::filesystem_impl (const mobius::io::reader& reader,
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
mobius::io::folder
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

  // create decoder
  mobius::io::reader reader = reader_;
  reader.seek (offset_ + 1024);
  mobius::decoder::data_decoder decoder (reader);

  // decode data
  auto signature = decoder.get_string_by_size (2);
  auto version = decoder.get_uint16_be ();
  auto attributes = decoder.get_uint32_be ();
  decoder.skip (8);             // lastMountedVersion, journalInfoBlock
  auto creation_time = decoder.get_hfs_datetime ();
  auto last_modification_time = decoder.get_hfs_datetime ();
  auto last_backup_time = decoder.get_hfs_datetime ();
  auto last_check_time = decoder.get_hfs_datetime ();
  auto file_count = decoder.get_uint32_be ();
  auto folder_count = decoder.get_uint32_be ();
  auto block_size = decoder.get_uint32_be ();
  auto total_blocks = decoder.get_uint32_be ();
  auto free_blocks = decoder.get_uint32_be ();
  decoder.skip (16);            // nextAllocation...nextCatalogID
  auto mount_count = decoder.get_uint32_be ();
  decoder.skip (32);            // encodingsBitmap..finderInfo[5]
  auto volume_uuid = decoder.get_hex_string_by_size (8);

  // set metadata
  metadata_.set ("signature", signature);
  metadata_.set ("version", version);
  metadata_.set ("attributes", attributes);
  metadata_.set ("creation_time", creation_time);
  metadata_.set ("last_modification_time", last_modification_time);
  metadata_.set ("last_backup_time", last_backup_time);
  metadata_.set ("last_check_time", last_check_time);
  metadata_.set ("file_count", file_count);
  metadata_.set ("folder_count", folder_count);
  metadata_.set ("block_size", block_size);
  metadata_.set ("total_blocks", total_blocks);
  metadata_.set ("free_blocks", free_blocks);
  metadata_.set ("mount_count", mount_count);
  metadata_.set ("volume_uuid", volume_uuid);

  // derived information
  size_ = total_blocks * block_size;
  type_ = (version == 4) ? "HFS+" : "HFSX";
  name_ = type_ + " (uuid: " + volume_uuid + ')';

  // Set data loaded
  data_loaded_ = true;
}


