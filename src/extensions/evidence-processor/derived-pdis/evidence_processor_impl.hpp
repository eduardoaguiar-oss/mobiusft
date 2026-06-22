#ifndef MOBIUS_EXTENSION_EVIDENCE_PROCESSOR_DERIVED_PDIS_EVIDENCE_PROCESSOR_IMPL_HPP
#define MOBIUS_EXTENSION_EVIDENCE_PROCESSOR_DERIVED_PDIS_EVIDENCE_PROCESSOR_IMPL_HPP

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
#include <mobius/framework/evidence_processor/evidence_processor_impl_base.hpp>
#include <mobius/framework/evidence_processor/mediator.hpp>
#include <mobius/framework/evidence_processor/profile.hpp>
#include <mobius/framework/model/item.hpp>
#include <atomic>

namespace mobius::extension::evidence_processor::derived_pdis
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief PDI evidence-processor implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class evidence_processor_impl
    : public mobius::framework::evidence_processor::evidence_processor_impl_base
{
  public:
    explicit evidence_processor_impl (
        const mobius::framework::model::item &,
        const mobius::framework::evidence_processor::profile &,
        const mobius::framework::evidence_processor::mediator &
    );

    void on_evidence_created (mobius::framework::model::evidence) final;
    void on_evidence_attribute_modified (
        mobius::framework::model::evidence, const std::string &
    ) final;
    void on_stop () final;

  private:
    // @brief Case item
    mobius::framework::model::item item_;

    // @brief Mediator
    mobius::framework::evidence_processor::mediator mediator_;

    // @brief Evidences processed
    std::atomic<std::uint64_t> evidences_processed_ {0};

    // @brief Evidences derived
    std::atomic<std::uint64_t> evidences_derived_ {0};

    // Helper functions
    bool _check_field_name (const std::string &, const std::string &) const;

    void _process_autofill (mobius::framework::model::evidence);
    void _process_searched_text (mobius::framework::model::evidence);
};

} // namespace mobius::extension::evidence_processor::derived_pdis

#endif