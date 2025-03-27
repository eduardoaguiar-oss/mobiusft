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
#include "file_arestra.h"
#include "common.h"
#include <mobius/core/log.h>
#include <mobius/decoder/data_decoder.h>
#include <mobius/encoder/hexstring.h>

namespace mobius::extension::app::ares
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @see read_details_DB_Download@helper_download_disk.pas (line 722)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_arestra::file_arestra (const mobius::io::reader& reader)
{
  if (!reader || reader.get_size () < 4096)
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create main section
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto decoder = mobius::decoder::data_decoder (reader);
  decoder.seek (reader.get_size () - 4096);

  section_ = mobius::core::file_decoder::section (reader, "File");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode header signature
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto header_section = section_.new_child ("header");
  auto signature = decoder.get_bytearray_by_size (13);

  if (signature == "___ARESTRA___")
    version_ = 1;

  else if (signature == "___ARESTRA__2")
    version_ = 2;

  else if (signature == "___ARESTRA__3")
    version_ = 3;

  else
    return;

  is_instance_ = true;
  signature_ = signature.to_string ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode file size and progress
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  file_size_ = 0;
  progress_ = 0;

  if (version_ == 1)
    {
      file_size_ = decoder.get_uint32_le ();
      progress_ = decoder.get_uint32_le ();
    }
  else
    {
      file_size_ = decoder.get_uint64_le ();
      progress_ = decoder.get_uint64_le ();
    }

  header_section.end ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode gaps
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto gaps_section = section_.new_child ("gaps");

  std::uint64_t start_pchunk = 0;
  std::uint64_t end_pchunk = 0xffffffffffffffff;

  while (end_pchunk)
    {
      if (version_ == 1)
        {
          end_pchunk = decoder.get_uint32_le ();
          start_pchunk = decoder.get_uint32_le ();
        }
      else
        {
          start_pchunk = decoder.get_uint64_le ();
          end_pchunk = decoder.get_uint64_le ();
        }

      if (end_pchunk)
        gaps_.emplace_back (start_pchunk, end_pchunk);
    }

  gaps_section.end ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode params
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto params_section = section_.new_child ("params");

  media_type_ = media_type_to_string (decoder.get_uint8 ());
  is_paused_ = (decoder.get_uint8 () == 1);
  param1_ = decoder.get_uint32_le ();
  param2_ = decoder.get_uint32_le ();
  param3_ = decoder.get_uint32_le ();

  params_section.end ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode metadata, if any
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto data_size = decoder.get_uint16_le ();

  if (data_size > 0 && data_size != 3500)
    {
      auto metadata_section = section_.new_child ("metadata");

      auto data = decoder.get_bytearray_by_size (data_size);
      _populate_metadata (data);

      metadata_section.end ();
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Padding section
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto padding_section = section_.new_child ("padding");
  decoder.seek (reader.get_size ());
  padding_section.end ();

  section_.end ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Populate entry metadata
// @param e Entry
// @param data Raw data
// @see read_details_DB_Download@helper_download_disk.pas
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_arestra::_populate_metadata (const mobius::bytearray& data)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  for (const auto& [i, v] : decode_metadata (data))
    {
      mobius::decoder::data_decoder decoder (v);

      switch (i)
        {
          case 1: kw_genre_ = v.to_string (); break;
          case 2: title_ = v.to_string (); break;
          case 3: artist_ = v.to_string (); break;
          case 4: album_ = v.to_string (); break;
          case 5: category_ = v.to_string (); break;
          case 6: year_ = v.to_string (); break;
          case 7: alt_sources_ = decode_old_alt_sources (v); break;
          case 8: language_ = v.to_string (); break;
          case 9: url_ = v.to_string (); break;
          case 10: comment_ = v.to_string (); break;
          case 13: alt_sources_ = decode_alt_sources (v); break;
          case 15: hash_sha1_ = v.to_hexstring (); break;
          case 19: subfolder_ = v.to_string (); break;
          case 20: phash_verified_ = decoder.get_uint64_le (); break;
          case 25: download_started_time_ = decoder.get_unix_datetime (); break;
          default:
              log.development (__LINE__, "unhandled field_type: " + std::to_string (i));
        }
    }
}

} // namespace mobius::extension::app::ares




