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
#include "registry_data_impl_pssp.h"
#include <mobius/core/crypt/cipher.hpp>
#include <mobius/io/bytearray_io.h>
#include <mobius/decoder/data_decoder.h>

namespace mobius::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// @param des_key DES cryptographic key
// @param item_data data from "Item Data" value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_data_impl_pssp::registry_data_impl_pssp (
  const mobius::bytearray& des_key,
  const mobius::bytearray& item_data)
  : des_key_ (des_key),
    item_data_ (item_data)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load data on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry_data_impl_pssp::_load_data () const
{
  // return if data is loaded
  if (data_loaded_)
    return;

  // decode "Item Data" data
  auto item_data_reader = mobius::io::new_bytearray_reader (item_data_);
  auto item_data_decoder = mobius::decoder::data_decoder (item_data_reader);
  item_data_decoder.skip (8);
  auto enc_des_key2 = item_data_decoder.get_bytearray_by_size (24);
  auto size = item_data_decoder.get_uint32_le ();
  auto enc_data = item_data_decoder.get_bytearray_by_size (size);

  // decrypt data
  auto des = mobius::core::crypt::new_cipher_cbc ("des", des_key_);
  auto des_key2 = des.decrypt (enc_des_key2).slice (0, 7);

  auto des2 = mobius::core::crypt::new_cipher_cbc ("des", des_key2);
  auto data = des2.decrypt (enc_data);

  // decode data
  auto data_reader = mobius::io::new_bytearray_reader (data);
  auto data_decoder = mobius::decoder::data_decoder (data_reader);
  auto data_size = data_decoder.get_uint32_le ();
  data_ = data_decoder.get_bytearray_by_size (data_size);

  // set subkeys loaded
  data_loaded_ = true;
}

} // namespace mobius::os::win::registry


