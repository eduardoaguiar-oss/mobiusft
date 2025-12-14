// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
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
#include "file_settings_dat.hpp"
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/decoder/btencode.hpp>
#include <mobius/core/encoder/hexstring.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/pod/map.hpp>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode version
// @param value Version value
// @return Version string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
_decode_version (const std::uint32_t value)
{
    std::string version;

    if (value)
        version = std::to_string ((value >> 25) & 0x0f) + '.' +
                  std::to_string ((value >> 21) & 0x0f) + '.' +
                  std::to_string ((value >> 17) & 0x0f) + '.' +
                  std::to_string (value & 0xffff);

    return version;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert timestamp to datetime
// @param timestamp NT timestamp
// @return datetime object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
_decode_datetime (const std::int64_t timestamp)
{
    if (timestamp < 0x80000000)
        return mobius::core::datetime::new_datetime_from_unix_timestamp (
            timestamp
        );

    return mobius::core::datetime::new_datetime_from_nt_timestamp (
        timestamp * 10000000
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert ISO timestamp to datetime
// @param iso_timestamp ISO timestamp string
// @return datetime object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
_decode_datetime (const std::string &iso_timestamp)
{
    mobius::core::datetime::datetime dt;

    if (!iso_timestamp.empty () && iso_timestamp != "not-a-date-time")
    {
        mobius::core::log log (__FILE__, __FUNCTION__);

        try
        {
            dt = mobius::core::datetime::new_datetime_from_iso_string (
                iso_timestamp
            );
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, e.what ());
        }
    }

    return dt;
}

} // namespace

namespace mobius::extension::app::utorrent
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_settings_dat::file_settings_dat (const mobius::core::io::reader &reader)
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Check for 'd' header
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (!reader)
        return;

    auto header_reader = reader;
    auto byte = header_reader.read (1);

    if (byte.size () != 1 || byte[0] != 'd')
        return;

    header_reader.rewind ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode btencode data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::pod::map data (mobius::core::decoder::btencode (reader));

    bind_port_ = data.pop<std::int64_t> ("bind_port");
    cached_host_ = data.pop<std::string> ("upnp_cached_host");
    computer_id_ = mobius::core::encoder::hexstring (
        data.pop<mobius::core::bytearray> ("cid")
    );
    dir_active_downloads_ = data.pop<std::string> ("dir_active_download");
    dir_completed_downloads_ = data.pop<std::string> ("dir_completed_download");
    dir_torrent_files_ = data.pop<std::string> ("dir_torrent_files");
    execution_count_ = data.pop<std::int64_t> ("runs_since_born");
    exe_path_ = data.pop<std::string> ("exe_path");
    external_ip_ = data.pop<std::string> ("upnp.external_ip");
    fileguard_ = data.pop<std::string> (".fileguard");
    flag_autostart_ = data.pop<std::int64_t> ("autostart", 1) == 1;

    installation_time_ = _decode_datetime (data.pop<std::int64_t> ("born_on"));
    if (!installation_time_)
        installation_time_ =
            _decode_datetime (data.pop<std::string> ("reset_ts"));

    installation_version_ =
        _decode_version (data.pop<std::int64_t> ("initial_install_version"));
    if (installation_version_.empty ())
        installation_version_ =
            _decode_version (data.pop<std::int64_t> ("reset_version"));

    last_used_time_ = _decode_datetime (data.pop<std::int64_t> ("cold_on"));
    last_bin_change_time_ =
        _decode_datetime (data.pop<std::int64_t> ("bin_change"));

    auto lang = data.pop<std::int64_t> ("language");
    if (lang)
        language_ =
            std::string () + char (lang & 0xff) + char ((lang >> 8) & 0xff);

    runtime_ = data.pop<std::int64_t> ("runtime_since_born");
    save_path_ = data.pop<std::string> ("save_path");
    settings_saved_time_ =
        _decode_datetime (data.pop<std::int64_t> ("settings_saved_systime"));
    statistics_time_ =
        _decode_datetime (data.pop<std::string> ("stats_timestamp"));
    ssdp_uuid_ = data.pop<std::string> ("webui.ssdp_uuid");

    total_bytes_downloaded_ =
        data.pop<std::int64_t> ("td") +
        data.pop<std::int64_t> ("deleted_torrents_total_download") +
        data.pop<std::int64_t> ("prev_deleted_torrents_total_download");

    total_bytes_uploaded_ =
        data.pop<std::int64_t> ("tu") +
        data.pop<std::int64_t> ("deleted_torrents_total_upload") +
        data.pop<std::int64_t> ("prev_deleted_torrents_total_upload");

    username_ = data.pop<std::string> ("webui.username");
    version_ = _decode_version (data.pop<std::int64_t> ("v"));

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // End decoding
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    metadata_ = data;
    is_instance_ = true;
}

} // namespace mobius::extension::app::utorrent
