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
#include "post_processor_impl.hpp"
#include <mobius/core/io/uri.hpp>
#include <mobius/core/log.hpp>

namespace mobius::extension::post_processor_opened_files
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
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Check if the evidence type is "visited-url"
        if (evidence.get_type () != "visited-url")
            return;

        // Check if URL scheme is "file"
        auto visited_url = evidence.get_attribute<std::string> ("url");
        auto url = mobius::core::io::uri (visited_url);

        if (url.get_scheme () != "file")
            return;

        // Create opened file evidence
        auto e = item_.new_evidence ("opened-file");
        auto metadata = evidence.get_attribute ("metadata").to_map ();

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
            "app_family", evidence.get_attribute<std::string> ("app_family")
        );

        // Set metadata
        metadata.set ("url", visited_url);
        metadata.set (
            "page_title", evidence.get_attribute<std::string> ("title")
        );
        e.set_attribute ("metadata", metadata);

        // Tags
        e.set_tag ("app.browser");

        // Add source
        e.add_source (evidence);
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::post_processor_opened_files
