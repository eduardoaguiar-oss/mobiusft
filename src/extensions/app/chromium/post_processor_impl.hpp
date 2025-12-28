#ifndef MOBIUS_EXTENSION_APP_CHROMIUM_POST_PROCESSOR_IMPL_HPP
#define MOBIUS_EXTENSION_APP_CHROMIUM_POST_PROCESSOR_IMPL_HPP

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
#include <mobius/core/bytearray.hpp>
#include <mobius/framework/ant/post_processor_impl_base.hpp>
#include <mobius/framework/model/item.hpp>
#include <unordered_map>
#include <set>
#include <string>

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Chromium decrypt post-processor implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class post_processor_impl
    : public mobius::framework::ant::post_processor_impl_base
{
  public:
    explicit post_processor_impl (
        mobius::framework::ant::post_processor_coordinator &coordinator,
        const mobius::framework::model::item &item
    );
    void process_evidence (mobius::framework::model::evidence) final;

  private:
    // @brief Post-processor coordinator
    mobius::framework::ant::post_processor_coordinator &coordinator_;

    // @brief Case item
    mobius::framework::model::item item_;

    // @brief DPAPI encryption keys
    std::unordered_map<std::string, mobius::core::bytearray> dpapi_keys_;

    // @brief Chromium encryption keys (both v10 and v20)
    std::set<mobius::core::bytearray> chromium_keys_;

    // @brief Pending evidences to process later
    std::vector<mobius::framework::model::evidence> pending_evidences_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _on_key (
        const std::string &,
        const std::string &,
        const mobius::core::bytearray &
    );

    void _process_cookie (mobius::framework::model::evidence evidence);
    void _process_encryption_key (mobius::framework::model::evidence evidence);
    void _process_generic_evidence (mobius::framework::model::evidence evidence);

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