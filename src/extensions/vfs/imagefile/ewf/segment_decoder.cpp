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
#include "segment_decoder.hpp"
#include <mobius/core/charset.hpp>
#include <mobius/core/crypt/hash_functor.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/io/reader_evaluator.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/core/zlib_functions.hpp>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr int FILE_HEADER_SIZE = 13;
static constexpr int SECTION_HEADER_SIZE = 76;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert an Adler-32 digest from bytearray to uint32_t
// @param digest bytearray digest (4 bytes length)
// @return std::uint32_t
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::uint32_t
digest_to_uint32_t (const mobius::core::bytearray &digest)
{
    return std::uint32_t (digest[0]) << 24 | std::uint32_t (digest[1]) << 16 |
           std::uint32_t (digest[2]) << 8 | std::uint32_t (digest[3]);
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief iterator constructor
// @param reader reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
segment_decoder::const_iterator::const_iterator (const segment_decoder *decoder,
                                                 offset_type offset)
    : decoder_ (decoder)
{
    section_ = decoder_->decode_section (offset);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if two iterators are equal
// @param i iterator
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
segment_decoder::const_iterator::operator== (const const_iterator &i)
{
    return section_.get_offset () == i.section_.get_offset ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief operator++
// @return reference to object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
segment_decoder::const_iterator &
segment_decoder::const_iterator::operator++ ()
{
    auto offset = section_.get_offset ();
    auto next_offset = section_.get_next_offset ();

    if (offset == next_offset)
        section_ = section ();

    else
        section_ = decoder_->decode_section (next_offset);

    return *this;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @see EWCF 2.1.1
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
segment_decoder::segment_decoder (mobius::core::io::reader reader)
    : reader_ (reader)
{
    const mobius::core::bytearray EWF_SIGNATURE = {'E',  'V',  'F',  0x09,
                                                   0x0d, 0x0a, 0xff, 0x00};

    mobius::core::decoder::data_decoder decoder (reader);
    mobius::core::bytearray signature =
        decoder.get_bytearray_by_size (EWF_SIGNATURE.size ());

    if (signature == EWF_SIGNATURE)
    {
        decoder.skip (1);
        segment_number_ = decoder.get_uint32_le ();
        is_valid_ = true;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get first section
// @return section
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
segment_decoder::const_iterator
segment_decoder::begin () const
{
    return const_iterator (this, FILE_HEADER_SIZE);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get ending section
// @return section
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
segment_decoder::const_iterator
segment_decoder::end () const
{
    return const_iterator ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode generic section
// @param offset offset from the beginning of data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
section
segment_decoder::decode_section (offset_type offset) const
{
    mobius::core::crypt::hash_functor hash_functor ("adler32");
    auto reader = mobius::core::io::reader_evaluator (reader_, hash_functor);

    mobius::core::decoder::data_decoder decoder (reader);
    decoder.seek (offset);

    section sec;
    sec.set_offset (offset);
    sec.set_name (decoder.get_string_by_size (16));
    sec.set_next_offset (decoder.get_uint64_le ());
    sec.set_size (decoder.get_uint64_le ());
    decoder.get_bytearray_by_size (40); // padding

    std::uint32_t calculated_adler32 =
        digest_to_uint32_t (hash_functor.get_digest ());
    sec.set_calculated_adler32 (calculated_adler32);

    sec.set_adler32 (decoder.get_uint32_le ());

    return sec;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode <header> and <header2> sections
// @return header_section
// @see EWCF 3.4
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
header_section
segment_decoder::decode_header_section (const section &arg_section) const
{
    mobius::core::decoder::data_decoder decoder (reader_);
    decoder.seek (arg_section.get_offset () + SECTION_HEADER_SIZE);

    // create header_section
    header_section section (arg_section);

    // get data from header section
    mobius::core::bytearray data = decoder.get_bytearray_by_size (
        section.get_size () - SECTION_HEADER_SIZE);
    data = mobius::core::zlib_decompress (data);

    const std::string text = mobius::core::conv_charset_to_utf8 (
        data, section.get_name () == "header2" ? "UTF-16" : "ASCII");

    // format header metadata lines
    auto lines = mobius::core::string::split (text, "\n");
    for (auto &line : lines)
        line = mobius::core::string::rstrip (line, "\r ");

    // check valid header
    if (lines.size () > 3 && lines[1] == "main")
    {
        auto vars = mobius::core::string::split (lines[2], "\t");
        auto values = mobius::core::string::split (lines[3], "\t");
        std::string section_text;

        for (std::size_t i = 0; i < vars.size (); i++)
        {
            auto var = vars[i];
            auto value = values[i];

            if (!section_text.empty ())
                section_text += '\n';
            section_text += var + " = " + value;

            if (var == "ov")
                section.set_acquisition_platform (value);

            else if (var == "e")
                section.set_acquisition_user (value);

            else if (var == "md")
                section.set_drive_model (value);

            else if (var == "sn")
                section.set_drive_serial_number (value);

            else if (var == "av")
            {
                if (value.length () > 0 && isdigit (value[0]))
                    section.set_acquisition_tool ("Encase v" + value);
                else
                    section.set_acquisition_tool (value);
            }

            else if (var == "m")
            {
                if (value.find (' ') != std::string::npos)
                {
                    auto d = mobius::core::string::split (value);
                    section.set_acquisition_time (
                        mobius::core::datetime::datetime (
                            stoi (d[0]), stoi (d[1]), stoi (d[2]), stoi (d[3]),
                            stoi (d[4]), stoi (d[5])));
                }
                else
                    section.set_acquisition_time (
                        mobius::core::datetime::
                            new_datetime_from_unix_timestamp (stol (value)));
            }
        }
        section.set_text (section_text);
    }

    return section;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode <hash> section
// @param arg_section generic section
// @return hash_section
// @see EWCF 3.18
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hash_section
segment_decoder::decode_hash_section (const section &arg_section) const
{
    mobius::core::decoder::data_decoder decoder (reader_);
    decoder.seek (arg_section.get_offset () + SECTION_HEADER_SIZE);

    hash_section section (arg_section);
    section.set_md5_hash (decoder.get_bytearray_by_size (16).to_hexstring ());
    decoder.skip (16);
    section.set_section_adler32 (decoder.get_uint32_le ());

    return section;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode <volume>, <disk> and <data> sections
// @param arg_section generic section
// @return volume_section
// @see EWCF 3.5
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
volume_section
segment_decoder::decode_volume_section (const section &arg_section) const
{
    mobius::core::decoder::data_decoder decoder (reader_);
    decoder.seek (arg_section.get_offset () + SECTION_HEADER_SIZE);

    volume_section section (arg_section);
    section.set_media_type (decoder.get_uint32_le ());
    section.set_chunk_count (decoder.get_uint32_le ());
    section.set_chunk_sectors (decoder.get_uint32_le ());
    section.set_sector_size (decoder.get_uint32_le ());
    section.set_sectors (decoder.get_uint64_le ());
    decoder.skip (12); // skip CHS data
    section.set_media_flags (decoder.get_uint32_le ());
    decoder.skip (12);
    section.set_compression_level (decoder.get_uint32_le ());
    decoder.skip (8);
    section.set_guid (decoder.get_bytearray_by_size (16).to_hexstring ());
    decoder.skip (968);
    section.set_section_adler32 (decoder.get_uint32_le ());

    return section;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode <table> section
// @param arg_section generic section
// @return table_section
// @see EWCF 3.9
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
table_section
segment_decoder::decode_table_section (const section &arg_section) const
{
    mobius::core::decoder::data_decoder decoder (reader_);
    decoder.seek (arg_section.get_offset () + SECTION_HEADER_SIZE);

    table_section section (arg_section);
    const auto chunk_count = decoder.get_uint32_le ();
    section.set_chunk_count (chunk_count);
    decoder.skip (4);
    section.set_base_offset (decoder.get_uint64_le ());
    decoder.skip (4);
    section.set_section_adler32 (decoder.get_uint32_le ());

    auto data = decoder.get_bytearray_by_size (4 * chunk_count);

    for (std::uint32_t i = 0; i < chunk_count * 4; i += 4)
    {
        auto offset = std::uint32_t (data[i]) |
                      (std::uint32_t (data[i + 1]) << 8) |
                      (std::uint32_t (data[i + 2]) << 16) |
                      (std::uint32_t (data[i + 3]) << 24);

        section.add_offset (offset);
    }

    section.set_table_adler32 (decoder.get_uint32_le ());

    return section;
}
