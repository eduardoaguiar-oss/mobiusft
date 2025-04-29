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
#include "CQueryHit.hpp"
#include "CShareaza.hpp"

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode QueryHit structure
// @see QueryHit.cpp - QueryHit::Serialize
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
CQueryHit::decode (mobius::core::decoder::mfc& decoder, int version)
{
  search_id_ = decoder.get_guid ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Protocol
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (version >= 9)
    {
      protocol_id_ = decoder.get_dword ();
      protocol_name_ = mobius::extension::app::shareaza::get_protocol_name (protocol_id_);
    }

  client_id_ = decoder.get_guid ();
  ip_ = decoder.get_ipv4 ();

  if (version >= 16)
    ipv6_ = decoder.get_ipv6 ();

  port_ = decoder.get_word ();
  speed_ = decoder.get_dword ();
  str_speed_ = decoder.get_string ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Vendor
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  vendor_code_ = decoder.get_string ();
  vendor_name_ = mobius::extension::app::shareaza::get_vendor_name (vendor_code_);
  tri_push_ = decoder.get_dword ();
  tri_busy_ = decoder.get_dword ();
  tri_stable_ = decoder.get_dword ();
  tri_measured_ = decoder.get_dword ();
  up_slots_ = decoder.get_int ();
  up_queue_ = decoder.get_int ();
  b_chat_ = decoder.get_bool ();
  b_browse_host_ = decoder.get_bool ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Hashes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  hash_sha1_ = decoder.get_hex_string (20);
  hash_tiger_ = decoder.get_hex_string (24);
  hash_ed2k_ = decoder.get_hex_string (16);

  if (version >= 13)
    {
      hash_bth_ = decoder.get_hex_string (20);
      hash_md5_ = decoder.get_hex_string (16);
    }

  url_ = decoder.get_string ();
  name_ = decoder.get_string ();
  index_ = decoder.get_dword ();
  b_size_ = decoder.get_bool ();

  if (version >= 10)
    size_ = decoder.get_qword ();
  else
    size_ = decoder.get_dword ();

  hit_sources_ = decoder.get_dword ();
  partial_ = decoder.get_dword ();
  b_preview_ = decoder.get_bool ();
  preview_ = decoder.get_string ();

  if (version >= 11)
    b_collection_ = decoder.get_bool ();

  schema_uri_ = decoder.get_string ();
  schema_plural_ = decoder.get_string ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // XML metadata
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (!schema_uri_.empty ())
    pxml_.decode (decoder);

  rating_ = decoder.get_int ();
  comments_ = decoder.get_string ();
  b_matched_ = decoder.get_bool ();

  if (version >= 12)
    b_exact_match_ = decoder.get_bool ();

  b_bogus_ = decoder.get_bool ();
  b_download_ = decoder.get_bool ();

  if (version >= 15)
    nick_ = decoder.get_string ();

  if (hit_sources_ == 0 && !url_.empty ())
    hit_sources_ = 1;
}

} // namespace mobius::extension::app::shareaza




