// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025
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
#include <mobius/core/crypt/hash.hpp>
#include <mobius/core/crypt/hmac.hpp>
#include <mobius/core/crypt/pkcs5.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/os/win/dpapi/blob.hpp>
#include <mobius/core/os/win/dpapi/cipher_info.hpp>
#include <mobius/core/os/win/dpapi/hash_info.hpp>

namespace mobius::core::os::win::dpapi
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Blob implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class blob::impl
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit impl (mobius::core::io::reader);
    bool test_key (const mobius::core::bytearray &,
                   const mobius::core::bytearray &);
    bool decrypt (const mobius::core::bytearray &,
                  const mobius::core::bytearray &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl &operator= (const impl &) = delete;
    impl &operator= (impl &&) = delete;

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
    // @brief Get provider guid
    // @return Provider guid
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_provider_guid () const
    {
        return provider_guid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get master key revision
    // @return Master key revision
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_master_key_revision () const
    {
        return master_key_revision_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get master key guid
    // @return Master key guid
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_master_key_guid () const
    {
        return master_key_guid_;
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
    // @brief Get description
    // @return Description
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_description () const
    {
        return description_;
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
    // @brief Get key length
    // @return Key length
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_key_length () const
    {
        return key_length_;
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
    // @brief Get hmac key
    // @return Hmac key
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::bytearray
    get_hmac_key () const
    {
        return hmac_key_;
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
    // @brief Get hash length
    // @return Hash length
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_hash_length () const
    {
        return hash_length_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get hmac value
    // @return Hmac value
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::bytearray
    get_hmac_value () const
    {
        return hmac_value_;
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
    // @brief Get plain text
    // @return Plain text
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::bytearray
    get_plain_text () const
    {
        return plain_text_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get signature
    // @return Signature
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::bytearray
    get_signature () const
    {
        return signature_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get signature data
    // @return Signature data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::bytearray
    get_signature_data () const
    {
        return signature_data_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if blob is decrypted
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_decrypted () const noexcept
    {
        return bool (plain_text_);
    }

  private:
    // @brief Struct revision
    std::uint32_t revision_;

    // @brief Provider GUID
    std::string provider_guid_;

    // @brief Master key revision
    std::uint32_t master_key_revision_;

    // @brief Master key GUID
    std::string master_key_guid_;

    // @brief Flags
    std::uint32_t flags_;

    // @brief Description
    std::string description_;

    // @brief Cipher algorithm ID
    std::uint32_t cipher_id_;

    // @brief Key length in bytes
    std::uint32_t key_length_;

    // @brief Salt
    mobius::core::bytearray salt_;

    // @brief HMAC key value
    mobius::core::bytearray hmac_key_;

    // @brief Hash algorithm ID
    std::uint32_t hash_id_;

    // @brief Hash length in bytes
    std::uint32_t hash_length_;

    // @brief HMAC value
    mobius::core::bytearray hmac_value_;

    // @brief Cipher text
    mobius::core::bytearray cipher_text_;

    // @brief Plain text
    mobius::core::bytearray plain_text_;

    // @brief Signature
    mobius::core::bytearray signature_;

    // @brief Signature data (for signature computation)
    mobius::core::bytearray signature_data_;
};

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Generate session key
// @param ms_hash_id MS Hash ID
// @param key Decryption key
// @param salt Salt value
// @param entropy Optional entropy
// @param data Generic data (blob_data or hmac_value)
// @return Session key
// @see https://github.com/mis-team/dpapick
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::bytearray
_generate_session_key (std::uint32_t ms_hash_id,
                       const mobius::core::bytearray &key,
                       const mobius::core::bytearray &salt,
                       const mobius::core::bytearray &data = {},
                       const mobius::core::bytearray &entropy = {})
{
    // generate prekey
    mobius::core::bytearray prekey;

    if (key.size () == 20)
        prekey = key;

    else
    {
        mobius::core::crypt::hash h ("sha1");
        h.update (key);
        prekey = h.get_digest ();
    }

    // generate session key
    const auto hash_id = get_hash_id (ms_hash_id);
    mobius::core::bytearray session_key;

    if (hash_id == "sha1")
    {
        const auto hash_block_size = get_hash_block_size (ms_hash_id);

        mobius::core::bytearray ipad (hash_block_size);
        ipad.fill (0x36);
        ipad ^= prekey;

        mobius::core::bytearray opad (hash_block_size);
        opad.fill (0x5c);
        opad ^= prekey;

        mobius::core::crypt::hash h1 ("sha1");
        h1.update (ipad);
        h1.update (salt);

        mobius::core::crypt::hash h2 ("sha1");
        h2.update (opad);
        h2.update (h1.get_digest ());

        if (entropy)
            h2.update (entropy);

        if (data)
            h2.update (data);

        session_key = h2.get_digest ();
    }

    else
    {
        mobius::core::crypt::hmac hmac_signature (hash_id, prekey);
        hmac_signature.update (salt);

        if (entropy)
            hmac_signature.update (entropy);

        if (data)
            hmac_signature.update (data);

        session_key = hmac_signature.get_digest ();
    }

    return session_key;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
blob::impl::impl (mobius::core::io::reader reader)
{
    mobius::core::decoder::data_decoder decoder (reader);

    revision_ = decoder.get_uint32_le ();
    provider_guid_ = decoder.get_guid ();

    auto pos_start = decoder.tell ();

    master_key_revision_ = decoder.get_uint32_le ();
    master_key_guid_ = decoder.get_guid ();
    flags_ = decoder.get_uint32_le ();

    auto size = decoder.get_uint32_le ();
    description_ = decoder.get_string_by_size (size, "UTF-16LE");

    cipher_id_ = decoder.get_uint32_le ();
    key_length_ = decoder.get_uint32_le ();

    size = decoder.get_uint32_le ();
    salt_ = decoder.get_bytearray_by_size (size);

    size = decoder.get_uint32_le ();
    hmac_key_ = decoder.get_bytearray_by_size (size);

    hash_id_ = decoder.get_uint32_le ();
    hash_length_ = decoder.get_uint32_le ();

    size = decoder.get_uint32_le ();
    hmac_value_ = decoder.get_bytearray_by_size (size);

    size = decoder.get_uint32_le ();
    cipher_text_ = decoder.get_bytearray_by_size (size);

    auto pos_end = reader.tell ();

    size = decoder.get_uint32_le ();
    signature_ = decoder.get_bytearray_by_size (size);

    // blob data
    decoder.seek (pos_start);
    signature_data_ = decoder.get_bytearray_by_size (pos_end - pos_start);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
blob::blob (mobius::core::io::reader reader)
    : impl_ (std::make_shared<impl> (reader))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if key is the right one
// @param key Key
// @param entropy Optional entropy value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
blob::impl::test_key (const mobius::core::bytearray &key,
                      const mobius::core::bytearray &entropy)
{
    auto signature = _generate_session_key (hash_id_, key, hmac_value_,
                                            signature_data_, entropy);

    return signature == signature_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt BLOB using key
// @param key Key
// @param entropy Optional entropy value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
blob::impl::decrypt (const mobius::core::bytearray &key,
                     const mobius::core::bytearray &entropy)
{
    // test if key is the right one
    if (!test_key (key, entropy))
        return false;

    // generate session key
    auto session_key =
        _generate_session_key (hash_id_, key, salt_, {}, entropy);

    // generate derived key
    const auto hash_id = mobius::core::os::win::dpapi::get_hash_id (hash_id_);
    const auto hash_digest_size =
        mobius::core::os::win::dpapi::get_hash_digest_size (hash_id_);
    const auto hash_block_size =
        mobius::core::os::win::dpapi::get_hash_block_size (hash_id_);
    const auto cipher_key_length =
        mobius::core::os::win::dpapi::get_cipher_key_length (cipher_id_);

    if (session_key.size () > hash_digest_size)
    {
        mobius::core::crypt::hash h (hash_id);
        h.update (session_key);
        session_key = h.get_digest ();
    }

    mobius::core::bytearray derived_key;

    if (session_key.size () >= cipher_key_length)
        derived_key = session_key;

    else
    {
        mobius::core::bytearray ipad (hash_block_size);
        ipad.fill (0x36);

        mobius::core::crypt::hash h1 (hash_id);
        h1.update (ipad ^ session_key);

        mobius::core::bytearray opad (hash_block_size);
        opad.fill (0x5c);

        mobius::core::crypt::hash h2 (hash_id);
        h2.update (opad ^ session_key);

        derived_key = h1.get_digest () + h2.get_digest ();
    }

    // decrypt cipher text
    const auto cipher_id =
        mobius::core::os::win::dpapi::get_cipher_id (cipher_id_);
    auto c = mobius::core::crypt::new_cipher_cbc (
        cipher_id, derived_key.slice (0, cipher_key_length - 1));
    plain_text_ = mobius::core::crypt::pkcs5_unpad (c.decrypt (cipher_text_));

    return true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if key is the right one
// @param key Key
// @param entropy Optional entropy value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
blob::test_key (const mobius::core::bytearray &key,
                const mobius::core::bytearray &entropy)
{
    return impl_->test_key (key, entropy);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt BLOB using key
// @param key Key
// @param entropy Optional entropy value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
blob::decrypt (const mobius::core::bytearray &key,
               const mobius::core::bytearray &entropy)
{
    return impl_->decrypt (key, entropy);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if BLOB is decrypted
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
blob::is_decrypted () const
{
    return impl_->is_decrypted ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get revision
// @return Revision
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
blob::get_revision () const
{
    return impl_->get_revision ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get provider guid
// @return Provider guid
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
blob::get_provider_guid () const
{
    return impl_->get_provider_guid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get master key revision
// @return Master key revision
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
blob::get_master_key_revision () const
{
    return impl_->get_master_key_revision ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get master key guid
// @return Master key guid
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
blob::get_master_key_guid () const
{
    return impl_->get_master_key_guid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get flags
// @return Flags
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
blob::get_flags () const
{
    return impl_->get_flags ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get description
// @return Description
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
blob::get_description () const
{
    return impl_->get_description ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get cipher id
// @return Cipher id
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
blob::get_cipher_id () const
{
    return impl_->get_cipher_id ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get key length
// @return Key length
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
blob::get_key_length () const
{
    return impl_->get_key_length ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get salt
// @return Salt
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
blob::get_salt () const
{
    return impl_->get_salt ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hmac key
// @return Hmac key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
blob::get_hmac_key () const
{
    return impl_->get_hmac_key ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash id
// @return Hash id
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
blob::get_hash_id () const
{
    return impl_->get_hash_id ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash length
// @return Hash length
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
blob::get_hash_length () const
{
    return impl_->get_hash_length ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hmac value
// @return Hmac value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
blob::get_hmac_value () const
{
    return impl_->get_hmac_value ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get cipher text
// @return Cipher text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
blob::get_cipher_text () const
{
    return impl_->get_cipher_text ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get plain text
// @return Plain text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
blob::get_plain_text () const
{
    return impl_->get_plain_text ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get signature
// @return Signature
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
blob::get_signature () const
{
    return impl_->get_signature ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get blob data
// @return BLOB data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
blob::get_signature_data () const
{
    return impl_->get_signature_data ();
}

} // namespace mobius::core::os::win::dpapi
