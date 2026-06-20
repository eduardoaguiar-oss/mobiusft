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
#include <mobius/framework/evidence_processor/evidence_processor_registry.hpp>
#include "evidence_processor_impl.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Extension data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C"
{
    const char *EXTENSION_ID = "app-chromium";
    const char *EXTENSION_NAME = "App Chromium";
    const char *EXTENSION_VERSION = "1.9";
    const char *EXTENSION_AUTHORS = "Eduardo Aguiar";
    const char *EXTENSION_DESCRIPTION = "Chromium based browsers support";
} // extern "C"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Start extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
start ()
{
    mobius::framework::evidence_processor::
        register_evidence_processor_implementation<
            mobius::extension::app::chromium::evidence_processor_impl> (
            EXTENSION_ID, EXTENSION_NAME
        );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Stop extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" void
stop ()
{
    mobius::framework::evidence_processor::
        unregister_evidence_processor_implementation (EXTENSION_ID);
}
