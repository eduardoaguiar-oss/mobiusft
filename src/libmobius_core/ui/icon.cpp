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
#include <mobius/core/ui/icon.hpp>
#include <mobius/core/ui/ui.hpp>
#include <mobius/core/ui/widget_impl_null.hpp>
#include <mobius/core/io/file.hpp>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Module data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string icon_path_;

} // namespace

namespace mobius::core::ui
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
icon::icon ()
  : widget (std::static_pointer_cast <widget_impl_base> (std::make_shared <widget_impl_null> ()))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor from implementation pointer
// @param impl Implementation pointer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
icon::icon (const std::shared_ptr <icon_impl_base>& impl)
  : widget (std::static_pointer_cast <widget_impl_base> (impl))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set icon by name
// @param name Icon name
// @param size Icon size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
icon::set_icon_by_name (const std::string& name, size_type size)
{
  if (!icon_path_.empty ())
    {
      auto path = icon_path_ + '/' + name + ".png";
      auto f = mobius::core::io::new_file_by_path (path);

      if (f.exists ())
        return set_icon_by_path (path, size);
    }

  _impl <icon_impl_base> ()->set_icon_by_name (name, size);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set icon by path
// @param path Icon path
// @param size Icon size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
icon::set_icon_by_path (const std::string& path, size_type size)
{
  auto f = mobius::core::io::new_file_by_path (path);
  auto reader = f.new_reader ();
  auto data = reader.read (reader.get_size ());

  _impl <icon_impl_base> ()->set_icon_from_data (data, size);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set icon by URL
// @param url Icon URL
// @param size Icon size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
icon::set_icon_by_url (const std::string& url, size_type size)
{
  auto f = mobius::core::io::new_file_by_url (url);
  auto reader = f.new_reader ();
  auto data = reader.read (reader.get_size ());

  _impl <icon_impl_base> ()->set_icon_from_data (data, size);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set icon directory path
// @param path Icon path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
set_icon_path (const std::string& path)
{
  icon_path_ = path;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new icon from data
// @param data Icon data
// @param size Size
// @return New icon object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
icon
new_icon_from_data (const mobius::core::bytearray& data, icon::size_type size)
{
  return icon (get_implementation ()->new_icon_from_data (data, size));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new icon by icon name
// @param name Icon name
// @param size Size
// @return New icon object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
icon
new_icon_by_name (const std::string& name, icon::size_type size)
{
  if (!icon_path_.empty ())
    {
      auto path = icon_path_ + '/' + name + ".png";
      auto f = mobius::core::io::new_file_by_path (path);

      if (f.exists ())
        return new_icon_by_path (path, size);
    }

  return icon (get_implementation ()->new_icon_by_name (name, size));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new icon by file path
// @param path File path
// @param size Size
// @return New icon object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
icon
new_icon_by_path (const std::string& path, icon::size_type size)
{
  auto f = mobius::core::io::new_file_by_path (path);
  auto reader = f.new_reader ();
  auto data = reader.read (reader.get_size ());

  return new_icon_from_data (data, size);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new icon by file URL
// @param url File URL
// @param size Size
// @return New icon object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
icon
new_icon_by_url (const std::string& url, icon::size_type size)
{
  auto f = mobius::core::io::new_file_by_url (url);
  auto reader = f.new_reader ();
  auto data = reader.read (reader.get_size ());

  return new_icon_from_data (data, size);
}

} // namespace mobius::core::ui


