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
#include "hash_info.h"
#include <mobius/exception.inc>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Hash information map
//! \see https://docs.microsoft.com/en-us/windows/desktop/seccrypto/alg-id
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct hash_info
{
  std::string id;			// crypt::hash hash_id
  std::uint32_t digest_size;		// digest size in bytes
  std::uint32_t block_size;		// block size in bytes
};

static const std::map <std::uint32_t, hash_info> HASHES =
{
  {0x8002, {"md4", 16, 64}},
  {0x8003, {"md5", 16, 64}},
  {0x8004, {"sha1", 20, 64}},
  {0x8009, {"sha1", 20, 64}},
  {0x800c, {"sha2-256", 32, 64}},
  {0x800d, {"sha2-384", 48, 128}},
  {0x800e, {"sha2-512", 64, 128}},
};

} // namespace

namespace mobius::os::win::dpapi
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get mobius::crypt hash ID from MS alg ID
//! \param ms_alg_id MS alg ID
//! \return mobius::crypt::hash ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_hash_id (std::uint32_t ms_alg_id)
{
  auto iter = HASHES.find (ms_alg_id);
  
  if (iter != HASHES.end ())
    return iter->second.id;
  
  else
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("Unknown MS alg ID"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get hash digest size from MS alg ID
//! \param ms_alg_id MS alg ID
//! \return Digest size in bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
get_hash_digest_size (std::uint32_t ms_alg_id)
{
  auto iter = HASHES.find (ms_alg_id);
  
  if (iter != HASHES.end ())
    return iter->second.digest_size;
  
  else
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("Unknown MS alg ID"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get hash block size from MS alg ID
//! \param ms_alg_id MS alg ID
//! \return Block size in bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
get_hash_block_size (std::uint32_t ms_alg_id)
{
  auto iter = HASHES.find (ms_alg_id);
  
  if (iter != HASHES.end ())
    return iter->second.block_size;
  
  else
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("Unknown MS alg ID"));
}

} // namespace mobius::os::win::dpapi
