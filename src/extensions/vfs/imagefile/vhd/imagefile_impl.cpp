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
#include "imagefile_impl.hpp"
#include "reader_impl_dynamic.hpp"
#include "reader_impl_fixed.hpp"
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/io/file.hpp>
#include <stdexcept>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr int SECTOR_SIZE = 512; // always 512 bytes
static constexpr int DISK_TYPE_FIXED = 2;
static constexpr int DISK_TYPE_DYNAMIC = 3;
static constexpr int DISK_TYPE_DIFFERENCING = 4;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode timestamp
// @return datetime object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
decode_timestamp (std::uint32_t timestamp)
{
    mobius::core::datetime::datetime dt;

    if (timestamp)
        dt = mobius::core::datetime::datetime (2000, 1, 1, 0, 0, 0) +
             mobius::core::datetime::timedelta (timestamp);

    return dt;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file is an instance of imagefile vhd
// @param f File object
// @return True/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
imagefile_impl::is_instance (const mobius::core::io::file &f)
{
    bool is_instance = false;

    if (f && f.exists () && f.get_size () >= SECTOR_SIZE)
    {
        auto reader = f.new_reader ();

        if (reader)
        {
            reader.seek (-SECTOR_SIZE,
                         mobius::core::io::reader::whence_type::end);
            auto data = reader.read (8);
            is_instance = data == "conectix";
        }
    }

    return is_instance;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile_impl::imagefile_impl (const mobius::core::io::file &f)
    : file_ (f)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get attribute
// @param name Attribute name
// @return Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
imagefile_impl::get_attribute (const std::string &name) const
{
    _load_metadata ();
    return attributes_.get (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set attribute
// @param name Attribute name
// @param value Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
imagefile_impl::set_attribute (const std::string &,
                               const mobius::core::pod::data &)
{
    throw std::runtime_error (
        MOBIUS_EXCEPTION_MSG ("set_attribute not implemented"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get attributes
// @return Attributes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::map
imagefile_impl::get_attributes () const
{
    _load_metadata ();
    return attributes_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new reader for imagefile
// @return reader
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::reader
imagefile_impl::new_reader () const
{
    _load_metadata ();

    if (disk_type_ == DISK_TYPE_FIXED)
        return mobius::core::io::reader (
            std::make_shared<reader_impl_fixed> (*this));

    else if (disk_type_ == DISK_TYPE_DYNAMIC)
        return mobius::core::io::reader (
            std::make_shared<reader_impl_dynamic> (*this));

    else
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("unsupported disk type"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new writer for imagefile
// @return writer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::writer
imagefile_impl::new_writer () const
{
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("writer not implemented"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get Block Allocation Table
// @return Table
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile_impl::table_type
imagefile_impl::get_block_allocation_table () const
{
    _load_metadata ();

    if (disk_type_ != DISK_TYPE_DYNAMIC)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("Disk type has no Block Allocation Table"));

    return block_allocation_table_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get block size
// @return block size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
imagefile_impl::get_block_size () const
{
    _load_metadata ();

    if (disk_type_ != DISK_TYPE_DYNAMIC)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("Disk type has no block size"));

    return block_size_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load metadata from imagefile
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
imagefile_impl::_load_metadata () const
{
    if (metadata_loaded_)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Check if imagefile exists
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (!file_ || !file_.exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("Image file not found"));

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode Hard Disk Footer
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto reader = file_.new_reader ();
    reader.seek (-SECTOR_SIZE, mobius::core::io::reader::whence_type::end);
    mobius::core::decoder::data_decoder decoder (reader);

    // test signature
    auto signature = decoder.get_string_by_size (8);

    if (signature != "conectix")
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("Invalid VHD signature"));

    // features
    decoder.skip (4);

    // format version
    std::string format_version = std::to_string (decoder.get_uint16_be ());
    format_version += '.' + std::to_string (decoder.get_uint16_be ());

    // data offset
    auto data_offset = decoder.get_uint64_be ();

    // acquisition info
    mobius::core::datetime::datetime acquisition_time =
        decode_timestamp (decoder.get_uint32_be ());

    std::string acquisition_tool = decoder.get_string_by_size (4) + " v";
    acquisition_tool += std::to_string (decoder.get_uint16_be ());
    acquisition_tool += '.' + std::to_string (decoder.get_uint16_be ());

    std::string acquisition_platform = decoder.get_string_by_size (4);

    // size
    decoder.skip (8); // original size
    size_ = decoder.get_uint64_be ();
    sector_size_ = SECTOR_SIZE; // constant
    sectors_ = (size_ + sector_size_ - 1) / sector_size_;

    // disk type
    decoder.skip (4); // disk geometry
    disk_type_ = decoder.get_uint32_be ();
    decoder.skip (4); // checksum

    if (disk_type_ != DISK_TYPE_FIXED && disk_type_ != DISK_TYPE_DYNAMIC)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("Unsupported disk type"));

    // UUID
    std::string disk_uuid = decoder.get_uuid ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode Dynamic Disk Header, if disk type == 3 (DISK_TYPE_DYNAMIC)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (disk_type_ == DISK_TYPE_DYNAMIC)
    {
        decoder.seek (data_offset);

        // test signature
        auto signature = decoder.get_string_by_size (8);

        if (signature != "cxsparse")
            throw std::runtime_error (MOBIUS_EXCEPTION_MSG (
                "Invalid VHD Dynamic Disk Header signature"));

        // data offset (unused)
        decoder.skip (8);

        // table offset
        auto table_offset = decoder.get_uint64_be ();

        // header version
        decoder.skip (4);

        // max_table_entries
        auto max_table_entries = decoder.get_uint32_be ();

        // block_size
        block_size_ = decoder.get_uint32_be ();

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode Block Allocation Table
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        decoder.seek (table_offset);

        block_allocation_table_.reserve (max_table_entries);

        for (std::uint64_t i = 0; i < max_table_entries; i++)
        {
            auto offset = decoder.get_uint32_be ();
            block_allocation_table_.push_back (offset);
        }
    }

    // fill attributes
    attributes_.set ("format_version", format_version);
    attributes_.set ("disk_type", disk_type_);
    attributes_.set ("uuid", disk_uuid);
    attributes_.set ("acquisition_time", acquisition_time);
    attributes_.set ("acquisition_tool", acquisition_tool);
    attributes_.set ("acquisition_platform", acquisition_platform);

    metadata_loaded_ = true;
}
