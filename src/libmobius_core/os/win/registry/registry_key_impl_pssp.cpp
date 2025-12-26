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
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/io/bytearray_io.hpp>
#include <mobius/core/os/win/registry/registry_data_impl_pssp.hpp>
#include <mobius/core/os/win/registry/registry_key_impl_pssp.hpp>
#include <mobius/core/os/win/registry/registry_value.hpp>
#include <mobius/core/os/win/registry/registry_value_impl_container.hpp>

namespace mobius::core::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// @param key delegated key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_key_impl_pssp::registry_key_impl_pssp (registry_key key,
                                                pssp_data2 data2)
    : key_ (key),
      name_ (key.get_name ()),
      data2_ (data2)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load subkeys on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry_key_impl_pssp::_load_subkeys () const
{
    // return if subkeys are loaded
    if (subkeys_loaded_)
        return;

    // load subkeys
    for (auto subkey : key_)
    {
        auto value = subkey.get_value_by_name ("Display String");

        if (value)
        {
            auto key = registry_key (
                std::make_shared<registry_key_impl_pssp> (subkey, data2_));
            subkeys_.append (key);
        }
    }

    // set subkeys loaded
    subkeys_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load values on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
registry_key_impl_pssp::_load_values () const
{
    // return if values are loaded
    if (values_loaded_)
        return;

    // load values from original key
    values_ = key_.get_values ();

    // load values from PSSP keys that have both "Behavior" and "Item Data"
    // values
    for (auto subkey : key_)
    {
        auto v_behavior = subkey.get_value_by_name ("Behavior");
        auto v_itemdata = subkey.get_value_by_name ("Item Data");

        if (v_behavior && v_itemdata)
        {
            // decode "behavior" value
            auto behavior_data = v_behavior.get_data ().get_data ();
            auto reader =
                mobius::core::io::new_bytearray_reader (behavior_data);
            auto decoder = mobius::core::decoder::data_decoder (reader);

            decoder.skip (8);
            auto size = decoder.get_uint32_le ();
            auto key_name = decoder.get_string_by_size (size, "UTF-16LE");
            auto des_key = data2_.get_key (key_name);

            // if cryptographic key has been found, create new value
            if (!des_key.empty ())
            {
                auto itemdata = v_itemdata.get_data ().get_data ();
                registry_data data (std::make_shared<registry_data_impl_pssp> (
                    des_key, itemdata));
                registry_value value (subkey.get_name (), data);
                values_.push_back (value);
            }
        }
    }

    // set values loaded
    values_loaded_ = true;
}

} // namespace mobius::core::os::win::registry
