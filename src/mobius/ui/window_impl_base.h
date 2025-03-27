#ifndef MOBIUS_UI_WINDOW_IMPL_BASE_H
#define MOBIUS_UI_WINDOW_IMPL_BASE_H

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
#include <mobius/ui/icon.h>
#include <mobius/ui/widget.h>
#include <cstdint>
#include <string>
#include <utility>

namespace mobius::ui
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>window</i> implementation base class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class window_impl_base
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors and destructor
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  window_impl_base () = default;
  window_impl_base (const window_impl_base&) = delete;
  window_impl_base (window_impl_base&&) = delete;
  virtual ~window_impl_base () = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Assignment operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  window_impl_base& operator= (const window_impl_base&) = delete;
  window_impl_base& operator= (window_impl_base&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  virtual void set_sensitive (bool) = 0;
  virtual void set_visible (bool) = 0;
  virtual void show_all () = 0;
  virtual void destroy () = 0;
  virtual void set_title (const std::string&) = 0;
  virtual std::string get_title () const = 0;
  virtual void set_icon (const mobius::ui::icon&) = 0;
  virtual void set_border_width (std::uint32_t) = 0;
  virtual void set_content (const widget&) = 0;
  virtual widget get_content () const = 0;
  virtual void remove_content () = 0;
  virtual void set_position (std::uint32_t, std::uint32_t) = 0;
  virtual std::pair <std::uint32_t, std::uint32_t> get_position () const = 0;
  virtual void set_size (std::uint32_t, std::uint32_t) = 0;
  virtual std::pair <std::uint32_t, std::uint32_t> get_size () const = 0;
  virtual void set_callback (const std::string&, const mobius::core::functor<bool>&) = 0;
  virtual void reset_callback (const std::string&) = 0;
};

} // namespace mobius::ui

#endif


