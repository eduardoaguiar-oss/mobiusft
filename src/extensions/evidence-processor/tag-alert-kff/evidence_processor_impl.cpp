// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "evidence_processor_impl.hpp"
#include <mobius/core/io/uri.hpp>
#include <mobius/core/log.hpp>
#include <format>

namespace mobius::extension::evidence_processor::tag_alert_kff
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Evidence-processor implementation constructor
// @param item Case item
// @param profile Evidence-processor profile
// @param mediator Evidence-processor mediator
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
evidence_processor_impl::evidence_processor_impl (
    const mobius::framework::model::item &item,
    const mobius::framework::evidence_processor::profile &,
    const mobius::framework::evidence_processor::mediator &mediator
)
    : item_ (item),
      mediator_ (mediator)
{
    kff_connection_set_ = kff_.new_connection ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if evidence hashes match any KFF hash
// @param e Evidence to check
// @return True if any evidence hashes matches a known KFF hash, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
evidence_processor_impl::_is_kff_alert (
    mobius::framework::model::evidence e
) const
{
    auto hash_map = e.get_hashes ();

    // If the evidence has hashes as attributes, check them directly.
    if (!hash_map.empty ())
    {
        for (const auto &[hash_type, hash_value] : hash_map)
        {
            if (kff_.lookup (hash_type, hash_value) == 'A')
                return true;
        }
    }

    // @deprecated since v2.27
    // Otherwise, check if it has a deprecated attribute "hashes" with a list of hashes and check them.
    else
    {
        // Check if the evidence has attribute "hashes"
        if (!e.has_attribute ("hashes"))
            return false;

        // Check if hashes attribute is a list
        auto hashes_data = e.get_attribute ("hashes");

        if (!hashes_data.is_list ())
            return false;

        // Check if any of the hashes is a known KFF hash
        for (const auto &hash_data : hashes_data.to_list ())
        {
            if (!hash_data.is_list ())
                return false;

            auto hash_list = hash_data.to_list ();
            std::string hash_type = hash_list[0].to_string ();
            std::string hash_value = hash_list[1].to_string ();

            if (kff_.lookup (hash_type, hash_value) == 'A')
                return true;
        }
    }

    // No hashes matched
    return false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Handle evidence creation
// @param evidence Evidence to process
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::on_evidence_created (
    mobius::framework::model::evidence evidence
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        evidences_processed_++;

        if (_is_kff_alert (evidence))
        {
            evidence.set_tag ("alert");
            evidence.set_tag ("alert.kff");

            evidences_tagged_++;

            mediator_.on_evidence_tag_modified (evidence, "alert");
            mediator_.on_evidence_tag_modified (evidence, "alert.kff");
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Handle processing stop event
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_processor_impl::on_stop ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    log.info (
        __LINE__, std::format (
                      "Evidences tagged/processed: {} of {}",
                      evidences_tagged_.load (), evidences_processed_.load ()
                  )
    );
}

} // namespace mobius::extension::evidence_processor::tag_alert_kff
