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
#include "registry_key_impl_lsa.h"
#include "registry_data_impl_lsa_polseckey.h"
#include "registry_data_impl_lsa_poleklist.h"
#include "registry_value.h"
#include <mobius/decoder/data_decoder.h>

namespace mobius::os::win::registry
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get the right LSA key for a given data
// @param lsa_key_stream LSA key stream data
// @param data value's data
// @return lsa_key or an empty key
// @see https://github.com/gentilkiwi/mimikatz/wiki
//
// The value's data has the folllowing structure:
//  0	uint32_le	version
//  4	byte (16)	key GUID
// 20	algorithm	algorithm used
// 24	flags		algorithm flags
// 28	byte (16)	Initialization vector (IV)
//
// lsa_key_stream contains an array of lsa_keys, where each one can be
// identified by a key GUID. We must choose the right key, using the
// key GUID field from value's data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
get_lsa_key (const mobius::bytearray& lsa_key_stream, const mobius::bytearray& data)
{
  mobius::bytearray lsa_key;
  auto data_key_guid = data.slice (4, 19);

  // decoder LSA key header
  mobius::decoder::data_decoder decoder (lsa_key_stream);
  decoder.skip (24);
  std::uint32_t key_count = decoder.get_uint32_le ();

  // search the right key
  std::uint32_t i = 0;

  while (i < key_count && !lsa_key)
    {
      auto key_guid = decoder.get_bytearray_by_size (16);
      decoder.skip (4);		// key type
      auto key_size = decoder.get_uint32_le ();
      auto key = decoder.get_bytearray_by_size (key_size);

      if (key_guid == data_key_guid)
        lsa_key = key;

      else
        i++;
    }

  return lsa_key;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// @param key delegated key
// @param lsa_key LSA Secrets encryption key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_key_impl_lsa::registry_key_impl_lsa (registry_key key, const mobius::bytearray& lsa_key, type t)
  : key_ (key),
    name_ (key.get_name ()),
    lsa_key_ (lsa_key),
    type_ (t)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load values on demand
// @see https://github.com/gentilkiwi/mimikatz/wiki
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry_key_impl_lsa::_load_values () const
{
  // return if values are loaded
  if (values_loaded_)
    return;

  // load values from LSA Secret key
  for (auto k_value : key_.get_values ())
    {
      registry_data data;

      auto v_data = k_value.get_data ().get_data ();

      if (type_ == type::polseckey)
        {
          data = registry_data (
                   std::make_shared <registry_data_impl_lsa_polseckey> (lsa_key_, v_data)
                 );
        }

      else if (type_ == type::poleklist)
        {
          auto lsa_key = get_lsa_key (lsa_key_, v_data);

          if (lsa_key)
            {
              data = registry_data (
                       std::make_shared <registry_data_impl_lsa_poleklist> (lsa_key, v_data)
                     );
            }
        }

      registry_value value (k_value.get_name (), data);
      values_.push_back (value);
    }

  // set values loaded
  values_loaded_ = true;
}

} // namespace mobius::os::win::registry


