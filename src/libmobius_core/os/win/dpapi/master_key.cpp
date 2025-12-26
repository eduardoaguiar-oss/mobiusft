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
#include <map>
#include <mobius/core/charset.hpp>
#include <mobius/core/crypt/cipher.hpp>
#include <mobius/core/crypt/hash.hpp>
#include <mobius/core/crypt/hmac.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/os/win/dpapi/cipher_info.hpp>
#include <mobius/core/os/win/dpapi/hash_info.hpp>
#include <mobius/core/os/win/dpapi/master_key.hpp>
#include <mobius/core/os/win/pbkdf2_hmac_ms.hpp>
#include <stdexcept>

namespace mobius::core::os::win::dpapi
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constant values
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
constexpr bool DEBUG = false;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief master_key implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class master_key::impl
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl (mobius::core::io::reader, std::uint32_t);
    impl (const impl &) = delete;
    impl (impl &&) = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl &operator= (const impl &) = delete;
    impl &operator= (impl &&) = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool decrypt_with_key (const mobius::core::bytearray &);
    bool decrypt_with_password_hash (const std::string &,
                                     const mobius::core::bytearray &);
    bool decrypt_with_password (const std::string &, const std::string &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get revision
    // @return Revision
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_revision () const
    {
        return revision_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Set revision
    // @param revision Struct revision
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    set_revision (std::uint32_t revision)
    {
        revision_ = revision;
        is_valid_ = true;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get salt
    // @return Salt
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::bytearray
    get_salt () const
    {
        return salt_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Set salt
    // @param salt Salt
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    set_salt (const mobius::core::bytearray &salt)
    {
        salt_ = salt;
        is_valid_ = true;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get iterations
    // @return Iterations
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_iterations () const
    {
        return iterations_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Set iterations
    // @param iterations Number of key iterations
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    set_iterations (std::uint32_t iterations)
    {
        iterations_ = iterations;
        is_valid_ = true;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get hash id
    // @return Hash id
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_hash_id () const
    {
        return hash_id_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Set hash id
    // @param hash_id Hash algorithm ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    set_hash_id (std::uint32_t hash_id)
    {
        hash_id_ = hash_id;
        is_valid_ = true;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get cipher id
    // @return Cipher id
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_cipher_id () const
    {
        return cipher_id_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Set cipher id
    // @param cipher_id Cipher algorithm ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    set_cipher_id (std::uint32_t cipher_id)
    {
        cipher_id_ = cipher_id;
        is_valid_ = true;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get cipher text
    // @return Cipher text
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::bytearray
    get_cipher_text () const
    {
        return cipher_text_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Set cipher text
    // @param cipher_text Cipher text
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    set_cipher_text (const mobius::core::bytearray &cipher_text)
    {
        cipher_text_ = cipher_text;
        is_valid_ = true;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get plain text
    // @return Plain text
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::bytearray
    get_plain_text () const
    {
        return plain_text_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Set plain text
    // @param plain_text Plain text
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    set_plain_text (const mobius::core::bytearray &plain_text)
    {
        plain_text_ = plain_text;
        is_valid_ = true;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get flags
    // @return Flags
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_flags () const
    {
        return flags_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Set flags
    // @param flags Master key file flags
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    set_flags (std::uint32_t flags)
    {
        flags_ = flags;
        is_valid_ = true;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get decrypted flag
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_decrypted () const
    {
        return plain_text_.size () > 0;
    }

  private:
    // @brief Struct revision
    std::uint32_t revision_;

    // @brief Salt
    mobius::core::bytearray salt_;

    // @brief Number of key iterations
    std::uint32_t iterations_;

    // @brief Hash algorithm ID
    std::uint32_t hash_id_;

    // @brief Cipher algorithm ID
    std::uint32_t cipher_id_;

    // @brief Cipher text
    mobius::core::bytearray cipher_text_;

    // @brief Master key file flags
    std::uint32_t flags_;

    // @brief Plain text
    mobius::core::bytearray plain_text_;

    // @brief True/false if object is valid
    std::uint32_t is_valid_ = false;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param flags Master key file flags
// @see https://www.passcape.com/index.php?section=blog&cmd=details&id=20
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
master_key::impl::impl (mobius::core::io::reader reader, std::uint32_t flags)
{
    mobius::core::decoder::data_decoder decoder (reader);
    constexpr std::uint64_t SALT_SIZE = 16;

    revision_ = decoder.get_uint32_le ();
    salt_ = decoder.get_bytearray_by_size (SALT_SIZE);
    flags_ = flags;

    if (revision_ == 1) // win2000
    {
        iterations_ = 1;
        hash_id_ = 0x8009;   // CALG_SHA1
        cipher_id_ = 0x6801; // CALG_RC4
        cipher_text_ = decoder.get_bytearray_by_size (reader.get_size () - 20);
    }

    else if (revision_ == 2)
    {
        iterations_ = decoder.get_uint32_le ();
        hash_id_ = decoder.get_uint32_le ();
        cipher_id_ = decoder.get_uint32_le ();
        cipher_text_ = decoder.get_bytearray_by_size (reader.get_size () - 32);
    }

    else
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("Unknown DPAPI master key revision"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt master key using key
// @param key Key
// @return true if successful, false if not
// @see dpapick v0.3
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
master_key::impl::decrypt_with_key (const mobius::core::bytearray &key)
{
    // generate prekey
    auto hash_id = mobius::core::os::win::dpapi::get_hash_id (hash_id_);
    auto hash_digest_size = get_hash_digest_size (hash_id_);
    auto cipher_key_length = get_cipher_key_length (cipher_id_);
    auto cipher_salt_length = get_cipher_salt_length (cipher_id_);
    auto value = mobius::core::os::win::pbkdf2_hmac_ms (
        key, salt_, iterations_, cipher_key_length + cipher_salt_length,
        hash_id);

    auto prekey = value.slice (0, cipher_key_length - 1);
    auto presalt = value.slice (cipher_key_length,
                                cipher_key_length + cipher_salt_length - 1);

    // decrypt cipher text
    auto cipher_id = mobius::core::os::win::dpapi::get_cipher_id (cipher_id_);
    auto c = mobius::core::crypt::new_cipher_cbc (cipher_id, prekey, presalt);
    auto cleartxt = c.decrypt (cipher_text_);

    // evaluate HMAC
    auto hmac_salt = cleartxt.slice (0, 15);
    auto hmac_value = cleartxt.slice (16, 16 + hash_digest_size - 1);
    cleartxt = cleartxt.slice (cleartxt.size () - 64, cleartxt.size () - 1);

    auto hmac_1 = mobius::core::crypt::hmac (hash_id, key);
    hmac_1.update (hmac_salt);

    auto hmac_2 = mobius::core::crypt::hmac (hash_id, hmac_1.get_digest ());
    hmac_2.update (cleartxt);

    // check if decryption is successful
    bool flag_decrypted = false;

    if (hmac_2.get_digest () == hmac_value)
    {
        plain_text_ = cleartxt;
        flag_decrypted = true;
    }

    return flag_decrypted;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt entry with password hash
// @param sid User SID as string
// @param h Password hash
// @return true if master key has been decrypted
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
master_key::impl::decrypt_with_password_hash (const std::string &sid,
                                              const mobius::core::bytearray &h)
{
    mobius::core::crypt::hmac hmac ("sha1", h);
    hmac.update (mobius::core::conv_charset (bytearray (sid) + bytearray ({0}),
                                             "ASCII", "UTF-16LE"));
    return decrypt_with_key (hmac.get_digest ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt entry with password
// @param sid User SID as string
// @param pwd Password
// @return true if master key has been decrypted
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
master_key::impl::decrypt_with_password (const std::string &sid,
                                         const std::string &pwd)
{
    constexpr std::uint32_t DPAPI_MASTER_KEY_SHA1 = 0x00000004;

    mobius::core::crypt::hash h (flags_ & DPAPI_MASTER_KEY_SHA1 ? "sha1"
                                                                : "md4");
    h.update (mobius::core::conv_charset (pwd, "UTF-8", "UTF-16LE"));

    return decrypt_with_password_hash (sid, h.get_digest ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param flags Master key file flags
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
master_key::master_key (mobius::core::io::reader reader, std::uint32_t flags)
    : impl_ (std::make_shared<impl> (reader, flags))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt master key using key
// @param key Key
// @return True if successful, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
master_key::decrypt_with_key (const mobius::core::bytearray &key)
{
    return impl_->decrypt_with_key (key);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt master key using password hash
// @param sid User SID as string
// @param password_hash Password hash
// @return True if successful, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
master_key::decrypt_with_password_hash (
    const std::string &sid, const mobius::core::bytearray &password_hash)
{
    return impl_->decrypt_with_password_hash (sid, password_hash);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt master key using password
// @param sid User SID as string
// @param password Password
// @return True if successful, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
master_key::decrypt_with_password (const std::string &sid,
                                   const std::string &password)
{
    return impl_->decrypt_with_password (sid, password);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get revision
// @return Revision
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
master_key::get_revision () const
{
    return impl_->get_revision ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get salt
// @return Salt
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
master_key::get_salt () const
{
    return impl_->get_salt ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get iterations
// @return Iterations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
master_key::get_iterations () const
{
    return impl_->get_iterations ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash id
// @return Hash id
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
master_key::get_hash_id () const
{
    return impl_->get_hash_id ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get cipher id
// @return Cipher id
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
master_key::get_cipher_id () const
{
    return impl_->get_cipher_id ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get cipher text
// @return Cipher text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
master_key::get_cipher_text () const
{
    return impl_->get_cipher_text ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get plain text
// @return Plain text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
master_key::get_plain_text () const
{
    return impl_->get_plain_text ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get flags
// @return Flags
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
master_key::get_flags () const
{
    return impl_->get_flags ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get decrypted flag
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
master_key::is_decrypted () const
{
    return impl_->is_decrypted ();
}

} // namespace mobius::core::os::win::dpapi
