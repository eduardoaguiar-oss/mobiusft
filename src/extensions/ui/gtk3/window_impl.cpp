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
#include "window_impl.hpp"
#include <mobius/core/exception.inc>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Callback for <i>delete-event</i>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static bool
_callback_delete_event (GtkWidget *, GdkEvent *, gpointer data)
{
    return reinterpret_cast<mobius::extension::ui::gtk3::window_impl *> (data)
        ->_on_delete_event ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Callback for <i>destroy</i>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_callback_destroy (GtkWidget *, gpointer data)
{
    reinterpret_cast<mobius::extension::ui::gtk3::window_impl *> (data)
        ->_on_destroy_event ();
}

} //  namespace

namespace mobius::extension::ui::gtk3
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
window_impl::window_impl ()
{
    widget_ = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    accel_group_ = gtk_accel_group_new ();

    gtk_window_add_accel_group (
        reinterpret_cast<GtkWindow *> (widget_), accel_group_
    );

    g_signal_connect (widget_, "destroy", G_CALLBACK (_callback_destroy), this);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
window_impl::~window_impl ()
{
    if (widget_)
        destroy ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set widget sensitivity
// @param flag Flag (true/false)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
window_impl::set_sensitive (bool flag)
{
    gtk_widget_set_sensitive (widget_, flag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set widget visibility
// @param flag Flag (true/false)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
window_impl::set_visible (bool flag)
{
    gtk_widget_set_visible (widget_, flag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show widget and its children
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
window_impl::show_all ()
{
    gtk_widget_show_all (widget_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destroy window
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
window_impl::destroy ()
{
    if (widget_)
    {
        content_ = {};

        if (accel_group_)
        {
            g_object_unref (G_OBJECT (accel_group_));
            accel_group_ = nullptr;
        }

        gtk_widget_destroy (widget_);
        widget_ = nullptr;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set title
// @param title Title text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
window_impl::set_title (const std::string &title)
{
    gtk_window_set_title (
        reinterpret_cast<GtkWindow *> (widget_), title.c_str ()
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get title
// @return Title
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
window_impl::get_title () const
{
    std::string title;
    auto gtk_title =
        gtk_window_get_title (reinterpret_cast<GtkWindow *> (widget_));

    if (gtk_title)
        title = gtk_title;

    return title;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set icon
// @param icon Iocn object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
window_impl::set_icon (const mobius::core::ui::icon &icon)
{
    auto gtk_widget = icon.get_ui_widget<GtkWidget *> ();

    gtk_window_set_icon (
        reinterpret_cast<GtkWindow *> (widget_),
        gtk_image_get_pixbuf (reinterpret_cast<GtkImage *> (gtk_widget))
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set border width
// @param width Border width in pixels
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
window_impl::set_border_width (std::uint32_t width)
{
    gtk_container_set_border_width (
        reinterpret_cast<GtkContainer *> (widget_), width
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set content
// @param w Widget
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
window_impl::set_content (const mobius::core::ui::widget &w)
{
    if (content_)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("window content already set")
        );

    content_ = w;

    gtk_container_add (
        reinterpret_cast<GtkContainer *> (widget_),
        content_.get_ui_widget<GtkWidget *> ()
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get content
// @return Contained widget
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::ui::widget
window_impl::get_content () const
{
    return content_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove content
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
window_impl::remove_content ()
{
    if (content_)
    {
        gtk_container_remove (
            reinterpret_cast<GtkContainer *> (widget_),
            content_.get_ui_widget<GtkWidget *> ()
        );

        content_ = {};
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set position
// @param x X pos
// @param y Y pos
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
window_impl::set_position (std::uint32_t x, std::uint32_t y)
{
    gtk_window_move (reinterpret_cast<GtkWindow *> (widget_), x, y);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get position
// @return X, Y coordinates, in pixels
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::pair<std::uint32_t, std::uint32_t>
window_impl::get_position () const
{
    gint x = 0;
    gint y = 0;

    gtk_window_get_position (reinterpret_cast<GtkWindow *> (widget_), &x, &y);

    return {x, y};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set size
// @param width Width
// @param height Height
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
window_impl::set_size (std::uint32_t width, std::uint32_t height)
{
    gtk_window_resize (reinterpret_cast<GtkWindow *> (widget_), width, height);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get size
// @return Width and height, in pixels
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::pair<std::uint32_t, std::uint32_t>
window_impl::get_size () const
{
    gint width = 0;
    gint height = 0;

    gtk_window_get_size (
        reinterpret_cast<GtkWindow *> (widget_), &width, &height
    );

    return {width, height};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set callback to event
// @param event_id Event ID
// @param f Function or functor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
window_impl::set_callback (
    const std::string &event_id, const mobius::core::functor<bool> &f
)
{
    if (event_id == "closing")
    {
        if (delete_event_handler_id_)
        {
            g_signal_handler_disconnect (
                G_OBJECT (widget_), delete_event_handler_id_
            );
        }

        closing_callback_ = f;

        delete_event_handler_id_ = g_signal_connect (
            G_OBJECT (widget_), "delete-event",
            G_CALLBACK (_callback_delete_event), this
        );
    }

    else
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("invalid event: " + event_id)
        );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset callback to event
// @param event_id Event ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
window_impl::reset_callback (const std::string &event_id)
{
    if (event_id == "closing")
    {
        if (delete_event_handler_id_)
        {
            if (g_signal_handler_is_connected (
                    G_OBJECT (widget_), delete_event_handler_id_
                ))
                g_signal_handler_disconnect (
                    G_OBJECT (widget_), delete_event_handler_id_
                );

            delete_event_handler_id_ = 0;
            closing_callback_ = {};
        }
    }

    else
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("invalid event: " + event_id)
        );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Handler for <i>delete-event</i>
// @return true to stop event propagation, false to continue
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
window_impl::_on_delete_event ()
{
    bool rc = false;

    if (closing_callback_)
        rc = closing_callback_ ();

    return rc;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Handler for <i>destroy</i> event
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
window_impl::_on_destroy_event ()
{
    widget_ = nullptr;
}

} // namespace mobius::extension::ui::gtk3
