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
#include "common.hpp"
#include <mobius/core/decoder/data_decoder.hpp>
#include <vector>

namespace mobius::extension::app::ares
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt data
// @param enc_data Encrypted data
// @return Decrypted data
// @see helper_library_db.pas
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
decrypt (const mobius::core::bytearray& enc_data, std::uint16_t seed)
{
  mobius::core::bytearray data (enc_data.size ());
  auto iter = data.begin ();

  for (auto c : enc_data)
    {
      *iter = (c ^ (seed >> 8));
      seed = (c + seed) * 23219 + 36126;
      ++iter;
    }

  return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert media type to string
// @param t Media type
// @see mediatype_to_str function (helper_mimetypes.pas)
// @see const_ares.pas
// @see vars_localiz.pas
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
media_type_to_string (std::uint8_t t)
{
  switch (t)
  {
    case 0: return "Other"; break;
    case 1: return "Audio"; break;
    case 2: return "Audio"; break;
    case 3: return "Software"; break;
    case 4: return "Audio"; break;
    case 5: return "Video"; break;
    case 6: return "Document"; break;
    case 7: return "Image"; break;

    default: return "Other";
  };
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decoder old altsources
// @param data Encoded data
// @return Alt sources
// @see add_sources function @helper_altsources (line 106)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <std::pair <std::string, std::uint16_t>>
decode_old_alt_sources (const mobius::core::bytearray& data)
{
  std::vector <std::pair <std::string, std::uint16_t>> sources;
  auto decoder = mobius::core::decoder::data_decoder (data);

  while (decoder)
    {
      auto ip = decoder.get_ipv4_be ();
      auto port = decoder.get_uint16_le ();
      sources.emplace_back (ip, port);
      decoder.skip (6);  // ip_server, port_server
    }

  return sources;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decoder altsources
// @param data Encoded data
// @return Alt sources
// @see add_sources function @helper_altsources (line 106)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <std::pair <std::string, std::uint16_t>>
decode_alt_sources (const mobius::core::bytearray& data)
{
  std::vector <std::pair <std::string, std::uint16_t>> sources;
  auto decoder = mobius::core::decoder::data_decoder (data);

  while (decoder)
    {
      auto ip = decoder.get_ipv4_be ();
      auto port = decoder.get_uint16_le ();
      sources.emplace_back (ip, port);

      decoder.skip (11);  // ip_server, port_server, ip_alt, dummy
    }

  return sources;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get metadata from data
// @param data Encoded data
// @param type Metadata type
// @see get_cached_metas@helper_library_db.pas
// @see get_trusted_metas@helper_library_db.pas
// @see read_details_DB_Download@helper_download_disk.pas
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::map <int, mobius::core::bytearray>
decode_metadata (const mobius::core::bytearray& data)
{
  std::map <int, mobius::core::bytearray> metadata;

  auto decoder = mobius::core::decoder::data_decoder (data);

  while (decoder)
    {
      auto field_type = decoder.get_uint8 ();
      auto field_size = decoder.get_uint16_le ();
      auto field_data = decoder.get_bytearray_by_size (field_size);
      metadata[field_type] = field_data;
    }

  return metadata;
}

} // namespace mobius::extension::app::ares




