// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025
// Eduardo Aguiar
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
#include "file_torrenth.hpp"
#include "common.hpp"
#include <algorithm>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/log.hpp>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Populate entry metadata
// @param e Entry
// @param data Raw data
// @see get_trusted_metas@helper_library_db.pas
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
_populate_metadata (mobius::extension::app::ares::file_torrenth::entry &e,
                    const mobius::core::bytearray &data)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    for (const auto &[i, v] :
         mobius::extension::app::ares::decode_metadata (data))
    {
        switch (i)
        {
        case 1:
            e.name = v.to_string ();
            break;

        case 2:
        {
            auto b_trackers = v.split ({0});
            std::transform (b_trackers.begin (), b_trackers.end (),
                            std::back_inserter (e.trackers),
                            [] (const auto &b) { return b.to_string (); });
        }

        break;

        case 3:
            e.evaluated_hash_sha1 = v.to_hexstring ();
            break;

        default:
            log.development (__LINE__,
                             "unhandled field_type: " + std::to_string (i));
        }
    }
}

} // namespace

namespace mobius::extension::app::ares
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @see tthread_dht.getMagnetFiles@DHT/thread_dht.pas (line 284)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_torrenth::file_torrenth (const mobius::core::io::reader &reader)
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
    auto header_section = section_.new_child ("file-header");
    auto signature = decoder.get_bytearray_by_size (14);

    // @see DHT/thread_dht.pas (line 319)
    if (signature != "__ARESDB1.02H_")
        return;

    is_instance_ = true;
    signature_ = signature.to_string ();

    header_section.end ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t idx = 0;

    while (decoder)
    {
        auto entry_section =
            section_.new_child ("Entry #" + std::to_string (++idx));

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Create data section
        // @see DHT/thread_dht.pas (line 335)
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto data_section = entry_section.new_child ("entry-header");

        auto enc_data = decoder.get_bytearray_by_size (39);
        auto data = decrypt (enc_data, 12971);

        data_section.set_data (data);
        data_section.end ();

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Create entry
        // @see DHT/thread_dht.pas (line 338)
        // @see https://en.wikipedia.org/wiki/Magnet_URI_scheme
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::decoder::data_decoder entry_decoder (data);

        entry e;
        e.idx = idx;
        e.timestamp = entry_decoder.get_unix_datetime ();
        e.media_type = media_type_to_string (entry_decoder.get_uint8 ());
        e.size = entry_decoder.get_uint64_le ();
        e.seeds = entry_decoder.get_uint32_le ();
        e.hash_sha1 = entry_decoder.get_hex_string_by_size (20);
        e.url = "magnet:?xt=urn:btih:" + e.hash_sha1;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode metadata, if any
        // @see DHT/thread_dht.pas (line 355)
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        std::uint16_t metadata_size = entry_decoder.get_uint16_le ();

        if (metadata_size)
        {
            auto metadata_section = entry_section.new_child ("entry-metadata");

            enc_data = decoder.get_bytearray_by_size (metadata_size);
            data = decrypt (enc_data, 13175);
            _populate_metadata (e, data);

            metadata_section.set_data (data);
            metadata_section.end ();
        }

        entries_.push_back (e);
        entry_section.end ();
    }
}

} // namespace mobius::extension::app::ares
