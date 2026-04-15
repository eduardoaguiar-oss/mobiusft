#ifndef MOBIUS_EXTENSION_OS_WIN_OS_HPP
#define MOBIUS_EXTENSION_OS_WIN_OS_HPP

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
#include <mobius/core/io/file.hpp>
#include <vector>

namespace mobius::extension::os::win
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Windows OS extension class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class os
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    os () = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void add_ntuser_dat_file (const mobius::core::io::file &);
    void add_system_file (const mobius::core::io::file &);
    void add_sam_file (const mobius::core::io::file &);
    void add_security_file (const mobius::core::io::file &);
    void add_software_file (const mobius::core::io::file &);

  private:
    // @brief SYSTEM file
    mobius::core::io::file system_file_;

    // @brief SAM file
    mobius::core::io::file sam_file_;

    // @brief SECURITY file
    mobius::core::io::file security_file_;

    // @brief SOFTWARE file
    mobius::core::io::file software_file_;

    // @brief NTUSER.DAT files
    std::vector<mobius::core::io::file> ntuser_dat_files_;
};

} // namespace mobius::extension::os::win

#endif // MOBIUS_EXTENSION_OS_WIN_OS_HPP