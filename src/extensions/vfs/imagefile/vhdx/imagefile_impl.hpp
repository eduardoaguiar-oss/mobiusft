#ifndef MOBIUS_EXTENSION_IMAGEFILE_IMPL_HPP
#define MOBIUS_EXTENSION_IMAGEFILE_IMPL_HPP

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
#include <cstdint>
#include <mobius/core/bytearray.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/vfs/imagefile_impl_base.hpp>
#include <string>
#include <vector>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Microsoft VHDX imagefile implementation class
// @author Eduardo Aguiar
// @see
// https://winprotocoldoc.blob.core.windows.net/productionwindowsarchives/MS-VHDX/%5bMS-VHDX%5d.pd
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class imagefile_impl : public mobius::core::vfs::imagefile_impl_base
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Class metadata functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    static bool is_instance (const mobius::core::io::file &);
    static std::string
    get_file_extensions ()
    {
        return "vhdx";
    }
    static bool
    is_writeable ()
    {
        return false;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Datatypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    using bat_type = std::vector<std::uint64_t>; // Block Allocation Table type

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit imagefile_impl (const mobius::core::io::file &);
    mobius::core::pod::data get_attribute (const std::string &) const override;
    void set_attribute (const std::string &,
                        const mobius::core::pod::data &) override;
    mobius::core::pod::map get_attributes () const override;
    mobius::core::io::reader new_reader () const override;
    mobius::core::io::writer new_writer () const override;
    bat_type get_block_allocation_table () const;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get type
    // @return Type as string
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_type () const override
    {
        return "vhdx";
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if object is valid
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const noexcept override { return true; }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if imagefile is available
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_available () const override
    {
        return file_ && file_.exists ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get file object
    // @return File object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::io::file
    get_file () const
    {
        return file_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get imagefile size
    // @return Size in bytes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    size_type
    get_size () const override
    {
        _load_metadata ();
        return size_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of sectors
    // @return Number of sectors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    size_type
    get_sectors () const override
    {
        _load_metadata ();
        return sectors_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get sector size
    // @return Sector size
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    size_type
    get_sector_size () const override
    {
        _load_metadata ();
        return sector_size_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get block size
    // @return Block size
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_block_size () const
    {
        return block_size_;
    }

  private:
    // @brief File object
    mobius::core::io::file file_;

    // @brief version
    mutable std::uint32_t version_ = 0;

    // @brief imagefile size in bytes
    mutable size_type size_ = 0;

    // @brief number of sectors
    mutable size_type sectors_ = 0;

    // @brief sector size in bytes
    mutable size_type sector_size_ = 512;

    // @brief physical sector size in bytes
    mutable size_type physical_sector_size_ = 512;

    // @brief disk type (2 - fixed, 3 - dynamic, 4 - differencing)
    mutable std::uint32_t disk_type_ = 0;

    // @brief disk UUID
    mutable std::string disk_uuid_;

    // @brief block size in bytes
    mutable std::uint32_t block_size_ = 33554432; // default = 32 MB

    // @brief acquisition tool
    mutable std::string acquisition_tool_;

    // @brief log file GUID
    mutable std::string logfile_guid_;

    // @brief Block Allocation Table offset
    mutable std::uint64_t bat_offset_ = 0;

    // @brief Block Allocation Table
    mutable bat_type block_allocation_table_;

    // @brief attributes
    mutable mobius::core::pod::map attributes_;

    // @brief flag: metadata loaded
    mutable bool metadata_loaded_ = false;

    // @brief flag: block allocation table loaded
    mutable bool block_allocation_table_loaded_ = false;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_metadata () const;
    void _load_block_allocation_table () const;
    void _load_file_type_identifier (mobius::core::io::reader) const;
    void _load_header (mobius::core::io::reader) const;
    void _load_region_table (mobius::core::io::reader) const;
    void _load_metadata_region (mobius::core::io::reader, std::uint64_t) const;
};

#endif
