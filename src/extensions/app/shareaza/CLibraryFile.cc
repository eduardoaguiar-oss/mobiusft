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
#include "CLibraryFile.h"
#include "CLibraryFolder.h"
#include <mobius/core/log.h>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Tristate constants
// @see ShareazaOM.h
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
constexpr std::uint32_t TRI_UNKNOWN = 0;
constexpr std::uint32_t TRI_FALSE = 1;
constexpr std::uint32_t TRI_TRUE = 2;

} // namespace

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode CLibraryFile structure
// @param decoder MFC decoder object
// @param version CLibrary structure version
// @param parent Parent CLibraryFile, if any
// @see SharedFile.cpp - CLibraryFile::Serialize (UNCHECKED)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
CLibraryFile::CLibraryFile (
  mobius::decoder::mfc& decoder,
  int version,
  const CLibraryFolder& parent
)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  name_ = decoder.get_string ();
  path_ = parent.get_path () + '\\' + name_;
  index_ = decoder.get_dword ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // size
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (version >= 17)
    size_ = decoder.get_qword ();

  else
    size_ = decoder.get_dword ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // last_modification_time
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  last_modification_time_ = decoder.get_nt_time ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // is_shared_ (true/false)
  // @see SharedFolder.cpp - CLibraryFile::IsShared
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t b_shared;

  if (version >= 5)
    b_shared = decoder.get_dword ();

  else
    b_shared = decoder.get_bool () ? TRI_UNKNOWN : TRI_FALSE;

  if (b_shared == TRI_UNKNOWN)
    is_shared_ = parent.is_shared ();

  else
    is_shared_ = (b_shared == TRI_TRUE);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // virtual_size
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (version >= 21)
    {
      virtual_size_ = decoder.get_qword ();

      if (virtual_size_ > 0)
        virtual_base_ = decoder.get_qword ();
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // hashes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  hash_sha1_ = decoder.get_hex_string (20);

  if (version >= 8)
    hash_tiger_ = decoder.get_hex_string (24);

  if (version >= 11)
    {
      hash_md5_ = decoder.get_hex_string (16);
      hash_ed2k_ = decoder.get_hex_string (16);
    }

  if (version >= 26)
    hash_bth_ = decoder.get_hex_string (20);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Other data
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (version >= 4)
    b_verify_ = decoder.get_dword ();

  uri_ = decoder.get_string ();

  if (!uri_.empty ())
    {
      if (version < 27)
        {
          b_metadata_auto_ = decoder.get_bool ();

          if (!b_metadata_auto_)
            metadata_time_ = decoder.get_nt_time ();
        }

      pxml_.decode (decoder);
    }

  if (version >= 13)
    {
      rating_ = decoder.get_int ();
      comments_ = decoder.get_string ();

      if (version >= 16)
        share_tags_ = decoder.get_string ();

      if (version >= 27)
        {
          b_metadata_auto_ = decoder.get_bool ();
          metadata_time_ = decoder.get_nt_time ();
        }

      else
        {
          if (b_metadata_auto_ && (rating_ != -1 || !comments_.empty ()))
            metadata_time_ = decoder.get_nt_time ();
        }
    }

  hits_total_ = decoder.get_dword ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @see WndMain.cpp - CMainWnd::OnNowUploading
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  uploads_started_ = decoder.get_dword ();

  if (version >= 14)
    b_cached_preview_ = decoder.get_bool ();

  if (version >= 20)
    b_bogus_ = decoder.get_bool ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Sources
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (version >= 2)
    {
      auto count = decoder.get_count ();

      for (std::uint32_t i = 0; i < count; i++)
        sources_.emplace_back (decoder, version);
    }
}

} // namespace mobius::extension::app::shareaza




