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
#include "file_bt_fastresume.h"
#include <mobius/core/log.h>
#include <mobius/decoder/btencode.h>
#include <mobius/decoder/data_decoder.h>
#include <mobius/io/path.h>
#include <mobius/pod/map.h>
#include <mobius/string_functions.h>
#include <algorithm>

#include <iostream>

namespace mobius::extension::app::emuletorrent
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_bt_fastresume::file_bt_fastresume (const mobius::io::reader& reader)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  if (!reader || reader.get_size () < 2)
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create main section
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto decoder = mobius::decoder::data_decoder (reader);
  decoder.seek (0);

  section_ = mobius::core::file_decoder::section (reader, "File");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode file
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto metadata = mobius::decoder::btencode (reader);

  if (metadata.is_map ())
    _load_metadata (mobius::pod::map (metadata));

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Derived attributes
  // @see https://en.wikipedia.org/wiki/Magnet_URI_scheme
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto p = mobius::io::path (path_);
  filename_ = p.get_filename ();
  magnet_uri_ = "xt=urn:btih:" + info_hash_;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // End decoding
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  section_.end ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load metadata
// @param metadata Map object
// @see write_resume_data@write_resume_data.cpp (libtorrent)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_bt_fastresume::_load_metadata (const mobius::pod::map& metadata)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get data
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  info_hash_ = metadata.get <mobius::bytearray> ("info-hash").to_hexstring ();
  if (info_hash_.empty ())
    info_hash_ = metadata.get <mobius::bytearray> ("info-hash2").to_hexstring ();
  path_ = metadata.get <mobius::bytearray> ("save_path").to_hexstring ();
  is_paused_ = metadata.get <std::int64_t> ("paused", 0);
  is_seeding_ = metadata.get <std::int64_t> ("seed_mode", 0) || metadata.get <std::int64_t> ("super_seeding", 0);
  downloaded_bytes_ = metadata.get <std::int64_t> ("total_downloaded", 0);
  uploaded_bytes_ = metadata.get <std::int64_t> ("total_uploaded", 0);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get times
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto added_time = metadata.get <std::int64_t> ("added_time", 0);
  auto completed_time = metadata.get <std::int64_t> ("completed_time", 0);
  auto finished_time = metadata.get <std::int64_t> ("finished_time", 0);
  auto active_time = metadata.get <std::int64_t> ("active_time", 0);
  auto seeding_time = metadata.get <std::int64_t> ("seeding_time", 0);
  auto last_download = metadata.get <std::int64_t> ("last_download", 0);
  auto last_upload = metadata.get <std::int64_t> ("last_upload", 0);
  auto last_seen_complete = metadata.get <std::int64_t> ("last_seen_complete", 0);

  download_started_time_ = mobius::datetime::new_datetime_from_unix_timestamp (added_time);
  download_last_time_ = mobius::datetime::new_datetime_from_unix_timestamp (added_time + last_download);
  download_completed_time_ = mobius::datetime::new_datetime_from_unix_timestamp (completed_time);
  upload_last_time_ = mobius::datetime::new_datetime_from_unix_timestamp (added_time + last_upload);
  last_seen_complete_time_ = mobius::datetime::new_datetime_from_unix_timestamp (last_seen_complete);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get metadata
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  metadata_.set ("network", "BitTorrent");
  metadata_.set ("file_format", metadata.get <std::string> ("file-format"));
  metadata_.set ("file_version", metadata.get <std::int64_t> ("file-version"));
  metadata_.set ("libtorrent_version", metadata.get <std::string> ("libtorrent-version"));
  metadata_.set ("num_downloaders", metadata.get <std::int64_t> ("num_downloaders"));
  metadata_.set ("num_complete", metadata.get <std::int64_t> ("num_complete"));
  metadata_.set ("num_incomplete", metadata.get <std::int64_t> ("num_incomplete"));
  metadata_.set ("num_seeds", metadata.get <std::int64_t> ("num_seeds"));
  metadata_.set ("active_time", active_time);
  metadata_.set ("seeding_time", seeding_time);
  metadata_.set ("finished_time", finished_time);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get pieces
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto pieces_map = metadata.get <mobius::bytearray> ("pieces");
  auto pieces_priority = metadata.get <mobius::bytearray> ("piece_priority");

  pieces_count_ = pieces_map.size ();
  pieces_downloaded_ = pieces_map.count (0x01);
  is_completed_ = (pieces_downloaded_ == pieces_count_);

  for (std::size_t i = 0; i < pieces_map.size (); i++)
    {
      auto p = piece ();
      //p.hash_sha1 = pieces_hashes[i];
      p.is_completed = (i < pieces_map.size () && pieces_map[i] == 0x01);
      p.priority = (i < pieces_priority.size ()) ? pieces_priority[i] : 1;
      p.idx = i + 1;

      pieces_.push_back (p);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get files
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto mapped_files = std::vector <std::string> (metadata.get ("mapped_files"));
  auto file_sizes = std::vector <mobius::pod::data> (metadata.get ("file sizes"));
  size_ = 0;

  for (std::size_t i = 0;i < file_sizes.size ();i++)
    {
      auto data = std::vector <std::int64_t> (file_sizes[i]);

      file f;
      f.idx = i + 1;
      f.name = (i < mapped_files.size ()) ? mapped_files[i] : std::string ();
      f.size = data[0];
      f.last_modification_time = mobius::datetime::new_datetime_from_unix_timestamp (data[1]);

      files_.push_back (f);
      size_ += f.size;
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get trackers
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& tracker_data : metadata.get <std::vector <mobius::pod::data>> ("trackers"))
    {
      auto trackers = std::vector <std::string> (tracker_data);
      std::copy (trackers.begin (), trackers.end (), std::back_inserter (trackers_));
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get peers
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto timestamp = mobius::datetime::new_datetime_from_unix_timestamp (added_time + active_time);
  auto peers_data = metadata.get <mobius::bytearray> ("peers");
  mobius::decoder::data_decoder decoder (peers_data);

  while (decoder)
    {
      peer p;
      p.timestamp = timestamp;
      p.ip = decoder.get_ipv4_be ();
      p.port = decoder.get_uint16_le ();

      peers_.push_back (p);
    }

  auto peers6_data = metadata.get <mobius::bytearray> ("peers6");
  if (peers6_data.size () > 0)
    {
      log.development (__LINE__, "peers6 data found:");
      log.development (__LINE__, peers6_data.dump ());
    }
}

} // namespace mobius::extension::app::emuletorrent




