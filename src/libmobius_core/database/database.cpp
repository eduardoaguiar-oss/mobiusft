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
#include <mobius/core/database/database.hpp>
#include <mobius/core/database/exception.inc>
#include <mobius/core/exception.inc>
#include <mobius/core/log.hpp>
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
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Initialize libsqlite3
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    sqlite3_init ()
    {
        sqlite3_initialize ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Cleanup libsqlite3
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// @brief Implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class database::impl
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors and destructor
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl () = default;
    impl (const std::string &, bool);
    ~impl ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Bool operator
    // @return true if the database path is set, false otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const
    {
        return !path_.empty ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get database file path
    // @return database file path
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_path () const
    {
        return path_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void execute (const std::string &);
    sqlite3_stmt *new_statement (const std::string &);
    bool new_transaction ();
    void end_transaction ();
    std::string get_error_message () const;
    std::int64_t get_last_insert_row_id () const;
    std::int64_t get_changes () const;
    bool has_table (const std::string &) const;
    bool table_has_column (const std::string &, const std::string &) const;

  private:
    // Database file path
    std::string path_;

    // Whether the database should be opened in serialized mode
    bool is_serialized_ = false;

    // Database handle. This is a pointer to the sqlite3 database object.
    mutable sqlite3 *db_ = nullptr;

    // Transaction level. This is used to track nested transactions.
    int transaction_level_ = 0;

    // Get database handle
    sqlite3 *_get_db () const;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create database implementation object
// @param path Database file path
// @param is_serialized Whether the database should be opened in serialized mode
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
database::impl::impl (const std::string &path, bool is_serialized)
    : path_ (path),
      is_serialized_ (is_serialized)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destroy database implementation object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
database::impl::~impl ()
{
    if (db_)
    {
        sqlite3_close (db_);
        db_ = nullptr;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get database handle
// @return sqlite3 Database handle
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
sqlite3 *
database::impl::_get_db () const
{
    // Open database if not already opened
    if (!db_)
    {
        if (path_.empty ())
            throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("null database"));

        if (sqlite3_threadsafe () == 0)
            throw std::runtime_error ("SQLite library built without thread support");

        // Open database
        int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
        flags |= is_serialized_ ? SQLITE_OPEN_FULLMUTEX : SQLITE_OPEN_NOMUTEX;

        int rc = sqlite3_open_v2 (path_.c_str (), &db_, flags, nullptr);

        if (rc != SQLITE_OK)
        {
            std::string err_msg = db_ ? sqlite3_errmsg (db_) : "Out of memory";
            sqlite3_close_v2 (db_);
            db_ = nullptr;

            throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("Failed to open database: " + err_msg));
        }
    }

    // Return database handle
    return db_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Execute SQL command
// @param sql SQL statement
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
database::impl::execute (const std::string &sql)
{
    int rc = SQLITE_BUSY;

    while (rc == SQLITE_BUSY)
    {
        auto db = _get_db ();
        rc = sqlite3_exec (db, sql.c_str (), nullptr, nullptr, nullptr);

        if (rc == SQLITE_BUSY)
            std::this_thread::sleep_for (std::chrono::microseconds (SLEEP_TIME));
    }

    if (rc != SQLITE_OK)
        throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new transaction object
// @return transaction object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
database::impl::new_transaction ()
{
    bool is_master = transaction_level_ == 0;
    ++transaction_level_;

    return is_master;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief End current transaction
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
database::impl::end_transaction ()
{
    transaction_level_--;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new statement object
// @param sql SQL command
// @return statement object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
sqlite3_stmt *
database::impl::new_statement (const std::string &sql)
{
    sqlite3_stmt *stmt = nullptr;
    auto db = _get_db ();
    int rc = SQLITE_BUSY;

    while (rc == SQLITE_BUSY)
    {
        rc = sqlite3_prepare_v2 (db, sql.c_str (), -1, &stmt, nullptr);

        if (rc == SQLITE_BUSY)
            std::this_thread::sleep_for (std::chrono::microseconds (SLEEP_TIME));
    }

    if (rc != SQLITE_OK)
    {
        mobius::core::log log (__FILE__, __FUNCTION__);
        log.warning (__LINE__, "Failed to prepare SQL statement: " + sql);

        throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);
    }

    return stmt;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last error message
// @return error message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
database::impl::get_error_message () const
{
    auto db = _get_db ();

    sqlite3_mutex *mutex = sqlite3_db_mutex (db);
    sqlite3_mutex_enter (mutex);
    std::string err_msg = sqlite3_errmsg (db);
    sqlite3_mutex_leave (mutex);

    return err_msg;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last insert row ID
// @return row ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int64_t
database::impl::get_last_insert_row_id () const
{
    return sqlite3_last_insert_rowid (_get_db ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of rows affected by last statement
// @return Number of rows affected
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int64_t
database::impl::get_changes () const
{
    return sqlite3_changes (_get_db ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if database has a given table
// @param table Table name
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
database::impl::has_table (const std::string &table) const
{
    return sqlite3_table_column_metadata (
               _get_db (),     // db
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
database::impl::table_has_column (const std::string &table, const std::string &column) const
{
    return sqlite3_table_column_metadata (
               _get_db (),      // db
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create null database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
database::database ()
    : impl_ (std::make_shared<impl> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create database object
// @param path database file path
// @param is_serialized Whether the database should be opened in serialized mode
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
database::database (const std::string &path, bool is_serialized)
    : impl_ (std::make_shared<impl> (path, is_serialized))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check whether database is not null
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
database::
operator bool () const
{
    return impl_->operator bool ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Execute SQL command
// @param sql SQL statement
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
database::execute (const std::string &sql)
{
    impl_->execute (sql);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new transaction object
// @return transaction object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
transaction
database::new_transaction ()
{
    return transaction (*this, impl_->new_transaction ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief End current transaction
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
database::end_transaction ()
{
    impl_->end_transaction ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new statement object
// @param sql SQL command
// @return statement object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
statement
database::new_statement (const std::string &sql)
{
    return statement (*this, impl_->new_statement (sql));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create SQL statement from template with dynamic column placeholders
//
// Transforms a SQL pattern containing {table.column[:value]} placeholders
// into a executable statement. Placeholders are resolved to actual column names
// if they exist in the schema, or to NULL otherwise.
//
// @param[in] pattern SQL template string with {table.column[:value]} syntax
// @return Prepared statement object
//
// @par Placeholder Syntax:
// - `{table_name[:table_alias].column_name}`: Column from named table. If
//   table alias is provided, it is used instead of the table_name.
//
// @par Example:
// @code
// auto stmt = db.new_statement_with_pattern(
//     "SELECT {users:u.id}, {users:u.name} FROM users u WHERE {users.active}=1"
// );
// @endcode
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
statement
database::new_statement_with_pattern (const std::string &pattern)
{
    std::string sql = pattern;
    std::string::size_type pos = sql.find ("{");

    while (pos != std::string::npos)
    {
        // Find the closing bracket
        auto end_pos = sql.find ("}", pos);

        if (end_pos == std::string::npos)
        {
            pos += 1; // Skip the "{" and continue
            continue;
        }

        // Get table_name, table_alias and column_name
        std::string expression = sql.substr (pos + 1, end_pos - pos - 1);
        std::string::size_type dot_pos = expression.find ('.');

        if (dot_pos == std::string::npos)
        {
            pos += 1;
            continue;
        }

        std::string column_name = expression.substr (dot_pos + 1);
        std::string table_name = expression.substr (0, dot_pos);
        std::string table_alias;

        std::string::size_type colon_pos = table_name.find (':');
        if (colon_pos != std::string::npos)
        {
            table_alias = table_name.substr (colon_pos + 1);
            table_name = table_name.substr (0, colon_pos);
        }

        // Replace expression
        std::string replacement = "NULL";

        if (table_has_column (table_name, column_name))
        {
            if (!table_alias.empty ())
                replacement = table_alias + "." + column_name;
            else
                replacement = column_name;
        }

        sql.replace (pos, end_pos - pos + 1, replacement);

        pos = sql.find ("{", pos);
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
    return impl_->get_path ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last error message
// @return error message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
database::get_error_message () const
{
    return impl_->get_error_message ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last insert row ID
// @return row ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int64_t
database::get_last_insert_row_id () const
{
    return impl_->get_last_insert_row_id ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of rows affected by last statement
// @return Number of rows affected
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int64_t
database::get_changes () const
{
    return impl_->get_changes ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if database has a given table
// @param table Table name
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
database::has_table (const std::string &table) const
{
    return impl_->has_table (table);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if table has a given column
// @param table Table name
// @param column Column name
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
database::table_has_column (const std::string &table, const std::string &column) const
{
    return impl_->table_has_column (table, column);
}

} // namespace mobius::core::database
