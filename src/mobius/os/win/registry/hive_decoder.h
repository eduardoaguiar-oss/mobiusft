#ifndef MOBIUS_CORE_REGISTRY_HIVE_DECODER_HPP
#define MOBIUS_CORE_REGISTRY_HIVE_DECODER_HPP

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
#include <mobius/io/reader.h>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/bytearray.h>
#include <cstdint>
#include <string>
#include <vector>

namespace mobius::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief hive file header struct
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct header_data
{
  // @brief file signature (regf)
  std::string signature;

  // @brief this number is incremented by 1 in the beginning of a write operation on the hive
  std::uint32_t sequence_1;

  // @brief this number is incremented by 1 at the end of a write operation on the hive
  std::uint32_t sequence_2;

  // @brief last written timestamp as NT datetime
  mobius::core::datetime::datetime mtime;

  // @brief major version of the hivefile
  std::uint32_t major_version;

  // @brief minor version of the hivefile
  std::uint32_t minor_version;

  // @brief file type: 0 - registry file, 1 - transaction log
  std::uint32_t file_type;

  // @brief file format: 1 - direct memory load
  std::uint32_t file_format;

  // @brief offset of the root cell in bytes, relative from the start of the hive bins data
  std::uint32_t root_offset;

  // @brief size of the hive bins data in bytes
  std::uint32_t hbin_data_size;

  // @brief sector size of the interlying disk, in bytes
  std::uint32_t disk_sector_size;

  // @brief last 32 characters of the filename, in UTF-16LE
  std::string filename;

  // @brief header checksum
  std::uint32_t header_checksum;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Windows 10 fields
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief GUID of the Resource Manager (RM)
  std::string rm_guid;

  // @brief unused
  std::string log_guid;

  // @brief flags (bit mask)
  std::uint32_t flags;

  // @brief GUID used to generate a file name of a log file for the Transaction Manager (TM)
  std::string tm_guid;

  // @brief GUID signature
  std::string guid_signature;

  // @brief last reorganization timestamp
  mobius::core::datetime::datetime rtime;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Key value struct
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct vk_data
{
  // @brief signature (vk)
  std::string signature;

  // @brief flags (bit mask)
  std::uint16_t flags = 0;

  // @brief data size
  std::uint32_t data_size;

  // @brief data type
  std::uint32_t data_type;

  // @brief data offset
  std::uint32_t data_offset;

  // @brief data value
  mobius::bytearray data_value;

  // @brief value name
  std::string name;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Windows' registry file decoder
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class hive_decoder
{
public:
  using offset_type = std::uint32_t;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  explicit hive_decoder (mobius::io::reader);
  header_data decode_header ();
  vk_data decode_vk (offset_type);
  std::vector <offset_type> decode_subkeys_list (offset_type);
  std::vector <offset_type> decode_values_list (offset_type, std::uint32_t);
  mobius::bytearray decode_data (offset_type, std::uint32_t);
  mobius::bytearray decode_data_cell (offset_type);
  mobius::bytearray decode_data_db (offset_type);

private:
  // @brief generic reader
  mobius::io::reader reader_;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // helper functions
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void _retrieve_subkeys (std::vector <offset_type>&, offset_type);

};

} // namespace mobius::os::win::registry

#endif


