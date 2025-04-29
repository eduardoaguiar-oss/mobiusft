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
#include <mobius/core/vfs/disk.hpp>
#include <mobius/core/vfs/disk_impl_null.hpp>
#include <mobius/core/vfs/disk_impl_device.hpp>
#include <mobius/core/vfs/disk_impl_imagefile.hpp>
#include <mobius/exception.inc>
#include <mobius/io/uri.h>
#include <stdexcept>

namespace mobius::core::vfs
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
disk::disk ()
 : impl_ (std::make_shared <disk_impl_null> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor from implementation pointer
// @param impl Implementation pointer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
disk::disk (const std::shared_ptr <disk_impl_base>& impl)
  : impl_ (impl)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param state Object state
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
disk::disk (const mobius::core::pod::map& state)
{
  auto type = std::string (state.get ("type"));

  if (type == "device")
    impl_ = std::make_shared <disk_impl_device> (state);

  else if (type == "imagefile")
    impl_ = std::make_shared <disk_impl_imagefile> (state);

  else
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid disk type: " + type));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create device disk
// @param uid Device UID
// @return Disk object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
disk
new_disk_by_device_uid (const std::string& uid)
{
  return disk (std::make_shared <disk_impl_device> (uid));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create imagefile disk
// @param url Imagefile URL
// @param type Imagefile type
// @return Disk object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
disk
new_disk_by_url (const std::string& url, const std::string& type)
{
  return disk (std::make_shared <disk_impl_imagefile> (url, type));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create imagefile disk
// @param path Imagefile path
// @param type Imagefile type
// @return Disk object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
disk
new_disk_by_path (const std::string& path, const std::string& type)
{
  auto uri = mobius::io::new_uri_from_path (path);
  return disk (std::make_shared <disk_impl_imagefile> (uri.get_value (), type));
}

} // namespace mobius::core::vfs


