#ifndef MOBIUS_UI_UI_IMPL_BASE_H
#define MOBIUS_UI_UI_IMPL_BASE_H

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
#include <any>
#include <memory>
#include <mobius/bytearray.h>
#include <mobius/ui/box_impl_base.h>
#include <mobius/ui/button_impl_base.h>
#include <mobius/ui/icon_impl_base.h>
#include <mobius/ui/label_impl_base.h>
#include <mobius/ui/message_dialog_impl_base.h>
#include <mobius/ui/widget_impl_base.h>
#include <mobius/ui/window_impl_base.h>

namespace mobius::ui
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief UI implementation base class
//! \author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ui_impl_base
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors and destructor
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  ui_impl_base () = default;
  ui_impl_base (const ui_impl_base&) = delete;
  ui_impl_base (ui_impl_base&&) = delete;
  virtual ~ui_impl_base () = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Assignment operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  ui_impl_base& operator= (const ui_impl_base&) noexcept = delete;
  ui_impl_base& operator= (ui_impl_base&&) noexcept = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  virtual void start () = 0;
  virtual void stop () = 0;
  virtual void flush () = 0;
  virtual std::shared_ptr <box_impl_base> new_box (box_impl_base::orientation_type) const = 0;
  virtual std::shared_ptr <button_impl_base> new_button () const = 0;
  virtual std::shared_ptr <icon_impl_base> new_icon_by_name (const std::string&, icon_impl_base::size_type) const = 0;
  virtual std::shared_ptr <icon_impl_base> new_icon_from_data (const mobius::bytearray&, icon_impl_base::size_type) const = 0;
  virtual std::shared_ptr <label_impl_base> new_label () const = 0;
  virtual std::shared_ptr <message_dialog_impl_base> new_message_dialog (message_dialog_impl_base::type) const = 0;
  virtual std::shared_ptr <widget_impl_base> new_widget (std::any, bool=false) const = 0;
  virtual std::shared_ptr <window_impl_base> new_window () const = 0;
};

} // namespace mobius::ui

#endif
