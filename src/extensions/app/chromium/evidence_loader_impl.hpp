#ifndef MOBIUS_EXTENSION_APP_CHROMIUM_EVIDENCE_LOADER_IMPL_HPP
#define MOBIUS_EXTENSION_APP_CHROMIUM_EVIDENCE_LOADER_IMPL_HPP

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
#include "profile.hpp"
#include <mobius/framework/evidence_loader_impl_base.hpp>
#include <mobius/framework/model/item.hpp>
#include <string>
#include <vector>

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>Chromium evidence_loader</i> implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class evidence_loader_impl : public mobius::framework::evidence_loader_impl_base
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Encryption Key structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct encryption_key
    {
        // @brief Key type
        std::string type;

        // @brief Key ID
        std::string id;

        // @brief Encrypted Key value
        mobius::core::bytearray value;

        // @brief Metadata
        mobius::core::pod::map metadata;

        // @brief File object
        mobius::core::io::file f;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit evidence_loader_impl (
        const mobius::framework::model::item &, scan_type
    );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Function prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void run () final;
    void scan_folder (const mobius::core::io::folder &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if object is valid
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit
    operator bool () const noexcept final
    {
        return true;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get evidence_loader type
    // @return Type as string
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_type () const final
    {
        return "app-chromium";
    }

  private:
    // @brief Case item
    mobius::framework::model::item item_;

    // @brief Scan type
    scan_type scan_type_;

    // @brief User name
    std::string username_;

    // @brief Current profile
    profile profile_;

    // @brief Profiles found
    std::vector<profile> profiles_;

    // @brief Encryption keys found
    std::vector<encryption_key> encryption_keys_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _scan_canonical_folders ();
    void _scan_canonical_root_folder (const mobius::core::io::folder &);
    void _scan_canonical_user_folder (const mobius::core::io::folder &);
    void _scan_all_folders (const mobius::core::io::folder &);
    void _scan_local_state (const mobius::core::io::folder &);
    void _scan_profile (const mobius::core::io::folder &);
    void _decode_local_state_file (const mobius::core::io::file &);

    void _save_evidences ();

    void _save_app_profiles ();
    void _save_autofills ();
    void _save_credit_cards ();
    void _save_encryption_keys ();
    void _save_pdis ();
    void _save_received_files ();
    void _save_visited_urls ();

    void _save_accounts ();
};

} // namespace mobius::extension::app::chromium

#endif
