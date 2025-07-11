// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025
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
#include <mobius/core/database/database.hpp>
#include <mobius/core/database/exception.inc>
#include <mobius/core/exception.inc>
#include <chrono>
#include <sqlite3.h>
#include <stdexcept>
#include <thread>

namespace
{
static constexpr int SLEEP_TIME = 200; // microseconds

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief libsqlite3 initialization class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class sqlite3_init
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Initialize libsqlite3
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    sqlite3_init () { sqlite3_initialize (); }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Cleanup libsqlite3
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    ~sqlite3_init () { sqlite3_shutdown (); }
};

static sqlite3_init sqlite3_instance;

} // namespace

namespace mobius::core::database
{

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Implementation struct
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct database_impl
{
    sqlite3 *db = nullptr;
    int transaction_level = 0;
    bool is_null = true;

    ~database_impl ();
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destroy shared implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
database_impl::~database_impl ()
{
    if (db != nullptr)
    {
        sqlite3_close (db);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create null database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
database::database ()
    : impl_ (std::make_shared<database_impl> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create database object
// @param path database file path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
database::database (const std::string &path)
    : impl_ (std::make_shared<database_impl> ())
{
    // define flags, including multi-thread support
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

    if (sqlite3_threadsafe () != 0)
        flags |= SQLITE_OPEN_NOMUTEX;

    // open database
    int rc = sqlite3_open_v2 (path.c_str (), &(impl_->db), flags, nullptr);

    if (rc != SQLITE_OK)
        throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);

    impl_->is_null = false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check whether database is not null
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
database::
operator bool () const
{
    return !impl_->is_null;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Execute SQL command
// @param sql SQL statement
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
database::execute (const std::string &sql)
{
    int rc = SQLITE_BUSY;

    while (rc == SQLITE_BUSY)
    {
        rc = sqlite3_exec (impl_->db, sql.c_str (), nullptr, nullptr, nullptr);

        if (rc == SQLITE_BUSY)
            std::this_thread::sleep_for (
                std::chrono::microseconds (SLEEP_TIME)
            );
    }

    if (rc != SQLITE_OK)
        throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new transaction object
// @return transaction object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
transaction
database::new_transaction ()
{
    bool is_master = impl_->transaction_level++ == 0;

    return transaction (*this, is_master);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief End current transaction
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
database::end_transaction ()
{
    impl_->transaction_level--;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new statement object
// @param sql SQL command
// @return statement object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
statement
database::new_statement (const std::string &sql)
{
    sqlite3_stmt *stmt = nullptr;
    int rc = SQLITE_BUSY;

    while (rc == SQLITE_BUSY)
    {
        rc = sqlite3_prepare_v2 (impl_->db, sql.c_str (), -1, &stmt, nullptr);

        if (rc == SQLITE_BUSY)
            std::this_thread::sleep_for (
                std::chrono::microseconds (SLEEP_TIME)
            );
    }

    if (rc != SQLITE_OK)
        throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);

    return statement (*this, stmt);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last error message
// @return error message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
database::get_error_message () const
{
    return sqlite3_errmsg (impl_->db);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last insert row ID
// @return row ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int64_t
database::get_last_insert_row_id () const
{
    return sqlite3_last_insert_rowid (impl_->db);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of rows affected by last statement
// @return Number of rows affected
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int64_t
database::get_changes () const
{
    return sqlite3_changes (impl_->db);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if database has a given table
// @param table Table name
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
database::has_table (const std::string &table) const
{
    return sqlite3_table_column_metadata (
               impl_->db,      // db
               nullptr,        // zDbName
               table.c_str (), // zTableName
               nullptr,        // zColumnName
               nullptr,        // pzDataType
               nullptr,        // pzCollSeq
               nullptr,        // pNotNull
               nullptr,        // pPrimaryKey
               nullptr         // pAutoinc
           ) == SQLITE_OK;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if table has a given column
// @param table Table name
// @param column Column name
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
database::table_has_column (
    const std::string &table, const std::string &column
) const
{
    return sqlite3_table_column_metadata (
               impl_->db,       // db
               nullptr,         // zDbName
               table.c_str (),  // zTableName
               column.c_str (), // zColumnName
               nullptr,         // pzDataType
               nullptr,         // pzCollSeq
               nullptr,         // pNotNull
               nullptr,         // pPrimaryKey
               nullptr          // pAutoinc
           ) == SQLITE_OK;
}

} // namespace mobius::core::database
