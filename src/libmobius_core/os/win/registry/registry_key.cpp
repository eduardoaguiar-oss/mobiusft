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
#include <mobius/core/os/win/registry/registry_key.hpp>
#include <mobius/core/os/win/registry/registry_key_impl_null.hpp>
#include <mobius/core/string_functions.hpp>

namespace mobius::core::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_key::registry_key ()
    : impl_ (std::make_shared<registry_key_impl_null> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor from implementation pointer
// @param impl implementation pointer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_key::registry_key (std::shared_ptr<registry_key_impl_base> impl)
    : impl_ (impl)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get subkey by name
// @param name subkey name
// @return subkey or empty key, if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_key
registry_key::get_key_by_name (const std::string &name) const
{
    const std::string lname = mobius::core::string::tolower (name);

    for (const auto &sk : *this)
        if (mobius::core::string::tolower (sk.get_name ()) == lname)
            return sk;

    return registry_key ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get subkey by path
// @param path subkey path
// @return subkey or empty key, if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_key
registry_key::get_key_by_path (const std::string &path) const
{
    std::string::size_type pos = 0;

    // skip heading '\'
    while (pos < path.size () && path[pos] == '\\')
        ++pos;

    // seek each subkey which name is limited by '\\'
    registry_key key = *this;
    std::string::size_type end = path.find ('\\', pos);

    while (pos != std::string::npos && key)
    {
        std::string name;

        if (end == std::string::npos)
        {
            name = path.substr (pos);
            pos = std::string::npos;
        }

        else
        {
            name = path.substr (pos, end - pos);
            pos = end + 1;
            end = path.find ('\\', pos);
        }

        key = key.get_key_by_name (name);
    }

    // return key
    return key;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get subkey by mask
// @param a_mask fnmatch mask
// @return subkeys or empty vector, if no keys were found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<registry_key>
registry_key::get_key_by_mask (const std::string &a_mask) const
{
    const std::string mask = mobius::core::string::tolower (a_mask);
    std::string::size_type pos = 0;

    // skip heading '\'
    while (pos < mask.size () && mask[pos] == '\\')
        ++pos;

    // seek each subkey which name is limited by '\\'
    std::vector<registry_key> subkeys = {*this};
    std::string::size_type end = mask.find ('\\', pos);

    while (pos != std::string::npos && !subkeys.empty ())
    {
        // get sub mask
        std::string submask;

        if (end == std::string::npos)
        {
            submask = mask.substr (pos);
            pos = std::string::npos;
        }

        else
        {
            submask = mask.substr (pos, end - pos);
            pos = end + 1;
            end = mask.find ('\\', pos);
        }

        // create new vector with all subkeys whose name matches the sub mask
        std::vector<registry_key> tmp_keys;

        for (auto key : subkeys)
        {
            for (auto sk : key)
            {
                const std::string name =
                    mobius::core::string::tolower (sk.get_name ());

                if (mobius::core::string::fnmatch (submask, name))
                    tmp_keys.push_back (sk);
            }
        }

        // overwrite the candidate vector
        subkeys = tmp_keys;
    }

    // return subkeys
    return subkeys;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get value by name
// @param name value name
// @return value or empty value, if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_value
registry_key::get_value_by_name (const std::string &name) const
{
    const std::string lname = mobius::core::string::tolower (name);

    for (const auto &v : get_values ())
        if (mobius::core::string::tolower (v.get_name ()) == lname)
            return v;

    return registry_value ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get value by path
// @param name value path
// @return value or empty value, if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_value
registry_key::get_value_by_path (const std::string &path) const
{
    // parse registry_key.path + '\' + value_name
    std::string key_path;
    std::string value_name;

    auto pos = path.rfind ('\\');

    if (pos == std::string::npos)
        value_name = path;

    else
    {
        key_path = path.substr (0, pos);
        value_name = path.substr (pos + 1);
    }

    // search key
    registry_key key;

    if (key_path.empty ())
        key = *this;

    else
        key = get_key_by_path (key_path);

    // if key found, return value
    if (key)
        return key.get_value_by_name (value_name);

    return registry_value ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get values by mask
// @param a_mask value mask
// @return values or empty vector, if no values were found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<registry_value>
registry_key::get_value_by_mask (const std::string &a_mask) const
{
    // parse registry_key.mask + '\' + value.mask
    const std::string mask = mobius::core::string::tolower (a_mask);
    std::string key_mask;
    std::string value_mask;

    auto pos = mask.rfind ('\\');

    if (pos == std::string::npos)
        value_mask = mask;

    else
    {
        key_mask = mask.substr (0, pos);
        value_mask = mask.substr (pos + 1);
    }

    // search keys
    std::vector<registry_key> keys;

    if (key_mask.empty ())
        keys.push_back (*this);

    else
        keys = get_key_by_mask (key_mask);

    // search for values
    std::vector<registry_value> values;

    for (const auto &key : keys)
    {
        for (auto v : key.get_values ())
        {
            const std::string name =
                mobius::core::string::tolower (v.get_name ());

            if (mobius::core::string::fnmatch (value_mask, name))
                values.push_back (v);
        }
    }

    return values;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get data by value name
// @param name value name
// @return data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_data
registry_key::get_data_by_name (const std::string &name) const
{
    registry_data data;

    auto v = get_value_by_name (name);
    if (v)
        data = v.get_data ();

    return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get value by path
// @param name value path
// @return value or empty value, if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_data
registry_key::get_data_by_path (const std::string &path) const
{
    registry_data data;

    auto v = get_value_by_path (path);
    if (v)
        data = v.get_data ();

    return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get data by mask
// @param mask value mask
// @return data or empty vector, if no values were found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<registry_data>
registry_key::get_data_by_mask (const std::string &mask) const
{
    std::vector<registry_data> data_list;

    for (const auto &v : get_value_by_mask (mask))
        data_list.push_back (v.get_data ());

    return data_list;
}

} // namespace mobius::core::os::win::registry
