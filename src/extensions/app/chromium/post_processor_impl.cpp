// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "post_processor_impl.hpp"
#include <mobius/core/crypt/cipher.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/os/win/dpapi/blob.hpp>
#include <string>

#include <iostream>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @see
// https://security.googleblog.com/2024/07/improving-security-of-chrome-cookies-on.html
// @see
// https://github.com/xaitax/Chrome-App-Bound-Encryption-Decryption/blob/main/docs/RESEARCH.md
// @see https://github.com/runassu/chrome_v20_decryption/tree/main
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// @brief Debug flag
static constexpr bool DEBUG = true;

// @brief App Bound Encrypted Key Signature (v20)
static const mobius::core::bytearray APP_BOUND_SIGNATURE = "APPB";

// @brief DPAPI master key GUID
static const mobius::core::bytearray DPAPI_SIGNATURE =
    "\x01\x00\x00\x00\xd0\x8c\x9d\xdf\x01\x15\xd1\x11\x8c\x7a\x00\xc0\x4f\xc2"
    "\x97\xeb";

// @brief V20 encryption key (protection level 1)
static const mobius::core::bytearray V20_PROTECTION_LEVEL_1_KEY =
    "\xB3\x1C\x6E\x24\x1A\xC8\x46\x72\x8D\xA9\xC1\xFA\xC4\x93\x66\x51"
    "\xCF\xFB\x94\x4D\x14\x3A\xB8\x16\x27\x6B\xCC\x6D\xA0\x28\x47\x87";

// @brief V20 encryption key (protection level 2)
static const mobius::core::bytearray V20_PROTECTION_LEVEL_2_KEY =
    "\xE9\x8F\x37\xD7\xF4\xE1\xFA\x43\x3D\x19\x30\x4D\xC2\x25\x80\x42"
    "\x09\x0E\x2D\x1D\x7E\xEA\x76\x70\xD4\x1F\x73\x8D\x08\x72\x96\x60";

} // namespace

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Post-processor implementation constructor
// @param coordinator Post-processor coordinator
// @param item Case item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
post_processor_impl::post_processor_impl (
    mobius::framework::ant::post_processor_coordinator &coordinator,
    const mobius::framework::model::item &item
)
    : coordinator_ (coordinator),
      item_ (item)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Process evidence
// @param evidence Evidence to process
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
post_processor_impl::process_evidence (
    mobius::framework::model::evidence evidence
)
{
    auto type = evidence.get_type ();

    if (type == "encryption-key")
        _process_encryption_key (evidence);

    auto app_family = evidence.get_attribute<std::string> ("app_family", {});

    // if (app_family != "chromium")
    //     return;

    if (type == "autofill")
        _process_autofill (evidence);

    else if (type == "cookie")
        _process_cookie (evidence);

    else if (type == "credit-card")
        _process_credit_card (evidence);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Process autofill evidence
// @param evidence Evidence to process
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
post_processor_impl::_process_autofill (
    mobius::framework::model::evidence evidence
)
{
    auto is_encrypted = evidence.get_attribute<bool> ("is_encrypted", false);

    if (is_encrypted)
        std::cout << "Autofill evidence is encrypted. Field name: "
                  << evidence.get_attribute<std::string> ("field_name")
                  << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Process cookie evidence
// @param evidence Evidence to process
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
post_processor_impl::_process_cookie (
    mobius::framework::model::evidence evidence
)
{
    if (DEBUG)
        std::cout << "Processing cookie evidence: "
                  << evidence.get_attribute<std::string> ("name") << std::endl;

    // Check if cookie is already decrypted
    auto value = evidence.get_attribute<mobius::core::bytearray> ("value", {});

    if (value)
        return;

    // If not, check if it has an encrypted value and process it accordingly
    auto encrypted_value =
        evidence.get_attribute<mobius::core::bytearray> ("encrypted_value", {});

    if (encrypted_value)
    {
        if (DEBUG)
            std::cout << "Cookie evidence has encrypted value: " << std::endl
                      << encrypted_value.dump () << std::endl;

        // Decrypt the cookie value
        auto decrypted_value = _decrypt_data (encrypted_value);

        if (decrypted_value)
        {
            if (DEBUG)
                std::cout << "Cookie evidence decrypted successfully. "
                             "Decrypted value: "
                          << std::endl
                          << decrypted_value.dump () << std::endl;

            evidence.set_attribute ("value", decrypted_value);

            // Notify the coordinator about the decrypted cookie
            // coordinator_.notify_decrypted (evidence);
        }
        else
        {
            if (DEBUG)
                std::cout
                    << "Failed to decrypt cookie evidence. Encrypted value: "
                    << encrypted_value.dump () << std::endl;

            // Store the evidence for later processing
            pending_evidences_.push_back (evidence);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Process credit card evidence
// @param evidence Evidence to process
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
post_processor_impl::_process_credit_card (
    mobius::framework::model::evidence evidence
)
{
    auto encrypted_card_number =
        evidence.get_attribute<mobius::core::bytearray> (
            "encrypted_number",
            {}
        );
    auto encrypted_name =
        evidence.get_attribute<mobius::core::bytearray> ("encrypted_name", {});

    std::cout << "Credit card evidence found. Encrypted number: "
              << encrypted_card_number.dump () << std::endl;
    std::cout << "Encrypted cardholder name: " << encrypted_name.dump ()
              << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Process encryption key evidence
// @param evidence Evidence to process
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
post_processor_impl::_process_encryption_key (
    mobius::framework::model::evidence evidence
)
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get key attributes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto key_type = evidence.get_attribute<std::string> ("key_type");
    auto id = evidence.get_attribute<std::string> ("id");
    auto value = evidence.get_attribute<mobius::core::bytearray> ("value", {});
    auto encrypted_value =
        evidence.get_attribute<mobius::core::bytearray> ("encrypted_value", {});

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // If key value is available, store it
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (value)
    {
        _on_key (key_type, id, value);
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Handle Chromium v10 encryption keys
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (key_type == "chromium.v10")
    {
        if (encrypted_value)
        {
            if (DEBUG)
                std::cout << "Attempting to decrypt v10 key: " << std::endl
                          << encrypted_value.dump () << std::endl;

            auto decrypted_value = _decrypt_dpapi_value (encrypted_value);

            if (decrypted_value)
            {
                if (DEBUG)
                    std::cout << "v10 key decrypted: " << std::endl
                              << decrypted_value.dump () << std::endl;

                evidence.set_attribute ("value", decrypted_value);
                _on_key (key_type, id, decrypted_value);
            }

            // If decryption failed, store the evidence for later processing
            else
                pending_evidences_.push_back (evidence);
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Handle Chromium v20 encryption keys
    // @see chrome/browser/os_crypt/app_bound_encryption_provider_win.cc file
    // @see
    // https://github.com/xaitax/Chrome-App-Bound-Encryption-Decryption/blob/main/docs/RESEARCH.md
    // @see https://github.com/runassu/chrome_v20_decryption/tree/main
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    else if (key_type == "chromium.v20")
    {
        if (encrypted_value)
        {
            if (DEBUG)
                std::cout << "Attempting to decrypt v20 key: " << std::endl
                          << encrypted_value.dump () << std::endl;

            auto decrypted_value = _decrypt_v20_encrypted_key (encrypted_value);

            if (decrypted_value)
            {
                if (DEBUG)
                    std::cout << "v20 key decrypted: " << std::endl
                              << decrypted_value.dump () << std::endl;

                evidence.set_attribute ("value", decrypted_value);
                _on_key (key_type, id, decrypted_value);
            }

            // If decryption failed, store the evidence for later processing
            else
                pending_evidences_.push_back (evidence);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt DPAPI value
// @param encrypted_value Encrypted DPAPI value to decrypt
// @return Decrypted value as bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
post_processor_impl::_decrypt_dpapi_value (
    const mobius::core::bytearray &encrypted_value
) const
{
    if (!encrypted_value)
        return {};

    // Create DPAPI blob from encrypted value
    mobius::core::os::win::dpapi::blob blob (encrypted_value);
    auto mk_guid = blob.get_master_key_guid ();

    // Find the master key in the DPAPI keys map
    auto iter = dpapi_keys_.find (mk_guid);
    if (iter == dpapi_keys_.end ())
    {
        if (DEBUG)
            std::cout << "No DPAPI master key found for decryption. ID: "
                      << mk_guid << ". Blob data: " << std::endl
                      << encrypted_value.dump () << std::endl;
        return {};
    }

    auto master_key = iter->second;
    if (DEBUG)
        std::cout << "DPAPI master key found for decryption: " << std::endl
                  << master_key.dump () << std::endl;

    // Decrypt the blob using the master key
    if (!blob.decrypt (master_key))
    {
        if (DEBUG)
            std::cout << "Failed to decrypt DPAPI value with master key: "
                      << mk_guid << std::endl;
        return {};
    }

    // If decryption was successful, get the decrypted value
    auto decrypted_value = blob.get_plain_text ();
    if (DEBUG)
        std::cout << "DPAPI value decrypted successfully. Blob data: "
                  << std::endl
                  << decrypted_value.dump () << std::endl;

    return decrypted_value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt v20 encrypted key
// @param encrypted_value Encrypted key to decrypt
// @return Decrypted value as bytearray
//
// @note This function tries to decrypt the encrypted value twice, assuming
// that the encrypted value is a valid DPAPI blob.
// v20 encrypted keys are encrypted using a User DPAPI key and then
// encrypted again with a system DPAPI key.
//
// @see https://github.com/runassu/chrome_v20_decryption/tree/main for
// protection level handling
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
post_processor_impl::_decrypt_v20_encrypted_key (
    const mobius::core::bytearray &encrypted_value
) const
{
    auto decrypted_value =
        _decrypt_dpapi_value (_decrypt_dpapi_value (encrypted_value));

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode the decrypted value
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto decoder = mobius::core::decoder::data_decoder (decrypted_value);
    auto validation_size = decoder.get_uint32_le ();
    auto validation_data = decoder.get_bytearray_by_size (validation_size);
    auto key_size = decoder.get_uint32_le ();

    if (DEBUG)
        std::cout << "V20 decrypted value: " << std::endl
                  << decrypted_value.dump () << std::endl
                  << "Validation data: " << std::endl
                  << validation_data.dump () << std::endl
                  << "Key size: " << key_size << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Key has no further protection (Simply AES-GCM key)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (key_size == 32)
        return decoder.get_bytearray_by_size (key_size);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // AES-GCM Encrypted Key (protection level 1 up to 3)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    else if (key_size == 61)
    {
        auto protection_level = decoder.get_uint8 ();
        auto iv = decoder.get_bytearray_by_size (12);
        auto encrypted_key = decoder.get_bytearray_by_size (32);
        auto tag = decoder.get_bytearray_by_size (16);

        mobius::core::crypt::cipher cipher;

        if (protection_level == 1)
        {
            cipher = mobius::core::crypt::new_cipher_gcm (
                "aes",
                V20_PROTECTION_LEVEL_1_KEY,
                iv
            );
        }
        else if (protection_level == 2 || protection_level == 3)
        {
            // @todo handle protection levels 2 and 3
            mobius::core::log log (__FILE__, __func__);
            log.development (
                __LINE__,
                "Unhandled protection level in v20 decrypted value: " +
                    std::to_string (protection_level)
            );
            return {};
        }
        else
        {
            mobius::core::log log (__FILE__, __func__);
            log.development (
                __LINE__,
                "Unhandled protection level in v20 decrypted value: " +
                    std::to_string (protection_level)
            );
            return {};
        }

        return cipher.decrypt (encrypted_key);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Unhandled key size
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    else
    {
        mobius::core::log log (__FILE__, __func__);

        auto key_data = decoder.get_bytearray_by_size (key_size);

        log.development (
            __LINE__,
            "Unhandled key size in v20 decrypted value: " +
                std::to_string (key_size)
        );
        log.development (__LINE__, "Key data: " + key_data.dump ());

        if (DEBUG)
            std::cout << "Unexpected key size in v20 decrypted value: "
                      << key_size << std::endl
                      << ". Key data: " << std::endl
                      << key_data.dump ();
    }

    return {};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt an encrypted value
// @param data Encrypted data to decrypt
// @return Decrypted data as bytearray
// @note This function checks if the value is encrypted with DPAPI or
// Chromium v10 or Chromium v20 encryption and decrypts it accordingly.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
post_processor_impl::_decrypt_data (const mobius::core::bytearray &data) const
{
    if (data.size () < 31)
    {
        if (DEBUG)
            std::cout << "Data is too short to be decrypted: " << data.dump ()
                      << std::endl;
        return {};
    }

    if (data.startswith ("v10") || data.startswith ("v20"))
    {
        auto version = data.slice (0, 2);
        auto iv = data.slice (3, 14);
        auto ciphertext = data.slice (15, data.size () - 17);
        auto tag = data.slice (data.size () - 16, data.size () - 1);

        for (const auto &key_value : chromium_keys_)
        {
            auto cipher =
                mobius::core::crypt::new_cipher_gcm ("aes", key_value, iv);
            auto plaintext = cipher.decrypt (ciphertext);

            if (cipher.check_tag (tag))
                return plaintext;
        }
    }

    else if (data.startswith (DPAPI_SIGNATURE))
        return _decrypt_dpapi_value (data);

    return {};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Called when a new encryption key is stored
// @param type Type of the key (e.g., "dpapi.sys", "chromium.v10")
// @param id Identifier of the key
// @param value Value of the key
// @note This function processes pending evidences that can be decrypted with
// the newly stored key.
// It checks the type of each pending evidence and processes it accordingly.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
post_processor_impl::_on_key (
    const std::string &type,
    const std::string &id,
    const mobius::core::bytearray &value
)
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Store the key in the appropriate set based on its type
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (type == "dpapi.sys" || type == "dpapi.user")
        dpapi_keys_[id] = value;

    else if (type == "chromium.v10" || type == "chromium.v20")
        chromium_keys_.insert (value);

    else
        return;

    if (DEBUG)
        std::cout << "Encryption key stored. Type: " << type << ", ID: " << id
                  << ", Value: " << std::endl
                  << value.dump () << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Reprocess pending evidences, trying to decrypt them with the new key
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<mobius::framework::model::evidence> pending;
    std::swap (pending, pending_evidences_);

    for (auto &evidence : pending)
        process_evidence (evidence);
}

} // namespace mobius::extension::app::chromium
