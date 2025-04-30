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
#include "box_impl.hpp"
#include "button_impl.hpp"
#include "icon_impl.hpp"
#include "label_impl.hpp"
#include "message_dialog_impl.hpp"
#include "ui_impl.hpp"
#include "widget_impl.hpp"
#include "window_impl.hpp"
#include <gtk/gtk.h>

namespace mobius::extension::ui::gtk3
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ui_impl::ui_impl ()
{
  int argc = 0;
  char **argv = nullptr;

  gtk_init (&argc, &argv);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Start user interface
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
ui_impl::start ()
{
  gtk_main ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Stop user interface
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
ui_impl::stop ()
{
  gtk_main_quit ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Flush UI events
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
ui_impl::flush ()
{
  while (gtk_events_pending ())
    gtk_main_iteration ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new box
// @param orientation Orientation type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr <mobius::core::ui::box_impl_base>
ui_impl::new_box (mobius::core::ui::box_impl_base::orientation_type orientation) const
{
  return std::make_shared <box_impl> (orientation);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new button
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr <mobius::core::ui::button_impl_base>
ui_impl::new_button () const
{
  return std::make_shared <button_impl> ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new icon by name
// @param name Icon name
// @param size Icon size
// @return Pointer to icon implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr <mobius::core::ui::icon_impl_base>
ui_impl::new_icon_by_name (const std::string& name, mobius::core::ui::icon_impl_base::size_type size) const
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

  return std::make_shared <icon_impl> (
     gtk_image_new_from_icon_name (name.c_str (), gtk_icon_size)
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new icon from data
// @param data Data
// @param size Icon size
// @return Pointer to icon implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr <mobius::core::ui::icon_impl_base>
ui_impl::new_icon_from_data (const mobius::core::bytearray& data, mobius::core::ui::icon_impl_base::size_type size) const
{
  GdkPixbufLoader *loader = gdk_pixbuf_loader_new ();
  gdk_pixbuf_loader_set_size (loader, static_cast <int> (size), static_cast <int> (size));
  gdk_pixbuf_loader_write (loader, data.data (), data.size (), nullptr);
  gdk_pixbuf_loader_close (loader, nullptr);

  GdkPixbuf *pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);

  return std::make_shared <icon_impl> (
     gtk_image_new_from_pixbuf (pixbuf)
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new Label
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr <mobius::core::ui::label_impl_base>
ui_impl::new_label () const
{
  return std::make_shared <label_impl> ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new Message Dialog
// @param t Message type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr <mobius::core::ui::message_dialog_impl_base>
ui_impl::new_message_dialog (mobius::core::ui::message_dialog_impl_base::type t) const
{
  return std::make_shared <message_dialog_impl> (t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new Widget
// @param w Low level widget object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr <mobius::core::ui::widget_impl_base>
ui_impl::new_widget (std::any w, bool) const
{
  return std::make_shared <widget_impl> (w);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new Window
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr <mobius::core::ui::window_impl_base>
ui_impl::new_window () const
{
  return std::make_shared <window_impl> ();
}

} // namespace mobius::extension::ui::gtk3


