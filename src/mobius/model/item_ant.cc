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
#include "item.h"
#include "ant.h"

namespace mobius
{
namespace model
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Check if ANT has been executed
//! \param id ANT ID
//! \return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
item::has_ant (const std::string& id) const
{
  auto db = get_database ();

  auto stmt = db.new_statement (
                "SELECT * "
                  "FROM ant "
                 "WHERE item_uid = ? "
                   "AND id = ?");

  stmt.bind (1, get_uid ());
  stmt.bind (2, id);

  return bool (stmt.fetch_row ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Set ANT
//! \param id ANT ID
//! \param name ANT name
//! \param version ANT version
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::set_ant (
  const std::string& id,
  const std::string& name,
  const std::string& version
)
{
  auto db = get_database ();
  mobius::database::statement stmt;

  if (has_ant (id))
    {
      stmt = db.new_statement (
               "UPDATE ant "
                  "SET name = ?, "
                      "version = ?, "
                      "last_execution_time = DATETIME ('now') "
                "WHERE item_uid = ? "
                  "AND id = ?");

      stmt.bind (1, name);
      stmt.bind (2, version);
      stmt.bind (3, get_uid ());
      stmt.bind (4, id);
    }

  else
    {
      stmt = db.new_statement (
               "INSERT INTO ant "
                    "VALUES (NULL, ?, ?, ?, ?, DATETIME ('now'))");

      stmt.bind (1, get_uid ());
      stmt.bind (2, id);
      stmt.bind (3, name);
      stmt.bind (4, version);
    }

  stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Reset ANT
//! \param id ANT ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::reset_ant (const std::string& id)
{
  auto db = get_database ();

  auto stmt = db.new_statement (
                "DELETE FROM ant "
                      "WHERE item_uid = ? "
                        "AND id = ?");

  stmt.bind (1, get_uid ());
  stmt.bind (2, id);
  stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get executed ANTs
//! \return Executed ANTs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <ant>
item::get_ants () const
{
  auto db = get_database ();

  auto stmt = db.new_statement (
                "SELECT uid "
                  "FROM ant "
                 "WHERE item_uid = ?");

  stmt.bind (1, get_uid ());

  std::vector <ant> ants;

  while (stmt.fetch_row ())
    {
      auto uid = stmt.get_column_int64 (0);
      ants.emplace_back (*this, uid);
    }

  return ants;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Remove ANTs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
item::remove_ants ()
{
  auto db = get_database ();

  auto stmt = db.new_statement (
                "DELETE FROM ant "
                      "WHERE item_uid = ?");

  stmt.bind (1, get_uid ());
  stmt.execute ();
}

} // namespace model
} // namespace mobius
