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
#include <mobius/core/decoder/hexstring.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/kff/file.hpp>
#include <mobius/core/kff/kff.hpp>

namespace mobius::core::kff
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Implementation class for file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file::impl
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Construct an implementation object with ID, type, and size
    // @param id The file ID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl (int64_t id)
        : id_ (id)
    {
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Construct an implementation object with ID, type, and size
    // @param id The file ID
    // @param type The type of the file
    // @param size The size of the file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl (int64_t id, std::uint16_t type, std::uint64_t size)
        : id_ (id),
          type_ (type),
          size_ (size),
          is_loaded_ (true)
    {
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if the file is an alert
    // @return True if the file is an alert, false otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_alert () const
    {
        _load_metadata ();
        return type_ > 0;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get file type
    // @return The type of the file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    int
    get_type () const
    {
        _load_metadata ();
        return type_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if the file has a known size
    // @return True if the file has a known size, false otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    has_size () const
    {
        _load_metadata ();
        return size_ != -1;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get file size
    // @return The size of the file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    int64_t
    get_size () const
    {
        _load_metadata ();
        return size_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void set_size (uint64_t size);
    bool has_hash (std::uint16_t) const;
    std::string get_hash (std::uint16_t) const;
    void set_hash (std::uint16_t, const std::string &);
    void remove_hash (std::uint16_t);
    mobius::core::bytearray get_thumbnail () const;
    void set_thumbnail (const mobius::core::bytearray &);

  private:
    // @brief File ID
    int64_t id_ = 0;

    // @brief Type
    mutable int64_t type_ = 0;

    // @brief Size in bytes or -1 if unknown
    mutable int64_t size_ = 0;

    // @brief Is loaded flag
    mutable bool is_loaded_ = false;

    // Helper functions
    void _load_metadata () const;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load file metadata from the database if not already loaded
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file::impl::_load_metadata () const
{
    if (is_loaded_)
        return;

    auto db = get_database ();

    auto stmt = db.new_statement ("SELECT type, size FROM file WHERE id = ?");
    stmt.bind (1, id_);

    if (stmt.fetch_row ())
    {
        type_ = stmt.get_column_int64 (0);
        size_ = stmt.is_column_null (1) ? -1 : stmt.get_column_int64 (1);
    }

    is_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set file size
// @param size The size to set
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file::impl::set_size (uint64_t size)
{
    auto db = get_database ();

    auto stmt = db.new_statement ("UPDATE file SET size = ? WHERE id = ?");
    stmt.bind (1, static_cast<int64_t> (size));
    stmt.bind (2, id_);
    stmt.execute ();

    size_ = size;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if the file has a specific hash
// @param hash_type The type of hash to check
// @return True if the hash exists, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
file::impl::has_hash (std::uint16_t hash_type) const
{
    auto db = get_database ();

    auto stmt = db.new_statement ("SELECT 1 FROM hash WHERE file_id = ? AND type = ?");
    stmt.bind (1, id_);
    stmt.bind (2, hash_type);

    return stmt.fetch_row ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file hash
// @param hash_type The type of hash to get
// @return The hash value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
file::impl::get_hash (std::uint16_t hash_type) const
{
    std::string hash;
    auto db = get_database ();

    auto stmt = db.new_statement ("SELECT value FROM hash WHERE file_id = ? AND type = ?");
    stmt.bind (1, id_);
    stmt.bind (2, hash_type);

    if (stmt.fetch_row ())
        hash = stmt.get_column_bytearray (0).to_hexstring ();

    return hash;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set file hash
// @param hash_type The type of hash to set
// @param hash_value The hash value to set
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file::impl::set_hash (std::uint16_t hash_type, const std::string &hash_value)
{
    thread_local mobius::core::database::statement stmt;

    if (!stmt)
    {
        auto db = mobius::core::kff::get_database ();

        stmt = db.new_statement (
            "INSERT OR IGNORE INTO hash (file_id, type, value) "
            "VALUES (?,?,?)"
        );
    }

    stmt.bind (1, id_);
    stmt.bind (2, hash_type);
    stmt.bind (3, mobius::core::decoder::hexstring (hash_value));
    stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove file hash
// @param hash_type The type of hash to remove
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file::impl::remove_hash (std::uint16_t hash_type)
{
    auto db = get_database ();

    auto stmt = db.new_statement ("DELETE FROM hash WHERE file_id = ? AND type = ?");
    stmt.bind (1, id_);
    stmt.bind (2, hash_type);
    stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file thumbnail
// @return The file thumbnail
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
file::impl::get_thumbnail () const
{
    mobius::core::bytearray thumbnail;

    auto db = get_database ();
    auto stmt = db.new_statement ("SELECT data FROM thumbnail WHERE file_id = ?");
    stmt.bind (1, id_);

    if (stmt.fetch_row ())
        thumbnail = stmt.get_column_bytearray (0);

    return thumbnail;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set thumbnail for the file
// @param thumbnail The thumbnail data to set
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file::impl::set_thumbnail (const mobius::core::bytearray &thumbnail)
{
    auto db = get_database ();

    auto stmt = db.new_statement ("INSERT OR REPLACE INTO thumbnail (file_id, data) VALUES (?, ?)");
    stmt.bind (1, id_);
    stmt.bind (2, thumbnail);
    stmt.execute ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param id The file ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file::file (int64_t id)
    : impl_ (std::make_shared<impl> (id))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param id The file ID
// @param type The type of the file
// @param size The size of the file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file::file (int64_t id, std::uint16_t type, uint64_t size)
    : impl_ (std::make_shared<impl> (id, type, size))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if the file is an alert
// @return True if the file is an alert, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
file::is_alert () const
{
    if (!impl_)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid KFF file"));

    return impl_->is_alert ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file type
// @return The type of the file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
file::get_type () const
{
    if (!impl_)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid KFF file"));

    return impl_->get_type ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file has size
// @return True if the file has a size, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
file::has_size () const
{
    if (!impl_)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid KFF file"));

    return impl_->has_size ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file size
// @return The size of the file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int64_t
file::get_size () const
{
    if (!impl_)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid KFF file"));

    return impl_->get_size ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set file size
// @param size The size to set
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file::set_size (uint64_t size)
{
    if (!impl_)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid KFF file"));

    impl_->set_size (size);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file has a hash
// @param hash_type The type of hash to check for
// @return True if the file has a hash, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
file::has_hash (std::uint16_t hash_type) const
{
    if (!impl_)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid KFF file"));

    return impl_->has_hash (hash_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file hash
// @param hash_type The type of hash to get
// @return The file hash
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
file::get_hash (std::uint16_t hash_type) const
{
    if (!impl_)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid KFF file"));

    return impl_->get_hash (hash_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set file hash
// @param hash_type The type of hash to set
// @param hash_value The hash value to set
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file::set_hash (std::uint16_t hash_type, const std::string &hash_value)
{
    if (!impl_)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid KFF file"));

    impl_->set_hash (hash_type, hash_value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove file hash
// @param hash_type The type of hash to remove
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file::remove_hash (std::uint16_t hash_type)
{
    if (!impl_)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid KFF file"));

    impl_->remove_hash (hash_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file thumbnail
// @return The file thumbnail
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
file::get_thumbnail () const
{
    if (!impl_)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid KFF file"));

    return impl_->get_thumbnail ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set thumbnail for the file
// @param thumbnail The thumbnail data to set
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file::set_thumbnail (const mobius::core::bytearray &thumbnail)
{
    if (!impl_)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid KFF file"));

    impl_->set_thumbnail (thumbnail);
}

} // namespace mobius::core::kff