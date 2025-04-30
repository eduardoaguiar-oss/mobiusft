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
#include <mobius/core/crypt/hash_impl_zip.hpp>
#include <mobius/core/crypt/crc32.hpp>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::uint32_t ZIPCONST = 0x08088405U;
static constexpr std::uint32_t INITK0 = 0x12345678;
static constexpr std::uint32_t INITK1 = 0x23456789;
static constexpr std::uint32_t INITK2 = 0x34567890;

} // namespace

namespace mobius::core::crypt
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hash_impl_zip::hash_impl_zip () noexcept
  : k0_ (INITK0), k1_ (INITK1), k2_ (INITK2)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset hash value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hash_impl_zip::reset () noexcept
{
  k0_ = INITK0;
  k1_ = INITK1;
  k2_ = INITK2;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update hash for a given bytearray
// @param data Data block
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hash_impl_zip::update (const mobius::core::bytearray& data) noexcept
{
  for (std::uint8_t c : data)
    {
      k0_ = crc32 (k0_, c);
      k1_ = (k1_ + (k0_ & 0xff)) * ZIPCONST + 1;
      k2_ = crc32 (k2_, k1_ >> 24);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Return hash digest
// @return Hash digest
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray
hash_impl_zip::get_digest ()
{
  return mobius::core::bytearray
  {
    std::uint8_t (k0_ >> 24),
    std::uint8_t (k0_ >> 16),
    std::uint8_t (k0_ >> 8),
    std::uint8_t (k0_),
    std::uint8_t (k1_ >> 24),
    std::uint8_t (k1_ >> 16),
    std::uint8_t (k1_ >> 8),
    std::uint8_t (k1_),
    std::uint8_t (k2_ >> 24),
    std::uint8_t (k2_ >> 16),
    std::uint8_t (k2_ >> 8),
    std::uint8_t (k2_),
  };
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clone object
// @return Pointer to newly created object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr <hash_impl_base>
hash_impl_zip::clone () const
{
  auto h = std::make_shared <hash_impl_zip> ();

  h->k0_ = k0_;
  h->k1_ = k1_;
  h->k2_ = k2_;

  return h;
}

} // namespace mobius::core::crypt


