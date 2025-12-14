#ifndef MOBIUS_CORE_OS_WIN_DPAPI_BLOB_HPP
#define MOBIUS_CORE_OS_WIN_DPAPI_BLOB_HPP

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
#include <mobius/core/bytearray.hpp>
#include <mobius/core/io/reader.hpp>
#include <cstdint>
#include <memory>
#include <string>

namespace mobius::core::os::win::dpapi
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief BLOB class
// @author Eduardo Aguiar
// @see
// https://elie.net/talk/reversing-dpapi-and-stealing-windows-secrets-offline/
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class blob
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit blob (const mobius::core::io::reader &);
    explicit blob (const mobius::core::bytearray &);
    blob (blob &&) noexcept = default;
    blob (const blob &) noexcept = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    blob &operator= (const blob &) noexcept = default;
    blob &operator= (blob &&) noexcept = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Function prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t get_revision () const;
    std::string get_provider_guid () const;
    std::uint32_t get_master_key_revision () const;
    std::string get_master_key_guid () const;
    std::uint32_t get_flags () const;
    std::string get_description () const;
    std::uint32_t get_cipher_id () const;
    std::uint32_t get_key_length () const;
    mobius::core::bytearray get_salt () const;
    mobius::core::bytearray get_hmac_key () const;
    std::uint32_t get_hash_id () const;
    std::uint32_t get_hash_length () const;
    mobius::core::bytearray get_hmac_value () const;
    mobius::core::bytearray get_cipher_text () const;
    mobius::core::bytearray get_plain_text () const;
    mobius::core::bytearray get_signature () const;
    mobius::core::bytearray get_signature_data () const;

    bool test_key (
        const mobius::core::bytearray &, const mobius::core::bytearray & = {}
    );
    bool decrypt (
        const mobius::core::bytearray &, const mobius::core::bytearray & = {}
    );
    bool is_decrypted () const;

  private:
    // @brief Implementation class forward declaration
    class impl;

    // @brief Implementation pointer
    std::shared_ptr<impl> impl_;
};

} // namespace mobius::core::os::win::dpapi

#endif
