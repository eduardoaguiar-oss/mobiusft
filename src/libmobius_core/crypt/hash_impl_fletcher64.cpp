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
#include <mobius/core/crypt/hash_impl_fletcher64.hpp>
#include <numeric>

namespace mobius::core::crypt
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset hash value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hash_impl_fletcher64::reset () noexcept
{
    sum1_ = 0xffffffff;
    sum2_ = 0xffffffff;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update hash by a given bytearray
// @param data Data buffer
// @see https://en.wikipedia.org/wiki/Fletcher%27s_checksum#Fletcher-64
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hash_impl_fletcher64::update (const mobius::core::bytearray &data) noexcept
{
    const uint8_t *p = data.data ();
    size_t siz = data.size ();
    size_t count = 0;
    constexpr size_t block_size = 90000;

    // Process 32-bit dwords
    while (siz >= 4)
    {
        std::uint64_t word = static_cast<std::uint64_t> (p[0]) |
                             (static_cast<std::uint64_t> (p[1]) << 8) |
                             (static_cast<std::uint64_t> (p[2]) << 16) |
                             (static_cast<std::uint64_t> (p[3]) << 24);

        sum1_ += word;
        sum2_ += sum1_;
        p += 4;
        siz -= 4;

        // Apply modulo to keep values within 32-bit range
        if (++count >= block_size)
        {
            sum1_ = (sum1_ & 0xFFFFFFFF) + (sum1_ >> 32);
            sum2_ = (sum2_ & 0xFFFFFFFF) + (sum2_ >> 32);
            count = 0;
        }
    }

    // Process remaining bytes (0-3)
    if (siz > 0)
    {
        std::uint64_t word =
            p[0] | (siz > 1 ? static_cast<std::uint64_t> (p[1]) << 8 : 0) |
            (siz > 2 ? static_cast<std::uint64_t> (p[2]) << 16 : 0) |
            (siz > 3 ? static_cast<std::uint64_t> (p[3]) << 24 : 0);

        sum1_ += word;
        sum2_ += sum1_;

        // Final modulo for the last partial word
        sum1_ = (sum1_ & 0xFFFFFFFF) + (sum1_ >> 32);
        sum2_ = (sum2_ & 0xFFFFFFFF) + (sum2_ >> 32);
    }

    // Final reduction to 32 bits (for 64-bit Fletcher variant)
    sum1_ = (sum1_ & 0xFFFFFFFF) + (sum1_ >> 32);
    sum2_ = (sum2_ & 0xFFFFFFFF) + (sum2_ >> 32);

    // One more reduction in case the previous step produced a carry
    sum1_ = (sum1_ & 0xFFFFFFFF) + (sum1_ >> 32);
    sum2_ = (sum2_ & 0xFFFFFFFF) + (sum2_ >> 32);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Return hash digest
// @return Hash digest
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray
hash_impl_fletcher64::get_digest ()
{
    std::uint64_t s = (sum2_ << 32) | sum1_;

    return mobius::core::bytearray (
        {std::uint8_t (s & 0xff), std::uint8_t (s >> 8), std::uint8_t (s >> 16),
         std::uint8_t (s >> 24), std::uint8_t (s >> 32), std::uint8_t (s >> 40),
         std::uint8_t (s >> 48), std::uint8_t (s >> 56)}
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clone object
// @return Pointer to newly created object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr<hash_impl_base>
hash_impl_fletcher64::clone () const
{
    auto h = std::make_shared<hash_impl_fletcher64> ();

    h->sum1_ = sum1_;
    h->sum2_ = sum2_;

    return h;
}

} // namespace mobius::core::crypt
