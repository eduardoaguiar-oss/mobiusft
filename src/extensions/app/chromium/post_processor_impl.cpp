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
#include "post_processor_impl.hpp"
#include <mobius/core/crypt/cipher.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/os/win/dpapi/blob.hpp>
#include <string>

#include <iostream>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @see
// https://security.googleblog.com/2024/07/improving-security-of-chrome-cookies-on.html
// @see
// https://github.com/xaitax/Chrome-App-Bound-Encryption-Decryption/blob/main/docs/RESEARCH.md
// @see https://github.com/runassu/chrome_v20_decryption/tree/main
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

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

// @brief Attribute names that can be encrypted, for each evidence type
// Every attribute that has encrypted value is stored as three attributes:
// <name>, <name>_encrypted, and <name>_is_encrypted.
static const std::unordered_map<std::string, std::vector<std::string>>
    ATTRIBUTES = {
        {"autofill", {"value"}},
        {"cookie", {"value"}},
        {"credit-card", {"number", "name"}},
        {"password", {"value"}},
        {"user-account", {"password"}},
};

} // namespace

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Post-processor implementation constructor
// @param coordinator Post-processor coordinator
// @param item Case item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
post_processor_impl::post_processor_impl (
    mobius::framework::ant::post_processor_coordinator &coordinator,
    const mobius::framework::model::item &item
)
    : coordinator_ (coordinator),
      item_ (item)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Process evidence
// @param evidence Evidence to process
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
post_processor_impl::process_evidence (
    mobius::framework::model::evidence evidence
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Handle encryption keys
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto type = evidence.get_type ();

        if (type == "encryption-key")
        {
            _process_encryption_key (evidence);
            return;
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Check APP_FAMILY for Chromium artifacts
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto app_family =
            evidence.get_attribute<std::string> ("app_family", {});

        if (app_family != "chromium")
            return;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Process evidence
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto iter = ATTRIBUTES.find (type);

        if (iter == ATTRIBUTES.end ())
            return;

        auto is_modified = false;
        auto is_encrypted = false;

        for (const auto &name : iter->second)
        {
            auto is_encrypted_value =
                evidence.get_attribute<bool> (name + "_is_encrypted", false);

            if (is_encrypted_value)
            {
                auto value =
                    evidence.get_attribute<mobius::core::bytearray> (name, {});

                auto encrypted_value =
                    evidence.get_attribute<mobius::core::bytearray> (
                        name + "_encrypted", {}
                    );

                auto [rc, decrypted_value] = _decrypt_data (encrypted_value);

                if (rc)
                {
                    evidence.set_attribute (name, decrypted_value);
                    evidence.set_attribute (name + "_is_encrypted", false);
                    is_modified = true;
                }
                else
                {
                    is_encrypted = true;
                }
            }
        }

        if (is_modified)
            ; // notify coordinator

        // Store the evidence for later processing
        if (is_encrypted)
            pending_evidences_.push_back (evidence);
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
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

            auto [rc, decrypted_value] = _decrypt_dpapi_value (encrypted_value);

            if (rc)
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
// @brief Decrypt an encrypted value
// @param data Encrypted data to decrypt
// @return Decrypted data as bytearray
// @note This function checks if the value is encrypted with DPAPI or
// Chromium v10 or Chromium v20 encryption and decrypts it accordingly.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::pair<bool, mobius::core::bytearray>
post_processor_impl::_decrypt_data (const mobius::core::bytearray &data) const
{
    try
    {
        if (!data)
            return {false, {}};

        if (data.size () < 31)
        {
            if (DEBUG)
                std::cout << "Data is too short to be decrypted: "
                          << data.dump () << std::endl;
            return {false, {}};
        }

        if (data.startswith (DPAPI_SIGNATURE))
            return _decrypt_dpapi_value (data);

        else if (data.startswith ("v10") || data.startswith ("v20"))
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
                {
                    if (DEBUG)
                        std::cout
                            << version.to_string ()
                            << " data decrypted with key: " << key_value.dump ()
                            << std::endl
                            << "Plaintext: " << std::endl
                            << plaintext.dump () << std::endl;
                    return {true, plaintext};
                }
            }

            if (DEBUG && version == "v10")
            {
                std::cout << "Failed to decrypt " << version.to_string ()
                          << " data. "
                          << "IV len: " << iv.size ()
                          << ". Ciphertext size: " << ciphertext.size ()
                          << ". TAG size: " << tag.size ()
                          << ". Data:" << std::endl
                          << data.dump () << std::endl;
                std::cout << "Chromium keys available:" << std::endl;
                for (const auto &key : chromium_keys_)
                {
                    std::cout << " - " << key.dump () << std::endl;
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        mobius::core::log log (__FILE__, __func__);
        log.warning (
            __LINE__,
            "Error occurred while decrypting data: " + std::string (e.what ())
        );
    }

    return {false, {}};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decrypt DPAPI value
// @param encrypted_value Encrypted DPAPI value to decrypt
// @return Decrypted value as bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::pair<bool, mobius::core::bytearray>
post_processor_impl::_decrypt_dpapi_value (
    const mobius::core::bytearray &encrypted_value
) const
{
    if (!encrypted_value)
        return {false, {}};

    // Create DPAPI blob from encrypted value
    mobius::core::os::win::dpapi::blob blob (encrypted_value);
    auto mk_guid = blob.get_master_key_guid ();

    // Find the master key in the DPAPI keys map
    auto iter = dpapi_keys_.find (mk_guid);

    if (iter == dpapi_keys_.end ())
        return {false, {}};

    auto master_key = iter->second;

    // Decrypt the blob using the master key
    if (!blob.decrypt (master_key))
        return {false, {}};

    // If decryption was successful, get the decrypted value
    auto decrypted_value = blob.get_plain_text ();

    return {true, decrypted_value};
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
    mobius::core::log log (__FILE__, __func__);

    if (DEBUG)
        std::cout << "V20 encrypted value: " << std::endl
                  << encrypted_value.dump () << std::endl;

    if (!encrypted_value)
        return {};

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Try to decrypt encrypted value
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        log.debug (
            __LINE__, "Trying to decrypt V20 encrypted value. Size: " +
                          std::to_string (encrypted_value.size ())
        );

        auto [rc_1, decrypted_value_1] = _decrypt_dpapi_value (encrypted_value);

        if (DEBUG)
            std::cout << "Decrypted V20 value (1st attempt): " << std::endl
                      << decrypted_value_1.dump () << std::endl;

        if (!rc_1)
            return {};

        
        if (decrypted_value_1.size () == 32)    // Edge v20 key
            return decrypted_value_1;

        auto [rc_2, decrypted_value] = _decrypt_dpapi_value (decrypted_value_1);

        if (!rc_2)
            return {};

        if (DEBUG)
            std::cout << "Decrypted V20 value (2nd attempt): " << std::endl
                      << decrypted_value.dump () << std::endl;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode the decrypted value
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto decoder = mobius::core::decoder::data_decoder (decrypted_value);
        auto validation_size = decoder.get_uint32_le ();
        auto validation_data = decoder.get_bytearray_by_size (validation_size);
        auto key_size = decoder.get_uint32_le ();

        if (DEBUG)
            std::cout << "Validation data: " << std::endl
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
                    "aes", V20_PROTECTION_LEVEL_1_KEY, iv
                );
            }
            else if (protection_level == 2 || protection_level == 3)
            {
                // @todo handle protection levels 2 and 3
                log.development (
                    __LINE__,
                    "Unhandled protection level in v20 decrypted value: " +
                        std::to_string (protection_level)
                );
                return {};
            }
            else
            {
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
            auto key_data = decoder.get_bytearray_by_size (key_size);

            log.development (
                __LINE__, "Unhandled key size in v20 decrypted value: " +
                              std::to_string (key_size)
            );
            log.development (__LINE__, "Key data: " + key_data.dump ());

            if (DEBUG)
                std::cout << "Unexpected key size in v20 decrypted value: "
                          << key_size << std::endl
                          << ". Key data: " << std::endl
                          << key_data.dump ();
        }
    }
    catch (const std::exception &e)
    {
        mobius::core::log log (__FILE__, __func__);

        log.warning (
            __LINE__, "Error occurred while processing v20 decrypted value: " +
                          std::string (e.what ())
        );
    }

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
