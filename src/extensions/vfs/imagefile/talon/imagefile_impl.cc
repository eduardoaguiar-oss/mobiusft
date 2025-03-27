// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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
#include "imagefile_impl.h"
#include <mobius/vfs/imagefile.h>
#include <mobius/vfs/util.h>
#include <mobius/string_functions.h>
#include <mobius/exception.inc>
#include <mobius/charset.h>
#include <stdexcept>
#include <regex>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file is an instance of imagefile talon
// @param f File object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
imagefile_impl::is_instance (const mobius::io::file& f)
{
  bool instance = false;

  if (f && f.exists ())
    {
      auto reader = f.new_reader ();

      if (reader)
        {
          mobius::bytearray data = reader.read (128);
          const std::string text = conv_charset_to_utf8 (data, "ASCII");

          instance = text.find ("FORENSIC TALON") != std::string::npos ||
                     text.find ("LOGICUBE TALON") != std::string::npos;
        }
    }

  return instance;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Construct object
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile_impl::imagefile_impl (const mobius::io::file& f)
  : file_ (f),
    split_imagefile_impl_ (
       mobius::vfs::build_imagefile_implementation (
          f.new_sibling_by_extension ("001"),
          "split"
       )
    )
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get attribute
// @param name Attribute name
// @return Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::pod::data
imagefile_impl::get_attribute (const std::string& name) const
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
imagefile_impl::set_attribute (
  const std::string&,
  const mobius::pod::data&
)
{
  throw std::runtime_error (mobius::MOBIUS_EXCEPTION_MSG ("set_attribute not implemented"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get attributes
// @return Attributes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::pod::map
imagefile_impl::get_attributes () const
{
  _load_metadata ();
  return attributes_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new reader for imagefile
// @return Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::io::reader
imagefile_impl::new_reader () const
{
  return split_imagefile_impl_->new_reader ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new writer for imagefile
// @return Writer object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::io::writer
imagefile_impl::new_writer () const
{
  throw std::runtime_error (mobius::MOBIUS_EXCEPTION_MSG ("writer not implemented"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief load metadata on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
imagefile_impl::_load_metadata () const
{
  if (metadata_loaded_)
    return;

  // Check if imagefile exists
  constexpr mobius::io::file::size_type LOG_MAX_SIZE = 65536;

  if (!file_ || !file_.exists ())
    throw std::runtime_error (mobius::MOBIUS_EXCEPTION_MSG ("Image file not found"));

  if (file_.get_size () > LOG_MAX_SIZE)
    throw std::runtime_error (mobius::MOBIUS_EXCEPTION_MSG ("Image file control file too large"));

  // Load metadata
  std::regex REGEX_TALON_SERIAL ("Serial No.: ?([0-9]+)");
  std::regex REGEX_TALON_SOFTWARE ("Software: ([A-Z0-9.]+)");
  std::regex REGEX_DRIVE_MODEL ("Drive Model: ([^*]+?) *\\*");
  std::regex REGEX_DRIVE_SERIAL ("Serial: ([0-9A-Z-]+) *\\*");
  std::regex REGEX_TOTAL_SECTORS ("Drive Size *\\*\r\n\\* *[0-9]+ *[0-9]+ *[0-9]+ *([0-9]+)");

  // parse .log file
  auto reader = file_.new_reader ();
  mobius::bytearray data = reader.read (file_.get_size ());
  const std::string text = conv_charset_to_utf8 (data, "ASCII");
  std::smatch match;

  std::string drive_vendor;
  std::string drive_model;
  std::string drive_serial_number;
  std::string acquisition_platform;
  std::string acquisition_tool;

  if (std::regex_search (text, match, REGEX_TALON_SERIAL))
    acquisition_platform = "Logicube Talon (s/n: " + match[1].str () + ')';

  if (std::regex_search (text, match, REGEX_TALON_SOFTWARE))
    acquisition_tool = "Talon software " + match[1].str ();

  if (std::regex_search (text, match, REGEX_DRIVE_MODEL))
    drive_model = mobius::string::strip (match[1].str ());

  if (std::regex_search (text, match, REGEX_DRIVE_SERIAL))
    drive_serial_number = match[1].str ();

  if (std::regex_search (text, match, REGEX_TOTAL_SECTORS))
    {
      sectors_ = stoll (match[1].str ());
      size_ = sectors_ * sector_size_;
    }

  size_type segments = std::int64_t (split_imagefile_impl_->get_attribute ("segments"));
  size_type segment_size = std::int64_t (split_imagefile_impl_->get_attribute ("segment_size"));
  std::string acquisition_user = file_.get_user_name ();
  mobius::datetime::datetime acquisition_time = file_.get_modification_time ();

  mobius::vfs::normalize_drive_info (drive_vendor, drive_model, drive_serial_number);

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

  // set metadata loaded
  metadata_loaded_ = true;
}


