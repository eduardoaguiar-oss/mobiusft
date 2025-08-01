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
const mobius::core::bytearray APP_BOUND_SIGNATURE = "APPB";

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
    std::cout << "Processing evidence: " << evidence.get_uid () << std::endl;

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

    // Store decrypted keys for later use
    if (value)
    {
        if (key_type == "dpapi.sys" || key_type == "dpapi.user")
            dpapi_keys_[id] = value;

        else if (key_type == "chromium.v10")
            chromium_v10_keys_.insert (value);

        else if (key_type == "chromium.v20")
            chromium_v20_keys_.insert (value);

        return;
    }

    // Try to decrypt the key
    if (key_type == "chromium.v20")
    {
        auto encrypted_value =
            evidence.get_attribute<mobius::core::bytearray> ("encrypted_value");

        if (encrypted_value.startswith (APP_BOUND_SIGNATURE))
        {
            mobius::core::os::win::dpapi::blob blob (
                encrypted_value.slice (4, encrypted_value.size ())
            );

            auto metadata =
                evidence.get_attribute<mobius::core::pod::map> ("metadata");
            metadata.set ("dpapi_revision", blob.get_revision ());
            metadata.set ("dpapi_provider_guid", blob.get_provider_guid ());
            metadata.set (
                "dpapi_master_key_revision",
                blob.get_master_key_revision ()
            );
            metadata.set ("dpapi_master_key_guid", blob.get_master_key_guid ());
            metadata.set ("dpapi_description", blob.get_description ());
            evidence.set_attribute ("metadata", metadata);
            
            std::cout << "Chromium v20 key is encrypted. ID: " << id
                      << std::endl;
            std::cout << encrypted_value.dump () << std::endl;
        }
    }
}

} // namespace mobius::extension::app::chromium
