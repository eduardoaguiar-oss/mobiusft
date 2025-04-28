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
#include "icon_impl.hpp"

namespace mobius::extension::ui::gtk3
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param widget Gtk Widget
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
icon_impl::icon_impl (GtkWidget *widget)
  : widget_ (widget)
{
  g_object_ref_sink (G_OBJECT (widget_));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
icon_impl::~icon_impl ()
{
  g_object_unref (G_OBJECT (widget_));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set widget sensitive
// @param flag true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
icon_impl::set_sensitive (bool flag)
{
  gtk_widget_set_sensitive (widget_, flag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set widget visible
// @param flag Flag (true/false)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
icon_impl::set_visible (bool flag)
{
  gtk_widget_set_visible (widget_, flag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set icon by name
// @param name Icon name
// @param size Icon size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
icon_impl::set_icon_by_name (const std::string& name, size_type size)
{
  GtkIconSize gtk_icon_size = GTK_ICON_SIZE_LARGE_TOOLBAR;

  switch (size)
  {
    case mobius::core::ui::icon_impl_base::size_type::menu: gtk_icon_size = GTK_ICON_SIZE_MENU; break;
    case mobius::core::ui::icon_impl_base::size_type::toolbar: gtk_icon_size = GTK_ICON_SIZE_LARGE_TOOLBAR; break;
    case mobius::core::ui::icon_impl_base::size_type::dnd: gtk_icon_size = GTK_ICON_SIZE_DND; break;
    case mobius::core::ui::icon_impl_base::size_type::dialog: gtk_icon_size = GTK_ICON_SIZE_DIALOG; break;
    default: gtk_icon_size = GTK_ICON_SIZE_LARGE_TOOLBAR;
  };

  gtk_image_set_from_icon_name (
      reinterpret_cast <GtkImage *>(widget_),
      name.c_str (),
      gtk_icon_size
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set icon from data
// @param data Icon data
// @param size Icon size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
icon_impl::set_icon_from_data (const mobius::bytearray& data, size_type size)
{
  GdkPixbufLoader *loader = gdk_pixbuf_loader_new ();
  gdk_pixbuf_loader_set_size (loader, static_cast <int> (size), static_cast <int> (size));
  gdk_pixbuf_loader_write (loader, data.data (), data.size (), nullptr);
  gdk_pixbuf_loader_close (loader, nullptr);

  GdkPixbuf *pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);

  gtk_image_set_from_pixbuf (reinterpret_cast <GtkImage *>(widget_), pixbuf);
}

} // namespace mobius::extension::ui::gtk3


