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
#include "segment_writer.h"
#include <mobius/encoder/data_encoder.h>
#include <mobius/core/application.h>
#include <mobius/datetime/datetime.h>
#include <mobius/datetime/conv_iso_string.h>
#include <mobius/string_functions.h>
#include <mobius/zlib_functions.h>
#include <mobius/charset.h>
#include <mobius/exception.inc>
#include <mobius/io/writer_evaluator.h>
#include <mobius/crypt/hash_functor.h>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Typical segment file sections
//
// E01:              E02:            EXX (last)         E01 (unique file)
// header2           data            data               header2
// header2           sectors         sectors            header2
// header            table           table              header
// volume            table2          table2             volume
// sectors           ...             ...                sectors
// table             next            hash               table
// table2                            done               table2
// ...                                                  ...
// next                                                 data
//                                                      hash
//                                                      done
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// class invariants:
//
// sector_offset_       offset for the open "sectors" section.
//                      -1 if there is no sectors section opened
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr int HEADER_SIZE = 76;
static constexpr int VOLUME_SECTION_SIZE = 1052;
static constexpr int CHUNKS_PER_TABLE = 16375;
static constexpr int CHUNK_SECTORS = 64;
static constexpr int SECTOR_SIZE = 512;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Convert an Adler-32 digest from bytearray to uint32_t
//! \param digest bytearray digest (4 bytes length)
//! \return std::uint32_t
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::uint32_t
digest_to_uint32_t (const mobius::bytearray& digest)
{
  return std::uint32_t (digest[0]) << 24 |
         std::uint32_t (digest[1]) << 16 |
         std::uint32_t (digest[2]) << 8 |
         std::uint32_t (digest[3]);
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Constructor
//! \param writer generic mobius::io::writer object
//! \param segment_idx segment index, starting from 1
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
segment_writer::segment_writer (mobius::io::writer writer, std::uint16_t segment_number)
  : writer_ (writer),
    segment_number_ (segment_number),
    chunk_sectors_ (CHUNK_SECTORS),
    sector_size_ (SECTOR_SIZE),
    guid_ (mobius::bytearray (16))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Create segment file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
segment_writer::create ()
{
  _write_file_header (segment_number_);

  if (segment_number_ == 1)
    {
      _write_header_section ();
      _write_volume_stub ("volume");
    }

  else
    _write_volume_stub ("data");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Write data
//! \param arg_data data
//! \return the number of bytes written
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
segment_writer::size_type
segment_writer::write (const mobius::bytearray& arg_data)
{
  mobius::bytearray data = chunk_data_ + arg_data;
  size_type bytes = 0;

  // write data chunks
  bool can_write = _can_write_chunk_data ();

  while (data.size () >= chunk_size_ && can_write)
    {
      _write_chunk_data (data.slice (0, chunk_size_ - 1));
      bytes += chunk_size_;
      data = data.slice (chunk_size_, data.size () - 1);
      can_write = _can_write_chunk_data ();
    }

  // if there is room for one more chunk data, write the remaining bytes into chunk_data_
  if (can_write)
    {
      bytes -= chunk_data_.size ();         // does not count those bytes twice
      chunk_data_ = data;
      bytes += data.size ();
    }

  // otherwise, if it has written at least one chunk, clear chunk data
  else if (bytes > 0)
    {
      bytes -= chunk_data_.size ();         // does not count those bytes twice
      chunk_data_.clear ();
    }

  // otherwise, error
  else if (chunk_data_.size () > 0)
    throw std::runtime_error (mobius::MOBIUS_EXCEPTION_MSG ("error writing chunk data"));

  return bytes;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Flush data to segment writer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
segment_writer::flush ()
{
  writer_.flush ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Close segment file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
segment_writer::close ()
{
  // flush remaining chunk_data_, if any
  if (!chunk_data_.empty ())
    {
      _write_chunk_data (chunk_data_);
      chunk_data_.clear ();
    }

  // write remaining bytes, if any, and close sectors, table and table2 sections
  if (sector_offset_ != -1)
    _close_sectors_section ();
  
  // update volume, disk and data sections
  auto offset = writer_.tell ();

  for (auto volume_info : volume_info_list_)
    {
      writer_.seek (volume_info.second);
      _write_volume_section (volume_info.first);
    }
  writer_.seek (offset);

  // write intermediate segment
  if (segment_number_ < segment_count_)
    _write_section_header ("next", 0);

  // otherwise, write last segment
  else
    {
      if (segment_number_ == 1)
        _write_volume_section ("data");

      _write_hash_section ();
      _write_section_header ("done", 0);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Write file header
//! \see EWCF 2.1
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
segment_writer::_write_file_header (std::uint16_t segment_idx)
{
  const mobius::bytearray EWF_SIGNATURE = {'E', 'V', 'F', 0x09, 0x0d, 0x0a, 0xff, 0x00};

  mobius::encoder::data_encoder encoder (writer_);
  encoder.encode_bytearray (EWF_SIGNATURE);
  encoder.encode_uint8 (0x01);
  encoder.encode_uint16_le (segment_idx);
  encoder.encode_uint16_le (0x00);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Write section header
//! \param name section name
//! \param size section size
//! \see EWCF 3.1
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
segment_writer::_write_section_header (const std::string& name, size_type size)
{
  std::uint64_t next_offset = writer_.tell ();

  if (size > 0)
    next_offset += HEADER_SIZE + size;

  // encode section header
  mobius::crypt::hash_functor hash_functor ("adler32");
  auto writer = mobius::io::writer_evaluator (writer_, hash_functor);

  mobius::encoder::data_encoder encoder (writer);
  encoder.encode_string_by_size (name, 16);
  encoder.encode_uint64_le (next_offset);
  encoder.encode_uint64_le (HEADER_SIZE + size);
  encoder.fill (40, 0);

  // encode ADLER-32 hash
  std::uint32_t hash_value = digest_to_uint32_t (hash_functor.get_digest ());
  encoder.encode_uint32_le (hash_value);                // Adler-32 CRC
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Write section data
//! \param data section data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
segment_writer::_write_section_data (const mobius::bytearray& data)
{
  writer_.write (data);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Write "header" section
// Writes twice "header2" sections, encoded in UTF-16, followed by an
// UTF-8 "header" section
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
segment_writer::_write_header_section ()
{
  // format header metadata
  mobius::core::application app;
  mobius::datetime::datetime dt_now = mobius::datetime::now ();

  std::string acquisition_tool = app.get_title ();
  std::string acquisition_platform = app.get_os_name ();
  std::string drive_model = drive_vendor_ + ' ' + drive_model_;

  std::string acquisition_datetime = mobius::datetime::datetime_to_iso_string (dt_now);
  acquisition_datetime = mobius::string::replace (acquisition_datetime, "-", " ");
  acquisition_datetime = mobius::string::replace (acquisition_datetime, ":", " ");
  acquisition_datetime = mobius::string::replace (acquisition_datetime, "T", " ");
  acquisition_datetime = mobius::string::replace (acquisition_datetime, "Z", "");

  std::string header_utf8 = "1\n"
                            "main\n"
                            "a\tc\tn\te\tt\tmd\tsn\tl\tav\tov\tm\tu\tp\tpid\tdc\text\tr\n"
                            "a"
                            "\tc"
                            "\tn"
                            "\t" + acquisition_user_ +                          // e
                            "\tt"
                            "\t" + drive_model +                                // md
                            "\t" + drive_serial_number_ +                       // sn
                            "\tl"
                            "\t" + acquisition_tool +                           // av
                            "\t" + acquisition_platform +                       // ov
                            "\t" + acquisition_datetime +                       // m
                            "\t" + acquisition_datetime +                       // u
                            "\t0"                                               // p
                            "\tpid"
                            "\tdc"
                            "\text"
                            "\t" + std::to_string (compression_level_) +        // r
                            "\n\n";

  // write two "header2" sections
  mobius::bytearray header_utf16 = mobius::conv_charset (header_utf8, "utf-8", "utf-16");

  const mobius::bytearray data_header2 = mobius::zlib_compress (header_utf16);
  _write_section_header ("header2", data_header2.size ());
  _write_section_data (data_header2);
  _write_section_header ("header2", data_header2.size ());
  _write_section_data (data_header2);

  // write "header" section
  const mobius::bytearray data_header = mobius::zlib_compress (header_utf8);
  _write_section_header ("header", data_header.size ());
  _write_section_data (data_header);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief write "volume" stub section
//! \param section_name section name (volume/disk/data)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
segment_writer::_write_volume_stub (const std::string& section_name)
{
  volume_info_list_.push_back (
    std::pair <std::string, offset_type> (section_name, writer_.tell ()));

  _write_volume_section (section_name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief write "volume" section
//! \param section_name section name (volume/disk/data)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
segment_writer::_write_volume_section (const std::string& section_name)
{
  _write_section_header (section_name, VOLUME_SECTION_SIZE);

  // encode section
  size_type sectors = (total_size_ + sector_size_ - 1) / sector_size_;
  size_type chunk_count = (total_size_ + chunk_size_ - 1) / chunk_size_;

  mobius::crypt::hash_functor hash_functor ("adler32");
  auto writer = mobius::io::writer_evaluator (writer_, hash_functor);

  // encode metadata
  mobius::encoder::data_encoder encoder (writer);
  encoder.encode_uint32_le (1);                         // \todo media type
  encoder.encode_uint32_le (chunk_count);
  encoder.encode_uint32_le (chunk_sectors_);
  encoder.encode_uint32_le (sector_size_);
  encoder.encode_uint64_le (sectors);
  encoder.encode_uint32_le (0);                         // C:H:S (C)
  encoder.encode_uint32_le (0);                         // C:H:S (H)
  encoder.encode_uint32_le (0);                         // C:H:S (S)
  encoder.encode_uint32_le (3);                         // \todo media flags
  encoder.encode_uint64_le (0);                         // PALM volume start sector
  encoder.encode_uint32_le (0);                         // SMART logs start sector
  encoder.encode_uint32_le (compression_level_);
  encoder.encode_uint64_le (0);                         // sector error granularity
  encoder.encode_bytearray (guid_);
  encoder.fill (968, 0);                                // padding

  // encode ADLER-32 hash
  std::uint32_t hash_value = digest_to_uint32_t (hash_functor.get_digest ());
  encoder.encode_uint32_le (hash_value);                // Adler-32 CRC
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief write "hash" section
//! \param section_name section name (volume/disk/data)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
segment_writer::_write_hash_section ()
{
  _write_section_header ("hash", 36);

  // encode section
  mobius::crypt::hash_functor hash_functor ("adler32");
  auto writer = mobius::io::writer_evaluator (writer_, hash_functor);

  mobius::encoder::data_encoder encoder (writer);
  encoder.encode_bytearray (md5_hash_);
  encoder.fill (16, 0);                                 // padding

  // encode ADLER-32 hash
  std::uint32_t adler32_value = digest_to_uint32_t (hash_functor.get_digest ());
  encoder.encode_uint32_le (adler32_value);             // Adler-32 CRC
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief write "table" sections
//! \param section_name section name (table/table2)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
segment_writer::_write_table_section (const std::string& name)
{
  // write section header
  constexpr int TABLE_HEADER_SIZE = 24;
  std::uint32_t entries = chunk_offset_list_.size ();
  size_type section_size = TABLE_HEADER_SIZE + 4 * entries + 4;

  _write_section_header (name, section_size);

  // write section metadata
  mobius::crypt::hash_functor hash_functor ("adler32");
  auto writer = mobius::io::writer_evaluator (writer_, hash_functor);

  mobius::encoder::data_encoder encoder (writer);
  encoder.encode_uint32_le (entries);
  encoder.fill (4, 0);
  encoder.encode_uint64_le (sector_offset_);
  encoder.fill (4, 0);

  std::uint32_t adler32_value = digest_to_uint32_t (hash_functor.get_digest ());
  encoder.encode_uint32_le (adler32_value);             // Adler-32 CRC

  // write offsets
  hash_functor = mobius::crypt::hash_functor ("adler32");
  writer = mobius::io::writer_evaluator (writer_, hash_functor);
  encoder = mobius::encoder::data_encoder (writer);

  for (auto offset : chunk_offset_list_)
    encoder.encode_uint32_le (offset);

  adler32_value = digest_to_uint32_t (hash_functor.get_digest ());
  encoder.encode_uint32_le (adler32_value);             // Adler-32 CRC
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief check if there is room for another chunk
//! \return true/false if chunk can be written
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
segment_writer::_can_write_chunk_data ()
{
  constexpr size_type TABLE_SECTION_SIZE = 76 + 24 + 4 + 4;
  constexpr size_type VOLUME_SECTION_SIZE = 1128;
  constexpr size_type HASH_SECTION_SIZE = 112;
  constexpr size_type DONE_SECTION_SIZE = 76;
  constexpr size_type FOOTER_SIZE =
    TABLE_SECTION_SIZE * 2 +                  // table and table2 sections
    VOLUME_SECTION_SIZE +                     // data section (equal to volume section)
    HASH_SECTION_SIZE +                       // hash section
    DONE_SECTION_SIZE;                        // done section

  return writer_.tell () + chunk_size_ * 2 + FOOTER_SIZE <= segment_size_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief write chunk data
//! \param data chunk data to be written
//! \return true/false if chunk could be written
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
segment_writer::_write_chunk_data (mobius::bytearray data)
{
  // create new sectors section, if necessary
  if (sector_offset_ == -1)
    {
      sector_offset_ = writer_.tell ();
      writer_.skip (HEADER_SIZE);
    }

  // evaluate chunk offset
  std::uint32_t chunk_offset = writer_.tell () - sector_offset_;

  // compress data, if necessary
  if (compression_level_ > 0)
    {
      int level = (compression_level_ == 2) ? 9 : 1;
      mobius::bytearray compressed_data = zlib_compress (data, level);

      if (compressed_data.size () < data.size ())
        {
          data = compressed_data;
          chunk_offset |= 0x80000000;            // set chunk data compressed flag
        }
    }

  // write data
  mobius::crypt::hash_functor hash_functor ("adler32");
  auto writer = mobius::io::writer_evaluator (writer_, hash_functor);
  writer.write (data);

  // encode ADLER-32 hash
  mobius::encoder::data_encoder encoder (writer);
  std::uint32_t hash_value = digest_to_uint32_t (hash_functor.get_digest ());
  encoder.encode_uint32_le (hash_value);                // Adler-32 CRC

  // update chunk offset list
  chunk_count_++;
  chunk_offset_list_.push_back (chunk_offset);

  // if chunk table is full, close sectors section
  if (chunk_offset_list_.size () == CHUNKS_PER_TABLE)
    {
      _close_sectors_section ();
      chunk_offset_list_.clear ();
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief close current "sectors" section
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
segment_writer::_close_sectors_section ()
{
  // encode "sectors" section header
  auto next_offset = writer_.tell ();

  writer_.seek (sector_offset_);
  size_type size = (next_offset - sector_offset_) - HEADER_SIZE;
  _write_section_header ("sectors", size);

  // write "table" section
  writer_.seek (next_offset);
  _write_table_section ("table");
  _write_table_section ("table2");

  // clear sector_offset_
  sector_offset_ = -1;
}
