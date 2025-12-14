#ifndef MOBIUS_CORE_DATABASE_STATEMENT_HPP
#define MOBIUS_CORE_DATABASE_STATEMENT_HPP

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
#include <mobius/core/bytearray.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/pod/data.hpp>
#include <memory>
#include <string>

struct sqlite3_stmt;

namespace mobius::core::database
{
class database;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief SQL statement class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class statement
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  statement ();
  statement (database, sqlite3_stmt *);
  statement (const statement&) noexcept = default;
  statement (statement&&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  statement& operator= (const statement&) noexcept = default;
  statement& operator= (statement&&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void bind (int, bool);
  void bind (int, int);
  void bind (int, std::int64_t);
  void bind (int, double);
  void bind (int, const char *);
  void bind (int, const std::string&);
  void bind (int, const mobius::core::bytearray&);
  void bind (int, const mobius::core::datetime::datetime&);
  void bind (int, const mobius::core::pod::data&);
  void bind_null (int);

  void execute ();
  void reset ();
  bool fetch_row ();

  int get_column_count ();
  bool is_column_null (int);

  bool get_column_bool (int);
  int get_column_int (int);
  std::int64_t get_column_int64 (int);
  double get_column_double (int);
  std::string get_column_string (int);
  mobius::core::bytearray get_column_bytearray (int);
  mobius::core::datetime::datetime get_column_datetime (int);
  mobius::core::pod::data get_column_pod (int);

private:
  struct impl;
  std::shared_ptr <impl> impl_;

  // helper function
  std::string get_error_message () const;
  int step ();
};

} // namespace mobius::core::database

#endif


