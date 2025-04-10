#ifndef MOBIUS_EXTENSION_SEGMENT_WRITER_HPP
#define MOBIUS_EXTENSION_SEGMENT_WRITER_HPP

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
#include <mobius/io/writer.h>
#include <cstdint>
#include <string>
#include <vector>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief EWF segment writer
// @author Eduardo Aguiar
// @see https://github.com/libyal/libewf/blob/master/documentation/Expert%20
//       Witness%20Compression%20Format%20(EWF).asciidoc
//       (version 0.0.80, visited in 2016-08-25)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class segment_writer
{
public:
  using size_type = std::uint64_t;
  using offset_type = std::int64_t;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  segment_writer () = default;
  segment_writer (mobius::io::writer, std::uint16_t);
  segment_writer (const segment_writer&) = default;
  segment_writer (segment_writer&&) = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // assignment operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  segment_writer& operator= (const segment_writer&) = default;
  segment_writer& operator= (segment_writer&&) = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief set segment size
  // @param segment_size segment size in bytes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_segment_size (size_type segment_size)
  {
    segment_size_ = segment_size;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief set chunk size
  // @param chunk_size chunk size in bytes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_chunk_size (size_type chunk_size)
  {
    chunk_size_ = chunk_size;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief set compression level
  // @param compression_level (0 - none, 1 - fast, 9 - best)
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_compression_level (int compression_level)
  {
    compression_level_ = compression_level;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief set drive vendor
  // @param drive_vendor drive vendor
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_drive_vendor (const std::string& drive_vendor)
  {
    drive_vendor_ = drive_vendor;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief set drive model
  // @param drive_model drive model
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_drive_model (const std::string& drive_model)
  {
    drive_model_ = drive_model;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief set drive serial number
  // @param drive_serial_number drive serial number
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_drive_serial_number (const std::string& drive_serial_number)
  {
    drive_serial_number_ = drive_serial_number;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief set acquisition user name
  // @param acquisition_user acquisition user name
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_acquisition_user (const std::string& acquisition_user)
  {
    acquisition_user_ = acquisition_user;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief set sector size
  // @param sector_size sector size in bytes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_sector_size (size_type sector_size)
  {
    sector_size_ = sector_size;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief set chunk sectors
  // @param chunk_sectors sectors per chunk
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_chunk_sectors (size_type chunk_sectors)
  {
    chunk_sectors_ = chunk_sectors;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief set segment count
  // @param segment_count segment count
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_segment_count (std::uint16_t segment_count)
  {
    segment_count_ = segment_count;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief set imagefile total size
  // @param size size in bytes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_total_size (size_type size)
  {
    total_size_ = size;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief set segment file set GUID
  // @param guid GUID as bytearray
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_guid (const mobius::bytearray& guid)
  {
    guid_ = guid;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief set MD5 hash
  // @param hd5_hash MD5 hash as bytearray
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_md5_hash (const mobius::bytearray& md5_hash)
  {
    md5_hash_ = md5_hash;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // methods
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  size_type write (const mobius::bytearray&);
  void create ();
  void flush ();
  void close ();

private:
  // control attributes
  mobius::io::writer writer_;
  std::uint16_t segment_number_;
  mobius::bytearray chunk_data_;
  std::vector <std::uint32_t> chunk_offset_list_;
  std::vector <std::pair <std::string, offset_type>> volume_info_list_;
  offset_type sector_offset_ = -1;
  size_type chunk_count_ = 0;
  size_type size_ = 0;

  // config metadata
  int compression_level_ = 0;
  size_type chunk_size_ = 0;
  size_type segment_size_ = 1024L * 1024L * 1024L * 1024L;          // 1 TB

  // header metadata
  std::string drive_vendor_;
  std::string drive_model_;
  std::string drive_serial_number_;
  std::string acquisition_user_;

  // volume metadata
  size_type total_size_ = 0;
  size_type chunk_sectors_ = 0;
  size_type sector_size_ = 0;
  std::uint16_t segment_count_ = 0;
  mobius::bytearray guid_;

  // hash metadata
  mobius::bytearray md5_hash_;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // private prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void _write_file_header (std::uint16_t);
  void _write_header_section ();
  void _write_volume_stub (const std::string&);
  void _write_volume_section (const std::string&);
  void _write_hash_section ();
  void _write_table_section (const std::string&);
  void _write_section_header (const std::string&, size_type);
  void _write_section_data (const mobius::bytearray&);
  bool _can_write_chunk_data ();
  void _write_chunk_data (mobius::bytearray);
  void _write_sectors_stub ();
  void _close_sectors_section ();
};

#endif


