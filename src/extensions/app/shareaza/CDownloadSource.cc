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
#include "CDownloadSource.h"
#include "CShareaza.h"
#include <mobius/io/uri.h>
#include <mobius/string_functions.h>

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode fragment
// @param decoder MFC decoder object
// @param version Structure version
// @see FileFragments/Compatibility.hpp - SerializeIn
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
CDownloadSource::decode_fragment (mobius::decoder::mfc& decoder, int version)
{
  CFragment fragment;

  if (version >= 29)
    {
      fragment.offset = decoder.get_qword ();
      fragment.size = decoder.get_qword ();
    }

  else
    {
      fragment.offset = decoder.get_dword ();
      fragment.size = decoder.get_dword ();
    }

  fragments_.push_back (fragment);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode fragments
// @param decoder MFC decoder object
// @param version Structure version
// @see FileFragments/Compatibility.hpp - SerializeIn2
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
CDownloadSource::decode_fragments (mobius::decoder::mfc& decoder, int version)
{
  if (version >= 20)
    {
      auto count = decoder.get_count ();

      for (std::uint32_t i = 0; i < count; i++)
        decode_fragment (decoder, version);
    }

  else if (version >= 5)
    {
      while (decoder.get_count ())
        decode_fragment (decoder, version);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param decoder MFC decoder object
// @param version Structure version
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
CDownloadSource::CDownloadSource (mobius::decoder::mfc& decoder, int version)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // version >= 21
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (version >= 21)
    {
      url_ = decoder.get_string ();
      protocol_id_ = decoder.get_int ();
      protocol_name_ = mobius::extension::app::shareaza::get_protocol_name (protocol_id_);

      auto b_has_guid = decoder.get_dword ();
      if (b_has_guid)
        guid_ = decoder.get_guid ();

      port_ = decoder.get_word ();
      if (port_)
        ip_ = decoder.get_ipv4 ();

      server_port_ = decoder.get_word ();
      if (server_port_)
        server_ip_ = decoder.get_ipv4 ();

      name_ = decoder.get_string ();
      index_ = decoder.get_dword ();
      b_hash_auth_ = decoder.get_bool ();
      b_hash_sha1_ = decoder.get_bool ();
      b_hash_tiger_ = decoder.get_bool ();
      b_hash_ed2k_ = decoder.get_bool ();

      if (version >= 37)
        {
          b_hash_bth_ = decoder.get_bool ();
          b_hash_md5_ = decoder.get_bool ();
        }

      server_ = decoder.get_string ();

      if (version >= 24)
        nick_ = decoder.get_string ();

      if (version >= 36)
        country_code_ = decoder.get_string ();

      if (version >= 38)
        country_name_ = decoder.get_string ();

      speed_ = decoder.get_dword ();
      b_push_only_ = decoder.get_bool ();
      b_close_conn_ = decoder.get_bool ();
      b_read_content_ = decoder.get_bool ();
      last_seen_time_ = decoder.get_nt_time ();

      decode_fragments (decoder, version);

      if (version >= 39)
        b_client_extended_ = decoder.get_bool ();

      if (version >= 42)
        b_meta_ignore_ = decoder.get_bool ();
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // version < 21
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  else
    {
      ip_ = decoder.get_ipv4 ();
      port_ = decoder.get_word ();
      speed_ = decoder.get_dword ();
      index_ = decoder.get_dword ();
      name_ = decoder.get_string ();

      if (version >= 4)
        url_ = decoder.get_string ();

      b_hash_sha1_ = decoder.get_bool ();

      if (version >= 13)
        {
          b_hash_tiger_ = decoder.get_bool ();
          b_hash_ed2k_ = decoder.get_bool ();
        }

      if (version >= 10)
        b_hash_auth_ = decoder.get_bool ();

      if (version == 8)
        server_ = decoder.get_data (4).to_string ();

      if (version >= 9)
        server_ = decoder.get_string ();

      b_push_only_ = decoder.get_bool ();
      b_read_content_ = decoder.get_bool ();

      if (version >= 7)
        b_close_conn_ = decoder.get_bool ();

      if (version >= 12)
        last_seen_time_ = decoder.get_nt_time ();

      decoder.skip (16);
      guid_ = decoder.get_guid ();

      decode_fragments (decoder, version);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // If IP is empty, try to get IP from ED2KFTP URL
  // @see DownloadWithSources.cpp - CDownloadWithSources::Serialize
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (ip_.empty () && mobius::string::startswith (url_, "ed2kftp://"))
    {
      auto uri = mobius::io::uri (url_);

      ip_ = uri.get_host ();
      port_ = uri.get_port_number ();
    }
}

} // namespace mobius::extension::app::shareaza




