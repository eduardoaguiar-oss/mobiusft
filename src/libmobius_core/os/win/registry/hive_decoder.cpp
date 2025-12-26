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
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/os/win/registry/hive_decoder.hpp>

namespace mobius::core::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::uint32_t INVALID_OFFSET = 0xffffffff;
static constexpr std::uint32_t HIVE_BASE_OFFSET = 4096;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get absolute offset
// @param offset relative offset
// @return absolute offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::uint32_t
get_offset (mobius::core::decoder::data_decoder &decoder)
{
    auto offset = decoder.get_uint32_le ();

    if (offset != INVALID_OFFSET)
        offset += HIVE_BASE_OFFSET;

    return offset;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// @param reader generic reader
// @see http://en.wikipedia.org/wiki/Windows_Registry
// @see http://www.sentinelchicken.com/data/TheWindowsNTRegistryFileFormat.pdf
// @see
// http://github.com/libyal/libregf/blob/master/documentation/Windows%20NT%20Registry%20File%20%28REGF%29%20format.asciidoc
// @see
// http://github.com/msuhanov/regf/blob/master/Windows%20registry%20file%20format%20specification.md
// @see https://binaryforay.blogspot.com.br/2015/01/registry-hive-basics.html
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hive_decoder::hive_decoder (mobius::core::io::reader reader)
    : reader_ (reader)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief decode hivefile header
// @return header_data structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
header_data
hive_decoder::decode_header ()
{
    header_data data;

    reader_.seek (0);
    mobius::core::decoder::data_decoder decoder (reader_);

    data.signature = decoder.get_string_by_size (4);
    data.sequence_1 = decoder.get_uint32_le ();
    data.sequence_2 = decoder.get_uint32_le ();
    data.mtime = decoder.get_nt_datetime ();
    data.major_version = decoder.get_uint32_le ();
    data.minor_version = decoder.get_uint32_le ();
    data.file_type = decoder.get_uint32_le ();
    data.file_format = decoder.get_uint32_le ();
    data.root_offset = get_offset (decoder);
    data.hbin_data_size = decoder.get_int32_le ();
    data.disk_sector_size = decoder.get_uint32_le () << 9;
    data.filename = decoder.get_string_by_size (64, "UTF-16LE");
    data.rm_guid = decoder.get_guid ();
    data.log_guid = decoder.get_guid ();
    data.flags = decoder.get_uint32_le ();
    data.tm_guid = decoder.get_guid ();
    data.guid_signature = decoder.get_string_by_size (4);
    data.rtime = decoder.get_nt_datetime ();

    // header checksum
    reader_.seek (508);
    data.header_checksum = decoder.get_uint32_le ();

    return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief decode vk structure
// @return vk_data structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
vk_data
hive_decoder::decode_vk (offset_type offset)
{
    vk_data data;

    // check offset
    if (offset == INVALID_OFFSET)
        return data;

    // create decoder
    reader_.seek (offset);
    mobius::core::decoder::data_decoder decoder (reader_);

    // get cell size
    std::int32_t cellsize = decoder.get_int32_le ();

    if (cellsize >= 0)
        return data;

    // decoder data
    data.signature = decoder.get_string_by_size (2);

    // name length in bytes
    std::uint16_t name_length = decoder.get_uint16_le ();

    // data size
    data.data_size = decoder.get_uint32_le ();

    // data offset
    // If the MSB of data_size is 1, data is stored directly in the data offset
    // field
    constexpr std::uint32_t HIVE_DATA_IN_OFFSET = 0x80000000;

    if (data.data_size & HIVE_DATA_IN_OFFSET)
        data.data_offset = decoder.get_uint32_le ();
    else
        data.data_offset = get_offset (decoder);

    data.data_type = decoder.get_uint32_le ();

    // flags
    data.flags = decoder.get_uint16_le ();

    // probably not used (spare)
    decoder.skip (2);

    // set name
    constexpr std::uint16_t VALUE_COMP_NAME = 0x0001;
    std::string encoding =
        (data.flags & VALUE_COMP_NAME) ? "CP1252" : "UTF-16LE";
    data.name = decoder.get_string_by_size (name_length, encoding);

    return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief decode subkeys list
// @return vector of offsets
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<hive_decoder::offset_type>
hive_decoder::decode_subkeys_list (offset_type offset)
{
    std::vector<offset_type> offsets;
    _retrieve_subkeys (offsets, offset);

    return offsets;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief decode values list
// @return vector of offsets
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<hive_decoder::offset_type>
hive_decoder::decode_values_list (offset_type offset, std::uint32_t count)
{
    std::vector<offset_type> offsets;

    // check offset
    if (offset == INVALID_OFFSET)
        return offsets;

    // create decoder
    reader_.seek (offset);
    mobius::core::decoder::data_decoder decoder (reader_);

    // get cell size
    std::int32_t cellsize = decoder.get_int32_le ();

    if (cellsize >= 0)
        return offsets;

    // retrieve values' offsets
    for (std::uint32_t i = 0; i < count; i++)
    {
        std::uint32_t vk_offset = get_offset (decoder);

        if (vk_offset != INVALID_OFFSET)
            offsets.push_back (vk_offset);
    }

    return offsets;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief retrieve subkeys from list, recursively
// @param offsets offset vector
// @param offset list offset
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hive_decoder::_retrieve_subkeys (std::vector<offset_type> &offsets,
                                 offset_type offset)
{
    // check offset
    if (offset == INVALID_OFFSET)
        return;

    // create decoder
    reader_.seek (offset);
    mobius::core::decoder::data_decoder decoder (reader_);

    // get cell size
    std::int32_t cellsize = decoder.get_int32_le ();

    if (cellsize >= 0)
        return;

    // get cell signature
    std::string signature = decoder.get_string_by_size (2);
    std::uint16_t count = decoder.get_uint16_le ();

    // index leaf
    if (signature == "li")
    {
        for (auto i = 0; i < count; i++)
        {
            std::uint32_t nk_offset = get_offset (decoder);

            if (nk_offset != INVALID_OFFSET)
                offsets.push_back (nk_offset);
        }
    }

    // fast leaf or hash leaf
    else if (signature == "lf" || signature == "lh")
    {
        for (auto i = 0; i < count; i++)
        {
            std::uint32_t nk_offset = get_offset (decoder);
            decoder.skip (4); // name or hash

            if (nk_offset != INVALID_OFFSET)
                offsets.push_back (nk_offset);
        }
    }

    // index root (list of subkeys lists)
    else if (signature == "ri")
    {
        std::vector<offset_type> ri_offsets;
        ri_offsets.reserve (count);

        for (auto i = 0; i < count; i++)
        {
            std::int32_t l_offset = get_offset (decoder);
            ri_offsets.push_back (l_offset);
        }

        // generate subkeys
        for (auto l_offset : ri_offsets)
            _retrieve_subkeys (offsets, l_offset);
    }

    else
    {
        ; // error
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief decode value's data
// @param offset offset in bytes
// @param size data size
// @return data
// @see
// https://binaryforay.blogspot.com.br/2015/08/registry-hive-basics-part-5-lists.html
//! \todo big data (db)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
hive_decoder::decode_data (offset_type offset, std::uint32_t size)
{
    mobius::core::bytearray data;

    // check offset
    if (offset == INVALID_OFFSET)
        return data;

    // decode cell
    reader_.seek (offset);
    mobius::core::decoder::data_decoder decoder (reader_);
    std::int32_t cellsize = decoder.get_int32_le ();

    if (cellsize >= 0)
        return data;

    cellsize = -cellsize - 4;

    // if data can be read from cell at once, read it all
    if (cellsize >= std::int32_t (size))
    {
        data = decoder.get_bytearray_by_size (size);
    }

    else if (cellsize > 1) // otherwise, check for 'db' list
    {
        std::string signature = decoder.get_string_by_size (2);

        if (signature == "db")
        {
            data = decode_data_db (offset);
            data.resize (size);
        }
    }

    return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief decode data cell
// @param offset offset in bytes
// @param size data size
// @return data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
hive_decoder::decode_data_cell (offset_type offset)
{
    mobius::core::bytearray data;

    if (offset == INVALID_OFFSET)
        return data;

    // create decoder
    reader_.seek (offset);
    mobius::core::decoder::data_decoder decoder (reader_);

    // get cell size
    std::int32_t cellsize = decoder.get_int32_le ();

    if (cellsize < 0)
        data = decoder.get_bytearray_by_size (-cellsize - 4);

    // return data
    return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief decode big data cell
// @param offset offset in bytes
// @param size data size
// @return data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
hive_decoder::decode_data_db (offset_type offset)
{
    mobius::core::bytearray data;

    if (offset == INVALID_OFFSET)
        return data;

    // decode "db" cell
    reader_.seek (offset);
    mobius::core::decoder::data_decoder decoder (reader_);

    std::int32_t cellsize = decoder.get_int32_le ();
    if (cellsize >= 0)
        return data;

    std::string signature = decoder.get_string_by_size (2);
    std::uint16_t count = decoder.get_uint16_le ();
    std::uint32_t l_offset = get_offset (decoder);

    if (l_offset == INVALID_OFFSET)
        return data;

    // decode offset list cell
    reader_.seek (l_offset);
    cellsize = decoder.get_int32_le ();

    if (cellsize >= 0)
        return data;

    std::vector<offset_type> offsets;
    offsets.reserve (count);

    for (std::uint32_t i = 0; i < count; i++)
        offsets.push_back (get_offset (decoder));

    // read data
    for (auto offset : offsets)
        data += decode_data_cell (offset);

    // return data
    return data;
}

} // namespace mobius::core::os::win::registry
