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
#include <chrono>
#include <sqlite3.h>
#include <stdexcept>
#include <thread>
#include <unordered_set>

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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get column_name or NULL according to the expression and the version given
// @param exp Expression in the format column_name:v1[-v2|*][,v3[-v4|*]]
// @param version Schema version
// @return Column_name or NULL if version match expression
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
get_column (const std::string &exp, std::int64_t version)
{
    // Split column_name:start_version-end_version
    std::string column_name;
    std::string version_exp;
    std::string::size_type colon_pos = exp.find (':');

    if (colon_pos != std::string::npos)
    {
        column_name = exp.substr (0, colon_pos);
        version_exp = exp.substr (colon_pos + 1);
    }

    else
        return exp;

    // Test versions in expression
    while (!version_exp.empty ())
    {
        std::string::size_type comma_pos = version_exp.find (',');
        std::string range_exp;

        if (comma_pos != std::string::npos)
        {
            range_exp = version_exp.substr (0, comma_pos);
            version_exp.erase (0, comma_pos);
        }

        else
        {
            range_exp = version_exp;
            version_exp.clear ();
        }

        // Parse start_version if provided
        int64_t start_version = -1;
        int64_t end_version = std::numeric_limits<int64_t>::max ();

        std::string::size_type dash_pos = range_exp.find ('-');

        if (dash_pos != std::string::npos)
        {
            try
            {
                start_version = std::stoll (range_exp.substr (0, dash_pos));

                auto end_version_str = range_exp.substr (dash_pos + 1);

                if (end_version_str == "*")
                    end_version = std::numeric_limits<int64_t>::max ();

                else
                    end_version = std::stoll (end_version_str);
            }
            catch (...)
            {
                // Keep default if parsing fails
            }
        }
        else
        {
            try
            {
                start_version = std::stoll (range_exp);
                end_version = start_version;
            }
            catch (...)
            {
                // Keep default if parsing fails
            }
        }

        if (version >= start_version && version <= end_version)
            return column_name;
    }

    return "NULL";
}

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
// @param pattern The SQL pattern string with ${column:start_version-end_version} placeholders
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

        // Replace the expression with the column name or empty string
        std::string expression = sql.substr (pos + 2, end_pos - pos - 2);
        std::string replacement = get_column (expression, schema_version);
        sql.replace (pos, end_pos - pos + 1, replacement);

        // Continue searching from current position
        // No need to advance pos since the replacement might be shorter than
        // the placeholder
    }

    return new_statement (sql);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new select statement object
// @param table_name Table name
// @param columns Vector of column names
// @return statement object
// Generate a SELECT statement for the given table and columns, checking if
// columns exist in the table. If not, use NULL as the column in the SELECT statement.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
statement
database::new_select_statement (
    const std::string &table_name, const std::vector<std::string> &columns
)
{
    // Query table columns via PRAGMA table_info and cache them
    std::unordered_set<std::string> existing_columns;
    sqlite3_stmt *pragma_stmt = nullptr;
    std::string pragma_sql = "PRAGMA table_info('" + table_name + "')";

    int rc = SQLITE_BUSY;
    while (rc == SQLITE_BUSY)
    {
        rc = sqlite3_prepare_v2 (
            impl_->db, pragma_sql.c_str (), -1, &pragma_stmt, nullptr
        );

        if (rc == SQLITE_BUSY)
            std::this_thread::sleep_for (
                std::chrono::microseconds (SLEEP_TIME)
            );
    }

    if (rc != SQLITE_OK)
    {
        if (pragma_stmt)
            sqlite3_finalize (pragma_stmt);
        throw std::runtime_error (MOBIUS_EXCEPTION_SQLITE);
    }

    while (sqlite3_step (pragma_stmt) == SQLITE_ROW)
    {
        const unsigned char *name =
            sqlite3_column_text (pragma_stmt, 1); // name is column index 1
        if (name)
            existing_columns.emplace (reinterpret_cast<const char *> (name));
    }

    sqlite3_finalize (pragma_stmt);

    // Local checker that uses cached pragma results (shadows the member function)
    auto has_column = [&] (const std::string &, const std::string &col) -> bool
    {
        return std::find (
                   existing_columns.begin (), existing_columns.end (), col
               ) != existing_columns.end ();
    };

    std::string sql = "SELECT ";
    bool first = true;

    for (const auto &column : columns)
    {
        std::string column_name;

        if (has_column (table_name, column))
            column_name = column;
        else
            column_name = "NULL";

        if (!first)
            sql += ", ";

        sql += column_name;
        first = false;
    }
    sql += " FROM " + table_name;

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
