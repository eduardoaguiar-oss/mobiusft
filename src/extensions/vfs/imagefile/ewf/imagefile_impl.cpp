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
#include "imagefile_impl.hpp"
#include "segment_decoder.hpp"
#include "reader_impl.hpp"
#include "writer_impl.hpp"
#include <mobius/io/file.h>
#include <mobius/core/vfs/imagefile.hpp>
#include <mobius/core/vfs/util.hpp>
#include <mobius/exception.inc>
#include <mobius/string_functions.h>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @see https://github.com/libyal/libewf/blob/master/documentation/Expert%20Witness%20Compression%20Format%20(EWF).asciidoc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get segment file extension
// @param idx Segment idx
// @return File extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
get_segment_extension (mobius::core::vfs::segment_array::idx_type idx)
{
  if (idx < 99)
    return std::string ("E") + mobius::string::to_string (idx + 1, 2);

  else if (idx < 14971)
    {
      auto value = idx - 99;

      return std::string (1, char ('E' + value / 676)) +
             char ('A' + (value % 676) / 26) +
             char ('A' + (value % 26));
    }

  else
    throw std::out_of_range (mobius::MOBIUS_EXCEPTION_MSG ("Segment index out of range"));
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file is an instance of imagefile EWF
// @param f File object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
imagefile_impl::is_instance (const mobius::io::file& f)
{
  bool instance = false;

  if (f && f.exists ())
    {
      auto reader = f.new_reader ();

      if (reader)
        {
          segment_decoder decoder (reader);
          instance = bool (decoder);
        }
    }

  return instance;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Construct object
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile_impl::imagefile_impl (const mobius::io::file& f)
  : file_ (f),
    segments_ (f, get_segment_extension)
{
  attributes_.set ("compression_level", 0); // no compression
  attributes_.set ("segment_size", 4 * 1024 * 1024 * 1024L);   // 4 GiB
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get attribute
// @param name Attribute name
// @return Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
imagefile_impl::get_attribute (const std::string& name) const
{
  _load_metadata ();
  return attributes_.get (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set attribute
// @param name Attribute name
// @param value Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
imagefile_impl::set_attribute (
  const std::string& name,
  const mobius::core::pod::data& value
)
{
  attributes_.set (name, value);
  metadata_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get attributes
// @return Attributes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::map
imagefile_impl::get_attributes () const
{
  _load_metadata ();
  return attributes_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new reader for imagefile
// @return Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::io::reader
imagefile_impl::new_reader () const
{
  return mobius::io::reader (std::make_shared <reader_impl> (*this));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new writer for imagefile
// @return Writer object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::io::writer
imagefile_impl::new_writer () const
{
  return mobius::io::writer (std::make_shared <writer_impl> (*this));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load metadata
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
imagefile_impl::_load_metadata () const
{
  if (metadata_loaded_)
    return;

  // Scan segment files
  segments_.scan ();

  if (segments_.get_size () == 0)
    return;

  // Set metadata
  std::string acquisition_user = file_.get_user_name ();
  std::string acquisition_platform;
  std::string acquisition_tool;
  std::string drive_vendor;
  std::string drive_model;
  std::string drive_serial_number;
  std::string hash_md5;
  mobius::datetime::datetime acquisition_time = file_.get_modification_time ();
  size_type segment_size = 4 * 1024 * 1024 * 1024L;   // 4 GiB
  std::uint32_t compression_level = 0;

  // walk through segment files
  bool header_loaded = false;

  for (mobius::core::vfs::segment_array::idx_type i = 0;i < segments_.get_size ();i++)
    {
      segment_decoder decoder (segments_.new_reader (i));

      // walk through sections, retrieving imagefile metadata
      for (const auto& section : decoder)
        {
          if (section.get_name () == "hash")
            {
              auto hash_section = decoder.decode_hash_section (section);
              hash_md5 = hash_section.get_md5_hash ();
            }

          else if (section.get_name () == "volume" || section.get_name () == "disk" || section.get_name () == "data")
            {
              auto volume_section = decoder.decode_volume_section (section);
              sectors_ = volume_section.get_sectors ();
              sector_size_ = volume_section.get_sector_size ();
              size_ = sectors_ * sector_size_;
              chunk_size_ = volume_section.get_chunk_sectors () * sector_size_;
              chunk_count_ = (size_ + chunk_size_ - 1) / chunk_size_;
              compression_level = volume_section.get_compression_level ();
            }

          else if (!header_loaded && (section.get_name () == "header2" || section.get_name () == "header"))
            {
              auto header_section = decoder.decode_header_section (section);
              drive_model = header_section.get_drive_model ();
              drive_serial_number = header_section.get_drive_serial_number ();
              acquisition_user = header_section.get_acquisition_user ();
              acquisition_time = header_section.get_acquisition_time ();
              acquisition_tool = header_section.get_acquisition_tool ();
              acquisition_platform = header_section.get_acquisition_platform ();
              header_loaded = true;
            }
        }
    }

  mobius::core::vfs::normalize_drive_info (drive_vendor, drive_model, drive_serial_number);

  // if there is only one segment, segment_size equals to size
  if (segments_.get_size () == 1)
    segment_size = size_;

  // fill attributes
  attributes_.set ("segments", segments_.get_size ());
  attributes_.set ("segment_size", segment_size);
  attributes_.set ("chunk_size", chunk_size_);
  attributes_.set ("chunk_count", chunk_count_);
  attributes_.set ("compression_level", compression_level);
  attributes_.set ("drive_vendor", drive_vendor);
  attributes_.set ("drive_model", drive_model);
  attributes_.set ("drive_serial_number", drive_serial_number);
  attributes_.set ("acquisition_time", acquisition_time);
  attributes_.set ("acquisition_tool", acquisition_tool);
  attributes_.set ("acquisition_platform", acquisition_platform);
  attributes_.set ("acquisition_user", acquisition_user);
  attributes_.set ("hash_md5", hash_md5);

  // set metadata loaded
  metadata_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load chunk offset table on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
imagefile_impl::_load_chunk_offset_table () const
{
  if (chunk_offset_table_loaded_)
    return;

  // scan segment files
  if (segments_.get_size () == 0)
    return;

  // walk through segment files
  size_type next_offset = 0;

  for (mobius::core::vfs::segment_array::idx_type i = 0;i < segments_.get_size ();i++)
    {
      segment_decoder decoder (segments_.new_reader (i));

      chunk_offset_table offset_table;
      offset_table.start = next_offset;
      offset_table.end = 0;

      // walk through sections, retrieving imagefile metadata
      for (const auto& section : decoder)
        {
          if (section.get_name () == "table")
            {
              auto table_section = decoder.decode_table_section (section);
              size_type size = chunk_size_ * static_cast <size_type> (table_section.get_chunk_count ());

	      if (offset_table.end)
                offset_table.end += size;
              else
                offset_table.end = offset_table.start + size - 1;

              auto table_offset_list = table_section.get_chunk_offset_list ();

              offset_table.offsets.insert (
                offset_table.offsets.end (),
                std::make_move_iterator (table_offset_list.begin ()),
                std::make_move_iterator (table_offset_list.end ())
              );

              next_offset += size;
            }
        }

      chunk_offset_table_.push_back (offset_table);
    }

  // set chunk offset table loaded
  chunk_offset_table_loaded_ = true;
}


