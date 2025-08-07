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
#include <mobius/core/log.hpp>
#include <mobius/core/os/win/dpapi/blob.hpp>
#include <string>

#include <iostream>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @see
// https://security.googleblog.com/2024/07/improving-security-of-chrome-cookies-on.html
// @see
// https://github.com/xaitax/Chrome-App-Bound-Encryption-Decryption/blob/main/docs/RESEARCH.md
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// @brief App Bound Encrypted Key Signature (v20)
static const mobius::core::bytearray APP_BOUND_SIGNATURE = "APPB";

// @brief Debug flag
static constexpr bool DEBUG = true;

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
    // std::cout << "Processing evidence: " << evidence.get_uid () << std::endl;

    auto type = evidence.get_type ();

    if (type == "autofill")
        _process_autofill (evidence);

    else if (type == "cookie")
        _process_cookie (evidence);

    else if (type == "credit-card")
        _process_credit_card (evidence);

    else if (type == "encryption-key")
        _process_encryption_key (evidence);
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
    auto app_family = evidence.get_attribute<std::string> ("app_family", {});
    if (app_family != "chromium")
        return;

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
    auto app_family = evidence.get_attribute<std::string> ("app_family", {});
    if (app_family != "chromium")
        return;

    std::cout << "cookie evidence found. Name: "
              << evidence.get_attribute<std::string> ("name") << std::endl;
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
    auto app_family = evidence.get_attribute<std::string> ("app_family", {});
    if (app_family != "chromium")
        return;

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
    // Get key attributes
    auto key_type = evidence.get_attribute<std::string> ("key_type");
    auto id = evidence.get_attribute<std::string> ("id");
    auto value = evidence.get_attribute<mobius::core::bytearray> ("value");
    auto encrypted_value =
        evidence.get_attribute<mobius::core::bytearray> ("encrypted_value");

    // Store decrypted keys for later use
    if (value)
    {
        std::cout << "Decrypted key found. Type: " << key_type << ", ID: " << id
                  << ", Value: " << value.dump () << std::endl;

        if (key_type == "dpapi.sys" || key_type == "dpapi.user")
            dpapi_keys_[id] = value;

        else if (key_type == "chromium.v10")
            chromium_v10_keys_.insert (value);

        else if (key_type == "chromium.v20")
            chromium_v20_keys_.insert (value);

        return;
    }

    if (key_type == "chromium.v10")
    {
        // Try to decrypt the key using DPAPI keys
        if (encrypted_value)
        {
            if (DEBUG)
                std::cout << "Attempting to decrypt v10 key: "
                          << encrypted_value.dump () << std::endl;

            auto decrypted_value = _decrypt_dpapi_value (
                evidence.get_attribute<mobius::core::bytearray> (
                    "encrypted_value"
                )
            );
            if (decrypted_value)
            {
                std::cout << "Decrypted v10 key: " << decrypted_value.dump ()
                          << std::endl;
                // chromium_v10_keys_.insert (decrypted_value);
                return;
            }
        }
    }

    // Try to decrypt the key
    else if (key_type == "chromium.v20")
    {
        auto decrypted_value = _decrypt_v20_encrypted_key (encrypted_value);
        if (decrypted_value)
        {
            std::cout << "Decrypted v20 key: " << decrypted_value.dump ()
                      << std::endl;
            // chromium_v20_keys_.insert (decrypted_value);
            return;
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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
post_processor_impl::_decrypt_v20_encrypted_key (
    const mobius::core::bytearray &encrypted_value
) const
{
    return _decrypt_dpapi_value (_decrypt_dpapi_value (encrypted_value));
}

} // namespace mobius::extension::app::chromium
