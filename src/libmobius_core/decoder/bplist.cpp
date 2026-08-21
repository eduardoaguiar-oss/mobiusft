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
#include <mobius/core/decoder/bplist.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/io/bytearray_io.hpp>
#include <mobius/core/pod/map.hpp>
#include <mobius/core/string_functions.hpp>
#include <vector>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief bplist implementation class
// @see https://github.com/opensource-apple/CF/blob/master/CFBinaryPList.c
// @see https://doubleblak.com/blogPost.php?k=plist
// @see https://medium.com/@karaiskc/understanding-apples-binary-property-list-format-281e6da00dbd
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class bplist_decoder
{
    public:
        bplist_decoder (const mobius::core::io::reader &);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // @brief Decode bplist data
        // @return Decoded data
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::pod::data
        get_data () const
        {
            return data_;
        }
        
    private:
        // @brief Decoded data
        mobius::core::pod::data data_;

        // @brief Reader object
        mobius::core::io::reader reader_;

        // @brief Offset table
        std::vector<std::uint64_t> offset_table_;

        // @brief Decode object at given index
        mobius::core::pod::data _decode_object (std::uint64_t) const;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bplist_decoder::bplist_decoder (const mobius::core::io::reader &reader)
    : reader_ (reader)
{
    mobius::core::decoder::data_decoder decoder (reader);

    // Check signature and version
    const auto signature = decoder.get_bytearray_by_size (7);

    if (signature != "bplist0")
        return;

    const auto version = decoder.get_uint8 ();
    if (!std::isdigit (version))
        return;

    // Read trailer
    decoder.seek (decoder.get_size () - 26); // ignore first 6 bytes of trailer
    auto offset_size = decoder.get_uint8 ();
    auto object_ref_size = decoder.get_uint8 ();
    auto num_objects = decoder.get_uint64_be ();
    auto top_object = decoder.get_uint64_be ();
    auto offset_table_offset = decoder.get_uint64_be ();

    if (num_objects < 1)
        return;

    if (top_object >= num_objects)
        return;

    // Read offset table
    decoder.seek (offset_table_offset);

    for (std::uint64_t i = 0; i < num_objects; ++i)
    {
        std::uint64_t offset = 0;

        if (offset_size == 1)
            offset = decoder.get_uint8 ();

        else if (offset_size == 2)
            offset = decoder.get_uint16_be ();

        else if (offset_size == 4)
            offset = decoder.get_uint32_be ();

        else if (offset_size == 8)
            offset = decoder.get_uint64_be ();

        else
            return;

        offset_table_.push_back (offset);
    }

    // Decode top object
    if (top_object < offset_table_.size ())
        data_ = _decode_object (top_object);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode object at given index
// @param index Index of object in offset table
// @return Decoded data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
bplist_decoder::_decode_object (std::uint64_t index) const
{
    // If index is out of bounds, return empty data
    if (index >= offset_table_.size ())
        return {};

    // Set decoder offset of object
    auto offset = offset_table_[index];
    mobius::core::decoder::data_decoder decoder (reader_);
    decoder.seek (offset);

    // Read object type
    auto type = decoder.get_uint8 ();

    if (type == 0x00)   // NULL
        return {};

    else if (type == 0x08)
        return mobius::core::pod::data (false); // Boolean false

    else if (type == 0x09)
        return mobius::core::pod::data (true); // Boolean true

    // Handle other types (integers, reals, dates, data, strings, arrays, dictionaries)
    auto d = type >> 4; // high nibble
    auto n = type & 0x0F; // low nibble

    switch(d)
    {
        
    if (d == 0x1) // Integer
    {
        std::int64_t value = 0;

        if (n == 0)
            value = decoder.get_int8 ();
        else if (n == 1)
            value = decoder.get_int16_be ();
        else if (n == 2)
            value = decoder.get_int32_be ();
        else if (n == 3)
            value = decoder.get_int64_be ();

        return mobius::core::pod::data (value);
    }

/*
    else if (d == 0x2) // Real
    {
        auto real_size = 1 << n;
        long double value = 0.0;

        if (real_size == 4)
            value = decoder.get_float32_be ();
        else if (real_size == 8)
            value = decoder.get_float64_be ();

        return mobius::core::pod::data (value);
    }

    else if ((type & 0xF0) == 0x30) // Date
    {
        auto date_value = decoder.get_float64_be ();
        return mobius::core::pod::data (
            mobius::core::datetime::datetime_from_epoch_seconds (date_value)
        );
    }

    else if ((type & 0xF0) == 0x40) // Data
    {
        auto length = type & 0x0F;
        std::uint64_t data_length = length;

        if (length == 15)
            data_length = decoder.get_uint64_be ();

        auto data_bytes = decoder.get_bytearray_by_size (data_length);
        return mobius::core::pod::data (data_bytes);
    }

    else if ((type & 0xF0) == 0x50) // ASCII String
    {
        auto length = type & 0x0F;
        std::uint64_t str_length = length;

        if (length == 15)
            str_length = decoder.get_uint64_be ();

        auto str_bytes = decoder.get_bytearray_by_size (str*/
}

} // namespace

namespace mobius::core::decoder
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode bplist data
// @param reader Reader object
// @return Pod object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
bplist (const mobius::core::io::reader &reader)
{
    bplist_decoder decoder (reader);
    return decoder.get_data ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode bplist data
// @param data Data
// @return Pod object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
bplist (const mobius::core::bytearray &data)
{
    bplist_decoder decoder (mobius::core::io::new_bytearray_reader (data));
    return decoder.get_data ();
}

} // namespace mobius::core::decoder
