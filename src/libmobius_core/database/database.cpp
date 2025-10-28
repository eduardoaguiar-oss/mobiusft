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
    sqlite3_init ()
    {
        sqlite3_initialize ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Cleanup libsqlite3
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    ~sqlite3_init ()
    {
        sqlite3_shutdown ();
    }
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
    // Database handle. This is a pointer to the sqlite3 database object.
    sqlite3 *db = nullptr;

    // Transaction level. This is used to track nested transactions.
    int transaction_level = 0;

    // Database file path
    std::string path;

    // Flag to indicate if the database is null
    bool is_null = true;

    // Destructor
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
        db = nullptr;
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

    impl_->path = path;
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
// @brief Generate new statement with version-aware SQL column replacements
// @param pattern The SQL patternstring with ${column:start_version-end_version} placeholders
// @param schema_version The current schema version to check against
// @return statement object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
statement
database::new_statement (const std::string &pattern, int64_t schema_version)
{
    std::string sql = pattern;
    std::string::size_type pos = 0;

    while ((pos = sql.find ("${", pos)) != std::string::npos)
    {
        // Find the closing bracket
        auto end_pos = sql.find ("}", pos);
        if (end_pos == std::string::npos)
        {
            pos += 2; // Skip the "${" and continue
            continue;
        }

        // Extract the placeholder content
        std::string placeholder = sql.substr (pos + 2, end_pos - pos - 2);

        // Split column_name:start_version-end_version
        std::string column_name;
        std::string::size_type colon_pos = placeholder.find (':');

        if (colon_pos != std::string::npos)
        {
            column_name = placeholder.substr (0, colon_pos);
            placeholder = placeholder.substr (colon_pos + 1);
        }

        else
        {
            column_name = placeholder;
            placeholder.clear ();
        }

        // Get version ranges
        int64_t start_version = -1;
        int64_t end_version = std::numeric_limits<int64_t>::max ();

        // Parse start_version if provided
        std::string::size_type dash_pos = placeholder.find ('-');
        if (dash_pos != std::string::npos)
        {
            try
            {
                start_version = std::stoll (placeholder.substr (0, dash_pos));
            }
            catch (...)
            {
                // Keep default if parsing fails
            }

            try
            {
                end_version = std::stoll (placeholder.substr (dash_pos + 1));
            }
            catch (...)
            {
                // Keep default if parsing fails
            }
        }
        else if (!placeholder.empty ())
        {
            try
            {
                start_version = std::stoll (placeholder);
            }
            catch (...)
            {
                // Keep default if parsing fails
            }
        }

        // Check if current schema version is within range
        std::string replacement = "NULL";

        if (schema_version >= start_version && schema_version <= end_version)
            replacement = column_name;

        // Replace the placeholder with the column name or empty string
        sql.replace (pos, end_pos - pos + 1, replacement);

        // Continue searching from current position
        // No need to advance pos since the replacement might be shorter than
        // the placeholder
    }

    return new_statement (sql);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get database file path
// @return database file path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
database::get_path () const
{
    return impl_->path;
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
