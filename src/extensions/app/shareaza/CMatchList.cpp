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
#include "CMatchList.hpp"
#include <mobius/core/log.hpp>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Shareaza 2.7.10.2
static constexpr int SER_VERSION = 15;

} // namespace

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode CMatchList structure
// @see MatchObjects.cpp - CMatchList::Serialize
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
CMatchList::decode (mobius::decoder::mfc& decoder)
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
  filter_name_ = decoder.get_string ();
  filter_busy_ = decoder.get_bool ();
  filter_push_ = decoder.get_bool ();
  filter_unstable_ = decoder.get_bool ();
  filter_reject_ = decoder.get_bool ();
  filter_local_ = decoder.get_bool ();
  filter_bogus_ = decoder.get_bool ();

  if (version_ >= 12)
    {
      filter_drm_ = decoder.get_bool ();
      filter_adult_ = decoder.get_bool ();
      filter_suspicious_ = decoder.get_bool ();
      regexp_ = decoder.get_bool ();
    }

  if (version_ >= 10)
    {
      filter_min_size_ = decoder.get_qword ();
      filter_max_size_ = decoder.get_qword ();
    }

  else
    {
      filter_min_size_ = decoder.get_dword ();
      filter_max_size_ = decoder.get_dword ();
    }

  filter_sources_ = decoder.get_dword ();
  sort_column_ = decoder.get_int ();
  sort_dir_ = decoder.get_bool ();

  auto count = decoder.get_count ();

  for (std::uint32_t i = 0;i < count; i++)
    {
      CMatchFile mf;
      mf.decode (decoder, version_);

      match_files_.push_back (mf);
    }
}

} // namespace mobius::extension::app::shareaza




