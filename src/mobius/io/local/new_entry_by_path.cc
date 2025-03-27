// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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
#include "new_entry_by_path.h"
#include <mobius/exception_posix.inc>
#include <mobius/io/file.h>
#include <mobius/io/folder.h>
#include <stdexcept>
#include <unistd.h>
#include <sys/stat.h>

namespace mobius::io::local
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create entry by path
// @param path Entry path
// @return Entry object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::io::entry
new_entry_by_path (const std::string& path)
{
  struct stat st;
  mobius::io::entry e;

  if (!stat (path.c_str (), &st))
    {
      if (S_ISDIR (st.st_mode))
        e = entry (mobius::io::new_folder_by_path (path));
      else
        e = entry (mobius::io::new_file_by_path (path));
    }

  else // error
    {
      if (errno != ENOENT && errno != ENOTDIR)
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
    }

  return e;
}

} // namespace mobius::io::local


