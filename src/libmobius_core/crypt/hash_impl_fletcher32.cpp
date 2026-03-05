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
#include <mobius/core/crypt/hash_impl_fletcher32.hpp>
#include <numeric>

namespace mobius::core::crypt
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset hash value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hash_impl_fletcher32::reset () noexcept
{
    sum1_ = 0xffff;
    sum2_ = 0xffff;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update hash by a given bytearray
// @param data Data buffer
// @see https://en.wikipedia.org/wiki/Fletcher%27s_checksum#Fletcher-32
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hash_impl_fletcher32::update (const mobius::core::bytearray &data) noexcept
{
    // Process data in blocks to avoid overflow in 64-bit accumulators
    constexpr size_t block_size = 20000000;
    const uint8_t *p = data.data ();
    size_t siz = data.size ();
    size_t count = 0;

    while (siz >= 2)
    {
        sum1_ += static_cast<std::uint64_t> (p[0]) |
                 (static_cast<std::uint64_t> (p[1]) << 8);
        sum2_ += sum1_;
        p += 2;
        siz -= 2;
        count++;

        if (count >= block_size)
        {
            // Apply modulo to keep values within 16-bit range
            sum1_ = (sum1_ & 0xFFFF) + (sum1_ >> 16);
            sum2_ = (sum2_ & 0xFFFF) + (sum2_ >> 16);
            count = 0;
        }
    }

    // Handle odd byte if present
    if (siz == 1)
    {
        sum1_ += static_cast<std::uint64_t> (p[0]);
        sum2_ += sum1_;
    }

    // Final reduction to 16 bits
    sum1_ = (sum1_ & 0xFFFF) + (sum1_ >> 16);
    sum2_ = (sum2_ & 0xFFFF) + (sum2_ >> 16);

    // One more reduction in case the previous step produced a carry
    sum1_ = (sum1_ & 0xFFFF) + (sum1_ >> 16);
    sum2_ = (sum2_ & 0xFFFF) + (sum2_ >> 16);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Return hash digest
// @return Hash digest
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray
hash_impl_fletcher32::get_digest ()
{
    std::uint32_t s = (sum2_ << 16) | sum1_;

    return mobius::core::bytearray (
        {std::uint8_t (s & 0xff), std::uint8_t ((s >> 8) & 0xff),
         std::uint8_t ((s >> 16) & 0xff), std::uint8_t ((s >> 24) & 0xff)}
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clone object
// @return Pointer to newly created object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr<hash_impl_base>
hash_impl_fletcher32::clone () const
{
    auto h = std::make_shared<hash_impl_fletcher32> ();

    h->sum1_ = sum1_;
    h->sum2_ = sum2_;

    return h;
}

} // namespace mobius::core::crypt
