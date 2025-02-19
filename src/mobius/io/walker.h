#ifndef MOBIUS_IO_WALKER_H
#define MOBIUS_IO_WALKER_H

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
#include <mobius/io/entry.h>
#include <mobius/io/file.h>
#include <mobius/io/folder.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace mobius::io
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief walker class
//! \author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class walker
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  explicit walker (const mobius::io::folder&);
  walker (walker&&) noexcept = default;
  walker (const walker&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  walker& operator= (const walker&) noexcept = default;
  walker& operator= (walker&&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Function prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void set_ignore_reallocated (bool);
  void set_case_sensitive (bool);
  std::vector <mobius::io::entry> get_entries () const;
  std::vector <mobius::io::entry> get_entries_by_name (const std::string&) const;
  std::vector <mobius::io::entry> get_entries_by_path (const std::string&) const;
  std::vector <mobius::io::entry> get_entries_by_pattern (const std::string&) const;
  std::vector <mobius::io::entry> get_entries_by_predicate (std::function <bool (const mobius::io::entry&)>) const;
  std::vector <mobius::io::entry> find_entries (std::function <bool (const mobius::io::entry&)>) const;

  std::vector <mobius::io::file> get_files () const;
  std::vector <mobius::io::file> get_files_by_name (const std::string&) const;
  std::vector <mobius::io::file> get_files_by_path (const std::string&) const;
  std::vector <mobius::io::file> get_files_by_pattern (const std::string&) const;
  std::vector <mobius::io::file> get_files_by_predicate (std::function <bool (const mobius::io::file&)>) const;
  std::vector <mobius::io::file> find_files (std::function <bool (const mobius::io::file&)>) const;

  std::vector <mobius::io::folder> get_folders () const;
  std::vector <mobius::io::folder> get_folders_by_name (const std::string&) const;
  std::vector <mobius::io::folder> get_folders_by_path (const std::string&) const;
  std::vector <mobius::io::folder> get_folders_by_pattern (const std::string&) const;
  std::vector <mobius::io::folder> get_folders_by_predicate (std::function <bool (const mobius::io::folder&)>) const;
  std::vector <mobius::io::folder> find_folders (std::function <bool (const mobius::io::folder&)>) const;

  mobius::io::file get_file_by_name (const std::string&) const;

private:
  //! \brief Implementation class forward declaration
  class impl;

  //! \brief Implementation pointer
  std::shared_ptr <impl> impl_;
};

} // namespace mobius::io

#endif
