// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
// Eduardo Aguiar
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
#include <mobius/core/exception_posix.inc>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/folder.hpp>
#include <mobius/core/io/local/new_entry_by_path.hpp>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>

namespace mobius::core::io::local
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create entry by path
// @param path Entry path
// @return Entry object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::entry
new_entry_by_path (const std::string &path)
{
    struct stat st;
    mobius::core::io::entry e;

    if (!stat (path.c_str (), &st))
    {
        if (S_ISDIR (st.st_mode))
            e = entry (mobius::core::io::new_folder_by_path (path));
        else
            e = entry (mobius::core::io::new_file_by_path (path));
    }

    else // error
    {
        if (errno != ENOENT && errno != ENOTDIR)
            throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
    }

    return e;
}

} // namespace mobius::core::io::local
