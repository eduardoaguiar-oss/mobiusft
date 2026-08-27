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
#include <mobius/core/datetime/conv_iso_string.hpp>
#include <mobius/core/decoder/base64.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/decoder/plist.hpp>
#include <mobius/core/decoder/xml/dom.hpp>
#include <mobius/core/io/bytearray_io.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/pod/map.hpp>
#include <mobius/core/string_functions.hpp>
#include <vector>

#include <iostream>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @see https://github.com/opensource-apple/CF/blob/master/CFBinaryPList.c
// HEADER
// 	magic number ("bplist")
// 	file format version (currently "0?")

// OBJECT TABLE
// 	variable-sized objects

// 	Object Formats (marker byte followed by additional info in some cases)
// 	null    0000 0000			    // null object [v"1?"+ only]
// 	bool	0000 1000			    // false
// 	bool	0000 1001			    // true
// 	url	    0000 1100	string		// URL with no base URL, recursive encoding of URL string [v"1?"+ only]
// 	url	    0000 1101	base string	// URL with base URL, recursive encoding of base URL, then recursive encoding of URL string [v"1?"+ only]
// 	uuid	0000 1110			    // 16-byte UUID [v"1?"+ only]
// 	fill	0000 1111			    // fill byte
// 	int	    0001 0nnn	...		    // # of bytes is 2^nnn, big-endian bytes
// 	real	0010 0nnn	...		    // # of bytes is 2^nnn, big-endian bytes
// 	date	0011 0011	...		    // 8 byte float follows, big-endian bytes
// 	data	0100 nnnn	[int]	...	// nnnn is number of bytes unless 1111 then int count follows, followed by bytes
// 	string	0101 nnnn	[int]	...	// ASCII string, nnnn is # of chars, else 1111 then int count, then bytes
// 	string	0110 nnnn	[int]	...	// Unicode string, nnnn is # of chars, else 1111 then int count, then big-endian 2-byte uint16_t
// 	string	0111 nnnn	[int]	...	// UTF8 string, nnnn is # of chars, else 1111 then int count, then bytes [v"1?"+ only]
// 	uid	    1000 nnnn	...		    // nnnn+1 is # of bytes
// 		    1001 xxxx			    // unused
// 	array	1010 nnnn	[int]	objref*	// nnnn is count, unless '1111', then int count follows
// 	ordset	1011 nnnn	[int]	objref* // nnnn is count, unless '1111', then int count follows [v"1?"+ only]
// 	set	    1100 nnnn	[int]	objref* // nnnn is count, unless '1111', then int count follows [v"1?"+ only]
// 	dict	1101 nnnn	[int]	keyref* objref*	// nnnn is count, unless '1111', then int count follows
// 		    1110 xxxx			    // unused
// 		    1111 xxxx			    // unused

// OFFSET TABLE
// 	list of ints, byte size of which is given in trailer
// 	-- these are the byte offsets into the file
// 	-- number of these is in the trailer

// TRAILER
// 	byte size of offset ints in offset table
// 	byte size of object refs in arrays and dicts
// 	number of offsets in offset table (also is number of objects)
// 	element # in offset table which is top level object
// 	offset table offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

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

    // @brief Decoder object
    mobius::core::decoder::data_decoder decoder_;

    // @brief Offset table
    std::vector<std::uint64_t> offset_table_;

    // @brief Object reference size
    std::uint8_t object_ref_size_ = 0;

    // Helper functions
    std::uint64_t _decode_int ();
    std::uint64_t _decode_object_ref ();
    mobius::core::pod::data _decode_object (std::uint64_t);
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bplist_decoder::bplist_decoder (const mobius::core::io::reader &reader)
    : decoder_ (reader)
{
    // Check signature and version
    const auto signature = decoder_.get_bytearray_by_size (7);

    if (signature != "bplist0")
        return;

    const auto version = decoder_.get_uint8 ();
    if (!std::isdigit (version))
        return;

    // Read trailer
    decoder_.seek (decoder_.get_size () - 26); // ignore first 6 bytes of trailer
    auto offset_size = decoder_.get_uint8 ();
    object_ref_size_ = decoder_.get_uint8 ();
    auto num_objects = decoder_.get_uint64_be ();
    auto top_object = decoder_.get_uint64_be ();
    auto offset_table_offset = decoder_.get_uint64_be ();

    if (num_objects < 1)
        return;

    if (top_object >= num_objects)
        return;

    // Read offset table
    decoder_.seek (offset_table_offset);

    for (std::uint64_t i = 0; i < num_objects; ++i)
    {
        std::uint64_t offset = 0;

        if (offset_size == 1)
            offset = decoder_.get_uint8 ();

        else if (offset_size == 2)
            offset = decoder_.get_uint16_be ();

        else if (offset_size == 4)
            offset = decoder_.get_uint32_be ();

        else if (offset_size == 8)
            offset = decoder_.get_uint64_be ();

        else
            return;

        offset_table_.push_back (offset);
    }

    // Decode top object
    if (top_object < offset_table_.size ())
        data_ = _decode_object (top_object);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode integer value
// @return Decoded integer value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
bplist_decoder::_decode_int ()
{
    std::uint8_t type = decoder_.get_uint8 ();
    auto d = type >> 4;   // high nibble
    auto n = type & 0x0F; // low nibble

    if (d != 1)
        return 0;

    std::uint64_t value = 0;

    switch (n)
    {
        case 0:
            value = decoder_.get_uint8 ();
            break;

        case 1:
            value = decoder_.get_uint16_be ();
            break;

        case 2:
            value = decoder_.get_uint32_be ();
            break;

        case 3:
            value = decoder_.get_uint64_be ();
            break;

        default:
            value = 0;
            break;
    }

    return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode object reference
// @return Decoded object reference
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
bplist_decoder::_decode_object_ref ()
{
    switch (object_ref_size_)
    {
        case 1:
            return decoder_.get_uint8 ();
            break;

        case 2:
            return decoder_.get_uint16_be ();
            break;

        case 4:
            return decoder_.get_uint32_be ();
            break;

        case 8:
            return decoder_.get_uint64_be ();
            break;

        default:
            return 0;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode object at given index
// @param index Index of object in offset table
// @return Decoded data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
bplist_decoder::_decode_object (std::uint64_t index)
{
    // If index is out of bounds, return empty data
    if (index >= offset_table_.size ())
        return {};

    // Set decoder offset of object
    auto offset = offset_table_[index];
    auto current_offset = decoder_.tell ();
    decoder_.seek (offset);

    // Read object type
    std::uint8_t type = decoder_.get_uint8 ();
    auto d = type >> 4;   // high nibble
    auto n = type & 0x0F; // low nibble

    mobius::core::pod::data result;

    switch (d)
    {
        case 0:
            if (n == 0x0) // NULL
                ;

            else if (n == 0x8) // Boolean false
                result = mobius::core::pod::data (false);

            else if (n == 0x9) // Boolean true
                result = mobius::core::pod::data (true);

            break;

        case 1: // Integer
        {
            std::int64_t value = 0;

            if (n == 0)
                value = decoder_.get_int8 ();

            else if (n == 1)
                value = decoder_.get_int16_be ();

            else if (n == 2)
                value = decoder_.get_int32_be ();

            else if (n == 3)
                value = decoder_.get_int64_be ();

            result = mobius::core::pod::data (value);
        }

        break;

        case 2: // Real
        {
            long double value = 0.0;

            if (n == 0)
                value = decoder_.get_float32_be ();

            else if (n == 1)
                value = decoder_.get_float64_be ();

            result = mobius::core::pod::data (value);
        }

        break;

        case 3: // Date
        {
            auto date_value = decoder_.get_float64_be ();
            result = mobius::core::pod::data (mobius::core::datetime::new_datetime_from_cocoa_timestamp (date_value));
        }

        break;

        case 4: // Data
        {
            std::uint64_t data_length = n;

            if (n == 0xF)
                data_length = _decode_int ();

            auto data_bytes = decoder_.get_bytearray_by_size (data_length);
            result = mobius::core::pod::data (data_bytes);
        }

        break;

        case 5: // ASCII String
        {
            std::uint64_t str_length = n;

            if (n == 0xF)
                str_length = _decode_int ();

            auto str = decoder_.get_string_by_size (str_length);
            result = mobius::core::pod::data (str);
        }

        break;

        case 6: // Unicode String
        {
            std::uint64_t str_length = n;

            if (n == 0xF)
                str_length = _decode_int ();

            auto str = decoder_.get_string_by_size (str_length * 2, "UTF-16BE");
            result = mobius::core::pod::data (str);
        }

        break;

        case 7: // UTF-8 String
        {
            std::uint64_t str_length = n;

            if (n == 0xF)
                str_length = _decode_int ();

            auto str = decoder_.get_string_by_size (str_length, "UTF-8");
            result = mobius::core::pod::data (str);
        }

        break;

        case 8: // UID
        {
            std::uint64_t uid_length = n + 1;

            auto uid_bytes = decoder_.get_bytearray_by_size (uid_length);
            result = mobius::core::pod::data (uid_bytes);
        }

        break;

        case 10: // Array
        case 11: // Ordered Set
        case 12: // Set
        {
            std::uint64_t array_length = n;

            if (n == 0xF)
                array_length = _decode_int ();

            std::vector<mobius::core::pod::data> array_data;

            for (std::uint64_t i = 0; i < array_length; ++i)
            {
                auto obj_ref = _decode_object_ref ();
                auto obj_data = _decode_object (obj_ref);

                array_data.push_back (obj_data);
            }

            result = mobius::core::pod::data (array_data);
        }

        break;

        case 13: // Dictionary
        {
            std::uint64_t dict_length = n;

            if (n == 0xF)
                dict_length = _decode_int ();

            mobius::core::pod::map dict_data;
            std::vector<std::string> keys (dict_length);

            for (std::uint64_t i = 0; i < dict_length; ++i)
            {
                auto key_ref = _decode_object_ref ();
                auto key_data = _decode_object (key_ref);
                keys[i] = key_data.to_string ();
            }

            for (std::uint64_t i = 0; i < dict_length; ++i)
            {
                auto value_ref = _decode_object_ref ();
                auto value_data = _decode_object (value_ref);

                if (!keys[i].empty ())
                    dict_data.set (keys[i], value_data);
            }

            result = dict_data;
        }

        break;
    }

    decoder_.seek (current_offset);
    return result;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode bplist data
// @param reader Reader object
// @return Pod object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
decode_bplist (const mobius::core::io::reader &reader)
{
    bplist_decoder decoder (reader);
    return decoder.get_data ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode XML item element
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
decode_xmlitem (const mobius::core::decoder::xml::element &element)
{
    if (!element)
        return {};

    auto tag_name = element.get_name ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Basic types
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (tag_name == "true")
        return true;

    else if (tag_name == "false")
        return false;

    else if (tag_name == "string")
        return element.get_content ();

    else if (tag_name == "data")
        return mobius::core::decoder::base64 (element.get_content ());

    else if (tag_name == "integer")
        return static_cast<std::int64_t> (std::stoll (element.get_content ()));

    else if (tag_name == "real")
        return std::stold (element.get_content ());

    else if (tag_name == "date")
        return mobius::core::datetime::new_datetime_from_iso_string (element.get_content ());

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Array
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    else if (tag_name == "array")
    {
        std::vector<mobius::core::pod::data> array_data;
        auto children = element.get_children ();

        std::transform (
            children.begin (),
            children.end (),
            std::back_inserter (array_data),
            [] (const auto &child) { return decode_xmlitem (child); }
        );

        return array_data;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Dict type
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    else if (tag_name == "dict")
    {
        mobius::core::pod::map dict_data;
        auto children = element.get_children ();

        for (std::size_t i = 0; i + 1 < children.size (); i += 2)
        {
            auto key_element = children[i];
            auto value_element = children[i + 1];

            if (key_element && key_element.get_name () == "key" && value_element)
            {
                auto key = key_element.get_content ();
                auto value = decode_xmlitem (value_element);

                dict_data.set (key, value);
            }
        }

        return dict_data;
    }

    return {};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode XML plist
// @param reader Reader object
// @return Decoded data
// @see https://en.wikipedia.org/wiki/Property_list
// @see https://javorszky.co.uk/2023/11/09/what-the-hell-is-true-in-a-plist/
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
decode_xmlplist (const mobius::core::io::reader &reader)
{
    try
    {
        auto dom = mobius::core::decoder::xml::dom (reader);
        auto root = dom.get_root_element ();

        if (!root || root.get_name () != "plist")
            return {};

        // The root element of a plist should contain exactly one child element, which is always a <dict> element.
        auto children = root.get_children ();

        if (children.size () != 1 || children[0].get_name () != "dict")
            return {};

        return decode_xmlitem (children[0]);
    }
    catch (const std::exception &e)
    {
    }

    return {};
}

} // namespace

namespace mobius::core::decoder
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode plist data
// @param reader Reader object
// @return Pod object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
plist (const mobius::core::io::reader &reader)
{
    mobius::core::pod::data data;

    // Read first 8 bytes to determine if it's a bplist or XML plist
    auto r = reader;
    auto bytes = r.read (8);
    r.rewind ();

    if (bytes.startswith ("bplist0"))
        data = decode_bplist (reader);

    else if (bytes.startswith ("<?xml"))
        data = decode_xmlplist (reader);

    return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode plist data
// @param data Data
// @return Pod object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
plist (const mobius::core::bytearray &data)
{
    return plist (mobius::core::io::new_bytearray_reader (data));
}

} // namespace mobius::core::decoder
