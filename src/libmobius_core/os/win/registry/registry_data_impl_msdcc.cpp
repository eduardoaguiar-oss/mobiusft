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
#include <mobius/core/os/win/registry/registry_data_impl_msdcc.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/crypt/cipher.hpp>
#include <mobius/core/crypt/hmac.hpp>

namespace mobius::core::os::win::registry
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief decrypt data using CTS AES-128 with CBC
// @param key AES key
// @param iv Initialization Vector
// @param ciphertext ciphertext
// @return plaintext
// @see https://en.wikipedia.org/wiki/Ciphertext_stealing
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray decrypt_cts_aes_128 (
  const mobius::core::bytearray& key,
  const mobius::core::bytearray& iv,
  const mobius::core::bytearray& ciphertext)
{
  mobius::core::bytearray plaintext;
  mobius::core::bytearray l_ciphertext = ciphertext;
  constexpr mobius::core::bytearray::size_type block_size = 16;

  // if ciphertext data has at least two blocks...
  if (ciphertext.size () >= block_size * 2)
    {
      std::uint32_t padsize = block_size - (ciphertext.size () % block_size);

      // decrypt Cn-1 (second to last block)
      if (padsize < block_size)
        {
          auto aes1 = mobius::core::crypt::new_cipher_ecb ("aes", key);
          mobius::core::bytearray::size_type pos = ciphertext.size () - block_size * 2 + padsize;
          mobius::core::bytearray dn = aes1.decrypt (ciphertext.slice (pos, pos + block_size - 1));

          // pad ciphertext with DN
          l_ciphertext += dn.slice (dn.size () - padsize, dn.size () - 1);
        }

      // swap last two blocks
      mobius::core::bytearray::size_type pos = l_ciphertext.size () - block_size * 2;
      l_ciphertext = l_ciphertext.slice (0, pos - 1) + l_ciphertext.slice (pos + block_size, pos + block_size * 2 - 1) + l_ciphertext.slice (pos, pos + block_size - 1);
    }

  // decrypt data
  auto c = mobius::core::crypt::new_cipher_cbc ("aes", key, iv);
  plaintext = c.decrypt (l_ciphertext);
  plaintext.resize (ciphertext.size ());

  return plaintext;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// @param nlkm NL$KM decrypted key
// @param encrypted_data value's encrypted data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_data_impl_msdcc::registry_data_impl_msdcc (
  const mobius::core::bytearray& nlkm,
  const mobius::core::bytearray& encrypted_data)
  : nlkm_ (nlkm),
    encrypted_data_ (encrypted_data)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load data on demand
// @see http://moyix.blogspot.com.br/2008/02/cached-domain-credentials.html
// @see https://github.com/Neohapsis/creddump7/blob/master/framework/win32/domcachedump.py
// @see https://github.com/gentilkiwi/mimikatz
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry_data_impl_msdcc::_load_data () const
{
  // return if data is loaded
  if (data_loaded_)
    return;

  // decode encrypted data
  mobius::core::decoder::data_decoder decoder (encrypted_data_);
  decoder.skip (48);
  std::uint16_t encrypted = decoder.get_uint16_le ();
  std::uint16_t algorithm = decoder.get_uint16_le ();
  decoder.skip (12);
  mobius::core::bytearray iv = decoder.get_bytearray_by_size (16);
  decoder.skip (16);
  mobius::core::bytearray data = decoder.get_bytearray_by_size (encrypted_data_.size () - 96);

  // decrypt data, if necessary
  if (encrypted)
    {
      // MSDCC1 - MS domain Cached Credentials v1
      if (algorithm == 0)
        {
          mobius::core::crypt::hmac hmac ("md5", nlkm_);
          hmac.update (iv);
          auto rc4_key = hmac.get_digest ();

          auto rc4 = mobius::core::crypt::new_cipher_stream ("rc4", rc4_key);
          data_ = encrypted_data_.slice (0, 95) + rc4.decrypt (data);
        }

      // MSDCC2 - MS domain Cached Credentials v2
      else if (algorithm == 10)
        {
          auto key = nlkm_.slice (0, 15);
          data_ = encrypted_data_.slice (0, 95) + decrypt_cts_aes_128 (key, iv, data);
        }

      // crop data
      if (data_.size () >= 0x8b)
        {
          mobius::core::decoder::data_decoder decoder (data_);
          decoder.skip (0x88);
          std::uint32_t size = decoder.get_uint32_le ();
          data_.resize (size);
        }
    }

  else
    data_ = encrypted_data_;

  // set subkeys loaded
  data_loaded_ = true;
}

} // namespace mobius::core::os::win::registry


