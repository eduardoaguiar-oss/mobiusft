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
#include <mobius/core/database/db_execute_guard.hpp>
#include <mobius/core/decoder/hexstring.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/line_reader.hpp>
#include <mobius/core/kff/file.hpp>
#include <mobius/core/kff/kff.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_view.hpp>
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
static std::atomic<bool> is_schema_created_ {false};

// @brief Flag for new KFF available
static std::atomic<bool> is_new_kff_available_ {false};

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
_create_db_schema (mobius::core::database::database &db)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (is_schema_created_.load (std::memory_order_acquire))
        return;

    auto transaction = db.new_transaction ();

    if (is_schema_created_.load (std::memory_order_acquire))
        return;

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

    // Create necessary indexes for the tables
    db.execute ("CREATE INDEX IF NOT EXISTS idx_file_size ON file (size);");
    db.execute ("CREATE INDEX IF NOT EXISTS idx_hash_file ON hash (file_id);");
    db.execute ("CREATE INDEX IF NOT EXISTS idx_attribute_file ON attribute (file_id);");

    // Commit the transaction to persist schema changes
    transaction.commit ();
    is_schema_created_.store (true, std::memory_order_release);
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
    explicit file_generator_impl (const mobius::core::database::statement &stmt)
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
        auto path = app.get_config_path ("kff.db");

        auto new_db = mobius::core::database::database (path, false);
        new_db.execute ("PRAGMA busy_timeout=60000");
        new_db.execute ("PRAGMA synchronous=NORMAL");
        new_db.execute ("PRAGMA journal_mode=WAL");
        new_db.execute ("PRAGMA foreign_keys=OFF;");

        // Create database schema if necessary
        _create_db_schema (new_db);

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
// @brief Get file by ID
// @param id The ID of the file
// @return The file object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file
get_file (std::int64_t id)
{
    mobius::core::kff::file f;

    auto db = get_database ();
    auto stmt = db.new_statement ("SELECT id, type, size FROM file WHERE id = ?");
    stmt.bind (1, id);

    if (stmt.fetch_row ())
    {
        auto id = stmt.get_column_int64 (0);
        auto type = stmt.get_column_int64 (1);
        auto size = stmt.is_column_null (2) ? -1 : stmt.get_column_int64 (2);

        f = file (id, type, size);
    }

    return f;
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Export collected data to a file
// @param path The path to the output file
// @param log Logging function or object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
export_collected_data (const std::string &path, std::function<void (const std::string &)> log)
{
    auto f_log = log ? log : [] (const std::string &) {};
    f_log (std::format ("Exporting collected data to {}", path));

    // Remove the output database file if it already exists
    auto f = mobius::core::io::new_file_by_path (path);

    if (f.exists ())
        f.remove ();

    try
    {
        auto db = get_database ();
        mobius::core::database::db_execute_guard guard (db);

        // Attach the output database
        f_log ("Attaching output database");
        guard.attach (path, "out_db");

        // Set cache and memory mapping settings for the main and out_db databases to optimize performance
        guard.set_pragma ("main", "cache_size", "-1000000");
        guard.set_pragma ("main", "mmap_size", "4294967296");
        db.execute ("PRAGMA out_db.synchronous = OFF;");
        db.execute ("PRAGMA out_db.journal_mode = OFF;");
        db.execute ("PRAGMA out_db.user_version = 1;");

        // Begin a new transaction for exporting collected data
        auto transaction = db.new_transaction ();

        // Map known sizes to all hashes of their file
        db.execute (
            "CREATE TABLE out_db.size_map ("
            "type INTEGER NOT NULL, value BLOB NOT NULL, size INTEGER NOT NULL, "
            "PRIMARY KEY (type, value)) WITHOUT ROWID;"
        );

        f_log ("Exporting known sizes");
        db.execute (
            "INSERT INTO out_db.size_map (type, value, size) "
            "SELECT h.type, h.value, f.size "
            "FROM hash h JOIN file f ON f.id = h.file_id "
            "WHERE +f.size IS NOT NULL "
            "ORDER BY h.type, h.value;"
        );

        // Map thumbnails to their corresponding hashes
        db.execute (
            "CREATE TABLE out_db.thumb_map ("
            "type INTEGER NOT NULL, value BLOB NOT NULL, data BLOB NOT NULL, "
            "PRIMARY KEY (type, value)) WITHOUT ROWID;"
        );

        f_log ("Exporting thumbnails");
        db.execute (
            "INSERT INTO out_db.thumb_map (type, value, data) "
            "SELECT h.type, h.value, t.data "
            "FROM thumbnail t JOIN hash h ON h.file_id = t.file_id "
            "ORDER BY h.type, h.value;"
        );

        // Map file attributes to their corresponding hashes
        db.execute (
            "CREATE TABLE out_db.attribute_map ("
            "type INTEGER NOT NULL, value BLOB NOT NULL, attr_type INTEGER NOT NULL, attr_value BLOB NOT NULL, "
            "PRIMARY KEY (type, value)) WITHOUT ROWID;"
        );

        f_log ("Exporting file attributes");
        db.execute (
            "INSERT INTO out_db.attribute_map (type, value, attr_type, attr_value) "
            "SELECT h.type, h.value, a.type AS attr_type, a.value AS attr_value "
            "FROM attribute a JOIN hash h ON h.file_id = a.file_id "
            "ORDER BY h.type, h.value;"
        );

        // Commit the transaction to save the exported data
        f_log ("Committing export transaction");
        transaction.commit ();

        f_log ("Export completed");
    }
    catch (const std::exception &e)
    {
        f_log (std::string ("Error exporting collected data: ") + e.what ());

        if (f.exists ())
            f.remove ();

        throw;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Export the current KFF database to a specified output file
// @param path Path to the output file
// @param log Logging function or object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
export_database (const std::string &path, std::function<void (const std::string &)> log)
{
    auto f = mobius::core::io::new_file_by_path (path);

    if (f.exists ())
        f.remove ();

    auto f_log = log ? log : [] (const std::string &) {};
    f_log (std::format ("Exporting full KFF database to {}", path));

    try
    {
        auto db = get_database ();
        auto stmt = db.new_statement ("VACUUM INTO ?");
        stmt.bind (1, path);
        stmt.execute ();
    }
    catch (...)
    {
        if (f.exists ())
            f.remove ();

        throw;
    }

    f_log ("Export completed");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Merge collected data from a specified source into the current database
// @param path Path to the input database from which to merge data
// @param callback Optional callback function for progress reporting
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
merge_collected_data (const std::string &path, std::function<void (const std::string &)> log)
{
    auto f_log = log ? log : [] (const std::string &) {};

    try
    {
        f_log ("Merging collected data from " + path);

        // Attach the input database
        auto db = get_database ();
        mobius::core::database::db_execute_guard guard (db);

        f_log ("Attaching input database");
        auto stmt = db.new_statement ("ATTACH DATABASE ? AS in_db");
        stmt.bind (1, path);
        stmt.execute ();

        guard.attach (path, "in_db");

        // Set cache and memory mapping settings for the main and out_db databases to optimize performance
        guard.set_pragma ("main", "cache_size", "-1000000");
        guard.set_pragma ("main", "mmap_size", "4294967296");
        db.execute ("PRAGMA in_db.cache_size = -1000000;");
        db.execute ("PRAGMA in_db.mmap_size = 4294967296;");

        // Start a new transaction for the import process
        auto transaction = db.new_transaction ();

        // Set missing sizes from the size_map based on hash information
        f_log ("Setting missing sizes");

        db.execute (
            "UPDATE file SET size = ("
            "  SELECT m.size "
            "  FROM hash h JOIN in_db.size_map m ON m.type = h.type AND m.value = h.value "
            "  WHERE h.file_id = file.id "
            "  LIMIT 1) "
            "WHERE size IS NULL "
            "AND EXISTS ("
            "  SELECT 1 "
            "  FROM hash h JOIN in_db.size_map m ON m.type = h.type AND m.value = h.value "
            "  WHERE h.file_id = file.id);"
        );

        f_log (std::format ("Sizes set: {}", db.get_changes ()));

        // Set thumbnails from the thumb_map based on hash information
        f_log ("Setting missing thumbnails");

        db.execute (
            "INSERT INTO thumbnail (file_id, data) "
            "SELECT h.file_id, m.data "
            "FROM in_db.thumb_map m "
            "JOIN hash h ON h.type = m.type AND h.value = m.value "
            "ORDER BY CASE m.type "
            "  WHEN 5 THEN 1 "
            "  WHEN 4 THEN 2 "
            "  WHEN 3 THEN 3 "
            "  WHEN 2 THEN 4 "
            "  WHEN 1 THEN 5 "
            "END "
            "ON CONFLICT (file_id) DO NOTHING;"
        );

        f_log (std::format ("Thumbnails set: {}", db.get_changes ()));

        db.execute (
            "INSERT INTO meta (key, value) VALUES "
            "('thumbnail_count', (SELECT COUNT(*) FROM thumbnail)) "
            "ON CONFLICT (key) DO UPDATE SET value = excluded.value;"
        );

        // Set attributes from the attribute_map based on hash information
        f_log ("Setting missing file attributes");

        db.execute (
            "INSERT INTO attribute (file_id, type, value) "
            "SELECT h.file_id, a.attr_type, a.attr_value "
            "FROM in_db.attribute_map a "
            "JOIN hash h ON h.type = a.type AND h.value = a.value "
            "ORDER BY CASE a.type "
            "  WHEN 5 THEN 1 "
            "  WHEN 4 THEN 2 "
            "  WHEN 3 THEN 3 "
            "  WHEN 2 THEN 4 "
            "  WHEN 1 THEN 5 "
            "END "
            "ON CONFLICT (file_id, type) DO NOTHING;"
        );

        f_log (std::format ("File attributes set: {}", db.get_changes ()));

        // Commit the transaction to save all changes
        f_log ("Committing the transaction to save all changes");
        transaction.commit ();

        db.execute ("PRAGMA wal_checkpoint (TRUNCATE);");

        f_log ("Import completed");
    }
    catch (const std::exception &e)
    {
        f_log (std::string ("Error importing collected data: ") + e.what ());
        throw;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Merge file sizes
// @param path The path to the size file
// @param log Logging function
//
// @details The file should contain tab-separated values with the format:
//          hash_type<TAB>hash_value<TAB>size
// e.g:
//          1<TAB>5f4dcc3b5aa765d61d8327deb882cf99<TAB>123456
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
merge_file_sizes (const std::string &path, std::function<void (const std::string &)> arg_log)
{
    auto log = arg_log ? arg_log : [] (const std::string &) {};
    log (std::format ("Merging file sizes from {}", path));

    // Open file for reading
    auto f = mobius::core::io::new_file_by_path (path);
    auto fp = mobius::core::io::line_reader (f.new_reader ());

    // Start a new transaction for the import process
    auto db = mobius::core::kff::get_database ();
    auto transaction = db.new_transaction ();

    // process each line, setting file size for KFF entries
    log (std::format ("Processing file {}", f.get_name ()));
    std::string line;
    std::uint64_t count = 0;

    while (fp.read (line))
    {
        if (!line.empty () && line[0] != '#')
        {
            auto line_view = mobius::core::string_view::rstrip (line);
            auto data = mobius::core::string_view::split (line_view, "\t");

            if (data.size () == 3)
            {
                auto hash_type = mobius::core::string_view::to_integer (data[0]);
                auto hash_value = std::string (data[1]);
                auto size = mobius::core::string_view::to_integer (data[2]);

                // lookup
                auto kff_f = mobius::core::kff::lookup (hash_type, hash_value);

                if (kff_f && !kff_f.has_size ())
                {
                    kff_f.set_size (size);
                    ++count;
                }
            }
        }
    }

    log (std::format ("Sizes set: {}", count));

    // Commit the transaction to save all changes made during the import process
    log ("Committing transaction");
    transaction.commit ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Merge passwords from a file into the KFF database
// @param path The path to the password file
// @param log Logging function
//
// @details The file should contain tab-separated values with the format:
//          hash_type<TAB>hash_value<TAB>password
// e.g:
//          1<TAB>5f4dcc3b5aa765d61d8327deb882cf99<TAB>password123
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
merge_passwords (const std::string &path, std::function<void (const std::string &)> arg_log)
{
    auto log = arg_log ? arg_log : [] (const std::string &) {};
    log (std::format ("Merging passwords from {}", path));

    // Open file for reading
    auto f = mobius::core::io::new_file_by_path (path);
    auto fp = mobius::core::io::line_reader (f.new_reader ());

    // Start a new transaction for the import process
    auto db = mobius::core::kff::get_database ();
    auto transaction = db.new_transaction ();

    // process each line, setting passwords for KFF entries
    log (std::format ("Processing file {}", f.get_name ()));
    std::string line;
    std::uint64_t count = 0;

    while (fp.read (line))
    {
        if (!line.empty () && line[0] != '#')
        {
            auto line_view = mobius::core::string_view::rstrip (line);
            auto data = mobius::core::string_view::split (line_view, "\t");

            if (data.size () == 3)
            {
                auto hash_type = mobius::core::string_view::to_integer (data[0]);
                auto hash_value = std::string (data[1]);
                auto password = std::string (data[2]);

                // lookup
                auto kff_f = mobius::core::kff::lookup (hash_type, hash_value);

                if (kff_f && !kff_f.has_password ())
                {
                    kff_f.set_password (password);
                    ++count;
                }
            }
        }
    }

    log (std::format ("Passwords set: {}", count));

    // Commit the transaction
    log ("Committing transaction");
    transaction.commit ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Populate the KFF database with IPED hashes
// @param path The path to the IPED database file
// @param arg_log A logging function to report progress and status
// @details This function imports hash information from an IPED database into the KFF database.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
populate_from_iped_hashes (const std::string &path, std::function<void (const std::string &)> arg_log)
{
    // Check if a new KFF is already available and prevent further population if so
    if (is_new_kff_available_.load (std::memory_order_acquire))
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("New KFF is already available"));

    // Set up the logging function
    auto log = arg_log ? arg_log : [] (const std::string &) {};
    log (std::format ("Populating from IPED hashes: Start importing from {}", path));

    // Attach the IPED database
    auto db = mobius::core::kff::get_database ();
    mobius::core::database::db_execute_guard guard (db);

    log ("Attaching IPED database");
    guard.attach (path, "iped");

    // Set cache and memory mapping settings for the main and IPED databases to optimize performance
    guard.set_pragma ("main", "cache_size", "-1000000");
    db.execute ("PRAGMA iped.cache_size = -1000000;");
    db.execute ("PRAGMA iped.mmap_size  = 4294967296;");

    auto transaction = db.new_transaction ();

    // Drop existing indexes and clear relevant tables before importing new data
    log ("Dropping indexes");
    db.execute ("DROP INDEX IF EXISTS idx_file_size;");
    db.execute ("DROP INDEX IF EXISTS idx_hash_file;");
    db.execute ("DROP INDEX IF EXISTS idx_attribute_file;");

    // Clear relevant tables before importing new data
    log ("Clearing old tables");
    db.execute ("DELETE FROM attribute;");
    db.execute ("DELETE FROM file;");
    db.execute ("DELETE FROM hash;");
    db.execute ("DELETE FROM thumbnail;");

    // Fill file table, setting file.type based on IPED status
    log ("Importing file table");

    db.execute (
        std::format (
            "INSERT INTO file (id, type, size) "
            "SELECT hash_id, CASE value WHEN 'pedo' THEN {} ELSE {} END, NULL "
            "  FROM iped.hashes_properties WHERE property_id = 12 "
            " ORDER BY hash_id;",
            FILE_TYPE_ALERT_CSAM,
            FILE_TYPE_IGNORED
        )
    );

    // Update file sizes based on IPED hashes properties
    log ("Updating file sizes");
    db.execute (
        "UPDATE file SET size = CAST (p.value AS INTEGER) "
        "  FROM iped.hashes_properties p "
        " WHERE p.property_id = 23 AND p.hash_id = file.id;"
    );

    // Import hash table
    log ("Importing hash table");
    db.execute (
        std::format (
            "INSERT INTO hash (type, value, file_id) "
            "SELECT type, value, file_id FROM ("
            "  SELECT {} AS type, md5 AS value, hash_id AS file_id FROM iped.hashes UNION ALL "
            "  SELECT {}, edonkey, hash_id FROM iped.hashes UNION ALL "
            "  SELECT {}, sha1,    hash_id FROM iped.hashes UNION ALL "
            "  SELECT {}, sha256,  hash_id FROM iped.hashes UNION ALL "
            "  SELECT {}, sha512,  hash_id FROM iped.hashes"
            ") "
            "WHERE value IS NOT NULL AND length (value) > 0 "
            "ORDER BY type, value "
            "ON CONFLICT (type, value) DO NOTHING;",
            HASH_TYPE_MD5,
            HASH_TYPE_ED2K,
            HASH_TYPE_SHA1,
            HASH_TYPE_SHA2_256,
            HASH_TYPE_SHA2_512
        )
    );

    // Recreate database indexes after importing new data
    log ("Recreating indexes");
    db.execute ("CREATE INDEX IF NOT EXISTS idx_file_size ON file (size);");
    db.execute ("CREATE INDEX IF NOT EXISTS idx_hash_file ON hash (file_id);");
    db.execute ("CREATE INDEX IF NOT EXISTS idx_attribute_file ON attribute (file_id);");

    // Update counters
    log ("Updating counters");
    db.execute (
        std::format (
            "INSERT INTO meta (key, value) VALUES "
            "('file_count',      (SELECT COUNT(*) FROM file)), "
            "('hash_count',      (SELECT COUNT(*) FROM hash)), "
            "('alert_count',     (SELECT COUNT(*) FROM file WHERE type >= {})), "
            "('thumbnail_count', (SELECT COUNT(*) FROM thumbnail)) "
            "ON CONFLICT (key) DO UPDATE SET value = excluded.value;",
            FILE_TYPE_ALERT_CSAM
        )
    );

    // Analyze the database to update statistics for query optimization
    log ("Analyzing database");
    db.execute ("PRAGMA analysis_limit=1000;");
    db.execute ("ANALYZE;");

    // Commit the transaction after all import and update operations
    log ("Committing transaction");
    transaction.commit ();

    db.execute ("PRAGMA wal_checkpoint (TRUNCATE);");
}

} // namespace mobius::core::kff
