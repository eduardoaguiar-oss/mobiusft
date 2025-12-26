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
#include "file_local_state.hpp"
#include <mobius/core/crypt/hash.hpp>
#include <mobius/core/decoder/base64.hpp>
#include <mobius/core/decoder/json/parser.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/os/win/dpapi/blob.hpp>
#include <mobius/core/string_functions.hpp>
#include "common.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get metadata from DPAPI blob
// @param data DPAPI blob data
// @return Metadata as a map
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::map
_get_metadata_from_dpapi_blob (const mobius::core::bytearray &data)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    mobius::core::pod::map metadata;

    try
    {
        auto blob = mobius::core::os::win::dpapi::blob (data);

        metadata.set ("dpapi_revision", blob.get_revision ());
        metadata.set ("dpapi_provider_guid", blob.get_provider_guid ());
        metadata.set (
            "dpapi_master_key_revision", blob.get_master_key_revision ()
        );
        metadata.set ("dpapi_master_key_guid", blob.get_master_key_guid ());
        metadata.set ("dpapi_flags", blob.get_flags ());
        metadata.set ("dpapi_description", blob.get_description ());
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, std::string (e.what ()));
    }

    return metadata;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get key ID from DPAPI blob
// @param data DPAPI blob data
// @return Key ID as a string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
_get_key_id_from_dpapi_blob (const mobius::core::bytearray &data)
{
    auto h = mobius::core::crypt::hash ("md5");
    h.update (data);
    return mobius::core::string::toupper (h.get_hex_digest ());
}

} // namespace

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_local_state::file_local_state (const mobius::core::io::reader &reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (!reader)
        return;

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Try to parse the Local State file as a JSON file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::decoder::json::parser parser (reader);
        auto data = parser.parse ();

        if (!data.is_map ())
        {
            log.info (__LINE__, "File is not a valid Local State file");
            return;
        }

        log.info (__LINE__, "File is a valid Local State file");

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get os_crypt dict
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto map = data.to_map ();
        auto os_crypt = map.get<mobius::core::pod::map> ("os_crypt");

        if (!os_crypt)
        {
            log.info (
                __LINE__, "Local State file does not contain 'os_crypt' data"
            );
            return;
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get v10 key from os_crypt dict
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto v10_encrypted_key = os_crypt.get<std::string> ("encrypted_key");

        if (!v10_encrypted_key.empty ())
        {
            auto value = mobius::core::decoder::base64 (v10_encrypted_key);

            if (value.startswith ("DPAPI"))
            {
                encryption_key ek;
                ek.type = "v10";
                ek.value = value.slice (5, value.size ());
                ek.id = _get_key_id_from_dpapi_blob (ek.value);
                ek.metadata = _get_metadata_from_dpapi_blob (ek.value);

                encryption_keys_.emplace_back (std::move (ek));
            }
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get v20 key from os_crypt dict
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto v20_encrypted_key =
            os_crypt.get<std::string> ("app_bound_encrypted_key");

        if (!v20_encrypted_key.empty ())
        {
            auto value = mobius::core::decoder::base64 (v20_encrypted_key);

            if (value.startswith ("APPB"))
            {
                encryption_key ek;
                ek.type = "v20";
                ek.value = value.slice (4, value.size ());
                ek.id = _get_key_id_from_dpapi_blob (ek.value);
                ek.metadata = _get_metadata_from_dpapi_blob (ek.value);

                encryption_keys_.emplace_back (std::move (ek));
            }
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Finish parsing
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::chromium
