#ifndef MOBIUS_EXTENSION_APP_INTERNET_EXPLORER_PROFILE_HPP
#define MOBIUS_EXTENSION_APP_INTERNET_EXPLORER_PROFILE_HPP

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
#include <mobius/core/pod/map.hpp>
#include <memory>
#include <string>
#include <vector>

namespace mobius::extension::app::internet_explorer
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Internet Explorer profile class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class profile
{
  public:
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
    std::string get_username () const;
    mobius::core::io::folder get_folder () const;
    std::string get_path () const;
    mobius::core::datetime::datetime get_creation_time () const;
    mobius::core::datetime::datetime get_last_modified_time () const;
    bool has_data () const;

    void add_ntuser_dat_file (const mobius::core::io::file &);
    void add_index_dat_file (const mobius::core::io::file &);
    void add_webcachev01_dat_file (const mobius::core::io::file &);

  private:
    // Implementation class forward declaration
    class impl;

    // Implementation pointer
    std::shared_ptr <impl> impl_;
};

} // namespace mobius::extension::app::internet_explorer

#endif
