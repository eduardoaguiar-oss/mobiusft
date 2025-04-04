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
#include "decoder_impl_sharel.hpp"
#include "file_sharel.hpp"
#include "common.hpp"
#include <mobius/decoder/data_decoder.h>

namespace mobius::extension::app::ares
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode file
// @see get_cached_metas (helper_library_db.pas)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
decoder_impl_sharel::decode (const mobius::io::reader& reader)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode file
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (!reader)
    return;

  file_sharel sharel (reader);

  if (!sharel)
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get file metadata
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  is_instance_ = true;
  metadata_.set_value ("general", "signature", sharel.get_signature ());
  metadata_.set_value ("general", "entries", sharel.get_count ());
  section_ = sharel.get_section ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get entries
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& entry : sharel)
    {
      mobius::core::file_decoder::entry e (entry.idx, entry.hash_sha1);

      e.set_metadata ("hash_sha1", entry.hash_sha1);
      e.set_metadata ("is_corrupted", entry.is_corrupted);
      e.set_metadata ("size", entry.size);
      e.set_metadata ("param1", entry.param1);
      e.set_metadata ("param2", entry.param2);
      e.set_metadata ("param3", entry.param3);
      e.set_metadata ("path", entry.path);
      e.set_metadata ("title", entry.title);
      e.set_metadata ("artist", entry.artist);
      e.set_metadata ("album", entry.album);
      e.set_metadata ("category", entry.category);
      e.set_metadata ("year", entry.year);
      e.set_metadata ("vidinfo", entry.vidinfo);
      e.set_metadata ("language", entry.language);
      e.set_metadata ("url", entry.url);
      e.set_metadata ("comment", entry.comment);
      e.set_metadata ("hash_of_phash", entry.hash_of_phash);
      e.set_metadata ("media_type", entry.media_type);

      entries_.push_back (e);
    }
}

} // namespace mobius::extension::app::ares




