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
#include "file_shareh.hpp"
#include "common.hpp"
#include <mobius/core/log.hpp>
#include <mobius/decoder/data_decoder.h>
#include <mobius/core/encoder/hexstring.hpp>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Populate entry metadata
// @param e Entry
// @param data Raw data
// @see get_trusted_metas@helper_library_db.pas
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
_populate_metadata (mobius::extension::app::ares::file_shareh::entry& e, const mobius::bytearray& data)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  for (const auto& [i, v] : mobius::extension::app::ares::decode_metadata (data))
    {
      switch (i)
        {
          case 2: e.title = v.to_string (); break;
          case 3: e.artist = v.to_string (); break;
          case 4: e.album = v.to_string (); break;
          case 5: e.category = v.to_string (); break;
          case 6: e.year = v.to_string (); break;
          case 8: e.language = v.to_string (); break;
          case 9: e.url = v.to_string (); break;
          case 10: e.comment = v.to_string (); break;
          case 11:
            {
              mobius::decoder::data_decoder decoder (v);
              e.download_completed_time = decoder.get_unix_datetime ();
            }
            break;
          case 17: e.is_corrupted = true; break;
          default:
              log.development (__LINE__, "unhandled field_type: " + std::to_string (i));
        }
    }
}

} // namespace

namespace mobius::extension::app::ares
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @see get_trusted_metas function (helper_library_db.pas)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_shareh::file_shareh (const mobius::io::reader& reader)
{
  if (!reader || reader.get_size () < 14)
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create main section
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto decoder = mobius::decoder::data_decoder (reader);
  decoder.seek (0);

  section_ = mobius::core::file_decoder::section (reader, "File");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode header
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto header_section = section_.new_child ("file-header");
  auto signature = decoder.get_bytearray_by_size (14);

  if (signature != "__ARESDB1.02H_")
    return;

  is_instance_ = true;
  signature_ = signature.to_string ();

  header_section.end ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode entries
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint64_t idx = 0;
  shared_count_ = 0;

  while (decoder)
    {
      auto entry_section = section_.new_child ("Entry #" + std::to_string (++idx));

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Create data section
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      auto data_section = entry_section.new_child ("entry-header");

      auto enc_data = decoder.get_bytearray_by_size (23);
      auto data = decrypt (enc_data, 13871);

      data_section.set_data (data);
      data_section.end ();

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Create entry
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      entry e;
      e.idx = idx;
      e.hash_sha1 = mobius::core::encoder::hexstring (data.slice (0,19));
      e.is_shared = (data[20] == 1);
      shared_count_ += e.is_shared;

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Decode metadata, if any
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      std::uint16_t data_size = data[21] | (static_cast<std::uint16_t> (data[22]) << 8);

      if (data_size)
        {
          auto metadata_section = entry_section.new_child ("entry-metadata");

          enc_data = decoder.get_bytearray_by_size (data_size);
          data = decrypt (enc_data, 13872);
          _populate_metadata (e, data);

          metadata_section.set_data (data);
          metadata_section.end ();
        }

      entries_.push_back (e);
      entry_section.end ();
    }
}

} // namespace mobius::extension::app::ares




