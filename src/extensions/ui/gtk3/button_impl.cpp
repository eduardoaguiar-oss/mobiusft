// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008-2026 Eduardo Aguiar
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
#include "button_impl.hpp"
#include <gtk/gtk.h>
#include <mobius/core/exception.inc>
#include <stdexcept>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Callback for <i>clicked</i>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static bool
_callback_clicked (GtkWidget *, gpointer data)
{
    return static_cast<mobius::core::functor<bool> *> (data)->operator() ();
}

} //  namespace

namespace mobius::extension::ui::gtk3
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
button_impl::button_impl ()
    : widget_ (gtk_button_new ())
{
    g_object_ref_sink (G_OBJECT (widget_));
    gtk_button_set_use_underline (reinterpret_cast<GtkButton *> (widget_),
                                  true);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
button_impl::~button_impl ()
{
    reset_callback ("clicked");
    g_object_unref (G_OBJECT (widget_));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set widget sensitive
// @param flag true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
button_impl::set_sensitive (bool flag)
{
    gtk_widget_set_sensitive (widget_, flag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set widget visible
// @param flag Flag (true/false)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
button_impl::set_visible (bool flag)
{
    gtk_widget_set_visible (widget_, flag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set text
// @param text Text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
button_impl::set_text (const std::string &text)
{
    gtk_button_set_label (reinterpret_cast<GtkButton *> (widget_),
                          text.c_str ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set button icon
// @param icon Icon object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
button_impl::set_icon (const mobius::core::ui::icon &icon)
{
    gtk_button_set_image (reinterpret_cast<GtkButton *> (widget_),
                          icon.get_ui_widget<GtkWidget *> ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set callback to event
// @param event_id Event ID
// @param f Function or functor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
button_impl::set_callback (const std::string &event_id,
                           const mobius::core::functor<bool> &f)
{
    if (event_id == "clicked")
    {
        if (on_clicked_callback_)
            g_object_disconnect (G_OBJECT (widget_), "clicked", nullptr);

        on_clicked_callback_ = f;

        g_signal_connect (G_OBJECT (widget_), "clicked",
                          G_CALLBACK (_callback_clicked),
                          &on_clicked_callback_);
    }

    else
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("invalid event: " + event_id));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset callback to event
// @param event_id Event ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
button_impl::reset_callback (const std::string &event_id)
{
    if (event_id == "clicked")
    {
        if (on_clicked_callback_)
        {
            g_signal_handlers_disconnect_by_data (widget_,
                                                  &on_clicked_callback_);
            on_clicked_callback_ = {};
        }
    }

    else
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("invalid event: " + event_id));
}

} // namespace mobius::extension::ui::gtk3
