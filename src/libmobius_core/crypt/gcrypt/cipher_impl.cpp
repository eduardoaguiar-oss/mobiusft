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
#include <mobius/core/crypt/gcrypt/cipher_impl.hpp>
#include <mobius/core/crypt/gcrypt/util.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/string_functions.hpp>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Map cipher alias. It contains only those IDs that differs from
// GCRYPT.name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::unordered_map<std::string, int> CIPHERS = {
    {"rc2", GCRY_CIPHER_RFC2268_40},
    {"rc2_40", GCRY_CIPHER_RFC2268_40},
    {"rc2_128", GCRY_CIPHER_RFC2268_128},
    {"rc4", GCRY_CIPHER_ARCFOUR},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Map cipher mode name to ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::unordered_map<std::string, int> MODES = {
    {"aeswrap", GCRY_CIPHER_MODE_AESWRAP},
    {"cbc", GCRY_CIPHER_MODE_CBC},
    {"cbc-cts", GCRY_CIPHER_MODE_CBC},
    {"ccm", GCRY_CIPHER_MODE_CCM},
    {"cfb", GCRY_CIPHER_MODE_CFB},
    {"cfb8", GCRY_CIPHER_MODE_CFB8},
    {"ctr", GCRY_CIPHER_MODE_CTR},
    {"eax", GCRY_CIPHER_MODE_EAX},
    {"ecb", GCRY_CIPHER_MODE_ECB},
    {"gcm", GCRY_CIPHER_MODE_GCM},
    //{"gcm-siv", GCRY_CIPHER_MODE_GCM_SIV},
    {"ofb", GCRY_CIPHER_MODE_OFB},
    {"poly1305", GCRY_CIPHER_MODE_POLY1305},
    //{"siv", GCRY_CIPHER_MODE_SIV},
    {"stream", GCRY_CIPHER_MODE_STREAM},
    {"xts", GCRY_CIPHER_MODE_XTS},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get cipher algorithm ID
// @param name Algorithm name
// @return Algo ID or 0, if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static int
_get_algo_id (const std::string &name)
{
    int algo_id = 0;

    auto iter = CIPHERS.find (name);

    if (iter == CIPHERS.end ())
        algo_id = gcry_cipher_map_name (
            mobius::core::string::toupper (name).c_str ()
        );

    else
        algo_id = iter->second;

    return algo_id;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get cipher mode ID
// @param name Mode name
// @return Mode ID or 0, if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static int
_get_mode_id (const std::string &name)
{
    int mode_id = 0;

    auto iter = MODES.find (name);

    if (iter != MODES.end ())
        mode_id = iter->second;

    return mode_id;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Format DES cipher key to 8 bytes
// @param key Key
// @return Key reformatted, if necessary
// @see
// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-samr/ebdb15df-8d0d-4347-9d62-082e6eccac40
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::bytearray
_des_key (const mobius::core::bytearray &key)
{
    mobius::core::bytearray d_key (8);

    d_key[0] = key[0] >> 1;
    d_key[1] = ((key[0] & 1) << 6) | (key[1] >> 2);
    d_key[2] = ((key[1] & 3) << 5) | (key[2] >> 3);
    d_key[3] = ((key[2] & 7) << 4) | (key[3] >> 4);
    d_key[4] = ((key[3] & 15) << 3) | (key[4] >> 5);
    d_key[5] = ((key[4] & 31) << 2) | (key[5] >> 6);
    d_key[6] = ((key[5] & 63) << 1) | (key[6] >> 7);
    d_key[7] = key[6] & 127;

    for (int i = 0; i < 8; i++)
        d_key[i] = (d_key[i] << 1) & 0xfe;

    return d_key;
}

} // namespace

namespace mobius::core::crypt::gcrypt
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param algo Algorithm name
// @param mode Mode name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
cipher_impl::cipher_impl (const std::string &algo, const std::string &mode)
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // initialize libgcrypt only once, at the first call of any thread
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    init ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // get algo ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    int algo_id = _get_algo_id (algo);

    if (!algo_id)
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("invalid cipher algorithm <" + algo + '>')
        );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // get mode ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    int mode_id = _get_mode_id (mode);

    if (!mode_id)
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("invalid cipher mode: " + mode)
        );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // get flags
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    unsigned int flags = 0;

    if (mode == "cbc-cts")
        flags |= GCRY_CIPHER_CBC_CTS;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // open cipher
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto rc = gcry_cipher_open (&hd_, algo_id, mode_id, flags);
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc))
        );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // allow weak keys
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    rc = gcry_cipher_ctl (hd_, GCRYCTL_SET_ALLOW_WEAK_KEY, NULL, 1);
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc))
        );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // set metadata
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    block_size_ = gcry_cipher_get_algo_blklen (algo_id);
    type_ = algo;
    is_stream_ = (mode_id == GCRY_CIPHER_MODE_STREAM);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
cipher_impl::~cipher_impl ()
{
    if (hd_)
        gcry_cipher_close (hd_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset cipher
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
cipher_impl::reset ()
{
    // reset cipher
    auto rc = gcry_cipher_reset (hd_);
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc))
        );

    // reset IV
    if (iv_)
        set_iv (iv_);

    // reset counter
    if (cv_)
        set_counter (cv_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set final flag on encryption/decryption
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
cipher_impl::final ()
{
    auto rc = gcry_cipher_final (hd_);
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc))
        );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Authenticate data
// @param data Data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
cipher_impl::authenticate (const mobius::core::bytearray &data)
{
    auto rc = gcry_cipher_authenticate (hd_, data.data (), data.size ());
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc))
        );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get authentication tag
// @return Tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
cipher_impl::get_tag () const
{
    mobius::core::bytearray tag (get_block_size ());

    auto rc = gcry_cipher_gettag (hd_, tag.data (), tag.size ());
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc))
        );

    return tag;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check authentication tag
// @param tag Tag to compare
// @return True if tag match
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
cipher_impl::check_tag (const mobius::core::bytearray &tag) const
{
    auto rc = gcry_cipher_checktag (hd_, tag.data (), tag.size ());

    if (rc && (gcry_err_code (rc) != GPG_ERR_CHECKSUM) &&
        (gcry_err_code (rc) != GPG_ERR_INV_LENGTH))
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc))
        );

    return rc == GPG_ERR_NO_ERROR;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set key
// @param key Key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
cipher_impl::set_key (const mobius::core::bytearray &key)
{
    mobius::core::bytearray arg_key = key;

    // reformat key, if algorithm = "des" and key length = 7
    if (type_ == "des" && key.size () == 7)
        arg_key = _des_key (key);

    // set key
    auto rc = gcry_cipher_setkey (hd_, arg_key.data (), arg_key.size ());

    if (rc && (gcry_err_code (rc) != GPG_ERR_WEAK_KEY))
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc))
        );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set initialization vector (IV)
// @param iv Initialization vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
cipher_impl::set_iv (const mobius::core::bytearray &iv)
{
    auto rc = gcry_cipher_setiv (hd_, iv.data (), iv.size ());
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc))
        );

    iv_ = iv;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set counter vector
// @param cv Counter vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
cipher_impl::set_counter (const mobius::core::bytearray &cv)
{
    auto rc = gcry_cipher_setctr (hd_, cv.data (), cv.size ());
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc))
        );

    cv_ = cv;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Encrypt data
// @param data Data to be encrypted
// @return Encrypted data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
cipher_impl::encrypt (const mobius::core::bytearray &data)
{
    mobius::core::bytearray out (data.size ());

    auto rc = gcry_cipher_encrypt (
        hd_, out.data (), out.size (), data.data (), data.size ()
    );
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc))
        );

    return out;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt data
// @param data Data to be decrypted
// @return Decrypted data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
cipher_impl::decrypt (const mobius::core::bytearray &data)
{
    mobius::core::bytearray out (data.size ());

    auto rc = gcry_cipher_decrypt (
        hd_, out.data (), out.size (), data.data (), data.size ()
    );
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc))
        );

    return out;
}

} // namespace mobius::core::crypt::gcrypt
