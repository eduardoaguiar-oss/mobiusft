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
#include "decoder_impl_torrenth.h"
#include "file_torrenth.h"
#include <algorithm>

namespace mobius::extension::app::ares
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Decode file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
decoder_impl_torrenth::decode (const mobius::io::reader& reader)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode file
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (!reader)
    return;

  file_torrenth torrenth (reader);

  if (!torrenth)
    return;

  is_instance_ = true;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get file metadata
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  metadata_.set_value ("general", "signature", torrenth.get_signature ());
  metadata_.set_value ("general", "entries", torrenth.get_count ());
  section_ = torrenth.get_section ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get entries
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& entry : torrenth)
    {
      mobius::core::file_decoder::entry e (entry.idx, entry.hash_sha1);

      e.set_metadata ("hash_sha1", entry.hash_sha1);
      e.set_metadata ("timestamp", entry.timestamp);
      e.set_metadata ("size", entry.size);
      e.set_metadata ("seeds", entry.seeds);
      e.set_metadata ("media_type", entry.media_type);
      e.set_metadata ("url", entry.url);
      e.set_metadata ("name", entry.name);
      e.set_metadata ("evaluated_hash_sha1", entry.evaluated_hash_sha1);

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Set trackers
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      std::vector <mobius::pod::data> trackers (entry.trackers.size ());

      std::transform (
          entry.trackers.begin (),
          entry.trackers.end (),
          trackers.begin (),
          [](const auto& t){ return mobius::pod::data (t); }
      );

      e.set_metadata ("trackers", trackers);

      entries_.push_back (e);
    }
}

} // namespace mobius::extension::app::ares
