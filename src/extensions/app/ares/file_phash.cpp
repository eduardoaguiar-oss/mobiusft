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
#include "file_phash.hpp"
#include "common.hpp"
#include <mobius/core/log.hpp>
#include <mobius/core/decoder/data_decoder.hpp>

namespace mobius::extension::app::ares
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @see ICH_loadPieces@helper_ICH.pas (line 528)
// @see ICH_load_phash_indexs@helper_ICH.pas (line 1023)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_phash::file_phash (const mobius::core::io::reader& reader)
{
  if (!reader || reader.get_size () < 14)
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create main section
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto decoder = mobius::core::decoder::data_decoder (reader);
  decoder.seek (0);

  section_ = mobius::core::file_decoder::section (reader, "File");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode header
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto header_section = section_.new_child ("header");

  auto signature = decoder.get_bytearray_by_size (14);
  version_ = 0;

  if (signature == "__ARESDBP102__")
    version_ = 2;

  else if (signature == "__ARESDBP103__")
    version_ = 3;

  else
    return;

  is_instance_ = true;
  signature_ = signature.to_string ();

  header_section.end ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode entries
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  while (decoder)
    {
      std::uint64_t idx = entries_.size () + 1;
      auto entry_section = section_.new_child ("Entry #" + std::to_string (idx));

      auto e = _decode_entry (decoder);
      e.idx = idx;
      entries_.push_back (e);

      entry_section.end ();
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode entry
// @param decoder Decoder object
// @return Entry
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_phash::entry
file_phash::_decode_entry (mobius::core::decoder::data_decoder& decoder)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create entry
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  entry e;

  e.idx = entries_.size () + 1;
  e.num64 = decoder.get_uint64_le ();
  auto pieces_data_size = decoder.get_uint32_le ();
  e.hash_sha1 = decoder.get_hex_string_by_size (20);
  e.num32 = decoder.get_uint32_le ();

  if (e.num64 != 1)
    log.development (__LINE__, "num64 = " + std::to_string (e.num64));

  if (e.num32 != 1)
    log.development (__LINE__, "num32 = " + std::to_string (e.num32));

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode pieces
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  e.pieces = _decode_pieces (decoder, pieces_data_size);
  e.progress = 0;
  e.pieces_count = e.pieces.size ();
  e.piece_size = 0;
  e.pieces_completed = 0;

  bool has_unknown = false;
  bool has_no = false;

  for (const auto& p : e.pieces)
    {
      e.progress += p.progress;
      e.piece_size = std::max (e.piece_size, p.progress);

      if (p.is_completed == STATE_UNKNOWN)
        has_unknown = true;

      else if (p.is_completed == STATE_NO)
        has_no = true;

      else if (p.is_completed == STATE_YES)
        ++e.pieces_completed;
    }

  e.pieces_to_go = e.pieces_count - e.pieces_completed;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Set entry.is_completed
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (has_unknown)
    e.is_completed = STATE_UNKNOWN;

  else if (has_no)
    e.is_completed = STATE_NO;

  else
    e.is_completed = STATE_YES;

  return e;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode pieces
// @param decoder Decoder object
// @return Pieces
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <file_phash::piece>
file_phash::_decode_pieces (mobius::core::decoder::data_decoder& decoder, std::uint32_t data_size)
{
  std::vector <piece> pieces;

  while (data_size > 0)
    {
      auto p = piece ();

      p.idx = pieces.size () + 1;
      p.progress = 0;
      p.is_completed = STATE_UNKNOWN;

      if (version_ == 3)
        {
          p.is_completed = (decoder.get_uint8 () == 1) ? STATE_YES : STATE_NO;
          p.progress = decoder.get_uint64_le ();
          data_size -= 9;
        }

      p.hash_sha1 = decoder.get_hex_string_by_size (20);
      pieces.push_back (p);

      data_size -= 20;
    }

  return pieces;
}

} // namespace mobius::extension::app::ares




