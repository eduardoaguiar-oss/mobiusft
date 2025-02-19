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
#include "CDownload.h"
#include <mobius/core/log.h>
#include <mobius/decoder/mfc.h>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//! \brief Shareaza 2.7.10.2
static constexpr int SER_VERSION = 42;

} // namespace

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Decode CFragmentedFile structure
//! \see FragmentedFile.cpp - CFragmentedFile::Serialize
//! \see FileFragments/Compatibility.hpp - SerializeIn1
//! \see FileFragments/Compatibility.hpp - SerializeIn
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
CDownload::decode_CFragmentedFile (mobius::decoder::mfc& decoder, int version)
{
  if (version >= 29)
    {
      downloaded_size_ = decoder.get_qword ();
      remaining_size_ = decoder.get_qword ();

      auto count = decoder.get_dword ();

      for (std::uint32_t i = 0; i < count; i++)
        {
          CFragment fragment;
          fragment.offset = decoder.get_qword ();
          fragment.size = decoder.get_qword ();

          fragments_.push_back (fragment);
          estimated_size_ = std::max (estimated_size_, fragment.offset + fragment.size);
        }
    }

  else
    {
      downloaded_size_ = decoder.get_dword ();
      remaining_size_ = decoder.get_dword ();

      auto count = decoder.get_dword ();

      for (std::uint32_t i = 0; i < count; i++)
        {
          CFragment fragment;
          fragment.offset = decoder.get_dword ();
          fragment.size = decoder.get_dword ();

          fragments_.push_back (fragment);
          estimated_size_ = std::max (estimated_size_, fragment.offset + fragment.size);
        }
    }

  if (version >= 40)
    {
      auto count = decoder.get_dword ();

      for (std::uint32_t i = 0; i < count; i++)
        {
          CVirtualFilePart part;

          part.path = decoder.get_string ();
          part.offset = decoder.get_qword ();
          part.size = decoder.get_qword ();
          part.b_write = decoder.get_bool ();

          if (version >= 41)
            {
              part.name = decoder.get_string ();
              part.priority = decoder.get_dword ();
            }

          parts_.push_back (part);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Decode CDownloadBase structure
//! \see DownloadBase.cpp - CDownloadBase::Serialize
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
CDownload::decode_CDownloadBase (mobius::decoder::mfc& decoder, int version)
{
  name_ = decoder.get_string ();

  if (version >= 33)
    search_keyword_ = decoder.get_string ();

  if (version >= 29)
    size_ = decoder.get_qword ();
  else
    size_ = decoder.get_dword ();

  hash_sha1_ = decoder.get_hex_string (20);
  hash_sha1_trusted_ = decoder.get_dword () != 0;

  hash_tiger_ = decoder.get_hex_string (24);
  hash_tiger_trusted_ = decoder.get_dword () != 0;

  if (version >= 22)
    {
      hash_md5_ = decoder.get_hex_string (16);
      hash_md5_trusted_ = decoder.get_dword () != 0;
    }

  if (version >= 13)
    {
      hash_ed2k_ = decoder.get_hex_string (16);
      hash_ed2k_trusted_ = decoder.get_dword () != 0;
    }

  if (version >= 37)
    {
      hash_bth_ = decoder.get_hex_string (20);
      hash_bth_trusted_ = decoder.get_dword () != 0;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Decode CDownloadWithSources structure
//! \see DownloadWithSources.cpp - CDownloadWithSources::Serialize
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
CDownload::decode_CDownloadWithSources (mobius::decoder::mfc& decoder, int version)
{
  decode_CDownloadBase (decoder, version);

  auto count = decoder.get_count ();

  for (std::uint32_t i = 0; i < count; i++)
    sources_.emplace_back (decoder, version);

  count = decoder.get_count ();

  if (count)
    pxml_.decode (decoder);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Decode CDownloadWithFile structure
//! \see DownloadWithFile.cpp - CDownloadWithFile::Serialize
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
CDownload::decode_CDownloadWithFile (mobius::decoder::mfc& decoder, int version)
{
  decode_CDownloadWithSources (decoder, version);

  if (version < 28)
    local_name_ = decoder.get_string ();

  if (version < 25 || decoder.get_count ())
    decode_CFragmentedFile (decoder, version);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Decode CDownloadWithTorrent structure
//! \see DownloadWithTorrent.cpp - CDownloadWithTorrent::Serialize
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
CDownload::decode_CDownloadWithTorrent (mobius::decoder::mfc& decoder, int version)
{
  decode_CDownloadWithFile (decoder, version);

  if (version >= 22)
    btinfo_ = CBTInfo (decoder);

  if (version >= 23 && !btinfo_.get_hash_bth ().empty ())
    {
      torrent_success_ = decoder.get_dword ();

      auto block_count = btinfo_.get_block_count ();

      if (block_count)
        {
          // blocks status (TRI_TRUE, TRI_FALSE, TRI_UNKNOWN)
          decoder.skip (block_count);

          if (version_ >= 34)
            {
              is_seeding_ = decoder.get_bool ();

              if (version_ < 41)
                serving_file_name_ = decoder.get_string ();
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Decode CDownloadWithTiger structure (IMPLEMENT)
//! \see DownloadWithTiger.cpp - CDownloadWithTiger::Serialize
//! \todo Full implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
CDownload::decode_CDownloadWithTiger (mobius::decoder::mfc& decoder, int version)
{
  decode_CDownloadWithTorrent (decoder, version);

  /*
  CHashDatabase::Serialize( ar, &m_pTigerTree );

  if ( m_pTigerTree.IsAvailable() )
    {
      ar >> m_nTigerBlock;
      ar >> m_nTigerSize;
      ar >> m_nTigerSuccess;

      ReadArchive( ar, m_pTigerBlock, sizeof(BYTE) * m_nTigerBlock );
    }

  if (version >= 19)
    {
      CHashDatabase::Serialize( ar, &m_pHashset );

      if ( m_pHashset.IsAvailable() )
        {
            ar >> m_nHashsetBlock;
            ar >> m_nHashsetSuccess;

            ReadArchive( ar, m_pHashsetBlock, sizeof(BYTE) * m_nHashsetBlock );
        }
    }
  */
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Decode CDownloadWithExtras structure
//! \see DownloadWithExtras.cpp - CDownloadWithExtras::Serialize
//! \todo Full implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
CDownload::decode_CDownloadWithExtras (mobius::decoder::mfc& decoder, int version)
{
  decode_CDownloadWithTiger (decoder, version);

/*
  // CDownloadWithTiger code not fully implemented. So do not execute from here
  auto count = decoder.get_count ();

  for (std::uint32_t i = 0;i < count;i++)
    {
      CString str;
      ar >> str;
      m_pPreviews.AddTail( str );
    }

  if (version >= 32
    {
      auto count = decoder.get_count ();

      for (std::uint32_t i = 0;i < count;i++)
        {
          CDownloadReview *pReview = new CDownloadReview;
          pReview->Serialize( ar, nVersion );
          AddReview( pReview );
        }
    }
*/
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Constructor
//! \param reader Reader object
//! \see Download.cpp - CDownload::Serialize
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
CDownload::CDownload (const mobius::io::reader& reader)
{
  mobius::core::log log (__FILE__, __FUNCTION__);
  mobius::decoder::mfc decoder (reader);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Check file signature
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto signature = decoder.get_data (3);

  if (signature != "SDL")
    return;

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
  decode_CDownloadWithExtras (decoder, version_);

  signature_ = signature.to_string ();
  is_expanded_ = decoder.get_bool ();
  is_paused_ = decoder.get_bool ();
  is_boosted_ = decoder.get_bool ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  //! \see Download.cpp - CDownload::IsShared
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (version_ >= 14)
    is_shared_ = decoder.get_bool () || !hash_sha1_.empty () || !hash_ed2k_.empty () || !hash_tiger_.empty ();

  if (version_ >= 26)
    ser_id_ = decoder.get_dword ();

  if (version_ == 32)
    search_keyword_ = decoder.get_string ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Recalculate size
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  size_ = std::max (size_, estimated_size_);

  is_instance_ = true;
}

} // namespace mobius::extension::app::shareaza
