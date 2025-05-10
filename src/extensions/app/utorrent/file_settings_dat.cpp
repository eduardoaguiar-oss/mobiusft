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
#include "file_settings_dat.hpp"
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/decoder/btencode.hpp>
#include <mobius/core/encoder/hexstring.hpp>
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

} // namespace

namespace mobius::extension::app::utorrent
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_settings_dat::file_settings_dat (const mobius::core::io::reader &reader)
{
    if (!reader)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode btencode data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::pod::map data (mobius::core::decoder::btencode (reader));

    total_bytes_downloaded_ = data.get<std::int64_t> ("td");
    total_bytes_uploaded_ = data.get<std::int64_t> ("tu");
    flag_autostart_ = data.get<std::int64_t> ("autostart", 1) == 1;
    computer_id_ = mobius::core::encoder::hexstring (
        data.get<mobius::core::bytearray> ("cid"));
    installation_time_ =
        mobius::core::datetime::new_datetime_from_nt_timestamp (
            data.get<std::int64_t> ("born_on", 0) * 10000000);
    last_used_time_ = mobius::core::datetime::new_datetime_from_nt_timestamp (
        data.get<std::int64_t> ("cold_on", 0) * 10000000);
    last_bin_change_time_ =
        mobius::core::datetime::new_datetime_from_nt_timestamp (
            data.get<std::int64_t> ("bin_change", 0) * 10000000);
    execution_count_ = data.get<std::int64_t> ("runs_since_born", 0);
    version_ = _decode_version (data.get<std::int64_t> ("v"));
    installation_version_ =
        _decode_version (data.get<std::int64_t> ("initial_install_version"));

    auto lang = data.get<std::int64_t> ("language", 0);
    if (lang)
        language_ =
            std::string () + char (lang & 0xff) + char ((lang >> 8) & 0xff);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // End decoding
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    is_instance_ = true;
}

} // namespace mobius::extension::app::utorrent
