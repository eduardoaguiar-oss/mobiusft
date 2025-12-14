// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
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
#include "writer_impl.hpp"
#include <mobius/core/exception.inc>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/uri.hpp>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief number of sectors per chunk
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr int SECTOR_SIZE = 512;
static constexpr int CHUNK_SECTORS = 64;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param imagefile_impl imagefile implementation object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
writer_impl::writer_impl (const imagefile_impl &imagefile_impl)
    : segment_size_ (
          std::int64_t (imagefile_impl.get_attribute ("segment_size"))),
      chunk_size_ (imagefile_impl.get_sector_size () * CHUNK_SECTORS),
      sector_size_ (imagefile_impl.get_sector_size ()),
      compression_level_ (
          std::int64_t (imagefile_impl.get_attribute ("compression_level"))),
      segments_ (imagefile_impl.get_segment_array ()),
      hash_ ("md5")
{
    // validate segment size. The segment size must be at least large enough
    // to store a chunk of data.
    constexpr size_type SECTION_HEADER_SIZE = 76;
    constexpr size_type HEADER_SECTION_SIZE = 500;
    constexpr size_type VOLUME_SECTION_SIZE = 1128;
    constexpr size_type HASH_SECTION_SIZE = 112;
    constexpr size_type DONE_SECTION_SIZE = 76;
    constexpr size_type TABLE_SECTION_SIZE = 76 + 24 + 4 + 4;

    const size_type min_segment_size =
        HEADER_SECTION_SIZE * 3 +           // header2, header2 and header
        VOLUME_SECTION_SIZE +               // volume section
        SECTION_HEADER_SIZE + chunk_size_ + // sectors section
        TABLE_SECTION_SIZE * 2 +            // table and table2 sections
        VOLUME_SECTION_SIZE + // data section (equal to volume section)
        HASH_SECTION_SIZE +   // hash section
        DONE_SECTION_SIZE;    // done section

    if (segment_size_ < min_segment_size)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("segment size too small"));

    // create imagefile GUID
    guid_ = mobius::core::bytearray (16);
    guid_.random ();

    // create first segment
    auto &segment_writer = _new_segment_writer ();

    auto acquisition_user =
        imagefile_impl.get_attribute ("get_acquisition_user");
    auto drive_vendor = imagefile_impl.get_attribute ("drive_vendor");
    auto drive_model = imagefile_impl.get_attribute ("drive_model");
    auto drive_serial_number =
        imagefile_impl.get_attribute ("drive_serial_number");

    if (drive_vendor.is_string ())
        segment_writer.set_drive_vendor (std::string (drive_vendor));

    if (drive_model.is_string ())
        segment_writer.set_drive_model (std::string (drive_model));

    if (drive_serial_number.is_string ())
        segment_writer.set_drive_serial_number (
            std::string (drive_serial_number));

    if (acquisition_user.is_string ())
        segment_writer.set_acquisition_user (std::string (acquisition_user));

    segment_writer.create ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
writer_impl::~writer_impl ()
{
    // pad image to SECTOR_SIZE, if necessary
    if (size_ % SECTOR_SIZE)
    {
        auto pad_size = SECTOR_SIZE - (size_ % SECTOR_SIZE);
        write (mobius::core::bytearray (pad_size));
    }

    // close segments
    for (auto &sw : segment_writer_list_)
    {
        sw.set_total_size (size_);
        sw.set_chunk_sectors (CHUNK_SECTORS);
        sw.set_sector_size (sector_size_);
        sw.set_segment_count (segment_writer_list_.size ());
        sw.set_md5_hash (hash_.get_digest ());
        sw.close ();
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set write position
// @param offset offset in bytes
// @param w either beginning, current or end
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
writer_impl::seek (offset_type, whence_type)
{
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("writer is not seekable"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Write bytes to stream
// @param data Data
// @return Number of bytes written
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
writer_impl::size_type
writer_impl::write (const mobius::core::bytearray &data)
{
    mobius::core::bytearray buffer = data;

    while (!buffer.empty ())
    {
        auto &segment_writer = _get_current_segment_writer ();

        auto bytes_written = segment_writer.write (buffer);
        size_ += bytes_written;

        if (bytes_written == buffer.size ())
            buffer.clear ();

        else
        {
            // discard bytes already written
            buffer = buffer.slice (bytes_written, buffer.size () - 1);

            // create next segment file
            auto &new_segment_writer = _new_segment_writer ();
            new_segment_writer.create ();
        }
    }

    hash_.update (data);
    return data.size ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Flush data to file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
writer_impl::flush ()
{
    auto &segment_writer = _get_current_segment_writer ();
    segment_writer.flush ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new segment file
// @return Reference to current segment writer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
segment_writer &
writer_impl::_new_segment_writer ()
{
    auto segment_number = segment_writer_list_.size () + 1;
    auto writer = segments_.new_writer (segment_number - 1);
    segment_writer_list_.emplace_back (writer, segment_number);

    auto &segment_writer =
        segment_writer_list_[segment_writer_list_.size () - 1];
    segment_writer.set_segment_size (segment_size_);
    segment_writer.set_chunk_size (chunk_size_);
    segment_writer.set_compression_level (compression_level_);
    segment_writer.set_guid (guid_);

    return segment_writer;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get current segment writer
// @return Reference to current segment writer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
segment_writer &
writer_impl::_get_current_segment_writer ()
{
    if (segment_writer_list_.size () == 0)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("no segment writer found"));

    return segment_writer_list_[segment_writer_list_.size () - 1];
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Return next extension
// @param extension
// @return next extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
writer_impl::_get_next_extension (const std::string &extension) const
{
    std::string tmp_extension;

    if (extension == "E99")
        tmp_extension = "EAA";

    else
    {
        tmp_extension = extension;
        int pos = extension.length () - 1;
        bool carry = true;

        while (pos >= 0 && carry)
        {
            if (tmp_extension[pos] == '9')
                tmp_extension[pos] = '0';

            else if (tmp_extension[pos] == 'Z')
                tmp_extension[pos] = 'A';

            else
            {
                ++tmp_extension[pos];
                carry = false;
            }

            if (carry)
                --pos;
        }
    }

    return tmp_extension;
}
