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
#include "stacked_container.h"
#include "widget_impl_base.h"
#include "box.h"
#include <mobius/exception.inc>
#include <algorithm>
#include <stdexcept>
#include <unordered_map>

namespace mobius::ui
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief stacked_container implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class stacked_container_impl : public widget_impl_base
{
public:

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  stacked_container_impl ();
  stacked_container_impl (const stacked_container_impl&) = delete;
  stacked_container_impl (stacked_container_impl&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  stacked_container_impl& operator= (const stacked_container_impl&) = delete;
  stacked_container_impl& operator= (stacked_container_impl&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  //! \brief Check whether widget object is valid
  //! \return true/false
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  explicit operator bool () const noexcept final
  {
    return true;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  //! \brief Get low level widget
  //! \return Low level widget
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::any
  get_ui_widget () const
  {
    return widget_.get_ui_widget ();
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  //! \brief Set widget sensitive
  //! \param flag Flag (true/false)
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_sensitive (bool flag) final
  {
    widget_.set_sensitive (flag);
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  //! \brief Set widget visible
  //! \param flag Flag (true/false)
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_visible (bool flag) final
  {
    widget_.set_visible (flag);
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::ui::widget get_widget () const;
  void add_child (const std::string&, const mobius::ui::widget&);
  void remove_child (const std::string&);
  mobius::ui::widget get_child (const std::string&) const;
  void select_child (const std::string&);
  std::string get_selected () const;

private:
  //! \brief Container widget
  mobius::ui::box widget_;

  //! \brief Children container
  std::unordered_map <std::string, mobius::ui::widget> children_;

  //! \brief Selected child ID
  std::string selected_widget_id_;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
stacked_container_impl::stacked_container_impl ()
  : widget_ (mobius::ui::box::orientation_type::horizontal)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Add child widget
//! \param widget_id Widget ID
//! \param w Widget object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
stacked_container_impl::add_child (const std::string& widget_id, const mobius::ui::widget& w)
{
  if (children_.find (widget_id) != children_.end ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("Widget <" + widget_id + "> already added"));

  children_[widget_id] = w;
  widget_.add_child (w, mobius::ui::box::fill_type::fill_with_widget);

  select_child (widget_id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Remove child widget
//! \param widget_id Widget ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
stacked_container_impl::remove_child (const std::string& widget_id)
{
  // get child
  auto iter = children_.find (widget_id);
  if (iter == children_.end ())
    return;

  // remove child
  widget_.remove_child (iter->second);
  auto next_iter = children_.erase (iter);

  // if child was selected, select another one
  if (selected_widget_id_ == widget_id)
    {
      if (next_iter != children_.end ())
        {
          next_iter->second.set_visible (true);
          selected_widget_id_ = next_iter->first;
        }
      else
        selected_widget_id_.clear ();
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get child widget
//! \param widget_id Widget ID
//! \return Child widget
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::ui::widget
stacked_container_impl::get_child (const std::string& widget_id) const
{
  mobius::ui::widget child;

  auto iter = children_.find (widget_id);

  if (iter != children_.end ())
    child = iter->second;

  return child;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Select child widget
//! \param widget_id Widget ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
stacked_container_impl::select_child (const std::string& widget_id)
{
  // return if widget is already selected
  if (widget_id == selected_widget_id_)
    return;

  // show selected widget, hide all others
  for (auto& [w_id, w] : children_)
    {
      if (w_id == widget_id)
        {
          w.set_visible (true);
          selected_widget_id_ = w_id;
        }

      else
        w.set_visible (false);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get selected widget
//! \return Selected widget ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
stacked_container_impl::get_selected () const
{
  return selected_widget_id_;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
stacked_container::stacked_container ()
  : widget (std::static_pointer_cast <widget_impl_base> (std::make_shared <stacked_container_impl> ()))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Add child widget
//! \param widget_id Widget ID
//! \param w Widget object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
stacked_container::add_child (const std::string& widget_id, const mobius::ui::widget& w)
{
  _impl <stacked_container_impl> ()->add_child (widget_id, w);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Remove child widget
//! \param widget_id Widget ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
stacked_container::remove_child (const std::string& widget_id)
{
  _impl <stacked_container_impl> ()->remove_child (widget_id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get child widget
//! \param widget_id Widget ID
//! \return Child widget
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::ui::widget
stacked_container::get_child (const std::string& widget_id) const
{
  return _impl <stacked_container_impl> ()->get_child (widget_id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Select child widget
//! \param widget_id Widget ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
stacked_container::select_child (const std::string& widget_id)
{
  _impl <stacked_container_impl> ()->select_child (widget_id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get selected widget
//! \return Selected widget ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
stacked_container::get_selected () const
{
  return _impl <stacked_container_impl> ()->get_selected ();
}

} // namespace mobius::ui
