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
#include "decoder_impl_arestra.hpp"
#include "file_arestra.hpp"

namespace mobius::extension::app::ares
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode file
// @see read_details_DB_Download - helper_download_disk.pas (line 722)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
decoder_impl_arestra::decode (const mobius::core::io::reader& reader)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode file
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (!reader)
    return;

  file_arestra arestra (reader);

  if (!arestra)
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get file metadata
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  is_instance_ = true;
  section_ = arestra.get_section ();

  metadata_.set_value ("general", "signature", arestra.get_signature ());
  metadata_.set_value ("general", "file_version", arestra.get_version ());
  metadata_.set_value ("general", "file_size", arestra.get_file_size ());
  metadata_.set_value ("general", "progress", arestra.get_progress ());
  metadata_.set_value ("general", "is_completed", arestra.is_completed ());
  metadata_.set_value ("general", "is_paused", arestra.is_paused ());
  metadata_.set_value ("general", "is_corrupted", arestra.is_corrupted ());
  metadata_.set_value ("general", "media_type", arestra.get_media_type ());
  metadata_.set_value ("general", "param1", arestra.get_param1 ());
  metadata_.set_value ("general", "param2", arestra.get_param2 ());
  metadata_.set_value ("general", "param3", arestra.get_param3 ());
  metadata_.set_value ("general", "hash_sha1", arestra.get_hash_sha1 ());
  metadata_.set_value ("metadata", "kwgenre", arestra.get_kw_genre ());
  metadata_.set_value ("metadata", "title", arestra.get_title ());
  metadata_.set_value ("metadata", "artist", arestra.get_artist ());
  metadata_.set_value ("metadata", "album", arestra.get_album ());
  metadata_.set_value ("metadata", "category", arestra.get_category ());
  metadata_.set_value ("metadata", "year", arestra.get_year ());
  metadata_.set_value ("metadata", "language", arestra.get_language ());
  metadata_.set_value ("metadata", "url", arestra.get_url ());
  metadata_.set_value ("metadata", "comment", arestra.get_comment ());
  metadata_.set_value ("metadata", "subfolder", arestra.get_subfolder ());
  metadata_.set_value ("metadata", "phash_verified", arestra.get_phash_verified ());
  metadata_.set_value ("metadata", "download_started_time", arestra.get_download_started_time ());

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get alt_sources
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::vector <mobius::core::pod::data> alt_sources;

  for (const auto& [ip, port] : arestra.get_alt_sources ())
    alt_sources.emplace_back (mobius::core::pod::data {ip, port});

  metadata_.set_value ("general", "alt_sources", alt_sources);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get gaps
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::vector <mobius::core::pod::data> gaps;

  for (const auto& [start, end] : arestra.get_gaps ())
    gaps.emplace_back (mobius::core::pod::data {start, end});

  metadata_.set_value ("general", "gaps", gaps);

}

} // namespace mobius::extension::app::ares




