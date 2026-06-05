#ifndef MOBIUS_FRAMEWORK_EVIDENCE_PROCESSOR_EVIDENCE_PROCESSOR_REGISTRY_HPP
#define MOBIUS_FRAMEWORK_EVIDENCE_PROCESSOR_EVIDENCE_PROCESSOR_REGISTRY_HPP

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
#include <mobius/framework/evidence_processor/engine.hpp>
#include <mobius/framework/evidence_processor/evidence_processor_impl_base.hpp>
#include <mobius/framework/evidence_processor/mediator.hpp>
#include <mobius/framework/evidence_processor/profile.hpp>
#include <mobius/framework/model/item.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace mobius::framework::evidence_processor
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Evidence processor implementation functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
using evidence_processor_implementation_builder =
    std::function<std::shared_ptr<evidence_processor_impl_base> (
        const mobius::framework::model::item &,
        const profile &,
        const mediator &
    )>;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Data structure to hold evidence processor implementation data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct evidence_processor_implementation_data
{
    // @brief Unique identifier for the processor
    std::string id;

    // @brief Name of the processor
    std::string name;

    // @brief Function to create an instance of the processor
    mobius::framework::evidence_processor::
        evidence_processor_implementation_builder factory;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Evidence processor implementation prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void register_evidence_processor_implementation (
    const std::string &,
    const std::string &,
    evidence_processor_implementation_builder
);

void unregister_evidence_processor_implementation (const std::string &);

std::optional<evidence_processor_implementation_data>
get_evidence_processor_implementation (const std::string &);

std::vector<evidence_processor_implementation_data>
list_evidence_processor_implementations ();

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Register a evidence processor implementation
// @param id Unique identifier for the processor
// @param name Name of the processor
// @tparam T Type of the processor implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T>
void
register_evidence_processor_implementation (
    const std::string &id, const std::string &name
)
{
    register_evidence_processor_implementation (
        id, name,
        [] (const mobius::framework::model::item &item, const profile &prof,
            const mediator &med)
        { return std::make_shared<T> (item, prof, med); }
    );
}

} // namespace mobius::framework::evidence_processor

#endif