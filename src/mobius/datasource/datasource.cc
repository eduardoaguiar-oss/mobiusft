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
#include "datasource.h"
#include "datasource_impl_null.h"
#include "datasource_impl_vfs.h"
#include "ufdr/datasource_impl.h"
#include <mobius/exception.inc>
#include <mobius/io/uri.h>
#include <stdexcept>

namespace mobius::datasource
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource::datasource ()
 : impl_ (std::make_shared <datasource_impl_null> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor from implementation pointer
// @param impl Implementation pointer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource::datasource (const std::shared_ptr <datasource_impl_base>& impl)
  : impl_ (impl)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param state Object state
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource::datasource (const mobius::pod::map& state)
{
  auto type = state.get <std::string> ("type");

  if (type == "ufdr")
    impl_ = std::make_shared <ufdr::datasource_impl> (state);

  else if (type == "vfs")
    impl_ = std::make_shared <datasource_impl_vfs> (state);

  else
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid datasource type: " + type));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create datasource from VFS object
// @param vfs VFS object
// @return Datasource object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource
new_datasource_from_vfs (const mobius::vfs::vfs& vfs)
{
  return datasource (std::make_shared <datasource_impl_vfs> (vfs));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create datasource from image file
// @param path Image file path
// @return Datasource object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource
new_datasource_by_imagefile_path (const std::string& path)
{
  mobius::vfs::vfs vfs;
  vfs.add_disk (mobius::vfs::new_disk_by_path (path));

  return new_datasource_from_vfs (vfs);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create datasource from image file
// @param url Image file URL
// @return Datasource object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource
new_datasource_by_imagefile_url (const std::string& url)
{
  mobius::vfs::vfs vfs;
  vfs.add_disk (mobius::vfs::new_disk_by_url (url));

  return new_datasource_from_vfs (vfs);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create datasource from UFDR file path
// @param path UFDR file path
// @return Datasource object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource
new_datasource_by_ufdr_path (const std::string& path)
{
  auto uri = mobius::io::new_uri_from_path (path);
  return mobius::datasource::new_datasource_by_ufdr_url (uri.get_value ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create datasource from UFDR file URL
// @param url UFDR file URL
// @return Datasource object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource
new_datasource_by_ufdr_url (const std::string& url)
{
  return datasource (std::make_shared <ufdr::datasource_impl> (url));
}

} // namespace mobius::datasource


