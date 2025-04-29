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
#include <mobius/string_functions.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if stream contains an instance of NTFS filesystem
// @param reader Reader object
// @param offset Offset from the beginning of the stream
// @return True/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
filesystem_impl::is_instance (mobius::io::reader reader, std::uint64_t offset)
{
  reader.seek (offset + 3);
  auto data = reader.read (8);

  return data == "NTFS    ";
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param offset Offset from the beginning of volume
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

  mobius::io::reader reader = reader_;
  reader.seek (offset_);
  mobius::decoder::data_decoder decoder (reader);

  // decode BPB
  decoder.skip (3);
  auto oem_name = mobius::string::rstrip (decoder.get_string_by_size (8));
  auto sector_size = decoder.get_uint16_le ();
  auto sectors_per_cluster = decoder.get_uint8 ();
  auto reserved_sectors = decoder.get_uint16_le ();
  decoder.skip (5);
  auto media_descriptor = decoder.get_uint8 ();
  decoder.skip (2);
  auto sectors_per_track = decoder.get_uint16_le ();
  auto heads = decoder.get_uint16_le ();

  // decode extended BPB
  auto hidden_sectors = decoder.get_uint32_le ();
  decoder.skip (8);
  auto sectors = decoder.get_uint64_le ();
  auto mft_cluster = decoder.get_uint64_le ();
  auto mft_mirror_cluster = decoder.get_uint64_le ();
  auto clusters_per_frs = decoder.get_uint32_le ();
  auto clusters_per_index_block = decoder.get_uint32_le ();
  auto volume_serial_number = mobius::string::to_hex (decoder.get_uint64_le (), 16);
  auto checksum = decoder.get_uint32_le ();

  auto cluster_size = sectors_per_cluster * sector_size;
  auto frs_size = clusters_per_frs * cluster_size;
  auto mft_offset = offset_ + mft_cluster * cluster_size;
  auto mft_mirror_offset = offset_ + mft_mirror_cluster * cluster_size;

  // set metadata
  metadata_.set ("oem_name", oem_name);
  metadata_.set ("sector_size", sector_size);
  metadata_.set ("sectors_per_cluster", sectors_per_cluster);
  metadata_.set ("reserved_sectors", reserved_sectors);
  metadata_.set ("media_descriptor", media_descriptor);
  metadata_.set ("sectors_per_track", sectors_per_track);
  metadata_.set ("heads", heads);
  metadata_.set ("hidden_sectors", hidden_sectors);
  metadata_.set ("sectors", sectors);
  metadata_.set ("mft_cluster", mft_cluster);
  metadata_.set ("mft_mirror_cluster", mft_mirror_cluster);
  metadata_.set ("clusters_per_frs", clusters_per_frs);
  metadata_.set ("clusters_per_index_block", clusters_per_index_block);
  metadata_.set ("volume_serial_number", volume_serial_number);
  metadata_.set ("checksum", checksum);
  metadata_.set ("cluster_size", cluster_size);
  metadata_.set ("frs_size", frs_size);
  metadata_.set ("mft_offset", mft_offset);
  metadata_.set ("mft_mirror_offset", mft_mirror_offset);

  // derived information
  size_ = (sectors + 1) * sector_size;
  type_ = "ntfs";
  name_ = "NTFS (s/n: " + volume_serial_number + ')';

  // Set data loaded
  data_loaded_ = true;
}


