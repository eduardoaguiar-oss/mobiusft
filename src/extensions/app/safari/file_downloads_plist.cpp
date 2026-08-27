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
#include "file_downloads_plist.hpp"
#include <mobius/core/decoder/plist.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>

namespace mobius::extension::app::safari
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_downloads_plist::file_downloads_plist (const mobius::core::io::reader &reader)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (!reader)
        return;

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Try to parse the Downloads.plist file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto data = mobius::core::decoder::plist (reader);

        if (!data.is_map ())
            return;

        auto download_history = data.to_map ().get ("DownloadHistory");

        if (!download_history.is_list ())
            return;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Retrieve data
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &item : download_history.to_list ())
        {
            if (item.is_map ())
            {
                auto map = item.to_map ();

                entry e;

                e.idx = entries_.size () + 1;
                e.local_path = map.get<std::string> ("DownloadEntryPath");
                e.url = map.get<std::string> ("DownloadEntryURL");
                e.file_size = map.get<std::int64_t> ("DownloadEntryProgressTotalToLoad");
                e.downloaded_bytes = map.get<std::int64_t> ("DownloadEntryProgressBytesSoFar");
                e.start_time = map.get<mobius::core::datetime::datetime> ("DownloadEntryDateAddedKey");
                e.end_time = map.get<mobius::core::datetime::datetime> ("DownloadEntryDateFinishedKey");
                e.profile_name = map.get<std::string> ("DownloadEntryProfileUUIDStringKey");
                e.identifier = map.get<std::string> ("DownloadEntryIdentifier");
                e.sandbox_identifier = map.get<std::string> ("DownloadEntrySandboxIdentifier");

                entries_.push_back (e);
            }
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Finish parsing
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        is_instance_ = true;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::safari
