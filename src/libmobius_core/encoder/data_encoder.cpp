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
#include <mobius/core/encoder/data_encoder.hpp>
#include <mobius/core/io/bytearray_io.hpp>

namespace mobius::core::encoder
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param writer Writer object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data_encoder::data_encoder (const mobius::core::io::writer &writer)
    : writer_ (writer)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param data Reference to bytearray object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
data_encoder::data_encoder (mobius::core::bytearray &data)
    : writer_ (mobius::core::io::new_bytearray_writer (data))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Encode uint8
// @param value uint8_t value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
data_encoder::encode_uint8 (std::uint8_t value)
{
    writer_.write ({value});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Encode uint16_t little endian
// @param value uint16_t value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
data_encoder::encode_uint16_le (std::uint16_t value)
{
    writer_.write ({std::uint8_t (value), std::uint8_t (value >> 8)});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Encode uint32_t little endian
// @param value uint32_t value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
data_encoder::encode_uint32_le (std::uint32_t value)
{
    writer_.write ({std::uint8_t (value), std::uint8_t (value >> 8),
                    std::uint8_t (value >> 16), std::uint8_t (value >> 24)});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Encode uint64_t little endian
// @param value uint64_t value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
data_encoder::encode_uint64_le (std::uint64_t value)
{
    writer_.write ({std::uint8_t (value), std::uint8_t (value >> 8),
                    std::uint8_t (value >> 16), std::uint8_t (value >> 24),
                    std::uint8_t (value >> 32), std::uint8_t (value >> 40),
                    std::uint8_t (value >> 48), std::uint8_t (value >> 56)});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Encode int64_t little endian
// @param value int64_t value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
data_encoder::encode_int64_le (std::int64_t value)
{
    encode_uint64_le (std::uint64_t (value));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Encode string by size
// @param value string
// @param size size in bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
data_encoder::encode_string_by_size (const std::string &value, std::size_t size)
{
    if (value.length () <= size)
    {
        writer_.write (value);

        mobius::core::bytearray pad (size - value.length ());
        pad.fill (0);

        writer_.write (pad);
    }

    else
        writer_.write (value.substr (0, size));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Encode bytearray
// @param data Data
// @param size size in bytes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
data_encoder::encode_bytearray (const mobius::core::bytearray &data)
{
    writer_.write (data);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Write siz bytes to output
// @param siz size in bytes
// @param value Value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
data_encoder::fill (std::size_t siz, std::uint8_t value)
{
    constexpr std::size_t BLOCK_SIZE = 32768;

    if (siz >= BLOCK_SIZE)
    {
        mobius::core::bytearray data (BLOCK_SIZE);
        data.fill (value);

        while (siz >= BLOCK_SIZE)
        {
            writer_.write (data);
            siz -= BLOCK_SIZE;
        }
    }

    if (siz > 0)
    {
        mobius::core::bytearray data (siz);
        data.fill (value);
        writer_.write (data);
    }
}

} // namespace mobius::core::encoder
