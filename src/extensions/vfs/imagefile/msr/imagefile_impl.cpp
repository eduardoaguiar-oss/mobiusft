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
#include "imagefile_impl.hpp"
#include "reader_impl_aes.hpp"
#include "reader_impl_blowfish.hpp"
#include "reader_impl_plaintext.hpp"
#include <mobius/core/bytearray.hpp>
#include <mobius/core/crypt/cipher.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/io/reader.hpp>
#include <mobius/core/string_functions.hpp>
#include <stdexcept>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief header size in bytes
constexpr int HEADER_SIZE = 16384;

// @brief header signatures
const mobius::core::bytearray HEADER_SIGNATURE_V0 = {0x98, 0x92, 0x04, 0x71};
const mobius::core::bytearray HEADER_SIGNATURE_V1 = {0x12, 0xa1, 0x58, 0x32};
const mobius::core::bytearray HEADER_SIGNATURE_V2 = {0xa7, 0xb2, 0x62, 0x5a};

// @brief header encryption key
const mobius::core::bytearray HEADER_ENCRYPTION_KEY = {
    0x06, 0x42, 0x21, 0x98, 0x03, 0x69, 0x5e, 0xb1,
    0x5f, 0x40, 0x60, 0x8c, 0x2e, 0x36, 0x00, 0x06};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Read and decrypt file header
// @param reader reader
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
read_header_data (mobius::core::io::reader reader)
{
    const mobius::core::bytearray encrypted_data = reader.read (HEADER_SIZE);
    auto aes =
        mobius::core::crypt::new_cipher_cbc ("aes", HEADER_ENCRYPTION_KEY);

    return aes.decrypt (encrypted_data);
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file is an instance of imagefile msr
// @param f File object
// @return true/false
// https://pt.slideshare.net/realitynet/insecure-secret-zone
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
imagefile_impl::is_instance (const mobius::core::io::file &f)
{
    bool is_instance = false;

    if (f && f.exists ())
    {
        auto reader = f.new_reader ();

        if (reader)
        {
            const mobius::core::bytearray data = read_header_data (reader);

            is_instance =
                data && (data.slice (8192, 8195) == HEADER_SIGNATURE_V0 ||
                         data.slice (8192, 8195) == HEADER_SIGNATURE_V1 ||
                         data.slice (8192, 8195) == HEADER_SIGNATURE_V2);
        }
    }

    return is_instance;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Construct object
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile_impl::imagefile_impl (const mobius::core::io::file &f)
    : file_ (f)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get attribute
// @param name Attribute name
// @return Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
imagefile_impl::get_attribute (const std::string &name) const
{
    _load_metadata ();
    return attributes_.get (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set attribute
// @param name Attribute name
// @param value Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
imagefile_impl::set_attribute (const std::string &,
                               const mobius::core::pod::data &)
{
    throw std::runtime_error (
        MOBIUS_EXCEPTION_MSG ("set_attribute not implemented"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get attributes
// @return Attributes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::map
imagefile_impl::get_attributes () const
{
    _load_metadata ();
    return attributes_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new reader for imagefile
// @return reader
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::reader
imagefile_impl::new_reader () const
{
    _load_metadata ();

    if (encryption_algorithm_ == 0)
        return mobius::core::io::reader (
            std::make_shared<reader_impl_plaintext> (*this));

    else if (encryption_algorithm_ == 1 || encryption_algorithm_ == 2)
        return mobius::core::io::reader (
            std::make_shared<reader_impl_aes> (*this));

    else if (encryption_algorithm_ == 3)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("Blowfish-448 encryption not supported"));
    // return mobius::core::io::reader (std::make_shared <reader_impl_blowfish>
    // (*this));

    else
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("unknown/invalid encryption algorithm"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create new writer for imagefile
// @return writer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::writer
imagefile_impl::new_writer () const
{
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("writer not implemented"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load metadata
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
imagefile_impl::_load_metadata () const
{
    if (metadata_loaded_)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // parse header data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (!file_ || !file_.exists ())
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // file metadata
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime last_metadata_time =
        file_.get_metadata_time ();
    mobius::core::datetime::datetime last_modification_time =
        file_.get_modification_time ();
    mobius::core::datetime::datetime last_access_time =
        file_.get_access_time ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // header metadata
    // 0x00      uint32_t        signature
    // *0x04      uint32_t        ??
    // 0x08      uint64_t        size
    // 0x10      uint32_t        flag: password checked (0x00 = no, 0x01 = yes)
    // 0x14      s (16)          device ID
    // *0x24      s (16)          some hash
    // 0x34      uint32_t        sector size
    // 0x38      uint32_t        encryption algorithm
    // 0x3c      var             encryption key
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto reader = file_.new_reader ();
    const mobius::core::bytearray data = read_header_data (reader);
    mobius::core::decoder::data_decoder decoder (data);

    decoder.skip (8192);
    std::uint32_t signature = decoder.get_uint32_le ();
    decoder.skip (4); // unknown1
    size_ = decoder.get_uint64_le ();
    decoder.skip (4); // flag: password checked (0x00 = no, 0x01 = yes)
    std::string device_id = decoder.get_string_by_size (16);
    auto password_hash = decoder.get_bytearray_by_size (16);
    sector_size_ = decoder.get_uint32_le ();
    encryption_algorithm_ = decoder.get_uint32_le ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // version
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t version = 0;

    if (signature == 0x71049298)
    {
        version = 0;
        sector_size_ = 512;
    }

    else if (signature == 0x3258a112)
        version = 1;

    else if (signature == 0x5a62b2a7)
        version = 2;

    else
        version = 0xffffffff;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // sectors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (sector_size_ > 0)
        sectors_ = (size_ + sector_size_ - 1) / sector_size_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // encryption key
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (version == 0)
    {
        //! \todo V0 keys are hardcoded
    }

    else
    {
        if (encryption_algorithm_ == 1) // AES-128
            encryption_key_ = decoder.get_bytearray_by_size (16);

        else if (encryption_algorithm_ == 2) // AES-256
            encryption_key_ = decoder.get_bytearray_by_size (32);

        else if (encryption_algorithm_ == 3) // Blowfish-448
            encryption_key_ = decoder.get_bytearray_by_size (16);

        else if (encryption_algorithm_)
            throw std::runtime_error (
                MOBIUS_EXCEPTION_MSG ("unknown/invalid encryption algorithm"));
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // fill attributes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    attributes_.set ("version", version);
    attributes_.set ("signature", signature);
    attributes_.set ("encryption_algorithm", encryption_algorithm_);
    attributes_.set ("encryption_key", encryption_key_);
    attributes_.set ("device_id", device_id);
    attributes_.set ("last_metadata_time", last_metadata_time);
    attributes_.set ("last_modification_time", last_modification_time);
    attributes_.set ("last_access_time", last_access_time);

    metadata_loaded_ = true;
}
