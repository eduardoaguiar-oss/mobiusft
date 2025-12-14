// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
// Eduardo Aguiar
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
#include <mobius/core/crypt/cipher.hpp>
#include <mobius/core/crypt/cipher_impl_ige.hpp>
#include <mobius/core/crypt/cipher_impl_null.hpp>
#include <mobius/core/crypt/cipher_impl_zip.hpp>
#include <mobius/core/crypt/gcrypt/cipher_impl.hpp>

namespace mobius::core::crypt
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
cipher::cipher ()
    : impl_ (std::make_shared<cipher_impl_null> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor from implementation pointer
// @param impl Implementation pointer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
cipher::cipher (const std::shared_ptr<cipher_impl_base> &impl)
    : impl_ (impl)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new cipher object with CBC mode
// @param id Cipher ID
// @param key Cipher key
// @param iv Initialization vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
cipher
new_cipher_cbc (const std::string &id, const bytearray &key,
                const bytearray &iv)
{
    auto c = std::make_shared<gcrypt::cipher_impl> (id, "cbc");
    c->set_key (key);

    if (iv)
        c->set_iv (iv);
    else
        c->set_iv (mobius::core::bytearray (c->get_block_size ()));

    return cipher (c);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new cipher object with CBC/CTS mode
// @param id Cipher ID
// @param key Cipher key
// @param iv Initialization vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
cipher
new_cipher_cbc_cts (const std::string &id, const bytearray &key,
                    const bytearray &iv)
{
    auto c = std::make_shared<gcrypt::cipher_impl> (id, "cbc-cts");
    c->set_key (key);

    if (iv)
        c->set_iv (iv);
    else
        c->set_iv (mobius::core::bytearray (c->get_block_size ()));

    return cipher (c);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new cipher object with CTR mode
// @param id Cipher ID
// @param key Cipher key
// @param cv Counter vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
cipher
new_cipher_ctr (const std::string &id, const bytearray &key,
                const bytearray &cv)
{
    auto c = std::make_shared<gcrypt::cipher_impl> (id, "ctr");
    c->set_key (key);

    if (cv)
        c->set_counter (cv);

    else
    {
        mobius::core::bytearray counter (c->get_block_size ());
        counter[counter.size () - 1] = 1;
        c->set_counter (counter);
    }

    return cipher (c);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new cipher object with ECB mode
// @param id Cipher ID
// @param key Cipher key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
cipher
new_cipher_ecb (const std::string &id, const bytearray &key)
{
    auto c = std::make_shared<gcrypt::cipher_impl> (id, "ecb");
    c->set_key (key);

    return cipher (c);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new cipher object with GCM mode
// @param id Cipher ID
// @param key Cipher key
// @param iv Initialization vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
cipher
new_cipher_gcm (const std::string &id, const bytearray &key,
                const bytearray &iv)
{
    auto c = std::make_shared<gcrypt::cipher_impl> (id, "gcm");
    c->set_key (key);

    if (iv)
        c->set_iv (iv);
    else
        c->set_iv (mobius::core::bytearray (c->get_block_size ()));

    return cipher (c);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new cipher object with Poly1305 mode
// @param id Cipher ID
// @param key Cipher key
// @param iv Initialization vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
cipher
new_cipher_poly1305 (const std::string &id, const bytearray &key,
                     const bytearray &iv)
{
    auto c = std::make_shared<gcrypt::cipher_impl> (id, "poly1305");
    c->set_key (key);

    if (iv)
        c->set_iv (iv);
    else
        c->set_iv (mobius::core::bytearray (c->get_block_size ()));

    return cipher (c);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new cipher object with IGE mode
// @param id Cipher ID
// @param key Cipher key
// @param iv Initialization vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
cipher
new_cipher_ige (const std::string &id, const bytearray &key,
                const bytearray &iv)
{
    return cipher (std::make_shared<cipher_impl_ige> (id, key, iv));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new cipher object with OFB mode
// @param id Cipher ID
// @param key Cipher key
// @param iv Initialization vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
cipher
new_cipher_ofb (const std::string &id, const bytearray &key,
                const bytearray &iv)
{
    auto c = std::make_shared<gcrypt::cipher_impl> (id, "ofb");
    c->set_key (key);

    if (iv)
        c->set_iv (iv);
    else
        c->set_iv (mobius::core::bytearray (c->get_block_size ()));

    return cipher (c);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new stream cipher object
// @param id Cipher ID
// @param key Cipher key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
cipher
new_cipher_stream (const std::string &id, const bytearray &key)
{
    if (id == "zip")
        return cipher (std::make_shared<cipher_impl_zip> (key));

    else
    {
        auto c = std::make_shared<gcrypt::cipher_impl> (id, "stream");
        c->set_key (key);
        return cipher (c);
    }
}

} // namespace mobius::core::crypt
