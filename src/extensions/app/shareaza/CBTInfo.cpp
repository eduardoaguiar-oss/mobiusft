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
#include "CBTInfo.hpp"
#include "CShareaza.hpp"
#include <mobius/core/log.hpp>
#include <mobius/core/decoder/btencode.hpp>
#include <mobius/string_functions.h>

#include <iostream>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Shareaza 2.7.10.2
static constexpr int SER_VERSION = 11;

} // namespace

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode CBTFile structure
// @param decoder MFC decoder object
// @param version Structure version
// @see CBTFile::Serialize@BTInfo.cpp
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
CBTInfo::decode_CBTFile (mobius::core::decoder::mfc& decoder, int version)
{
  CBTFile f;

  if (version >= 2)
    f.size = decoder.get_qword ();

  else
    f.size = decoder.get_dword ();

  f.path = decoder.get_string ();

  if (version >= 9)
    f.name = decoder.get_string ();

  f.hash_sha1 = decoder.get_hex_string (20);

  if (version >= 4)
    {
      f.hash_ed2k = decoder.get_hex_string (16);
      f.hash_tiger = decoder.get_hex_string (24);

      if (version < 8)
        f.priority = decoder.get_int ();
    }

  if (version >= 6)
    f.hash_md5 = decoder.get_hex_string (16);

  files_.push_back (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode tracker structure
// @param decoder MFC decoder object
// @see CBTTracker::Serialize@BTInfo.cpp
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
CBTInfo::decode_CBTTracker (mobius::core::decoder::mfc& decoder)
{
  CBTTracker tracker;

  tracker.address = decoder.get_string ();
  tracker.last_access_time = decoder.get_unix_time ();
  tracker.last_success_time = decoder.get_unix_time ();
  tracker.next_try_time = decoder.get_unix_time ();
  tracker.failures = decoder.get_dword ();
  tracker.tier = decoder.get_int ();
  tracker.type = decoder.get_int ();

  trackers_.push_back (tracker);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param decoder MFC decoder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
CBTInfo::CBTInfo (mobius::core::decoder::mfc& decoder)
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

  if (version_ < 1)
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get BTH hash
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  hash_bth_ = decoder.get_hex_string (20);

  if (hash_bth_.empty ())
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode data
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (version_ >= 2)
    size_ = decoder.get_qword ();

  else
    size_ = decoder.get_dword ();

  block_size_ = decoder.get_dword ();
  block_count_ = decoder.get_dword ();

  for (std::uint32_t i = 0; i < block_count_; i++)
    block_hashes_.push_back (decoder.get_data (20).to_hexstring ());

  if (version_ >= 4)
    total_uploaded_ = decoder.get_qword ();

  if (version_ >= 6)
    total_downloaded_ = decoder.get_qword ();

  name_ = decoder.get_string ();

  if (version_ >= 3)
    {
      encoding_ = decoder.get_dword ();
      comments_ = decoder.get_string ();
      creation_time_ = decoder.get_unix_time ();
      created_by_ = decoder.get_string ();
    }

  if (version_ >= 5)
    b_private_ = decoder.get_bool ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode files
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto count = decoder.get_count ();

  for (std::uint32_t i = 0; i < count; i++)
    decode_CBTFile (decoder, version_);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode trackers
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (version_ < 7)
    tracker_ = decoder.get_string ();

  if (version_ >= 4)
    {
      tracker_index_ = decoder.get_int ();
      tracker_mode_ = decoder.get_int ();

      if (version_ < 7)
        {
          if (decoder.get_count ())
            decode_CBTTracker (decoder);
        }

      count = decoder.get_count ();

      for (std::uint32_t i = 0; i < count; i++)
        decode_CBTTracker (decoder);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode .torrent data
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (version_ >= 10)
    {
      auto length = decoder.get_dword ();

      if (length > 0)
        {
          auto data = decoder.get_data (length);

          if (data)
            data_ = mobius::core::decoder::btencode (data);

          if (version_ >= 11)
            {
              info_start_ = decoder.get_dword ();       // info section start?
              info_size_ = decoder.get_dword ();        // info section size
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get metadata from torrent data
// @return Torrent metadata
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::map <std::string, std::string>
CBTInfo::get_metadata () const
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get info sub-dictionary
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto info = data_.get ("info");

  if (!info.is_map ())
    return {};

  mobius::core::pod::map m (info);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Get data from info
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::map <std::string, std::string> metadata;

  for (const auto& [k, v] : m)
    {
      bool flag_known = true;
      std::string value;

      if (k == "length")
        value = v.to_string ();

      else if (k == "name")
        value = std::string (v);

      else if (k == "name.utf-8")
        value = std::string (v);

      else if (k == "piece length")
        value = v.to_string ();

      else
        flag_known = false;

      if (flag_known)
        {
          const std::string key = "torrent_info_" + mobius::string::replace (k, " ", "_");
          metadata[key] = value;
        }
    }

  return metadata;
}

} // namespace mobius::extension::app::shareaza




