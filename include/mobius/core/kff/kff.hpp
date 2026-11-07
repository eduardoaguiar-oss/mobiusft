#ifndef MOBIUS_CORE_KFF_KFF_HPP
#define MOBIUS_CORE_KFF_KFF_HPP

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
#include <mobius/core/database/connection_set.hpp>
#include <mobius/core/database/database.hpp>
#include <mobius/core/kff/hashset.hpp>
#include <mobius/core/kff/file.hpp>
#include <unordered_map>
#include <utility>
#include <map>
#include <string>
#include <vector>

namespace mobius::core::kff
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File types
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
constexpr std::uint16_t FILE_TYPE_REGULAR = 0;
constexpr std::uint16_t FILE_TYPE_ALERT_CSAM = 1;
constexpr std::uint16_t FILE_TYPE_ALERT_MALWARE = 2;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Hash types
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
constexpr std::uint16_t HASH_TYPE_ED2K = 1;
constexpr std::uint16_t HASH_TYPE_MD5 = 2;
constexpr std::uint16_t HASH_TYPE_SHA1 = 3;
constexpr std::uint16_t HASH_TYPE_SHA2_256 = 4;
constexpr std::uint16_t HASH_TYPE_SHA2_512 = 5;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::database::database get_database ();
mobius::core::database::transaction new_transaction ();
bool has_size (uint64_t);
file lookup (std::uint16_t, const std::string &);
std::map<std::string, int64_t> get_file_count_by_type ();

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief KFF database (singleton)
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class kff
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    kff ();
    kff (kff &&) noexcept = default;
    kff (const kff &) noexcept = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    kff &operator= (const kff &) noexcept = default;
    kff &operator= (kff &&) noexcept = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Function prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::database::connection_set new_connection ();

    hashset new_hashset (const std::string &, bool);
    void remove_hashset (const std::string &);
    std::vector<std::pair<std::string, hashset>> get_hashsets () const;
    std::vector<std::string> alert_lookup (const std::string &, const std::string &) const;
    char lookup (const std::string &, const std::string &) const;
};

} // namespace mobius::core::kff

#endif
