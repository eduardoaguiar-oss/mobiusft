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
#include <mobius/core/database/database.hpp>
#include <mobius/core/generator.hpp>
#include <mobius/core/kff/file.hpp>
#include <cstdint>
#include <functional>
#include <iterator>
#include <ranges>
#include <string>

namespace mobius::core::kff
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File types
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
constexpr std::uint16_t FILE_TYPE_INFO = 1;
constexpr std::uint16_t FILE_TYPE_IGNORED = 2;
constexpr std::uint16_t FILE_TYPE_ALERT_CSAM = 3;
constexpr std::uint16_t FILE_TYPE_ALERT_MALWARE = 4;

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
file new_file (std::int64_t, std::uint16_t, std::int64_t = -1);
std::int64_t get_next_file_id ();
bool has_size (std::uint64_t);
file lookup (std::uint16_t, const std::string &);
std::uint16_t get_file_type (const std::string &);
std::uint16_t get_hash_type (const std::string &);
std::string get_file_type_string (std::uint16_t);
std::string get_hash_type_string (std::uint16_t);
std::uint64_t get_file_count ();
std::uint64_t get_hash_count ();
std::uint64_t get_alert_count ();
std::uint64_t get_thumbnail_count ();
mobius::core::generator<file> iter_files ();
mobius::core::generator<file> iter_files_by_type (std::uint16_t);

void export_collected_data (const std::string&, std::function<void (const std::string &)> = {});
void export_kff (const std::string&, std::function<void (const std::string &)> = {});
void import_collected_data (const std::string&, std::function<void (const std::string &)> = {});

} // namespace mobius::core::kff

#endif
