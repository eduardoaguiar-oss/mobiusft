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
#include "imagefile_impl.hpp"
#include <cctype>
#include <mobius/core/charset.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/string_functions.hpp>
#include <mobius/core/vfs/imagefile.hpp>
#include <mobius/core/vfs/util.hpp>
#include <regex>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file is an instance of imagefile solo
// @param f File object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
imagefile_impl::is_instance (const mobius::core::io::file &f)
{
    bool instance = false;

    if (f && f.exists ())
    {
        auto reader = f.new_reader ();

        if (reader)
        {
            mobius::core::bytearray data = reader.read (14);
            instance = data == "[SEIZE HEADER]";
        }
    }

    return instance;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Construct object
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile_impl::imagefile_impl (const mobius::core::io::file &f)
    : file_ (f),
      split_imagefile_impl_ (mobius::core::vfs::build_imagefile_implementation (
          f.new_sibling_by_extension ("001"), "split"))
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
// @return Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::reader
imagefile_impl::new_reader () const
{
    return split_imagefile_impl_->new_reader ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new writer for imagefile
// @return Writer object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::writer
imagefile_impl::new_writer () const
{
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("writer not implemented"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load metadata on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
imagefile_impl::_load_metadata () const
{
    if (metadata_loaded_)
        return;

    // Check if imagefile exists
    constexpr mobius::core::io::file::size_type LOG_MAX_SIZE = 1048576; // 1 MiB

    if (!file_ || !file_.exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("Image file not found"));

    if (file_.get_size () > LOG_MAX_SIZE)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("Image file control file too large"));

    // Load metadata
    std::regex REGEX_SEIZE_TIME (
        "\\nSeize Time = *([0-9]+):([0-9]+):([0-9]+)\\n");
    std::regex REGEX_SEIZE_DATE (
        "\\nSeize Date = *([0-9]+)/([0-9]+)/([0-9]+)\\n");
    std::regex REGEX_TOTAL_SECTORS ("\\nSeize Size = ([0-9]+)\\n");
    std::regex REGEX_MD5 ("\\nMD5: (.+)\\n");
    std::regex REGEX_SUSPECT (
        "Suspect: Model: ([^.]+?)\\. Serial Number: ([^.]+?)\\..*Block size: "
        "([0-9]+)\\..*\\. Evidence");
    std::regex REGEX_SOLO_PRODUCT ("\\nProduct = (.+)\\n");
    std::regex REGEX_SOLO_SERIAL ("\\nSerial #: (.+)\\n");
    std::regex REGEX_SOLO_SOFTWARE ("\\nSoftware Version (.+)\\n");

    // parse .txt file
    auto reader = file_.new_reader ();
    mobius::core::bytearray data = reader.read (file_.get_size ());
    const std::string text = mobius::core::conv_charset_to_utf8 (data, "ASCII");
    std::smatch match;

    std::string acquisition_platform;
    std::string acquisition_tool;
    mobius::core::datetime::date acquisition_date;
    mobius::core::datetime::datetime acquisition_time;
    std::string product;
    std::string serial;
    std::string drive_vendor;
    std::string drive_model;
    std::string drive_serial_number;
    std::string hash_md5;

    if (std::regex_search (text, match, REGEX_SUSPECT))
    {
        drive_model = match[1].str ();
        drive_serial_number = match[2].str ();
        sector_size_ = stoi (match[3].str ());
    }

    if (std::regex_search (text, match, REGEX_SEIZE_DATE))
    {
        acquisition_date = mobius::core::datetime::date (
            stoi (match[3].str ()), stoi (match[1].str ()),
            stoi (match[2].str ()));
    }

    if (std::regex_search (text, match, REGEX_SEIZE_TIME))
    {
        mobius::core::datetime::time time = mobius::core::datetime::time (
            stoi (match[1].str ()), stoi (match[2].str ()),
            stoi (match[3].str ()));

        if (acquisition_date)
            acquisition_time =
                mobius::core::datetime::datetime (acquisition_date, time);
    }

    if (std::regex_search (text, match, REGEX_TOTAL_SECTORS))
    {
        sectors_ = stoll (match[1].str ());
        size_ = sectors_ * sector_size_;
    }

    if (std::regex_search (text, match, REGEX_MD5))
    {
        std::string value =
            mobius::core::string::remove_char (match[1].str (), ' ');
        hash_md5 = mobius::core::string::tolower (value);
    }

    if (std::regex_search (text, match, REGEX_SOLO_PRODUCT))
        product = match[1].str ();

    if (std::regex_search (text, match, REGEX_SOLO_SERIAL) && !product.empty ())
        acquisition_platform = product + " (s/n: " + match[1].str () + ')';

    if (std::regex_search (text, match, REGEX_SOLO_SOFTWARE))
        acquisition_tool = "Solo software v" + match[1].str ();

    size_type segments =
        std::int64_t (split_imagefile_impl_->get_attribute ("segments"));
    size_type segment_size =
        std::int64_t (split_imagefile_impl_->get_attribute ("segment_size"));
    std::string acquisition_user = file_.get_user_name ();

    mobius::core::vfs::normalize_drive_info (drive_vendor, drive_model,
                                             drive_serial_number);

    // fill attributes
    attributes_.set ("drive_vendor", drive_vendor);
    attributes_.set ("drive_model", drive_model);
    attributes_.set ("drive_serial_number", drive_serial_number);
    attributes_.set ("segments", segments);
    attributes_.set ("segment_size", segment_size);
    attributes_.set ("acquisition_time", acquisition_time);
    attributes_.set ("acquisition_tool", acquisition_tool);
    attributes_.set ("acquisition_platform", acquisition_platform);
    attributes_.set ("acquisition_user", acquisition_user);
    attributes_.set ("hash_md5", hash_md5);

    // set metadata loaded
    metadata_loaded_ = true;
}
