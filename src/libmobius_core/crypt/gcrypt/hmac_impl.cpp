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
#include <mobius/core/crypt/gcrypt/hmac_impl.hpp>
#include <mobius/core/crypt/gcrypt/util.hpp>
#include <mobius/core/exception.inc>
#include <stdexcept>
#include <unordered_map>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Struct to hold HMAC algorithm data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct hmac_data
{
    int algo_id;
    std::size_t block_size = 0;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief HMAC data map
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::unordered_map<std::string, hmac_data> HMACS = {
    {"blake2b-160", {GCRY_MAC_HMAC_BLAKE2B_160, 128}},
    {"blake2b-256", {GCRY_MAC_HMAC_BLAKE2B_256, 128}},
    {"blake2b-384", {GCRY_MAC_HMAC_BLAKE2B_384, 128}},
    {"blake2b-512", {GCRY_MAC_HMAC_BLAKE2B_512, 128}},
    {"blake2s-128", {GCRY_MAC_HMAC_BLAKE2S_128, 64}},
    {"blake2s-160", {GCRY_MAC_HMAC_BLAKE2S_160, 64}},
    {"blake2s-224", {GCRY_MAC_HMAC_BLAKE2S_224, 64}},
    {"blake2s-256", {GCRY_MAC_HMAC_BLAKE2S_256, 64}},
    {"gostr-3411-94", {GCRY_MAC_HMAC_GOSTR3411_94, 32}},
    {"gostr-3411-cp", {GCRY_MAC_HMAC_GOSTR3411_CP, 64}},
    {"md4", {GCRY_MAC_HMAC_MD4, 64}},
    {"md5", {GCRY_MAC_HMAC_MD5, 64}},
    {"ripemd-160", {GCRY_MAC_HMAC_RMD160, 64}},
    {"sha1", {GCRY_MAC_HMAC_SHA1, 64}},
    {"sha2-224", {GCRY_MAC_HMAC_SHA224, 64}},
    {"sha2-256", {GCRY_MAC_HMAC_SHA256, 64}},
    {"sha2-384", {GCRY_MAC_HMAC_SHA384, 128}},
    {"sha2-512", {GCRY_MAC_HMAC_SHA512, 128}},
    {"sha2-512-224", {GCRY_MAC_HMAC_SHA512_224, 128}},
    {"sha2-512-256", {GCRY_MAC_HMAC_SHA512_256, 128}},
    {"sha3-224", {GCRY_MAC_HMAC_SHA3_224, 144}},
    {"sha3-256", {GCRY_MAC_HMAC_SHA3_256, 136}},
    {"sha3-384", {GCRY_MAC_HMAC_SHA3_384, 104}},
    {"sha3-512", {GCRY_MAC_HMAC_SHA3_512, 72}},
    {"sm3", {GCRY_MAC_HMAC_SM3, 64}},
    {"stribog-256", {GCRY_MAC_HMAC_STRIBOG256, 64}},
    {"stribog-512", {GCRY_MAC_HMAC_STRIBOG512, 64}},
    {"whirlpool", {GCRY_MAC_HMAC_WHIRLPOOL, 64}},
};

} // namespace

namespace mobius::core::crypt::gcrypt
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if algorithm is available
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_hmac_available (const std::string &type)
{
    return HMACS.find (type) != HMACS.end ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param hash_type Hash type
// @param key Key
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hmac_impl::hmac_impl (const std::string &hash_type,
                      const mobius::core::bytearray &key)
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // initialize libgcrypt only once, at the first call of any thread
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    init ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // get hash algorithm data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto iter = HMACS.find (hash_type);

    if (iter == HMACS.end ())
        throw std::invalid_argument (MOBIUS_EXCEPTION_MSG (
            "invalid hash algorithm <" + hash_type + '>'));

    auto data = iter->second;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // check if algo is available
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto rc = gcry_mac_test_algo (data.algo_id);
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc)));

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // open mac object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    rc = gcry_mac_open (&hd_, data.algo_id, 0, ctx_);
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc)));

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // set key
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    rc = gcry_mac_setkey (hd_, key.data (), key.size ());
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc)));

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // get digest size
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    digest_size_ = gcry_mac_get_algo_maclen (data.algo_id);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // set metadata
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    type_ = hash_type;
    block_size_ = data.block_size;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hmac_impl::~hmac_impl ()
{
    if (hd_)
        gcry_mac_close (hd_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset hmac value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hmac_impl::reset ()
{
    auto rc = gcry_mac_reset (hd_);
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc)));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update hmac value
// @param data Data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hmac_impl::update (const mobius::core::bytearray &data)
{
    auto rc = gcry_mac_write (hd_, data.data (), data.size ());
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc)));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash digest
// @return Hash digest
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
hmac_impl::get_digest ()
{
    mobius::core::bytearray digest (digest_size_);
    std::size_t length = digest.size ();

    auto rc = gcry_mac_read (hd_, digest.data (), &length);
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc)));

    return digest;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clone object
// @return Pointer to newly created object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr<hmac_impl_base>
hmac_impl::clone () const
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // create new object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto h = std::make_shared<hmac_impl> ();
    h->key_ = key_;
    h->type_ = type_;
    h->block_size_ = block_size_;
    h->digest_size_ = digest_size_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // open mac object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    int algo_id = gcry_mac_get_algo (hd_);

    auto rc = gcry_mac_open (&h->hd_, algo_id, 0, h->ctx_);
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc)));

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // set key
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    rc = gcry_mac_setkey (h->hd_, h->key_.data (), h->key_.size ());
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc)));

    return h;
}

} // namespace mobius::core::crypt::gcrypt
