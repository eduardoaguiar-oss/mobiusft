// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025
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
#include "box_impl.hpp"
#include <algorithm>
#include <mobius/core/exception.inc>
#include <stdexcept>

namespace mobius::extension::ui::gtk3
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param orientation Widget orientation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
box_impl::box_impl (orientation_type orientation)
{
    switch (orientation)
    {
    case orientation_type::vertical:
        widget_ = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        break;

    case orientation_type::horizontal:
        widget_ = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        break;

    default:
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("invalid orientation type"));
    }

    g_object_ref_sink (G_OBJECT (widget_));
    gtk_widget_set_no_show_all (widget_, true);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
box_impl::~box_impl ()
{
    children_.clear ();
    g_object_unref (G_OBJECT (widget_));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set widget sensitive
// @param flag true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
box_impl::set_sensitive (bool flag)
{
    gtk_widget_set_sensitive (widget_, flag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set widget visible
// @param flag Flag (true/false)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
box_impl::set_visible (bool flag)
{
    gtk_widget_set_visible (widget_, flag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set spacing between widgets
// @param siz Size in pixels
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
box_impl::set_spacing (std::uint32_t siz)
{
    gtk_box_set_spacing (reinterpret_cast<GtkBox *> (widget_), siz);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set border width
// @param siz Size in pixels
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
box_impl::set_border_width (std::uint32_t siz)
{
    gtk_container_set_border_width (reinterpret_cast<GtkContainer *> (widget_),
                                    siz);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add child widget
// @param w Widget to be added
// @param filling Child widget filling mode
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
box_impl::add_child (const mobius::core::ui::widget &w, fill_type filling)
{
    bool expand = false;
    bool fill = false;

    switch (filling)
    {
    case fill_type::fill_none:
        break;
    case fill_type::fill_with_space:
        expand = true;
        break;
    case fill_type::fill_with_widget:
        expand = true;
        fill = true;
        break;
    };

    gtk_box_pack_start (reinterpret_cast<GtkBox *> (widget_),
                        w.get_ui_widget<GtkWidget *> (), expand, fill, 0);

    children_.push_back (w);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove child widget
// @param w Widget to be removed
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
box_impl::remove_child (const mobius::core::ui::widget &w)
{
    children_.erase (
        std::find_if (children_.begin (), children_.end (),
                      [w] (const mobius::core::ui::widget &item)
                      {
                          return item.get_ui_widget<GtkWidget *> () ==
                                 w.get_ui_widget<GtkWidget *> ();
                      }));

    gtk_container_remove (reinterpret_cast<GtkContainer *> (widget_),
                          w.get_ui_widget<GtkWidget *> ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clear widget
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
box_impl::clear ()
{
    children_.clear ();

    gtk_container_foreach (reinterpret_cast<GtkContainer *> (widget_),
                           reinterpret_cast<GtkCallback> (gtk_widget_destroy),
                           nullptr);
}

} // namespace mobius::extension::ui::gtk3
