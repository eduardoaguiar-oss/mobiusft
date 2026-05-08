// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008-2026 Eduardo Aguiar
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
#include "file_download_list_dat.hpp"
#include <mobius/core/decoder/json/parser.hpp>
#include <mobius/core/decoder/qdatastream.hpp>
#include <mobius/core/io/uri.hpp>
#include <mobius/core/log.hpp>
#include "common.hpp"

namespace mobius::extension::app::itubego
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode entry
// @param decoder QDataStream decoder
// @return Entry
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_download_list_dat::entry
_get_entry (mobius::core::decoder::qdatastream &decoder)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_download_list_dat::entry entry;
    entry.url = decoder.get_qstring ();

    entry.type = decoder.get_qint32 ();
    if (entry.type != 4 && entry.type != 5)
        log.development (
            __LINE__, "Unhandled entry type: " + std::to_string (entry.type)
        );

    auto sniff_data = mobius::core::bytearray (decoder.get_qvariant ());
    auto finished_data = mobius::core::bytearray (decoder.get_qvariant ());
    entry.destination_folder = decoder.get_qstring ();

    // u1 is probably one of the following: quint32,qbitarray,qdate,qhash,qicon,
    // qimage,qlist,qmap,qpicture,qtime,qvector
    auto u1 = decoder.get_qint32 ();
    log.development (__LINE__, "u1: " + std::to_string (u1));

    entry.thumbnail_path = decoder.get_qstring ();
    auto downloading_data = mobius::core::bytearray (decoder.get_qvariant ());

    auto uri = mobius::core::io::uri (entry.url);
    entry.name = uri.get_filename ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Retrieve data from sniff dictionary
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &[k, v] : decode_json (sniff_data))
    {
        if (k == "title")
            entry.title = v.to_string ();

        else if (k == "thumbnail")
            entry.thumbnail_url = v.to_string ();

        else if (k == "filepath")
            entry.path = v.to_string ();

        else
            entry.metadata.set (k, v);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Retrieve data from finished dictionary
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &[k, v] : decode_json (finished_data))
    {
        log.debug (__LINE__, "k = " + k + ", v = " + v.to_string ());

        if (k == "filesize")
            entry.size = std::stoll (v.to_string ());

        else if (k == "filetype")
            entry.file_type = v.to_string ();

        else if (k == "filepath")
            entry.temp_file_path = v.to_string ();

        else if (k == "ret_code")
        {
            auto ret_code = std::stoll (v.to_string ());

            if (ret_code == 0)
                entry.state = "completed";

            else if (ret_code == -1)
                entry.state = "failed";

            else
            {
                entry.state = "unknown";
                log.development (
                    __LINE__, "Unhandled ret_code: " + std::to_string (ret_code)
                );
            }
        }

        else
            entry.metadata.set (k, v);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Retrieve data from downloading dictionary
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &[k, v] : decode_json (downloading_data))
        entry.metadata.set (k, v);

    return entry;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_download_list_dat::file_download_list_dat (
    const mobius::core::io::reader &reader
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Check if reader is valid and has enough data for decoding
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        if (!reader || reader.get_size () < 4)
            return;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode entries
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto decoder = mobius::core::decoder::qdatastream (reader);
        auto num_entries = decoder.get_qint32 ();

        for (std::int32_t i = 0; i < num_entries; ++i)
        {
            auto entry = _get_entry (decoder);
            entry.idx = i + 1;

            entries_.push_back (entry);
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Set instance flag
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::itubego
