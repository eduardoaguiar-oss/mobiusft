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
#include "file_ed2k_fastresume.hpp"
#include <mobius/core/log.hpp>
#include <mobius/core/decoder/btencode.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/pod/map.hpp>
#include <mobius/core/string_functions.hpp>
#include <algorithm>

namespace mobius::extension::app::emuletorrent
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_ed2k_fastresume::file_ed2k_fastresume (const mobius::core::io::reader& reader)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  if (!reader || reader.get_size () < 34)
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create main section
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto decoder = mobius::core::decoder::data_decoder (reader);
  decoder.seek (0);

  section_ = mobius::core::file_decoder::section (reader, "File");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode file
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto header_section = section_.new_child ("header");

  hash_ed2k_ = decoder.get_hex_string_by_size (16);

  auto path_size = decoder.get_uint16_le ();
  path_ = mobius::core::string::replace (decoder.get_string_by_size (path_size), "\\", "/");

  file_size_ = decoder.get_uint64_le ();
  auto u1 = decoder.get_uint32_le ();
  log.development (__LINE__, "u1 = " + std::to_string (u1));

  header_section.end ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Derived attributes
  // @see https://en.wikipedia.org/wiki/Ed2k_URI_scheme
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto p = mobius::core::io::path (path_);
  filename_ = p.get_filename ();
  url_ = "ed2k://|file|" + filename_ + '|' + std::to_string (file_size_) + '|' + hash_ed2k_ + "|/";

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode metadata, if any
  // @see transfer::write_resume_data@transfer.cpp (libed2k)
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto metadata_section = section_.new_child ("metadata");

  auto metadata_size = decoder.get_uint32_le ();
  auto metadata_data = decoder.get_bytearray_by_size (metadata_size);
  auto metadata = mobius::core::decoder::btencode (metadata_data);

  if (metadata.is_map ())
    _load_metadata (mobius::core::pod::map (metadata));

  metadata_section.end ();
  section_.end ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load metadata
// @param metadata Map object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_ed2k_fastresume::_load_metadata (const mobius::core::pod::map& metadata)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get data
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::core::pod::map m (metadata);

  is_paused_ = m.get <std::int64_t> ("paused", 0);
  downloaded_bytes_ = m.get <std::int64_t> ("total_downloaded", 0);
  uploaded_bytes_ = m.get <std::int64_t> ("total_uploaded", 0);

  auto pieces_map = m.get <mobius::core::bytearray> ("pieces");
  auto pieces_priority = m.get <mobius::core::bytearray> ("piece_priority");
  auto pieces_hashes = m.get <std::vector <std::string>> ("hashset-values");

  pieces_count_ = pieces_map.size ();
  pieces_downloaded_ = pieces_map.count (0x01);
  is_completed_ = (pieces_downloaded_ == pieces_count_);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get metadata
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  metadata_.set ("network", "eDonkey");
  metadata_.set ("file_format", m.get <std::string> ("file-format"));
  metadata_.set ("file_version", m.get <std::int64_t> ("file-version"));
  metadata_.set ("libed2k_version", m.get <std::string> ("libed2k-version"));
  metadata_.set ("num_downloaders", m.get <std::int64_t> ("num_downloaders"));
  metadata_.set ("num_seeds", m.get <std::int64_t> ("num_seeds"));
  metadata_.set ("transfer_hash", m.get <std::string> ("transfer-hash"));

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get pieces
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (std::size_t i = 0; i < pieces_hashes.size (); i++)
    {
      auto p = piece ();
      p.hash_ed2k = pieces_hashes[i];
      p.is_completed = (i < pieces_map.size () && pieces_map[i] == 0x01);
      p.priority = (i < pieces_priority.size ()) ? pieces_priority[i] : 1;
      p.idx = i + 1;

      pieces_.push_back (p);
    }
}

} // namespace mobius::extension::app::emuletorrent




