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
#include <mobius/core/crypt/hash_impl_fletcher16.hpp>
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
hash_impl_fletcher16::reset () noexcept
{
    sum1_ = 0xff;
    sum2_ = 0xff;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update hash by a given bytearray
// @param data Data buffer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hash_impl_fletcher16::update (const mobius::core::bytearray &data) noexcept
{
    for (const std::uint32_t b : data)
    {
        sum1_ += b;
        sum2_ += sum1_;

        // Reduce before we risk overflowing 16 bits significantly
        if (sum1_ >= 0xFF00 || sum2_ >= 0xFF00)
        {
            sum1_ %= 255;
            sum2_ %= 255;
        }
    }

    // Final reduction
    sum1_ %= 255;
    sum2_ %= 255;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Return hash digest
// @return Hash digest
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bytearray
hash_impl_fletcher16::get_digest ()
{
    std::uint16_t s = static_cast<std::uint16_t> (sum2_ << 8) |
                      static_cast<std::uint16_t> (sum1_);

    return mobius::core::bytearray (
        {std::uint8_t (s & 0xff), std::uint8_t (s >> 8)}
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clone object
// @return Pointer to newly created object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr<hash_impl_base>
hash_impl_fletcher16::clone () const
{
    auto h = std::make_shared<hash_impl_fletcher16> ();

    h->sum1_ = sum1_;
    h->sum2_ = sum2_;

    return h;
}

} // namespace mobius::core::crypt
