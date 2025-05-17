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
#include <mobius/core/crypt/cipher.hpp>
#include <mobius/core/crypt/hash.hpp>
#include <mobius/core/os/win/registry/registry_data_impl_lsa_poleklist.hpp>
#include <mobius/core/os/win/registry/registry_key.hpp>
#include <mobius/core/os/win/registry/registry_key_impl_lsa.hpp>

namespace mobius::core::os::win::registry
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get decrypted LSAKEY
// @param syskey registry SysKey
// @param data PolSecKey value's data
// @return decrypted LSAKEY
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::bytearray
get_lsa_key_from_polseckey (const mobius::core::bytearray &syskey,
                            const mobius::core::bytearray &data)
{
    mobius::core::crypt::hash md5 ("md5");
    md5.update (syskey);

    auto salt = data.slice (60, 75);
    for (int i = 0; i < 1000; i++)
        md5.update (salt);

    auto rc4 =
        mobius::core::crypt::new_cipher_stream ("rc4", md5.get_digest ());
    auto rc4_key = data.slice (12, 59);

    return rc4.decrypt (rc4_key).slice (16, 31);
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set LSA Secrets keys
// @see https://www.passcape.com/index.php?section=docsys&cmd=details&id=23
// @see http://moyix.blogspot.com.br/2008/02/decrypting-lsa-secrets.html
// @see
// https://github.com/Neohapsis/creddump7/blob/master/framework/win32/lsasecrets.py
// @see https://github.com/gentilkiwi/mimikatz/wiki
// @param root registry root key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry_set_lsa_keys (registry_key root, const mobius::core::bytearray &syskey)
{
    if (!syskey)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // get PolRevision
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto polrevision_data = root.get_data_by_path (
        "\\HKEY_LOCAL_MACHINE\\SECURITY\\Policy\\PolRevision\\(default)");

    if (!polrevision_data)
        return;

    std::uint32_t revision = polrevision_data.get_data_as_dword ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // decrypt LSA key
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::bytearray lsa_key;
    registry_key_impl_lsa::type type;

    // Vista and newer ones
    if (revision > 0x10009)
    {
        auto poleklist_data = root.get_data_by_path (
            "\\HKEY_LOCAL_MACHINE\\SECURITY\\Policy\\PolEKList\\(default)");
        lsa_key = decrypt_aes (syskey, poleklist_data.get_data ());
        type = registry_key_impl_lsa::type::poleklist;
    }

    // Win2k - WinXp
    else
    {
        auto polseckey_data = root.get_data_by_path (
            "\\HKEY_LOCAL_MACHINE\\SECURITY\\Policy\\PolSecretEncryptionKey\\("
            "default)");
        lsa_key =
            get_lsa_key_from_polseckey (syskey, polseckey_data.get_data ());
        type = registry_key_impl_lsa::type::polseckey;
    }

    if (!lsa_key)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // set LSA Secrets keys
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (auto key : root.get_key_by_mask (
             "\\HKEY_LOCAL_MACHINE\\SECURITY\\Policy\\Secrets\\*"))
    {
        auto currval_key = key.get_key_by_name ("Currval");

        if (currval_key)
            key.add_key (registry_key (std::make_shared<registry_key_impl_lsa> (
                currval_key, lsa_key, type)));

        auto oldval_key = key.get_key_by_name ("Oldval");

        if (oldval_key)
            key.add_key (registry_key (std::make_shared<registry_key_impl_lsa> (
                oldval_key, lsa_key, type)));
    }
}

} // namespace mobius::core::os::win::registry
