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
#include <mobius/core/os/win/registry/registry_data_impl_lsa_polseckey.hpp>
#include <mobius/core/crypt/cipher.hpp>
#include <mobius/core/decoder/data_decoder.hpp>

namespace mobius::core::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief decrypt data according to SystemFunction005
// @param lsa_key LSA Secrets decryption key
// @param data values' data
// @return plaintext
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const mobius::core::bytearray
decrypt_sysfcn5 (const mobius::core::bytearray& lsa_key, const mobius::core::bytearray& data)
{
  mobius::core::bytearray plaintext;
  const mobius::core::bytearray ciphertext = data.slice (12, data.size () - 1);

  std::uint32_t i = 0;
  std::uint32_t j = 0;
  std::uint32_t l = ciphertext.size ();

  while (i < l)
    {
      auto des = mobius::core::crypt::new_cipher_ecb ("des", lsa_key.slice (j, j + 6));
      plaintext += des.decrypt (ciphertext.slice (i, i + 7));
      i += 8;
      j += 7;

      if (j > lsa_key.size () - 7)
        j = lsa_key.size () - j;
    }

  return plaintext;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// @param lsa_key LSA Secrets decryption key
// @param encrypted_data value's encrypted data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_data_impl_lsa_polseckey::registry_data_impl_lsa_polseckey (
  const mobius::core::bytearray& lsa_key,
  const mobius::core::bytearray& encrypted_data)
  : lsa_key_ (lsa_key),
    encrypted_data_ (encrypted_data)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load data on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry_data_impl_lsa_polseckey::_load_data () const
{
  // return if data is loaded
  if (data_loaded_)
    return;

  // decrypt data
  if (encrypted_data_.size () > 11)
    {
      auto data = decrypt_sysfcn5 (lsa_key_, encrypted_data_);

      // decode plaintext
      auto decoder = mobius::core::decoder::data_decoder (data);
      auto size = decoder.get_uint32_le ();
      decoder.skip (4);
      data_ = decoder.get_bytearray_by_size (size);
    }

  // set subkeys loaded
  data_loaded_ = true;
}


} // namespace mobius::core::os::win::registry


