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
#include "CThumbCache.hpp"
#include <mobius/core/database/database.hpp>
#include <mobius/io/tempfile.h>
#include <mobius/string_functions.h>

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
CThumbCache::CThumbCache (const mobius::io::reader& reader)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Copy reader content to temporary file
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::io::tempfile tfile;
  tfile.copy_from (reader);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Read cache from db file
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::core::database::database db (tfile.get_path ());

  auto stmt = db.new_statement (
                "SELECT Filename, FileSize, LastWriteTime, Image "
                  "FROM Files");

  while (stmt.fetch_row ())
    {
      entry e;

      e.is_valid = true;
      e.path = stmt.get_column_string (0);
      e.size = stmt.get_column_int64 (1);
      e.last_write_time = mobius::datetime::new_datetime_from_nt_timestamp (stmt.get_column_int64 (2));
      e.image_data = stmt.get_column_bytearray (3);

      cache_[e.path] = e;
    }

  is_instance_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get cache entry
// @param Path
// @return Cache entry, if any
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::optional <CThumbCache::entry>
CThumbCache::get (const std::string& path) const
{
  std::optional <entry> data;

  auto iter = cache_.find (mobius::string::tolower (path));

  if (iter != cache_.end ())
    data = iter->second;

  return data;
}

} // namespace mobius::extension::app::shareaza




