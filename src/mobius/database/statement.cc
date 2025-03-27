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
#include <mobius/database/statement.h>
#include <mobius/database/database.h>
#include <mobius/database/exception.inc>
#include <mobius/datetime/conv_iso_string.h>
#include <mobius/exception.inc>
#include <sqlite3.h>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <thread>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// I prefer to use SQLITE_TRANSIENT instead of SQLITE_STATIC on bind calls
// because when using SQLITE_STATIC in bind functions, we must keep the pointers
// passed to it valid until calling sqlite3_step. SQLITE_TRANSIENT allocates
// and copies data in bind functions.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace mobius::database
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Implementation struct
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct statement::impl
{
  database db;
  sqlite3_stmt *stmt = nullptr;

  ~impl ();
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destroy shared implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
statement::impl::~impl ()
{
  if (stmt != nullptr)
    sqlite3_finalize (stmt);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create statement object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
statement::statement ()
{
  impl_ = std::make_shared <impl> ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create statement object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
statement::statement (database db, sqlite3_stmt *stmt)
{
  impl_ = std::make_shared <impl> ();
  impl_->stmt = stmt;
  impl_->db = db;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Bind bool value
// @param idx Value index
// @param value value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
statement::bind (int idx, bool value)
{
  if (sqlite3_bind_int (impl_->stmt, idx, value ? 1 : 0) != SQLITE_OK)
    throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Bind int value
// @param idx Value index
// @param value value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
statement::bind (int idx, int value)
{
  if (sqlite3_bind_int (impl_->stmt, idx, value) != SQLITE_OK)
    throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Bind uint64_t value
// @param idx Value index
// @param value value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
statement::bind (int idx, std::int64_t value)
{
  if (sqlite3_bind_int64 (impl_->stmt, idx, value) != SQLITE_OK)
    throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Bind double value
// @param idx Value index
// @param v Value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
statement::bind (int idx, double value)
{
  if (sqlite3_bind_double (impl_->stmt, idx, value) != SQLITE_OK)
    throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Bind C string value
// @param idx Value index
// @param v Value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
statement::bind (int idx, const char *value)
{
  if (sqlite3_bind_text (impl_->stmt, idx, value, strlen (value), SQLITE_TRANSIENT) != SQLITE_OK)
    throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Bind string lvalue
// @param idx Value index
// @param v Value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
statement::bind (int idx, const std::string& value)
{
  auto rc = sqlite3_bind_text64 (impl_->stmt, idx, value.c_str (), value.length (), SQLITE_TRANSIENT, SQLITE_UTF8);

  if (rc == SQLITE_TOOBIG)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("data too big to persist"));

  else if (rc != SQLITE_OK)
    throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Bind bytearray lvalue
// @param idx Value index
// @param v Value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
statement::bind (int idx, const mobius::bytearray& value)
{
  auto rc = sqlite3_bind_blob64 (impl_->stmt, idx, value.begin (), value.size (), SQLITE_TRANSIENT);

  if (rc == SQLITE_TOOBIG)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("data too big to persist"));

  else if (rc != SQLITE_OK)
    throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Bind datetime value
// @param idx Value index
// @param v Value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
statement::bind (int idx, const mobius::datetime::datetime& value)
{
  if (value)
    {
      auto timestamp = to_string (value);

      if (sqlite3_bind_text (impl_->stmt, idx, timestamp.c_str (), timestamp.length (), SQLITE_TRANSIENT) != SQLITE_OK)
        throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);
    }

  else
    bind_null (idx);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Bind pod::data value
// @param idx Value index
// @param v Value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
statement::bind (int idx, const mobius::pod::data& value)
{
  if (value.is_null ())
    bind_null (idx);

  else
    bind (idx, mobius::pod::serialize (value));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Bind null value
// @param idx Value index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
statement::bind_null (int idx)
{
  if (sqlite3_bind_null (impl_->stmt, idx) != SQLITE_OK)
    throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Execute statement
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
statement::execute ()
{
  int rc = step ();

  if (rc == SQLITE_DONE)
    sqlite3_reset (impl_->stmt);

  else
    throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset statement
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
statement::reset ()
{
  sqlite3_reset (impl_->stmt);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Fetch one row
// @return true if a row has been found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
statement::fetch_row ()
{
  // try to fetch a row
  int rc = step ();

  // check if a row was fetched
  bool has_row = false;

  if (rc == SQLITE_DONE)
    sqlite3_reset (impl_->stmt);

  else if (rc == SQLITE_ROW)
    has_row = true;

  else
    throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);

  return has_row;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of columns returned by statement
// @return number of columns
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
statement::get_column_count ()
{
  return sqlite3_column_count (impl_->stmt);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check whether column value is null
// @param idx Value index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
statement::is_column_null (int idx)
{
  return sqlite3_column_type (impl_->stmt, idx) == SQLITE_NULL;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get bool column value
// @param idx Value index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
statement::get_column_bool (int idx)
{
  return sqlite3_column_int (impl_->stmt, idx) == 1;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get int column value
// @param idx Value index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
statement::get_column_int (int idx)
{
  return sqlite3_column_int (impl_->stmt, idx);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get int64_t column value
// @param idx Value index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int64_t
statement::get_column_int64 (int idx)
{
  return sqlite3_column_int64 (impl_->stmt, idx);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get double column value
// @param idx Value index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
double
statement::get_column_double (int idx)
{
  return sqlite3_column_double (impl_->stmt, idx);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get string column value
// @param idx Value index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
statement::get_column_string (int idx)
{
  const char *p_text = reinterpret_cast <const char *> (sqlite3_column_text (impl_->stmt, idx));
  std::string value;

  if (p_text != nullptr)
    value = std::string (p_text);

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get bytearray column value
// @param idx Value index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
statement::get_column_bytearray (int idx)
{
  mobius::bytearray value;

  auto size = sqlite3_column_bytes (impl_->stmt, idx);
  const void *p_blob = sqlite3_column_blob (impl_->stmt, idx);

  if (p_blob && size > 0)
    value = mobius::bytearray (reinterpret_cast <const uint8_t *> (p_blob), size);

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get datetime column value
// @param idx Value index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::datetime::datetime
statement::get_column_datetime (int idx)
{
  mobius::datetime::datetime value;

  const char *p_text = reinterpret_cast <const char *> (sqlite3_column_text (impl_->stmt, idx));

  if (p_text != nullptr)
    value = mobius::datetime::new_datetime_from_iso_string (p_text);

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get pod::data column value
// @param idx Value index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::pod::data
statement::get_column_pod (int idx)
{
  mobius::pod::data value;

  auto size = sqlite3_column_bytes (impl_->stmt, idx);
  const void *p_blob = sqlite3_column_blob (impl_->stmt, idx);

  if (p_blob && size > 0)
    {
      mobius::bytearray bytes (reinterpret_cast <const uint8_t *> (p_blob), size);
      value = mobius::pod::unserialize (bytes);
    }

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get database error message
// @return error message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
statement::get_error_message () const
{
  return impl_->db.get_error_message ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Execute a statement step
// @return sqlite error code
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
statement::step ()
{
  constexpr int SLEEP_TIME = 100;        // microseconds
  int rc = SQLITE_BUSY;

  while (rc == SQLITE_BUSY)
    {
      rc = sqlite3_step (impl_->stmt);

      if (rc == SQLITE_BUSY)
        std::this_thread::sleep_for (std::chrono::microseconds (SLEEP_TIME));
    }

  return rc;
}

} // namespace mobius::database


