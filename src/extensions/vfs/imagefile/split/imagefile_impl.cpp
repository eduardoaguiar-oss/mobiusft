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
#include "imagefile_impl.hpp"
#include "reader_impl.hpp"
#include "writer_impl.hpp"
#include <mobius/core/exception.inc>
#include <mobius/core/io/reader.hpp>
#include <mobius/core/io/writer.hpp>
#include <mobius/core/io/uri.hpp>
#include <mobius/core/string_functions.hpp>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if file is an instance of imagefile split
// @param f File object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
imagefile_impl::is_instance (const mobius::core::io::file& f)
{
  return f.get_extension () == "001";
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Construct object
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile_impl::imagefile_impl (const mobius::core::io::file& f)
  : file_ (f),
    segments_ (f, [](mobius::core::vfs::segment_array::idx_type idx){
                    return mobius::core::string::to_string (idx + 1, 3);
                  })
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get attribute
// @param name Attribute name
// @return Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
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
  const std::string& name,
  const mobius::core::pod::data& value
)
{
  attributes_.set (name, value);
  metadata_loaded_ = true;
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
  return mobius::core::io::reader (std::make_shared <reader_impl> (*this));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new writer for imagefile
// @return Writer object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::writer
imagefile_impl::new_writer () const
{
  return mobius::core::io::writer (std::make_shared <writer_impl> (*this));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load metadata
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
imagefile_impl::_load_metadata () const
{
  if (metadata_loaded_)
    return;

  // Check if first segment exists
  if (!file_ || !file_.exists ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("Image file not found"));

  // Scan segments
  segments_.scan ();

  // Set metadata
  size_ = segments_.get_data_size ();
  sectors_ = (size_ + sector_size_ - 1) / sector_size_;

  // fill attributes
  attributes_.set ("segments", segments_.get_size ());
  attributes_.set ("segment_size", file_.get_size ());
  attributes_.set ("acquisition_user", file_.get_user_name ());
  attributes_.set ("acquisition_time", file_.get_modification_time ());

  // set metadata loaded
  metadata_loaded_ = true;
}


