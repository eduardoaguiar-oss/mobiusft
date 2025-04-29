#ifndef MOBIUS_CORE_DATABASE_CONNECTION_POOL_HPP
#define MOBIUS_CORE_DATABASE_CONNECTION_POOL_HPP

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
#include <memory>
#include <string>
#include <mobius/core/database/connection.hpp>
#include <mobius/core/database/database.hpp>

namespace mobius::core::database
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Database connection pool
// @author Eduardo Aguiar
//
// * Rationale:
//
// 1. Open database is relatively slow (20000/s)
//
// 2. connection_pool stores one database object per thread.
//
// 3. connection_pool object has a connection object dedicated to the main
//    thread.
//
// 4. All the other threads but the main thread must call pool.acquire to
//    create a new database object. This function returns a connection object
//    that must be alive while you use the database object.
//
// 5. Successive calls to pool.get_database from the same thread are guaranteed
//    to return the same database object.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class connection_pool
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  connection_pool ();
  explicit connection_pool (const std::string&, unsigned int = 16);
  connection_pool (const connection_pool&) noexcept = default;
  connection_pool (connection_pool&&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  connection_pool& operator= (const connection_pool&) noexcept = default;
  connection_pool& operator= (connection_pool&&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void set_path (const std::string&);
  connection acquire ();
  database get_database () const;
  void release ();

private:
  // @brief Forward declaration
  class impl;

  // @brief Smart pointer to implementation class
  std::shared_ptr <impl> impl_;
};

} // namespace mobius::core::database

#endif


