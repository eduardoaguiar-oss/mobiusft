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
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/folder.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/kff/file.hpp>
#include <mobius/core/kff/kff.hpp>
#include <mobius/core/log.hpp>
#include <atomic>
#include <format>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
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
    "regular",
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
        "(1, 'regular'), "
        "(2, 'alert.csam'), "
        "(3, 'alert.malware');"
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
// @brief Check if file size exists in the database
// @param size The file size to check
// @return True if the size exists, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
has_size (uint64_t size)
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
// @brief Set file count in the metadata
// @param count The new file count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
set_file_count (std::uint64_t count)
{
    auto db = get_database ();
    auto stmt = db.new_statement ("UPDATE meta SET value = ? WHERE key = 'file_count'");
    stmt.bind (1, static_cast<int64_t> (count));
    stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set hash count in the metadata
// @param count The new hash count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
set_hash_count (std::uint64_t count)
{
    auto db = get_database ();
    auto stmt = db.new_statement ("UPDATE meta SET value = ? WHERE key = 'hash_count'");
    stmt.bind (1, static_cast<int64_t> (count));
    stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set alert count in the metadata
// @param count The new alert count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
set_alert_count (std::uint64_t count)
{
    auto db = get_database ();
    auto stmt = db.new_statement ("UPDATE meta SET value = ? WHERE key = 'alert_count'");
    stmt.bind (1, static_cast<int64_t> (count));
    stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set thumbnail count in the metadata
// @param count The new thumbnail count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
set_thumbnail_count (std::uint64_t count)
{
    auto db = get_database ();
    auto stmt = db.new_statement ("UPDATE meta SET value = ? WHERE key = 'thumbnail_count'");
    stmt.bind (1, static_cast<int64_t> (count));
    stmt.execute ();
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
new_file (std::int64_t file_id, std::uint16_t type, std::uint64_t size)
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
    stmt.bind (3, static_cast<int64_t> (size));
    stmt.execute ();
    stmt.reset ();

    return file (file_id, type, size);
}

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// KFF data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Hashsets
static std::unordered_map<std::string, hashset> hashsets_;

// @brief Hashsets mutex
static std::mutex mutex_;

// @brief Is data loaded
static std::once_flag is_loaded_;

// @brief Database schema version
static constexpr int SCHEMA_VERSION = 1;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize kff data
// Read .sqlite files from kff config dir
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_init ()
{
    mobius::core::application app;
    auto path = app.get_config_path ("kff");
    auto folder = mobius::core::io::new_folder_by_path (path);

    if (!folder.exists ())
        folder.create ();

    else
    {
        for (const auto &c : folder.get_children ())
        {
            if (c.is_file () && c.get_extension () == "sqlite")
            {
                mobius::core::io::path p (path + '/' + c.get_name ());
                auto id = p.get_filename_prefix ();
                hashsets_.emplace (id, hashset (p.get_value ()));
            }
        }
    }
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
kff::kff ()
{
    std::call_once (is_loaded_, _init);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create connection_set object to hashset databases
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::database::connection_set
kff::new_connection ()
{
    mobius::core::database::connection_set cs;

    const std::lock_guard<std::mutex> lock (mutex_);

    for (auto &p : hashsets_)
        cs.add (p.second.new_connection ());

    return cs;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create hash set
// @param id Hash set ID
// @param is_alert If hash set is alert type
// @return hash set object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hashset
kff::new_hashset (const std::string &id, bool is_alert)
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // check if hashset already exists
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    {
        const std::lock_guard<std::mutex> lock (mutex_);

        if (hashsets_.find (id) != hashsets_.end ())
            throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("hashset '" + id + "' already exists"));
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // create database
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::application app;
    auto path = app.get_config_path ("kff/" + id + ".sqlite");

    hashset h (path);
    h.create (is_alert);

    {
        const std::lock_guard<std::mutex> lock (mutex_);
        hashsets_.emplace (id, h);
    }

    return h;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove hash set
// @param id Hash set ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
kff::remove_hashset (const std::string &id)
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // remove from hashset_
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    {
        const std::lock_guard<std::mutex> lock (mutex_);
        hashsets_.erase (id);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // delete database file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::application app;
    auto path = app.get_config_path ("kff/" + id + ".sqlite");
    auto f = mobius::core::io::new_file_by_path (path);
    f.remove ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hashsets
// @return hashsets
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<std::pair<std::string, hashset>>
kff::get_hashsets () const
{
    const std::lock_guard<std::mutex> lock (mutex_);

    return std::vector<std::pair<std::string, hashset>> (hashsets_.begin (), hashsets_.end ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Lookup hash in alert hashset
// @param type Hash type
// @param value Hash value
// @return Hash sets
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<std::string>
kff::alert_lookup (const std::string &type, const std::string &value) const
{
    std::vector<std::string> hashset_ids;

    for (const auto &p : get_hashsets ())
    {
        auto id = p.first;
        auto hashset = p.second;

        if (hashset.is_alert () && hashset.lookup (type, value))
            hashset_ids.push_back (id);
    }

    return hashset_ids;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Lookup hash in all hash sets
// @param type Hash type
// @param value Hash value
// @return 'A' alert, 'I' ignored, 'N' not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
char
kff::lookup (const std::string &type, const std::string &value) const
{
    auto hashsets = get_hashsets ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // search first into is_alert hashsets
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &p : get_hashsets ())
    {
        auto id = p.first;
        auto hashset = p.second;

        if (hashset.is_alert () && hashset.lookup (type, value))
            return 'A';
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // then, search into non is_alert hashsets
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &p : get_hashsets ())
    {
        auto id = p.first;
        auto hashset = p.second;

        if (!hashset.is_alert () && hashset.lookup (type, value))
            return 'I';
    }

    return 'N';
}

} // namespace mobius::core::kff
