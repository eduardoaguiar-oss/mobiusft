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
#include "registry_data_impl_lsa_poleklist.h"
#include <mobius/core/crypt/cipher.hpp>
#include <mobius/core/crypt/hash.hpp>
#include <mobius/decoder/data_decoder.h>

namespace mobius::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt data
// @param key LSA Secrets decryption key
// @param ciphertext Ciphertext
// @return plaintext
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
decrypt_aes (const mobius::bytearray& key, const mobius::bytearray& ciphertext)
{
  // generate AES key
  mobius::core::crypt::hash sha256 ("sha2-256");
  sha256.update (key);

  const mobius::bytearray iv = ciphertext.slice (28, 59);

  for (int i = 0; i < 1000; i++)
    sha256.update (iv);

  const auto aes_key = sha256.get_digest ();

  // decrypt data
  mobius::bytearray tmp;

  for (std::uint64_t i = 60; i < ciphertext.size (); i += 16)
    {
      auto c = mobius::core::crypt::new_cipher_cbc ("aes", aes_key);
      mobius::bytearray buffer = ciphertext.slice (i, i + 15);

      if (buffer.size () < 16)
        buffer.rpad (16);

      tmp += c.decrypt (buffer);
    }

  // decode plaintext
  auto decoder = mobius::decoder::data_decoder (tmp);
  auto size = decoder.get_uint64_le ();
  auto control = decoder.get_uint64_le ();

  if (control == 0)
    return decoder.get_bytearray_by_size (size);

  return mobius::bytearray ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// @param lsa_key LSA Secrets decryption key
// @param encrypted_data value's encrypted data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_data_impl_lsa_poleklist::registry_data_impl_lsa_poleklist (
  const mobius::bytearray& lsa_key,
  const mobius::bytearray& encrypted_data)
  : lsa_key_ (lsa_key),
    encrypted_data_ (encrypted_data)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load data on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry_data_impl_lsa_poleklist::_load_data () const
{
  // return if data is loaded
  if (data_loaded_)
    return;

  // decrypt data
  if (encrypted_data_.size () >= 60)
    {
      data_ = decrypt_aes (lsa_key_, encrypted_data_);
    }

  // set subkeys loaded
  data_loaded_ = true;
}

} // namespace mobius::os::win::registry


