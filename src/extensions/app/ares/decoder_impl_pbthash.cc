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
#include "decoder_impl_pbthash.h"
#include "file_pbthash.h"

namespace mobius::extension::app::ares
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Decode file
//! \see read_details_DB_Download - helper_download_disk.pas (line 722)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
decoder_impl_pbthash::decode (const mobius::io::reader& reader)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode file
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (!reader)
    return;

  file_pbthash pbthash (reader);

  if (!pbthash)
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get file metadata
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  is_instance_ = true;
  section_ = pbthash.get_section ();

  metadata_.set_value ("general", "file_version", pbthash.get_version ());
  metadata_.set_value ("general", "hash_sha1", pbthash.get_hash_sha1 ());
  metadata_.set_value ("general", "is_paused", pbthash.is_paused ());
  metadata_.set_value ("general", "is_seeding", pbthash.is_seeding ());
  metadata_.set_value ("general", "is_completed", pbthash.is_completed ());
  metadata_.set_value ("general", "file_size", pbthash.get_file_size ());
  metadata_.set_value ("general", "piece_size", pbthash.get_piece_size ());
  metadata_.set_value ("general", "bytes_downloaded", pbthash.get_bytes_downloaded ());
  metadata_.set_value ("general", "bytes_uploaded", pbthash.get_bytes_uploaded ());
  metadata_.set_value ("general", "pieces_count", pbthash.get_pieces_count ());
  metadata_.set_value ("general", "files_count", pbthash.get_files_count ());
  metadata_.set_value ("general", "torrent_path", pbthash.get_torrent_path ());
  metadata_.set_value ("general", "torrent_name", pbthash.get_torrent_name ());
  metadata_.set_value ("general", "name", pbthash.get_name ());
  metadata_.set_value ("general", "comment", pbthash.get_comment ());
  metadata_.set_value ("general", "download_started_time", pbthash.get_download_started_time ());
  metadata_.set_value ("general", "elapsed_time", pbthash.get_elapsed_time ());

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get pieces
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::vector <mobius::pod::data> pieces;

  for (const auto& p : pbthash.get_pieces ())
    pieces.emplace_back (mobius::pod::data {p.hash_sha1, p.is_checked});

  metadata_.set_value ("general", "pieces", pieces);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get files
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::vector <mobius::pod::data> files;

  for (const auto& f : pbthash.get_files ())
    files.emplace_back (mobius::pod::data {f.name, f.path, f.size, f.last_modification_time});

  metadata_.set_value ("general", "files", files);
}

} // namespace mobius::extension::app::ares
