#ifndef MOBIUS_EXTENSION_APP_SHAREAZA_COMMON_HPP
#define MOBIUS_EXTENSION_APP_SHAREAZA_COMMON_HPP

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
#include <string>
#include <vector>

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get vector of hashes for a given file
// @param f File structure
// @return Vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<mobius::core::pod::data>
get_file_hashes (const auto &f)
{
    std::vector<std::pair<std::string, std::string>> values = {
        {"sha1", f.get_hash_sha1 ()}, {"tiger", f.get_hash_tiger ()},
        {"md5", f.get_hash_md5 ()},   {"ed2k", f.get_hash_ed2k ()},
        {"bth", f.get_hash_bth ()},
    };

    std::vector<mobius::core::pod::data> hashes;

    for (const auto &[k, v] : values)
    {
        if (!v.empty ())
            hashes.push_back ({k, v});
    }

    return hashes;
}

} // namespace mobius::extension::app::shareaza

#endif