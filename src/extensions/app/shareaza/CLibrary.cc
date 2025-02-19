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
#include "CLibrary.h"
#include <mobius/core/log.h>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//! \brief Shareaza 2.7.10.2
static constexpr int SER_VERSION = 29;

} // namespace

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Decode CLibrary structure
//! \see Library.cpp - CLibrary::Serialize
//! \todo AlbumFolder, LibraryHistory and LibraryMaps
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
CLibrary::CLibrary (mobius::decoder::mfc& decoder)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

  last_modification_time_ = decoder.get_nt_time ();

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
  // LibraryDictionary.cpp - CLibraryDictionary::Serialize
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (version_ >= 29)
    words_count_ = decoder.get_dword ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // LibraryMaps.cpp - CLibraryMaps::Serialize1
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  next_index_ = decoder.get_dword ();

  if (version_ >= 28)
    {
      index_map_count_ = decoder.get_dword ();
      name_map_count_ = decoder.get_dword ();
      path_map_count_ = decoder.get_dword ();
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // CLibraryFolders
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto count = decoder.get_count ();

  for (std::uint32_t i = 0;i < count; i++)
    folders_.emplace_back (decoder, version_, CLibraryFolder ());

  is_instance_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get all folders from CLibrary
//! \return Folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <CLibraryFolder>
CLibrary::get_all_folders () const
{
  std::vector <CLibraryFolder> folders = get_folders ();
  std::size_t i = 0;

  while (i < folders.size ())
    {
      auto children = folders[i].get_children ();
      std::copy (children.begin (), children.end (), std::back_inserter (folders));
      ++i;
    }

  return folders;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get all files from CLibrary
//! \return Files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <CLibraryFile>
CLibrary::get_all_files () const
{
  std::vector <CLibraryFile> files;

  for (const auto& folder : get_all_folders ())
    {
      auto f_files = folder.get_files ();
      std::copy (f_files.begin (), f_files.end (), std::back_inserter (files));
    }

  return files;
}

} // namespace mobius::extension::app::shareaza
