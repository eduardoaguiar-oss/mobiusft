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
#include "credhist_entry.h"
#include "cipher_info.h"
#include "hash_info.h"
#include <mobius/core/charset.hpp>
#include <mobius/core/crypt/cipher.hpp>
#include <mobius/core/crypt/hash.hpp>
#include <mobius/core/crypt/hmac.hpp>
#include <mobius/decoder/data_decoder.h>
#include <mobius/exception.inc>
#include <mobius/os/win/pbkdf2_hmac_ms.h>
#include <stdexcept>

namespace mobius::os::win::dpapi
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief credhist_entry implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class credhist_entry::impl
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl (const impl&) = delete;
  impl (impl&&) = delete;
  impl (mobius::io::reader, std::uint32_t);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl& operator= (const impl&) = delete;
  impl& operator= (impl&&) = delete;

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
  // @brief Get guid
  // @return Guid
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_guid () const
  {
    return guid_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get next link size
  // @return Size in bytes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_next_link_size () const
  {
    return next_link_size_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get type
  // @return Type
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_type () const
  {
    return type_;
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
  // @brief Get iterations
  // @return Iterations
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t
  get_iterations () const
  {
    return iterations_;
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
  // @brief Get salt
  // @return Salt
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::bytearray
  get_salt () const
  {
    return salt_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get SID
  // @return Sid
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_sid () const
  {
    return sid_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get cipher text
  // @return Cipher text
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::bytearray
  get_cipher_text () const
  {
    return cipher_text_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get hash sha1
  // @return Hash sha1
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::bytearray
  get_hash_sha1 () const
  {
    return hash_sha1_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get hash ntlm
  // @return Hash ntlm
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::bytearray
  get_hash_ntlm () const
  {
    return hash_ntlm_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get is decrypted flag
  // @return true/false
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bool
  is_decrypted () const
  {
    return is_decrypted_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bool decrypt_with_key (const mobius::bytearray&);
  bool decrypt_with_password_hash (const mobius::bytearray&);
  bool decrypt_with_password (const std::string&);

private:
  // @brief Struct revision
  std::uint32_t revision_;

  // @brief GUID
  std::string guid_;

  // @brief Next link size in bytes
  std::uint32_t next_link_size_;

  // @brief Type
  std::uint32_t type_ = 0;

  // @brief Hash algorithm ID
  std::uint32_t hash_id_ = 0;

  // @brief Number of key iterations
  std::uint32_t iterations_ = 0;

  // @brief Cipher algorithm ID
  std::uint32_t cipher_id_ = 0;

  // @brief Salt
  mobius::bytearray salt_;

  // @brief SID
  std::string sid_;

  // @brief Cipher text
  mobius::bytearray cipher_text_;

  // @brief SHA1 hash value
  mobius::bytearray hash_sha1_;

  // @brief MD4 hash value
  mobius::bytearray hash_ntlm_;

  // @brief SHA1 hash length
  std::uint32_t sha1_length_ = 0;

  // @brief MD4 hash length
  std::uint32_t ntlm_length_ = 0;

  // @brief Is decrypted flag
  bool is_decrypted_ = false;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
credhist_entry::impl::impl (mobius::io::reader reader, std::uint32_t size)
{
  constexpr std::uint32_t FOOTER_SIZE = 24;

  // read entry footer
  mobius::decoder::data_decoder decoder (reader);
  auto end_pos = decoder.tell () + size;

  revision_ = decoder.get_uint32_le ();
  guid_ = decoder.get_guid ();
  next_link_size_ = decoder.get_uint32_le ();

  // read entry data, if available
  if (size > FOOTER_SIZE)
    {
      type_ = decoder.get_uint32_le ();
      hash_id_ = decoder.get_uint32_le ();
      iterations_ = decoder.get_uint32_le ();
      auto sid_size = decoder.get_uint32_le ();
      cipher_id_ = decoder.get_uint32_le ();
      sha1_length_ = decoder.get_uint32_le ();
      ntlm_length_ = decoder.get_uint32_le ();
      salt_ = decoder.get_bytearray_by_size (16);

      // read user SID, if available
      if (sid_size > 0)
        sid_ = decoder.get_sid ();

      // read cipher text
      cipher_text_ = decoder.get_bytearray_by_size (end_pos - reader.tell ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt entry with key
// @param key Decryption key
// @return true if entry has been decrypted using key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
credhist_entry::impl::decrypt_with_key (const mobius::bytearray& key)
{
  bool rc = false;

  if (!is_decrypted_)
    {
      // generate prekey
      auto hash_id = mobius::os::win::dpapi::get_hash_id (hash_id_);
      auto cipher_key_length = get_cipher_key_length (cipher_id_);
      auto cipher_salt_length = get_cipher_salt_length (cipher_id_);
      auto value = mobius::os::win::pbkdf2_hmac_ms (key, salt_, iterations_, cipher_key_length + cipher_salt_length, hash_id);

      auto prekey = value.slice (0, cipher_key_length - 1);
      auto presalt = value.slice (cipher_key_length, cipher_key_length + cipher_salt_length - 1);

      // decrypt cipher text
      auto cipher_id = mobius::os::win::dpapi::get_cipher_id (cipher_id_);
      auto c = mobius::core::crypt::new_cipher_cbc (cipher_id, prekey, presalt);
      auto cleartxt = c.decrypt (cipher_text_);

      // check decryption
      constexpr std::uint32_t NT_HASH_LENGTH = 16;
      std::uint32_t ntlm_real_length = std::min (NT_HASH_LENGTH, ntlm_length_);

      if (ntlm_length_ > ntlm_real_length)
        {
          auto remain = cleartxt.slice (sha1_length_ + ntlm_real_length, sha1_length_ + ntlm_length_ - 1);

          if (remain.all_equal (0))
            {
              is_decrypted_ = true;
              rc = true;
              hash_sha1_ = cleartxt.slice (0, sha1_length_ - 1);
              hash_ntlm_ = cleartxt.slice (sha1_length_, sha1_length_ + ntlm_real_length - 1);
            }
        }
    }

  return rc;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt entry with password hash
// @param h Password hash
// @return true if entry has been decrypted using password hash
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
credhist_entry::impl::decrypt_with_password_hash (const mobius::bytearray& h)
{
  bool rc = false;

  if (!is_decrypted_)
    {
      mobius::core::crypt::hmac hmac ("sha1", h);
      hmac.update (mobius::core::conv_charset (bytearray (sid_) + bytearray ({0}), "ASCII", "UTF-16LE"));
      rc = decrypt_with_key (hmac.get_digest ());
    }

  return rc;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt entry with password
// @param password Password
// @return true if entry has been decrypted using password
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
credhist_entry::impl::decrypt_with_password (const std::string& password)
{
  bool rc = false;

  if (!is_decrypted_)
    {
      mobius::core::crypt::hash h ("sha1");
      h.update (mobius::core::conv_charset (password, "UTF-8", "UTF-16LE"));
      rc = decrypt_with_password_hash (h.get_digest ());
    }

  return rc;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// @param size Entry size in bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
credhist_entry::credhist_entry (mobius::io::reader reader, std::uint32_t size)
  : impl_ (std::make_shared <impl> (reader, size))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get revision
// @return Revision
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
credhist_entry::get_revision () const
{
  return impl_->get_revision ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get guid
// @return Guid
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
credhist_entry::get_guid () const
{
  return impl_->get_guid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get next link size
// @return Size in bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
credhist_entry::get_next_link_size () const
{
  return impl_->get_next_link_size ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get type
// @return Type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
credhist_entry::get_type () const
{
  return impl_->get_type ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash id
// @return Hash id
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
credhist_entry::get_hash_id () const
{
  return impl_->get_hash_id ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get iterations
// @return Iterations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
credhist_entry::get_iterations () const
{
  return impl_->get_iterations ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get cipher id
// @return Cipher id
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
credhist_entry::get_cipher_id () const
{
  return impl_->get_cipher_id ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get salt
// @return Salt
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
credhist_entry::get_salt () const
{
  return impl_->get_salt ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get SID
// @return SID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
credhist_entry::get_sid () const
{
  return impl_->get_sid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get cipher text
// @return Cipher text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
credhist_entry::get_cipher_text () const
{
  return impl_->get_cipher_text ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash sha1
// @return Hash sha1
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
credhist_entry::get_hash_sha1 () const
{
  return impl_->get_hash_sha1 ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash ntlm
// @return Hash ntlm
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
credhist_entry::get_hash_ntlm () const
{
  return impl_->get_hash_ntlm ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt cipher text using key
// @param key Key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
credhist_entry::decrypt_with_key (const mobius::bytearray& key)
{
  return impl_->decrypt_with_key (key);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt cipher text using password hash
// @param h Password hash
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
credhist_entry::decrypt_with_password_hash (const mobius::bytearray& h)
{
  return impl_->decrypt_with_password_hash (h);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt cipher text using password
// @param password Password encoded in UTF-8
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
credhist_entry::decrypt_with_password (const std::string& password)
{
  return impl_->decrypt_with_password (password);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if entry is decrypted
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
credhist_entry::is_decrypted () const
{
  return impl_->is_decrypted ();
}

} // namespace mobius::os::win::dpapi


