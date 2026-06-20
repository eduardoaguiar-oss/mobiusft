#ifndef MOBIUS_EXTENSION_APP_CHROMIUM_EVIDENCE_PROCESSOR_IMPL_HPP
#define MOBIUS_EXTENSION_APP_CHROMIUM_EVIDENCE_PROCESSOR_IMPL_HPP

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008-2026 Eduardo Aguiar
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
#include <mobius/framework/evidence_processor/evidence_processor_impl_base.hpp>
#include <mobius/framework/evidence_processor/mediator.hpp>
#include <mobius/framework/evidence_processor/profile.hpp>
#include <mobius/framework/model/item.hpp>
#include <unordered_map>
#include <set>
#include <string>
#include <vector>
#include "file_local_state.hpp"
#include "profile.hpp"

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Chromium <i>evidence_processor</i> implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class evidence_processor_impl
    : public mobius::framework::evidence_processor::evidence_processor_impl_base
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Datatypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    using encryption_key = file_local_state::encryption_key;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit evidence_processor_impl (
        const mobius::framework::model::item &,
        const mobius::framework::evidence_processor::profile &,
        const mobius::framework::evidence_processor::mediator &
    );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Function prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void on_folder_entered (const mobius::core::io::folder &) final;
    void on_evidence_created (mobius::framework::model::evidence) final;
    void on_evidence_loaded (mobius::framework::model::evidence) final;
    void on_complete () final;

  private:
    // @brief Case item
    mobius::framework::model::item item_;

    // @brief Mediator
    mobius::framework::evidence_processor::mediator mediator_;

    // @brief User name
    std::string username_;

    // @brief Current profile
    profile profile_;

    // @brief Profiles found
    std::vector<profile> profiles_;

    // @brief Encryption keys found
    std::vector<encryption_key> encryption_keys_;

    // @brief DPAPI encryption keys
    std::unordered_map<std::string, mobius::core::bytearray> dpapi_keys_;

    // @brief Chromium encryption keys (both v10 and v20)
    std::set<mobius::core::bytearray> chromium_keys_;

    // @brief Pending evidences to process later
    std::vector<mobius::framework::model::evidence> pending_evidences_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Scanning functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _scan_local_state (const mobius::core::io::folder &);
    void _scan_profile (const mobius::core::io::folder &);
    void _decode_local_state_file (const mobius::core::io::file &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Evidence saving functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _save_app_profiles ();
    void _save_autofills ();
    void _save_bookmarked_urls ();
    void _save_cookies ();
    void _save_credit_cards ();
    void _save_encryption_keys ();
    void _save_passwords ();
    void _save_pdis ();
    void _save_received_files ();
    void _save_user_accounts ();
    void _save_visited_urls ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Evidence processing functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _process_evidence (mobius::framework::model::evidence);
    void _process_cookie (mobius::framework::model::evidence);
    void _process_encryption_key (mobius::framework::model::evidence);
    void _process_generic_evidence (mobius::framework::model::evidence);
    void _process_password (mobius::framework::model::evidence);
    void _process_new_encryption_key (
        const std::string &,
        const std::string &,
        const mobius::core::bytearray &
    );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decryption functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::pair<bool, mobius::core::bytearray>
    _decrypt_data (const mobius::core::bytearray &) const;
    std::pair<bool, mobius::core::bytearray>
    _decrypt_dpapi_value (const mobius::core::bytearray &) const;
    std::pair<bool, mobius::core::bytearray>
    _decrypt_v_value (const mobius::core::bytearray &) const;
    mobius::core::bytearray
    _decrypt_v20_encrypted_key (const mobius::core::bytearray &) const;
};

} // namespace mobius::extension::app::chromium

#endif
