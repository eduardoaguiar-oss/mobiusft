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
#include "CQuerySearch.hpp"
#include <mobius/core/log.hpp>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Shareaza 2.7.10.2
static constexpr int SER_VERSION = 8;

} // namespace

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode CQuerySearch structure
// @see QuerySearch.cpp - CQuerySearch::Serialize
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
CQuerySearch::decode (mobius::core::decoder::mfc& decoder)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Check version
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  version_ = decoder.get_int ();

  if (version_ > SER_VERSION)
    {
      log.development (__LINE__, "Unhandled version: " + std::to_string (version_));
      return;
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode data
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  guid_ = decoder.get_guid ();
  text_ = decoder.get_string ();
  hash_sha1_ = decoder.get_hex_string (20);
  hash_tiger_ = decoder.get_hex_string (24);
  hash_ed2k_ = decoder.get_hex_string (16);
  hash_bth_ = decoder.get_hex_string (20);

  if (version_ >= 7)
    hash_md5_ = decoder.get_hex_string (16);

  uri_ = decoder.get_string ();

  if (!uri_.empty ())
    pxml_.decode (decoder);

  if (version_ >= 5)
    {
      flag_want_url_ = decoder.get_bool ();
      flag_want_dn_ = decoder.get_bool ();
      flag_want_xml_ = decoder.get_bool ();
      flag_want_com_ = decoder.get_bool ();
      flag_want_pfs_ = decoder.get_bool ();
    }

  if (version_ >= 8)
    {
      min_size_ = decoder.get_qword ();
      max_size_ = decoder.get_qword ();
    }
}

} // namespace mobius::extension::app::shareaza




