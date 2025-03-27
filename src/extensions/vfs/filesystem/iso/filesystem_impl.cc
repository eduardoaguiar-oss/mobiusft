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
#include <mobius/decoder/data_decoder.h>
#include <mobius/string_functions.h>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr int SYSTEM_AREA_SIZE = 32768;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if stream contains an instance of ISO9660 filesystem
// @param reader Reader object
// @param offset Offset from the beginning of the stream
// @return True/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
filesystem_impl::is_instance (mobius::io::reader reader, std::uint64_t offset)
{
  reader.seek (offset + SYSTEM_AREA_SIZE + 1);
  auto data = reader.read (5);

  return data == "CD001";
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
mobius::pod::data
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
  reader.seek (offset_ + SYSTEM_AREA_SIZE);
  mobius::decoder::data_decoder decoder (reader);

  // decode Primary Volume Descriptor (ECMA-119 8.4)
  decoder.skip (6);
  auto volume_descr_version = decoder.get_uint8 ();
  decoder.skip (1);
  auto system_id = mobius::string::strip (decoder.get_string_by_size (32));
  auto volume_id = mobius::string::strip (decoder.get_string_by_size (32));
  decoder.skip (8);
  auto volume_space_size = decoder.get_uint32_le ();
  decoder.skip (36);
  auto volume_set_size = decoder.get_uint16_le ();
  decoder.skip (2);
  auto volume_sequence_number = decoder.get_uint16_le ();
  decoder.skip (2);
  auto logical_block_size = decoder.get_uint16_le ();
  decoder.skip (2);
  auto path_table_size = decoder.get_uint32_le ();
  decoder.skip (4);
  auto type_l_path_table = decoder.get_uint16_le ();
  decoder.skip (2);
  auto type_l_optional_path_table = decoder.get_uint16_le ();
  decoder.skip (2);
  auto type_m_path_table = decoder.get_uint16_le ();
  decoder.skip (2);
  auto type_m_optional_path_table = decoder.get_uint16_le ();
  decoder.skip (36);
  auto volume_set_id = mobius::string::strip (decoder.get_string_by_size (128));
  auto publisher_id = mobius::string::strip (decoder.get_string_by_size (128));
  auto data_preparer_id = mobius::string::strip (decoder.get_string_by_size (128));
  auto application_id = mobius::string::strip (decoder.get_string_by_size (128));
  auto copyright_file_id = mobius::string::strip (decoder.get_string_by_size (37));
  auto abstract_file_id = mobius::string::strip (decoder.get_string_by_size (37));
  auto bibliographic_file_id = mobius::string::strip (decoder.get_string_by_size (37));
  auto creation_time = decoder.get_iso9660_datetime ();
  auto last_modification_time = decoder.get_iso9660_datetime ();
  auto expiration_time = decoder.get_iso9660_datetime ();
  auto effective_time = decoder.get_iso9660_datetime ();
  auto file_structure_version = decoder.get_uint8 ();

  // set metadata
  metadata_.set ("volume_descr_version", volume_descr_version);
  metadata_.set ("system_id", system_id);
  metadata_.set ("volume_id", volume_id);
  metadata_.set ("volume_space_size", volume_space_size);
  metadata_.set ("volume_set_size", volume_set_size);
  metadata_.set ("volume_sequence_number", volume_sequence_number);
  metadata_.set ("logical_block_size", logical_block_size);
  metadata_.set ("path_table_size", path_table_size);
  metadata_.set ("type_l_path_table", type_l_path_table);
  metadata_.set ("type_l_optional_path_table", type_l_optional_path_table);
  metadata_.set ("type_m_path_table", type_m_path_table);
  metadata_.set ("type_m_optional_path_table", type_m_optional_path_table);
  metadata_.set ("volume_set_id", volume_set_id);
  metadata_.set ("publisher_id", publisher_id);
  metadata_.set ("data_preparer_id", data_preparer_id);
  metadata_.set ("application_id", application_id);
  metadata_.set ("copyright_file_id", copyright_file_id);
  metadata_.set ("abstract_file_id", abstract_file_id);
  metadata_.set ("bibliographic_file_id", bibliographic_file_id);
  metadata_.set ("creation_time", creation_time);
  metadata_.set ("last_modification_time", last_modification_time);
  metadata_.set ("expiration_time", expiration_time);
  metadata_.set ("effective_time", effective_time);
  metadata_.set ("file_structure_version", file_structure_version);

  // derived information
  size_ = volume_space_size * logical_block_size;
  name_ = "ISO " + volume_id;
  type_ = "iso9660";

  // Set data loaded
  data_loaded_ = true;
}


