#ifndef MOBIUS_EXTENSION_OS_WIN_PROFILE_HPP
#define MOBIUS_EXTENSION_OS_WIN_PROFILE_HPP

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
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/folder.hpp>
#include <mobius/core/os/win/registry/hive_file.hpp>
#include <mobius/core/pod/map.hpp>
#include <memory>
#include <string>
#include <vector>
#include "common.hpp"

namespace mobius::extension::os::win
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Windows OS profile class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class profile
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Autofill structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct autofill
    {
        // Field name (e.g., "username", "password", "credit_card_number")
        std::string field_name;

        // Value of the autofill entry
        std::string value;

        // Metadata associated with the autofill entry
        mobius::core::pod::map metadata;

        // File object
        mobius::core::io::file f;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Opened file structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct opened_file
    {
        // Timestamp
        mobius::core::datetime::datetime timestamp;

        // Path of the opened file
        std::string path;

        // Metadata associated with the opened file
        mobius::core::pod::map metadata;

        // File object
        mobius::core::io::file f;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    profile ();
    profile (const profile &) noexcept = default;
    profile (profile &&) noexcept = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    profile &operator= (const profile &) noexcept = default;
    profile &operator= (profile &&) noexcept = default;
    operator bool () const noexcept;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_active () const;
    bool is_deleted () const;
    std::string get_username () const;
    mobius::core::io::folder get_folder () const;
    std::string get_path () const;
    mobius::core::datetime::datetime get_creation_time () const;
    mobius::core::datetime::datetime get_last_modified_time () const;
    mobius::core::pod::map get_metadata () const;
    std::vector<installed_program> get_installed_programs () const;
    std::size_t get_installed_programs_count () const;
    std::vector<autofill> get_autofill_entries () const;
    std::size_t get_autofill_entries_count () const;
    std::vector<opened_file> get_opened_files () const;
    std::size_t get_opened_files_count () const;
    void add_ntuser_dat_file (const mobius::core::io::file &);
    void add_recent_lnk_file (const mobius::core::io::file &);

  private:
    // Implementation class forward declaration
    class impl;

    // Implementation pointer
    std::shared_ptr<impl> impl_;

    // @brief Folder object
    mobius::core::io::folder folder_;

    // @brief Username
    std::string username_;

    // @brief Is valid flag
    bool is_valid_ = false;

    // @brief Is deleted flag
    bool is_deleted_ = false;

    // @brief Is active flag
    bool is_active_ = false;

    // @brief Creation time
    mobius::core::datetime::datetime creation_time_;

    // @brief Last modified time
    mobius::core::datetime::datetime last_modified_time_;

    // @brief NTUSER.DAT file
    mobius::core::io::file ntuser_dat_file_;

    // @brief Profile metadata
    mobius::core::pod::map metadata_;

    // @brief Autofill entries
    std::vector<autofill> autofill_entries_;

    // @brief Installed programs
    std::vector<installed_program> installed_programs_;

    // @brief Opened files
    std::vector<opened_file> opened_files_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _set_folder (const mobius::core::io::folder &);
    void _update_mtime (const mobius::core::io::file &);

    void _load_installed_programs (
        const mobius::core::os::win::registry::hive_file &
    );
    void _load_metadata (const mobius::core::os::win::registry::hive_file &);
    void _load_search_assist_entries (
        const mobius::core::os::win::registry::hive_file &
    );
    void _load_wordwheel_queries (
        const mobius::core::os::win::registry::hive_file &
    );
};

} // namespace mobius::extension::os::win

#endif
