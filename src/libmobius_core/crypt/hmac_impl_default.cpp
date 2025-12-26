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
#include <mobius/core/crypt/hmac_impl_default.hpp>

namespace mobius::core::crypt
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param hash_type Hash type
// @param key Key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hmac_impl_default::hmac_impl_default (const std::string &hash_type,
                                      const mobius::core::bytearray &key)
    : h_inner_ (hash_type),
      h_outer_ (hash_type)
{
    std::size_t block_size = h_inner_.get_block_size ();

    bytearray K (block_size);
    K.fill (0);

    // keys longer than H::block_size are first hashed using H (RFC 2104 -
    // section 3)
    if (key.size () > block_size)
    {
        hash h (hash_type);
        h.update (key);
        auto digest = h.get_digest ();
        std::copy (digest.begin (), digest.end (), K.begin ());
    }

    else
    {
        std::copy (key.begin (), key.end (), K.begin ());
    }

    // initialize inner hash object (RFC 2104 - section 2)
    bytearray ipad (block_size);
    ipad.fill (0x36);
    h_inner_.update (K ^ ipad);

    // initialize outer hash object (RFC 2104 - section 2)
    bytearray opad (block_size);
    opad.fill (0x5c);
    h_outer_.update (K ^ opad);

    h_inner_0_ = h_inner_.clone ();
    h_outer_0_ = h_outer_.clone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset hmac value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hmac_impl_default::reset ()
{
    h_inner_ = h_inner_0_.clone ();
    h_outer_ = h_outer_0_.clone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash digest
// @return Hash digest
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
hmac_impl_default::get_digest ()
{
    h_outer_.update (h_inner_.get_digest ());
    return h_outer_.get_digest ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clone object
// @return Pointer to newly created object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr<hmac_impl_base>
hmac_impl_default::clone () const
{
    auto hmac = std::make_shared<hmac_impl_default> ();

    hmac->h_inner_ = h_inner_.clone ();
    hmac->h_outer_ = h_outer_.clone ();
    hmac->h_inner_0_ = h_inner_0_.clone ();
    hmac->h_outer_0_ = h_outer_0_.clone ();

    return hmac;
}

} // namespace mobius::core::crypt
