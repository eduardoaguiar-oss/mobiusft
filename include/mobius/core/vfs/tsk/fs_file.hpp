#ifndef MOBIUS_CORE_VFS_TSK_FS_FILE_HPP
#define MOBIUS_CORE_VFS_TSK_FS_FILE_HPP

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
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/stream_impl_base.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <tsk/libtsk.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief libtsk exception message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
namespace mobius::core::vfs::tsk
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief TSK_FS_FILE C++ class
// @author Eduardo Aguiar
// Shared owner of TSK_FS_FILE pointer, calling tsk_fs_file_close when
// pointer is finally deallocated.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class fs_file
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief File subtype
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    enum class fs_file_type
    {
        none,
        block_device,
        char_device,
        fifo,
        symlink,
        regular,
        socket,
        folder
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Datatypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    using stream_type = std::shared_ptr<mobius::core::io::stream_impl_base>;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    fs_file ();
    explicit fs_file (TSK_FS_FILE *);
    fs_file (const fs_file &) = default;
    fs_file (fs_file &&) noexcept = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    fs_file &operator= (const fs_file &) = default;
    fs_file &operator= (fs_file &&) noexcept = default;
    operator bool () const noexcept;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    TSK_FS_FILE *get_pointer () const;
    bool exists () const;
    bool is_deleted () const;
    bool is_reallocated () const;
    bool is_hidden () const;
    std::string get_name () const;
    std::string get_short_name () const;
    std::string get_path () const;
    void set_path (const std::string &);
    std::uint64_t get_inode () const;
    std::uint64_t get_size () const;
    fs_file_type get_type () const;
    int get_user_id () const;
    int get_group_id () const;
    int get_permissions () const;
    mobius::core::datetime::datetime get_creation_time () const;
    mobius::core::datetime::datetime get_access_time () const;
    mobius::core::datetime::datetime get_modification_time () const;
    mobius::core::datetime::datetime get_metadata_time () const;
    mobius::core::datetime::datetime get_deletion_time () const;
    mobius::core::datetime::datetime get_backup_time () const;
    void reload ();
    fs_file get_parent () const;
    std::vector<fs_file> get_children () const;
    std::vector<stream_type> get_streams () const;

  private:
    // @brief Forward declaration of implementation class
    class impl;

    // @brief Pointer to implementation
    std::shared_ptr<impl> impl_;

  private:
    // Helper functions
    void _load_streams () const;
};

} // namespace mobius::core::vfs::tsk

#endif
