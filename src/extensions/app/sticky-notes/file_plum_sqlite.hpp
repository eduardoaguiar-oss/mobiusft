#ifndef MOBIUS_EXTENSION_APP_STICKY_NOTES_FILE_PLUM_SQLITE_HPP
#define MOBIUS_EXTENSION_APP_STICKY_NOTES_FILE_PLUM_SQLITE_HPP

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

namespace mobius::extension::app::sticky_notes
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Plum.sqlite file decoder
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_plum_sqlite
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Note structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct note
    {
        // @brief Record index number
        std::uint64_t idx = 0;

        // @brief Change key
        std::string change_key;

        // @brief Created at
        mobius::core::datetime::datetime created_at;

        // @brief Creation note id anchor
        std::string creation_note_id_anchor;

        // @brief Deleted at
        mobius::core::datetime::datetime deleted_at;

        // @brief Id
        std::string id;

        // @brief Is always on top
        bool is_always_on_top = false;

        // @brief Is future note
        bool is_future_note = false;

        // @brief Is open
        bool is_open = false;

        // @brief Is remote data invalid
        bool is_remote_data_invalid = false;

        // @brief Last server version
        std::string last_server_version;

        // @brief Parent ID
        std::string parent_id;

        // @brief Pending insights scan
        std::int64_t pending_insights_scan;

        // @brief Remote ID
        std::string remote_id;

        // @brief Remote schema version
        std::int64_t remote_schema_version;

        // @brief Text
        std::string text;

        // @brief Theme
        std::string theme;

        // @brief Type
        std::string type;

        // @brief Updated at
        mobius::core::datetime::datetime updated_at;

        // @brief Window position
        std::string window_position;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_plum_sqlite (const mobius::core::io::reader &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if stream is an instance of plum.sqlite file
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const noexcept
    {
        return is_instance_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get notes
    // @return Vector of notes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<note>
    get_notes () const
    {
        return notes_;
    }

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

    // @brief Notes vector
    std::vector<note> notes_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_notes (mobius::core::database::database &);
};

} // namespace mobius::extension::app::sticky_notes

#endif
