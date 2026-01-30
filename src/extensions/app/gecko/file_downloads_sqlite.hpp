#ifndef MOBIUS_EXTENSION_APP_GECKO_FILE_DOWNLOADS_SQLITE_HPP
#define MOBIUS_EXTENSION_APP_GECKO_FILE_DOWNLOADS_SQLITE_HPP

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
#include <mobius/core/database/database.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/reader.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace mobius::extension::app::gecko
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Downloads.sqlite file decoder
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_downloads_sqlite
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Download structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct download
    {
        // @brief Record index number
        std::uint64_t idx = 0;

        // @brief Autoresume
        std::int64_t auto_resume;

        // @brief Currbytes
        std::int64_t curr_bytes;

        // @brief Endtime
        mobius::core::datetime::datetime end_time;

        // @brief Entityid
        std::string entity_id;

        // @brief Guid
        std::string guid;

        // @brief Id
        std::int64_t id;

        // @brief Maxbytes
        std::int64_t max_bytes;

        // @brief Mimetype
        std::string mime_type;

        // @brief Name
        std::string name;

        // @brief Preferredaction
        std::int64_t preferred_action;

        // @brief Preferredapplication
        std::string preferred_application;

        // @brief Referrer
        std::string referrer;

        // @brief Source
        std::string source;

        // @brief Starttime
        mobius::core::datetime::datetime start_time;

        // @brief State
        std::int64_t state;

        // @brief Target
        std::string target;

        // @brief Temppath
        std::string temp_path;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_downloads_sqlite (const mobius::core::io::reader &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if stream is an instance of downloads.sqlite file
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const noexcept
    {
        return is_instance_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get downloads
    // @return Vector of downloads
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<download>
    get_downloads () const
    {
        return downloads_;
    }

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

    // @brief Downloads vector
    std::vector<download> downloads_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_downloads (mobius::core::database::database &);
};

} // namespace mobius::extension::app::gecko

#endif
