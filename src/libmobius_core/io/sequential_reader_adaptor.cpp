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
#include <mobius/core/exception.inc>
#include <mobius/core/io/sequential_reader_adaptor.hpp>
#include <mobius/core/string_functions.hpp>

namespace mobius::core::io
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Generic reader object
// @param block_size Read ahead block size in bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
sequential_reader_adaptor::sequential_reader_adaptor (
    const mobius::core::io::reader &reader, size_type block_size)
    : reader_ (reader),
      block_size_ (block_size)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get one byte
// @return Byte read
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
sequential_reader_adaptor::byte_type
sequential_reader_adaptor::get ()
{
    mobius::core::bytearray data = get (1);

    if (data.empty ())
        throw std::runtime_error ("EOF reached");

    return data[0];
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Peek one byte, without moving reading position
// @return Byte read
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
sequential_reader_adaptor::byte_type
sequential_reader_adaptor::peek ()
{
    mobius::core::bytearray data = peek (1);

    if (data.empty ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG (std::string ("EOF reached at position ") +
                                  mobius::core::string::to_string (tell ())));

    return data[0];
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get <i>size</i> bytes
// @param size Size in bytes
// @return Bytes read
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
sequential_reader_adaptor::get (size_type size)
{
    mobius::core::bytearray data = peek (size);
    skip (size);

    return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Peek <i>size</i> bytes, without moving reading position
// @param size Size in bytes
// @return Bytes read
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
sequential_reader_adaptor::peek (size_type size)
{
    // fill buffer, if necessary
    if (pos_ + size >= buffer_.size ())
    {
        const size_type bytes_to_read =
            std::max (size - (buffer_.size () - pos_),
                      block_size_); // read at least block_size bytes
        buffer_ += reader_.read (bytes_to_read);
    }

    // return data
    mobius::core::bytearray data;

    if (pos_ < buffer_.size ())
    {
        const size_type bytes_available =
            std::min (buffer_.size () - pos_, size);
        data = buffer_.slice (pos_, pos_ + bytes_available - 1);
    }

    return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Skip ahead <i>size</i> bytes
// @param size Size in bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
sequential_reader_adaptor::skip (size_type size)
{
    // consume from buffer first
    const size_type bytes_available = std::min (buffer_.size () - pos_, size);
    pos_ += bytes_available;
    size -= bytes_available;

    // clear buffer, if necessary
    if (pos_ >= buffer_.size ())
    {
        pos_ = 0;
        buffer_.clear ();
    }

    // skip remaining bytes
    reader_.skip (size);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get current reading position
// @return Reading position from the beginning of the reader
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
sequential_reader_adaptor::size_type
sequential_reader_adaptor::tell () const
{
    return reader_.tell () - buffer_.size () + pos_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if end-of-file (EOF) is reached
// @return True/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
sequential_reader_adaptor::eof () const
{
    return buffer_.empty () && reader_.eof ();
}

} // namespace mobius::core::io
