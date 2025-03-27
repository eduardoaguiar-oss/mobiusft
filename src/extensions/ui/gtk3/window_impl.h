#ifndef MOBIUS_EXTENSION_UI_GTK3_WINDOW_IMPL_H
#define MOBIUS_EXTENSION_UI_GTK3_WINDOW_IMPL_H

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
#include <mobius/core/functor.h>
#include <mobius/ui/window_impl_base.h>
#include <gtk/gtk.h>

namespace mobius::extension::ui::gtk3
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>gtk3 window</i> implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class window_impl : public mobius::ui::window_impl_base
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Function prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  window_impl ();
  ~window_impl ();
  void set_sensitive (bool) final;
  void set_visible (bool) final;
  void show_all () final;
  void destroy () final;
  void set_title (const std::string&) final;
  std::string get_title () const final;
  void set_icon (const mobius::ui::icon&) final;
  void set_border_width (std::uint32_t) final;
  void set_content (const mobius::ui::widget&) final;
  mobius::ui::widget get_content () const final;
  void remove_content () final;
  void set_position (std::uint32_t, std::uint32_t) final;
  std::pair<std::uint32_t, std::uint32_t> get_position () const final;
  void set_size (std::uint32_t, std::uint32_t) final;
  std::pair<std::uint32_t, std::uint32_t> get_size () const final;
  void set_callback (const std::string&, const mobius::core::functor<bool>&) final;
  void reset_callback (const std::string&) final;

private:
  // @brief Low level widget
  GtkWidget *widget_ = nullptr;

  // @brief Accel group
  GtkAccelGroup *accel_group_ = nullptr;

  // @brief Content widget
  mobius::ui::widget content_;

  // @brief Callback for closing event
  mobius::core::functor <bool> closing_callback_;
};

} // namespace mobius::extension::ui::gtk3

#endif


