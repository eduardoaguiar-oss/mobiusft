#ifndef MOBIUS_EXTENSION_APP_EMULE_COMMON_HPP
#define MOBIUS_EXTENSION_APP_EMULE_COMMON_HPP

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
#include <mobius/core/pod/data.hpp>
#include <mobius/core/string_functions.hpp>
#include <utility>
#include <string>
#include <vector>

namespace mobius::extension::app::emule
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get vector of hashes for a given file
// @param f File structure
// @return Vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<mobius::core::pod::data>
get_file_hashes (const auto &f)
{
    std::vector<mobius::core::pod::data> hashes = {
        {"ed2k", mobius::core::string::toupper (f.hash_ed2k)}
    };

    auto iter = std::find_if (
        f.tags.begin (), f.tags.end (),
        [] (const auto &t) { return t.get_id () == 0x27; } // FT_AICH_HASH
    );

    if (iter != f.tags.end ())
    {
        auto hash_value = iter->get_value ();
        hashes.push_back ({"aich", static_cast<std::string> (hash_value)});
    }

    return hashes;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Function prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::pair<std::string, std::string> get_app_from_path (const std::string &);

} // namespace mobius::extension::app::emule

#endif