#ifndef MOBIUS_UI_GTK3_UI_IMPL_H
#define MOBIUS_UI_GTK3_UI_IMPL_H

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
#include <mobius/ui/ui_impl_base.h>
#include <memory>
#include <string>

namespace mobius::extension::ui::gtk3
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief GTK3 UI implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ui_impl : public mobius::ui::ui_impl_base
{
public:
  ui_impl ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get UI implementation ID
  // @return ID
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  static std::string
  get_id ()
  {
    return "gtk3";
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get UI implementation description
  // @return Description
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  static std::string
  get_description ()
  {
    return "GTK v3";
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Function prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void start () final;
  void stop () final;
  void flush () final;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Builders prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::shared_ptr <mobius::ui::box_impl_base> new_box (mobius::ui::box_impl_base::orientation_type) const final;
  std::shared_ptr <mobius::ui::button_impl_base> new_button () const final;
  std::shared_ptr <mobius::ui::icon_impl_base> new_icon_by_name (const std::string&, mobius::ui::icon_impl_base::size_type) const final;
  std::shared_ptr <mobius::ui::icon_impl_base> new_icon_from_data (const mobius::bytearray&, mobius::ui::icon_impl_base::size_type) const final;
  std::shared_ptr <mobius::ui::label_impl_base> new_label () const final;
  std::shared_ptr <mobius::ui::message_dialog_impl_base> new_message_dialog (mobius::ui::message_dialog_impl_base::type) const final;
  std::shared_ptr <mobius::ui::widget_impl_base> new_widget (std::any, bool) const final;
  std::shared_ptr <mobius::ui::window_impl_base> new_window () const final;
};

} // namespace mobius::extension::ui::gtk3

#endif


