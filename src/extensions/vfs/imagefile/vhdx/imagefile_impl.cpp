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
#include "reader_impl.hpp"
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/reader.hpp>
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
static constexpr int PAYLOAD_BLOCK_NOT_PRESENT = 0;
static constexpr int PAYLOAD_BLOCK_UNDEFINED = 1;
static constexpr int PAYLOAD_BLOCK_ZERO = 2;
static constexpr int PAYLOAD_BLOCK_UNMAPPED = 3;
static constexpr int PAYLOAD_BLOCK_FULLY_PRESENT = 6;
static constexpr int PAYLOAD_BLOCK_PARTIALLY_PRESENT = 7;

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

    if (f && f.exists ())
    {
        auto reader = f.new_reader ();

        if (reader)
        {
            auto data = reader.read (8);
            is_instance = data == "vhdxfile";
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

    if (disk_type_ == DISK_TYPE_FIXED || disk_type_ == DISK_TYPE_DYNAMIC)
        return mobius::core::io::reader (std::make_shared<reader_impl> (*this));

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
imagefile_impl::bat_type
imagefile_impl::get_block_allocation_table () const
{
    _load_metadata ();

    if (disk_type_ != DISK_TYPE_DYNAMIC)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("disk type has no Block Allocation Table"));

    _load_block_allocation_table ();
    return block_allocation_table_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load metadata from imagefile
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
imagefile_impl::_load_metadata () const
{
    if (metadata_loaded_)
        return;

    if (!file_ || !file_.exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("image file not found"));

    // decode metadata
    auto reader = file_.new_reader ();

    _load_file_type_identifier (reader);
    _load_header (reader);
    _load_region_table (reader);

    // other data
    mobius::core::datetime::datetime acquisition_time =
        file_.get_modification_time ();
    sectors_ = size_ / sector_size_;

    // fill attributes
    attributes_.set ("physical_sector_size", physical_sector_size_);
    attributes_.set ("block_size", block_size_);
    attributes_.set ("disk_type", disk_type_);
    attributes_.set ("uuid", disk_uuid_);
    attributes_.set ("acquisition_time", acquisition_time);
    attributes_.set ("acquisition_tool", acquisition_tool_);
    attributes_.set ("logfile_guid", logfile_guid_);

    metadata_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load data from File Type Identifier
// @param reader Reader object
// @see MS-VHDX - section 2.2.1
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
imagefile_impl::_load_file_type_identifier (
    mobius::core::io::reader reader) const
{
    mobius::core::decoder::data_decoder decoder (reader);

    // test signature
    auto signature = decoder.get_string_by_size (8);

    if (signature != "vhdxfile")
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("invalid VHDX signature"));

    // creator
    acquisition_tool_ = decoder.get_string_by_size (512, "utf-16le");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load data from Header
// @param reader Reader object
// @see MS-VHDX - section 2.2.2
//
// A header is valid if the Signature and Checksum fields both validate
// correctly. A header is current if it is the only valid header or if it is
// valid and its SequenceNumber field is greater than the other header's
// SequenceNumber field. The implementation MUST only use data from the current
// header. If there is no current header, then the VHDX file is corrupt.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
imagefile_impl::_load_header (mobius::core::io::reader reader) const
{
    mobius::core::decoder::data_decoder decoder (reader);
    decoder.seek (65536);

    std::uint64_t h1_sequence_number = 0;
    std::uint64_t h2_sequence_number = 0;

    // test signature - first header
    auto signature_1 = decoder.get_string_by_size (4);

    if (signature_1 == "head")
    {
        decoder.skip (4); // checksum
        h1_sequence_number = decoder.get_uint64_le ();
    }

    decoder.seek (131072);

    // test signature - second header
    auto signature_2 = decoder.get_string_by_size (4);

    if (signature_2 == "head")
    {
        decoder.skip (4); // checksum
        h2_sequence_number = decoder.get_uint64_le ();
    }

    // check which header is valid
    if (h1_sequence_number == 0 && h2_sequence_number == 0)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid VHDX header"));

    if (h1_sequence_number >= h2_sequence_number)
        decoder.seek (65536 + 16);

    else
        decoder.seek (131072 + 16);

    // decoder data
    decoder.skip (32); // FileWriteGuid, DataWriteGuid
    logfile_guid_ = decoder.get_guid ();
    decoder.skip (2); // LogVersion
    version_ = decoder.get_uint16_le ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load data from Region Table
// @param reader Reader object
// @see MS-VHDX - section 2.2.3
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
imagefile_impl::_load_region_table (mobius::core::io::reader reader) const
{
    mobius::core::decoder::data_decoder decoder (reader);
    decoder.seek (196608);

    // test signature
    auto signature = decoder.get_string_by_size (4);

    if (signature != "regi")
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("invalid VHDX Region Table signature"));

    // read Region Table metadata
    decoder.skip (4); // checksum
    auto count = decoder.get_uint32_le ();
    decoder.skip (4); // reserved

    // read Regions data
    for (std::uint32_t i = 0; i < count; i++)
    {
        auto guid = decoder.get_guid ();
        auto file_offset = decoder.get_uint64_le ();
        decoder.skip (4); // length
        auto is_required = decoder.get_uint32_le ();

        if (guid == "8B7CA206-4790-4B9A-B8FE-575F050F886E")
            _load_metadata_region (reader, file_offset);

        else if (guid == "2DC27766-F623-4200-9D64-115E9BFD4A08")
            bat_offset_ = file_offset;

        else if (is_required)
            throw std::runtime_error (MOBIUS_EXCEPTION_MSG (
                "unknown and required Region Table entry"));
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load data from Metadata Region
// @param reader Reader object
// @param file_offset File offset
// @see MS-VHDX - section 2.6
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
imagefile_impl::_load_metadata_region (mobius::core::io::reader reader,
                                       std::uint64_t file_offset) const
{
    mobius::core::decoder::data_decoder decoder (reader);
    decoder.seek (file_offset);

    // test signature
    auto signature = decoder.get_string_by_size (8);

    if (signature != "metadata")
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("invalid VHDX Metadata Region signature"));

    // read Metadata Table header (section 2.6.1.1)
    decoder.skip (2); // Reserved
    auto count = decoder.get_uint16_le ();
    decoder.skip (20); // reserved

    // read Metadata Region
    for (std::uint16_t i = 0; i < count; i++)
    {
        auto guid = decoder.get_guid ();
        auto offset = decoder.get_uint32_le ();
        decoder.skip (12); // length, flags, reserved

        auto pos = decoder.tell ();
        decoder.seek (file_offset + offset);

        // File Parameters - section 2.6.2.1
        if (guid == "CAA16737-FA36-4D43-B3B6-33F0AA44E76B")
        {
            block_size_ = decoder.get_uint32_le ();
            auto flags = decoder.get_uint32_le ();

            if (flags & 0x80000000)
                disk_type_ = DISK_TYPE_FIXED;

            else if (flags & 0x40000000) // differencing
                throw std::runtime_error (
                    MOBIUS_EXCEPTION_MSG ("unsupported disk type"));

            else
                disk_type_ = DISK_TYPE_DYNAMIC;
        }

        // Virtual Disk Size - section 2.6.2.2
        else if (guid == "2FA54224-CD1B-4876-B211-5DBED83BF4B8")
            size_ = decoder.get_uint64_le ();

        // Virtual Disk ID - section 2.6.2.3
        else if (guid == "BECA12AB-B2E6-4523-93EF-C309E000C746")
            disk_uuid_ = decoder.get_guid ();

        // Logical Sector Size - section 2.6.2.4
        else if (guid == "8141BF1D-A96F-4709-BA47-F233A8FAAB5F")
            sector_size_ = decoder.get_uint32_le ();

        // Physical Sector Size - section 2.6.2.5
        else if (guid == "CDA348C7-445D-4471-9CC9-E9885251C556")
            physical_sector_size_ = decoder.get_uint32_le ();

        decoder.seek (pos);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load block allocation table from imagefile
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
imagefile_impl::_load_block_allocation_table () const
{
    if (block_allocation_table_loaded_)
        return;

    // load metadata
    _load_metadata ();

    if (block_size_ == 0)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("block size must be greater than 0"));

    // decode BAT
    auto reader = file_.new_reader ();
    mobius::core::decoder::data_decoder decoder (reader);
    decoder.seek (bat_offset_);

    auto chunk_ratio = (std::uint64_t (sector_size_) << 23) / block_size_;
    std::uint64_t block_count = (size_ + block_size_ - 1) / block_size_;
    std::uint64_t i = 0;

    while (block_count > 0)
    {
        auto value = decoder.get_uint64_le ();

        // for every chunk_ration data blocks, there is one bitmap block,
        // which we skip for now.
        if (i % (chunk_ratio + 1) != chunk_ratio)
        {
            auto state = value & 0x07;
            auto offset = value & 0xfffffffffff00000;

            if (state == PAYLOAD_BLOCK_FULLY_PRESENT)
                block_allocation_table_.push_back (offset);
            else
                block_allocation_table_.push_back (0);

            block_count--;
        }

        i++;
    }

    block_allocation_table_loaded_ = true;
}
