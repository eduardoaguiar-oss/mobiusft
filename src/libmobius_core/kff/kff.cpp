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
#include <mobius/core/application.hpp>
#include <mobius/core/database/database.hpp>
#include <mobius/core/decoder/hexstring.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/kff/file.hpp>
#include <mobius/core/kff/kff.hpp>
#include <mobius/core/log.hpp>
#include <atomic>
#include <format>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Control variables
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Schema version
static constexpr int SCHEMA_VERSION = 1;

// @brief Flag to ensure the DB schema is created only once
static std::atomic<bool> schema_flag_ {false};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief File types
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::vector<std::string> FILE_TYPES = {
    "info",
    "ignored",
    "alert.csam",
    "alert.malware",
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Hash types
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::vector<std::string> HASH_TYPES = {
    "ed2k",
    "md5",
    "sha1",
    "sha2_256",
    "sha2_512",
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create DB tables
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_create_db_tables (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Table: meta
    db.execute (
        "CREATE TABLE IF NOT EXISTS meta "
        "(key TEXT PRIMARY KEY, value BLOB);"
    );

    db.execute (
        "INSERT OR IGNORE INTO meta (key, value) VALUES "
        "('schema_version', '1'), "
        "('creation_time', datetime ('now')), "
        "('file_count', '0'), "
        "('hash_count', '0'), "
        "('alert_count', '0'), "
        "('thumbnail_count', '0');"
    );

    // Table: file
    db.execute (
        "CREATE TABLE IF NOT EXISTS file "
        "(id INTEGER PRIMARY KEY, type INTEGER NOT NULL, size INTEGER NULL);"
    );

    // Table: hash
    db.execute (
        "CREATE TABLE IF NOT EXISTS hash "
        "(type INTEGER NOT NULL, value BLOB NOT NULL, file_id INTEGER NOT NULL, PRIMARY KEY (type, value)) WITHOUT "
        "ROWID;"
    );

    // Table: thumbnail
    db.execute (
        "CREATE TABLE IF NOT EXISTS thumbnail "
        "(file_id INTEGER PRIMARY KEY, data BLOB);"
    );

    // Table: attribute
    db.execute (
        "CREATE TABLE IF NOT EXISTS attribute "
        "(file_id INTEGER NOT NULL, type INTEGER NOT NULL, value BLOB);"
    );

    // Table: file_type
    db.execute (
        "CREATE TABLE IF NOT EXISTS file_type "
        "(id INTEGER PRIMARY KEY, name TEXT NOT NULL);"
    );

    db.execute (
        "INSERT OR REPLACE INTO file_type (id, name) VALUES "
        "(1, 'info'), "
        "(2, 'ignored'), "
        "(3, 'alert.csam'), "
        "(4, 'alert.malware');"
    );

    // Table: hash_type
    db.execute (
        "CREATE TABLE IF NOT EXISTS hash_type "
        "(id INTEGER PRIMARY KEY, name TEXT NOT NULL);"
    );

    db.execute (
        "INSERT OR REPLACE INTO hash_type (id, name) VALUES "
        "(1, 'ed2k'), "
        "(2, 'md5'), "
        "(3, 'sha1'), "
        "(4, 'sha2_256'), "
        "(5, 'sha2_512');"
    );

    // Table: attribute_type
    db.execute (
        "CREATE TABLE IF NOT EXISTS attribute_type "
        "(id INTEGER NOT NULL, name TEXT NOT NULL);"
    );

    db.execute (
        "INSERT OR REPLACE INTO attribute_type (id, name) VALUES "
        "(1, 'password');"
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create DB indexes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_create_db_indexes (mobius::core::database::database &db)
{
    db.execute ("CREATE INDEX IF NOT EXISTS idx_file_size ON file (size);");
    db.execute ("CREATE INDEX IF NOT EXISTS idx_hash_file ON hash (file_id);");
    db.execute ("CREATE INDEX IF NOT EXISTS idx_attribute_file ON attribute (file_id);");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create DB schema and constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_create_db_schema (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        db.execute ("BEGIN IMMEDIATE;");

        if (!schema_flag_.load (std::memory_order_acquire))
        {
            _create_db_tables (db);
            _create_db_indexes (db);
        }

        db.execute ("COMMIT;");
    }
    catch (const std::exception &e)
    {
        db.execute ("ROLLBACK;");
        log.error (__LINE__, std::format ("Failed to create KFF database schema: {}", e.what ()));
    }
    catch (...)
    {
        db.execute ("ROLLBACK;");
        log.error (__LINE__, "Failed to create KFF database schema");
    }
}

} // namespace

namespace mobius::core::kff
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File generator implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_generator_impl : public mobius::core::generator_impl_base<file>
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Constructor
    // @param stmt Database statement to fetch files from
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit file_generator_impl (const mobius::core::database::statement& stmt)
        : stmt_ (stmt)
    {
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Fetch next file from the database
    // @param f File object to populate
    // @return True if a file was fetched, false if no more files are available
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    next (file &f) final
    {
        if (!stmt_.fetch_row ())
            return false;

        f = file (
            stmt_.get_column_int64 (0),
            static_cast<int> (stmt_.get_column_int64 (1)),
            stmt_.is_column_null (2) ? -1 : stmt_.get_column_int64 (2)
        );

        return true;
    }

  private:
    mobius::core::database::statement stmt_;
};

static_assert (std::ranges::input_range<mobius::core::generator<file>>);

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get database instance
// @return Database object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::database::database
get_database ()
{
    thread_local mobius::core::database::database db;

    if (!db)
    {
        // Create database instance
        mobius::core::application app;
        auto path = app.get_config_path ("kff.sqlite");

        auto new_db = mobius::core::database::database (path, false);
        new_db.execute ("PRAGMA busy_timeout=60000");
        new_db.execute ("PRAGMA synchronous=NORMAL");
        new_db.execute ("PRAGMA journal_mode=WAL");
        new_db.execute ("PRAGMA foreign_keys=OFF;");

        // Create database schema if necessary
        if (!schema_flag_.load (std::memory_order_acquire))
        {
            _create_db_schema (new_db);
            schema_flag_.store (true, std::memory_order_release);
        }

        // Assign the newly created database instance to the thread-local variable
        db = new_db;
    }

    return db;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new transaction to KFF database
// @return New database transaction
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::database::transaction
new_transaction ()
{
    auto db = get_database ();

    return db.new_transaction ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get next file ID
// @return Next file ID as int64_t
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int64_t
get_next_file_id ()
{
    auto db = get_database ();

    auto stmt = db.new_statement ("SELECT COALESCE(MAX(id), 0) + 1 FROM file");
    stmt.fetch_row ();

    return stmt.get_column_int64 (0);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file size exists in the database
// @param size The file size to check
// @return True if the size exists, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
has_size (std::uint64_t size)
{
    auto db = get_database ();

    auto stmt = db.new_statement ("SELECT 1 FROM file WHERE size = ? LIMIT 1");
    stmt.bind (1, static_cast<int64_t> (size));

    return stmt.fetch_row ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file type index by name
// @param name The name of the file type
// @return The index of the file type, or an invalid index if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint16_t
get_file_type (const std::string &name)
{
    auto iter = std::find (FILE_TYPES.begin (), FILE_TYPES.end (), name);

    if (iter != FILE_TYPES.end ())
        return static_cast<std::uint16_t> (std::distance (FILE_TYPES.begin (), iter)) + 1;

    return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file type by index
// @param index The index of the file type
// @return The file type as a string, or an empty string if the index is invalid
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_file_type_string (std::uint16_t index)
{
    if (index > 0 && index <= FILE_TYPES.size ())
        return FILE_TYPES[index - 1];

    return {};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash type index by name
// @param name The name of the hash type
// @return The index of the hash type, or 0 if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint16_t
get_hash_type (const std::string &name)
{
    auto iter = std::find (HASH_TYPES.begin (), HASH_TYPES.end (), name);

    if (iter != HASH_TYPES.end ())
        return static_cast<std::uint16_t> (std::distance (HASH_TYPES.begin (), iter)) + 1;

    return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash type by index
// @param index The index of the hash type
// @return The hash type as a string, or an empty string if the index is invalid
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_hash_type_string (std::uint16_t index)
{
    if (index > 0 && index <= HASH_TYPES.size ())
        return HASH_TYPES[index - 1];

    return {};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get total file count in the database
// @return Total number of files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
get_file_count ()
{
    auto db = get_database ();
    auto stmt = db.new_statement ("SELECT value FROM meta WHERE key = 'file_count'");

    if (stmt.fetch_row ())
        return stmt.get_column_int64 (0);

    return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get total hash count in the database
// @return Total number of hashes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
get_hash_count ()
{
    auto db = get_database ();
    auto stmt = db.new_statement ("SELECT value FROM meta WHERE key = 'hash_count'");

    if (stmt.fetch_row ())
        return stmt.get_column_int64 (0);

    return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get total alert count in the database
// @return Total number of alerts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
get_alert_count ()
{
    auto db = get_database ();
    auto stmt = db.new_statement ("SELECT value FROM meta WHERE key = 'alert_count'");

    if (stmt.fetch_row ())
        return stmt.get_column_int64 (0);

    return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get total thumbnail count in the database
// @return Total number of thumbnails
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
get_thumbnail_count ()
{
    auto db = get_database ();
    auto stmt = db.new_statement ("SELECT value FROM meta WHERE key = 'thumbnail_count'");

    if (stmt.fetch_row ())
        return stmt.get_column_int64 (0);

    return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Lookup a file by its hash in the database
// @param hash_type The type of hash (e.g., HASH_TYPE_MD5, HASH_TYPE_SHA1)
// @param hash_value The hash value to look up
// @return File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file
lookup (std::uint16_t hash_type, const std::string &hash_value)
{
    auto db = get_database ();

    auto stmt = db.new_statement ("SELECT file_id FROM hash WHERE type = ? AND value = ?");
    stmt.bind (1, hash_type);
    stmt.bind (2, mobius::core::decoder::hexstring (hash_value));

    if (stmt.fetch_row ())
    {
        auto file_id = stmt.get_column_int64 (0);
        return file (file_id);
    }

    return {};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Insert a new file into the database
// @param db The database connection
// @param file_id The ID of the new file
// @param type The type of the new file
// @param size The size of the new file
// @return File object representing the newly inserted file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file
new_file (std::int64_t file_id, std::uint16_t type, std::int64_t size)
{
    thread_local mobius::core::database::statement stmt;

    if (!stmt)
    {
        auto db = mobius::core::kff::get_database ();

        stmt = db.new_statement (
            "INSERT INTO file (id, type, size) "
            "VALUES (?,?,?)"
        );
    }

    stmt.bind (1, file_id);
    stmt.bind (2, type);

    if (size == -1)
        stmt.bind_null (3);
    else
        stmt.bind (3, size);

    stmt.execute ();
    stmt.reset ();

    return file (file_id, type, size);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Iterate over all files
// @return A file_range object for iterating over all files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::generator<file>
iter_files ()
{
    auto db = get_database ();
    auto stmt = db.new_statement ("SELECT id, type, size FROM file ORDER BY id");

    return mobius::core::generator<file> (std::make_shared<file_generator_impl> (stmt));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Iterate over all files
// @param type The type of files to iterate over
// @return A file_range object for iterating over all files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::generator<file>
iter_files_by_type (std::uint16_t type)
{
    auto db = get_database ();
    auto stmt = db.new_statement ("SELECT id, type, size FROM file WHERE type = ? ORDER BY id");
    stmt.bind (1, static_cast<std::int64_t> (type));

    return mobius::core::generator<file> (std::make_shared<file_generator_impl> (stmt));
}

} // namespace mobius::core::kff
