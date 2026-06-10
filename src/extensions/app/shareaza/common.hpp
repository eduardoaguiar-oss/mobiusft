#ifndef MOBIUS_EXTENSION_APP_SHAREAZA_COMMON_HPP
#define MOBIUS_EXTENSION_APP_SHAREAZA_COMMON_HPP

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
#include <mobius/core/pod/data.hpp>
#include <map>
#include <string>

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get map of hashes for a given file
// @param f File structure
// @return Map of hashes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::map<std::string, std::string>
get_file_hashes (const auto &f)
{
    std::map<std::string, std::string> hashes;

    auto hash_sha1 = f.get_hash_sha1 ();
    if (!hash_sha1.empty ())
        hashes["sha1"] = hash_sha1;

    auto hash_tiger = f.get_hash_tiger ();
    if (!hash_tiger.empty ())
        hashes["tiger"] = hash_tiger;

    auto hash_md5 = f.get_hash_md5 ();
    if (!hash_md5.empty ())
        hashes["md5"] = hash_md5;

    auto hash_ed2k = f.get_hash_ed2k ();
    if (!hash_ed2k.empty ())
        hashes["ed2k"] = hash_ed2k;

    auto hash_bth = f.get_hash_bth ();
    if (!hash_bth.empty ())
        hashes["bth"] = hash_bth;

    return hashes;
}

} // namespace mobius::extension::app::shareaza

#endif