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
#include <mobius/core/crypt/hash_impl_adler32.hpp>
#include <numeric>

namespace mobius::core::crypt
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
constexpr std::uint64_t BLOCK_SIZE = 0x10000000;  // max block size per iteration
constexpr std::uint32_t BASE = 65521;		  // mod value

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset hash value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hash_impl_adler32::reset () noexcept
{
  a_ = 1;
  b_ = 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update hash by a given bytearray
// @param data Data buffer
//
// Original Adler-32 is:
//
//   for (std::uint8_t c : data)
//     {
//       a_ = (a_ + c) % BASE;
//       b_ = (b_ + a_) % BASE;
//     }
//
// I did some improvements:
//
//   1. if type (a) is large enough, a is sum (bytes) % BASE
//
//   2. b[1] = b[0] + a[1] = b[0] + a[0] + c[0]
//      b[2] = b[1] + a[2] = b[0] + a[0] + c[0] + a[0] + c[0] + c[1]
//      b[n] = b[n-1] + an = b[0] + a[0] * n + c[0] * n + c[1] * n-1 ... c[n] * 1
//
//   3. The BLOCK_SIZE value assures that 64-bit values can hold a[n] and b[n]
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hash_impl_adler32::update (const mobius::bytearray& data) noexcept
{
  const std::uint8_t *p = data.begin ();
  const std::uint64_t size = data.size ();

  for (std::uint64_t i = 0; i < size; i += BLOCK_SIZE)
    {
      std::uint64_t n = std::min (size - i, BLOCK_SIZE);
      std::uint64_t b1 = b_ + a_ * n;
      a_ = std::accumulate (p, p + n, std::uint64_t (a_)) % BASE;

      while (n)
        {
          b1 += std::uint64_t (*p) * n;
          n--;
          p++;
        }

      b_ = b1 % BASE;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Return hash digest
// @return Hash digest
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray
hash_impl_adler32::get_digest ()
{
  return mobius::bytearray (
    {
      std::uint8_t (b_ >> 8),
      std::uint8_t (b_),
      std::uint8_t (a_ >> 8),
      std::uint8_t (a_)
    }
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clone object
// @return Pointer to newly created object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr <hash_impl_base>
hash_impl_adler32::clone () const
{
  auto h = std::make_shared <hash_impl_adler32> ();

  h->a_ = a_;
  h->b_ = b_;

  return h;
}

} // namespace mobius::core::crypt


