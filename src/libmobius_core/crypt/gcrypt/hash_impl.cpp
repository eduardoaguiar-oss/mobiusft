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
#include <mobius/core/crypt/gcrypt/hash_impl.hpp>
#include <mobius/core/crypt/gcrypt/util.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/string_functions.hpp>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Struct to hold hash algorithm data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct hash_data
{
    int algo_id;
    std::size_t block_size = 0;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Hash data map
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::unordered_map<std::string, hash_data> HASHES = {
    {"blake2b-160", {GCRY_MD_BLAKE2B_160, 128}},
    {"blake2b-256", {GCRY_MD_BLAKE2B_256, 128}},
    {"blake2b-384", {GCRY_MD_BLAKE2B_384, 128}},
    {"blake2b-512", {GCRY_MD_BLAKE2B_512, 128}},
    {"blake2s-128", {GCRY_MD_BLAKE2S_128, 64}},
    {"blake2s-160", {GCRY_MD_BLAKE2S_160, 64}},
    {"blake2s-224", {GCRY_MD_BLAKE2S_224, 64}},
    {"blake2s-256", {GCRY_MD_BLAKE2S_256, 64}},
    {"crc32", {GCRY_MD_CRC32, 0}},
    {"crc32-rfc1510", {GCRY_MD_CRC32_RFC1510, 0}},
    {"crc24-rfc2440", {GCRY_MD_CRC24_RFC2440, 0}},
    {"gostr-3411-94", {GCRY_MD_GOSTR3411_94, 32}},
    {"gostr-3411-cp", {GCRY_MD_GOSTR3411_CP, 64}},
    {"haval", {GCRY_MD_HAVAL, 64}},
    {"md2", {GCRY_MD_MD2, 16}},
    {"md4", {GCRY_MD_MD4, 64}},
    {"md5", {GCRY_MD_MD5, 64}},
    {"ripemd-160", {GCRY_MD_RMD160, 64}},
    {"sha1", {GCRY_MD_SHA1, 64}},
    {"sha2-224", {GCRY_MD_SHA224, 64}},
    {"sha2-256", {GCRY_MD_SHA256, 64}},
    {"sha2-384", {GCRY_MD_SHA384, 128}},
    {"sha2-512", {GCRY_MD_SHA512, 128}},
    {"sha2-512-224", {GCRY_MD_SHA512_224, 128}},
    {"sha2-512-256", {GCRY_MD_SHA512_256, 128}},
    {"sha3-224", {GCRY_MD_SHA3_224, 144}},
    {"sha3-256", {GCRY_MD_SHA3_256, 136}},
    {"sha3-384", {GCRY_MD_SHA3_384, 104}},
    {"sha3-512", {GCRY_MD_SHA3_512, 72}},
    {"shake-128", {GCRY_MD_SHAKE128, 168}},
    {"shake-256", {GCRY_MD_SHAKE256, 136}},
    {"sm3", {GCRY_MD_SM3, 64}},
    {"stribog-256", {GCRY_MD_STRIBOG256, 64}},
    {"stribog-512", {GCRY_MD_STRIBOG512, 64}},
    {"tiger", {GCRY_MD_TIGER, 64}},
    {"tiger1", {GCRY_MD_TIGER1, 64}},
    {"tiger2", {GCRY_MD_TIGER2, 64}},
    {"whirlpool", {GCRY_MD_WHIRLPOOL, 64}},
};

} // namespace

namespace mobius::core::crypt::gcrypt
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param algo Algorithm name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hash_impl::hash_impl (const std::string &algo)
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // initialize libgcrypt only once, at the first call of any thread
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    init ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // get hash algorithm data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto iter = HASHES.find (algo);

    if (iter == HASHES.end ())
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("invalid hash algorithm <" + algo + '>'));

    auto data = iter->second;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // check if algo is available
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto rc = gcry_md_test_algo (data.algo_id);
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc)));

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // open message digest (hash)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    rc = gcry_md_open (&md_, data.algo_id, 0);
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc)));

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // get digest size
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    digest_size_ = gcry_md_get_algo_dlen (data.algo_id);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // set metadata
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    type_ = algo;
    block_size_ = data.block_size;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hash_impl::~hash_impl ()
{
    if (md_)
        gcry_md_close (md_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update data
// @param data Data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hash_impl::update (const mobius::core::bytearray &data) noexcept
{
    gcry_md_write (md_, data.data (), data.size ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset cipher
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hash_impl::reset () noexcept
{
    gcry_md_reset (md_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clone object
// @return Pointer to newly created object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr<hash_impl_base>
hash_impl::clone () const
{
    gcry_md_hd_t new_md;

    auto rc = gcry_md_copy (&new_md, md_);
    if (rc)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (get_error_message (rc)));

    auto h = std::make_shared<hash_impl> ();

    h->md_ = new_md;
    h->type_ = type_;
    h->block_size_ = block_size_;
    h->digest_size_ = digest_size_;

    return h;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get digest
// @return Digest
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
hash_impl::get_digest ()
{
    unsigned char *digest = gcry_md_read (md_, 0);

    return mobius::core::bytearray (digest, digest_size_);
}

} // namespace mobius::core::crypt::gcrypt
