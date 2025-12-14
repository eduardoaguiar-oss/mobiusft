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
#include <mobius/core/datasource/datasource.hpp>
#include <mobius/core/datasource/datasource_impl_null.hpp>
#include <mobius/core/datasource/datasource_impl_vfs.hpp>
#include <mobius/core/datasource/ufdr/datasource_impl.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/io/uri.hpp>
#include <stdexcept>

namespace mobius::core::datasource
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource::datasource ()
    : impl_ (std::make_shared<datasource_impl_null> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor from implementation pointer
// @param impl Implementation pointer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource::datasource (const std::shared_ptr<datasource_impl_base> &impl)
    : impl_ (impl)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param state Object state
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource::datasource (const mobius::core::pod::map &state)
{
    auto type = state.get<std::string> ("type");

    if (type == "ufdr")
        impl_ = std::make_shared<ufdr::datasource_impl> (state);

    else if (type == "vfs")
        impl_ = std::make_shared<datasource_impl_vfs> (state);

    else
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("invalid datasource type: " + type));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create datasource from VFS object
// @param vfs VFS object
// @return Datasource object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource
new_datasource_from_vfs (const mobius::core::vfs::vfs &vfs)
{
    return datasource (std::make_shared<datasource_impl_vfs> (vfs));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create datasource from image file
// @param path Image file path
// @return Datasource object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource
new_datasource_by_imagefile_path (const std::string &path)
{
    mobius::core::vfs::vfs vfs;
    vfs.add_disk (mobius::core::vfs::new_disk_by_path (path));

    return new_datasource_from_vfs (vfs);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create datasource from image file
// @param url Image file URL
// @return Datasource object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource
new_datasource_by_imagefile_url (const std::string &url)
{
    mobius::core::vfs::vfs vfs;
    vfs.add_disk (mobius::core::vfs::new_disk_by_url (url));

    return new_datasource_from_vfs (vfs);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create datasource from UFDR file path
// @param path UFDR file path
// @return Datasource object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource
new_datasource_by_ufdr_path (const std::string &path)
{
    auto uri = mobius::core::io::new_uri_from_path (path);
    return mobius::core::datasource::new_datasource_by_ufdr_url (
        uri.get_value ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create datasource from UFDR file URL
// @param url UFDR file URL
// @return Datasource object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
datasource
new_datasource_by_ufdr_url (const std::string &url)
{
    return datasource (std::make_shared<ufdr::datasource_impl> (url));
}

} // namespace mobius::core::datasource
