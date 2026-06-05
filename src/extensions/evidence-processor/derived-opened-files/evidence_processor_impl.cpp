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

namespace mobius::extension::evidence_processor::derived_opened_files
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

        // Check if the evidence type is "visited-url"
        if (evidence.get_type () != "visited-url")
            return;

        // Check if URL scheme is "file"
        auto visited_url = evidence.get_attribute<std::string> ("url");
        auto url = mobius::core::io::uri (visited_url);

        if (url.get_scheme () != "file")
            return;

        // Create opened file
        auto metadata = evidence.get_attribute ("metadata").to_map ();

        auto e = item_.new_evidence ("opened-file");
        e.set_attribute ("path", url.get_path ());
        e.set_attribute (
            "timestamp",
            evidence.get_attribute<mobius::core::datetime::datetime> (
                "timestamp"
            )
        );
        e.set_attribute (
            "username", evidence.get_attribute<std::string> ("username")
        );
        e.set_attribute ("app_id", metadata.get<std::string> ("app_id"));
        e.set_attribute ("app_name", metadata.get<std::string> ("app_name"));
        e.set_attribute (
            "app_family", metadata.get<std::string> ("app_family")
        );

        // Set metadata
        mobius::core::pod::map e_metadata = metadata.clone ();
        e_metadata.set ("url", visited_url);
        e_metadata.set (
            "page_title", evidence.get_attribute<std::string> ("title")
        );

        e.set_attribute ("metadata", e_metadata);

        // Tags
        e.set_tag ("app.browser");

        // Add source
        e.add_source (evidence);

        // Update evidences derived count
        evidences_derived_++;

        // Tell mediator about new evidence
        mediator_.on_evidence_created (e);
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
                      "Evidences derived/processed: {} of {}",
                      evidences_derived_.load (), evidences_processed_.load ()
                  )
    );
}

} // namespace mobius::extension::evidence_processor::derived_opened_files
