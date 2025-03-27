#ifndef MOBIUS_OS_WIN_DPAPI_BLOB_H
#define MOBIUS_OS_WIN_DPAPI_BLOB_H

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
#include <mobius/bytearray.h>
#include <mobius/io/reader.h>
#include <cstdint>
#include <memory>
#include <string>

namespace mobius::os::win::dpapi
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief BLOB class
// @author Eduardo Aguiar
// @see https://elie.net/talk/reversing-dpapi-and-stealing-windows-secrets-offline/
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class blob
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  explicit blob (mobius::io::reader);
  blob (blob&&) noexcept = default;
  blob (const blob&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  blob& operator= (const blob&) noexcept = default;
  blob& operator= (blob&&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Function prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t get_revision () const;
  std::string get_provider_guid () const;
  std::uint32_t get_master_key_revision () const;
  std::string get_master_key_guid () const;
  std::uint32_t get_flags () const;
  std::string get_description () const;
  std::uint32_t get_cipher_id () const;
  std::uint32_t get_key_length () const;
  mobius::bytearray get_salt () const;
  mobius::bytearray get_hmac_key () const;
  std::uint32_t get_hash_id () const;
  std::uint32_t get_hash_length () const;
  mobius::bytearray get_hmac_value () const;
  mobius::bytearray get_cipher_text () const;
  mobius::bytearray get_plain_text () const;
  mobius::bytearray get_signature () const;
  mobius::bytearray get_signature_data () const;

  bool test_key (const mobius::bytearray&, const mobius::bytearray& = {});
  bool decrypt (const mobius::bytearray&, const mobius::bytearray& = {});
  bool is_decrypted () const;

private:
  // @brief Implementation class forward declaration
  class impl;

  // @brief Implementation pointer
  std::shared_ptr <impl> impl_;
};

} // namespace mobius::os::win::dpapi

#endif


