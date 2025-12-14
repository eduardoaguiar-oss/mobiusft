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
#include <mobius/core/charset.hpp>
#include <mobius/core/crypt/cipher.hpp>
#include <mobius/core/crypt/hash.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/os/win/registry/pssp_data2.hpp>
#include <mobius/core/os/win/registry/registry_key.hpp>
#include <mobius/core/os/win/registry/registry_key_impl_pssp.hpp>

namespace mobius::core::os::win::registry
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief retrieve pssp cryptographic keys from SID key
// @param sid_key SID key
// @return PSSP key collection
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
pssp_data2
get_data2 (registry_key sid_key)
{
    pssp_data2 data2;

    // get "Data 2" key
    auto data2_key = sid_key.get_key_by_name ("Data 2");

    if (data2_key)
    {
        // evaluate SID based hash value
        mobius::core::crypt::hash sid_hash ("sha1");
        sid_hash.update ({0x66, 0x41, 0xa3, 0x29});
        sid_hash.update (mobius::core::conv_charset (sid_key.get_name (),
                                                     "UTF-8", "UTF-16LE"));

        if (sid_key.get_name ().length () % 2)
            sid_hash.update ({0x14, 0x9a});

        auto sid_hash_value = sid_hash.get_digest ();

        // decrypt all cryptographic keys in "Data 2"
        for (auto subkey : data2_key)
        {
            // decode "Value" data
            auto data = subkey.get_data_by_name ("Value").get_data ();
            auto decoder = mobius::core::decoder::data_decoder (data);

            decoder.skip (8);
            auto flag = decoder.get_uint32_le ();                    //  8 - 11
            auto encrypted_key = decoder.get_bytearray_by_size (24); // 12 - 35
            decoder.skip (4);
            auto salt = decoder.get_bytearray_by_size (16); // 40 - 55

            // build DES key
            mobius::core::crypt::hash data_hash ("sha1");

            data_hash.update (salt);

            if (flag == 2)
                data_hash.update (sid_hash_value.slice (0, 3));

            else if (flag == 3)
                data_hash.update (sid_hash_value);

            auto des_key = data_hash.get_digest ().slice (0, 7);

            // decrypt cryptographic key
            auto des = mobius::core::crypt::new_cipher_cbc ("des", des_key);
            auto key_name = subkey.get_name ();
            auto key_value = des.decrypt (encrypted_key).slice (0, 7);

            // add cryptographic key to collection
            data2.set_key (key_name, key_value);
        }
    }

    return data2;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set Protected Storage System Provider (PSSP) keys
// @see https://msdn.microsoft.com/library/bb432403.aspx
// @param root registry root key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry_set_pssp_keys (registry_key root)
{
    for (auto sid_key :
         root.get_key_by_mask ("HKEY_USERS\\*\\Software\\Microsoft\\Protected "
                               "Storage System Provider\\*"))
    {
        auto data_key = sid_key.get_key_by_name ("Data");
        auto data2_key = sid_key.get_key_by_name ("Data 2");

        if (data_key && data2_key)
        {
            auto data2 = get_data2 (sid_key);

            for (auto subkey : data_key)
            {
                auto key = registry_key (
                    std::make_shared<registry_key_impl_pssp> (subkey, data2));
                data_key.add_key (key);
            }
        }
    }
}

} // namespace mobius::core::os::win::registry
