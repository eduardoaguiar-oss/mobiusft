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
#include <mobius/core/vfs/imagefile.hpp>
#include <mobius/core/vfs/imagefile_impl_null.hpp>
#include <mobius/core/resource.hpp>
#include <mobius/exception.inc>
#include <mobius/io/file.h>
#include <mobius/string_functions.h>
#include <stdexcept>

namespace mobius::core::vfs
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Construct object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile::imagefile ()
{
  impl_ = std::make_shared <imagefile_impl_null> ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor from implementation pointer
// @param impl implementation pointer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile::imagefile (const std::shared_ptr <imagefile_impl_base>& impl)
  : impl_ (impl)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor from file object
// @param f File object
// @param type Imagefile type (default = "autodetect")
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile::imagefile (const mobius::io::file& f, const std::string& type)
  : impl_ (build_imagefile_implementation (f, type))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get metadata
// @return imagefile metadata
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::metadata
imagefile::get_metadata () const
{
  // imagefile metadata
  mobius::core::metadata metadata =
  {
    {
      "type",
      "Type",
      "std::string",
      get_type ()
    },
    {
      "size",
      "Size",
      "size_type",
      std::to_string (get_size ()) + " bytes"
    },
    {
      "sectors",
      "Number of sectors",
      "size_type",
      std::to_string (get_sectors ())
    },
    {
      "sector_size",
      "Sector size",
      "size_type",
      std::to_string (get_sector_size ()) + " bytes"
    }
  };

  // implementation specific metadata
  for (const auto& p : get_attributes ())
    {
      auto description = mobius::string::capitalize (mobius::string::replace (p.first, "_", " "));
      metadata.add (p.first, description, "std::string", p.second.to_string ());
    }

  return metadata;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Build implementation, according to ID
// @param f File object
// @param id Implementation ID
// @return shared_ptr to implementation object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr <imagefile_impl_base>
build_imagefile_implementation (const mobius::io::file& f, const std::string& id)
{
  // If type == "autodetect", use f_is_instance function to check if file
  // is an instance of this imagefile type.
  if (id == "autodetect")
    {
      for (const auto& resource : mobius::core::get_resources ("vfs.imagefile"))
        {
          auto img_resource = resource.get_value <imagefile_resource_type> ();

          if (img_resource.is_instance (f))
            {
              // Use f_builder function to create imagefile implementation from
              // file <i>f</i>
              return img_resource.build (f);
            }
        }

      // fallback: raw imagefile
      return build_imagefile_implementation (f, "raw");
    }

  // Otherwise, if type is given, create imagefile using type implementation
  else
    {
      auto img_resource = mobius::core::get_resource_value <imagefile_resource_type> ("vfs.imagefile." + id);
      return img_resource.build (f);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new imagefile object by URL
// @param url Imagefile URL
// @param type Imagefile type (default = "autodetect")
// @return Imagefile object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile
new_imagefile_by_url (const std::string& url, const std::string& type)
{
  auto f = mobius::io::new_file_by_url (url);
  return imagefile (f, type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new imagefile object by path
// @param path Imagefile path
// @param type Imagefile type (default = "autodetect")
// @return Imagefile object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile
new_imagefile_by_path (const std::string& path, const std::string& type)
{
  auto f = mobius::io::new_file_by_path (path);
  return imagefile (f, type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new imagefile object from file
// @param f File object
// @param type Imagefile type (default = "autodetect")
// @return Imagefile object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
imagefile
new_imagefile_from_file (const mobius::io::file& f, const std::string& type)
{
  return imagefile (f, type);
}

} // namespace mobius::core::vfs


