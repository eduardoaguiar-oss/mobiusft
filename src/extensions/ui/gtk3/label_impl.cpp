// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
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
#include "label_impl.hpp"
#include <gtk/gtk.h>
#include <pango/pango.h>

namespace mobius::extension::ui::gtk3
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
label_impl::label_impl ()
    : widget_ (gtk_label_new (""))
{
    g_object_ref_sink (G_OBJECT (widget_));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
label_impl::~label_impl () { g_object_unref (G_OBJECT (widget_)); }

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set widget sensitive
// @param flag true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
label_impl::set_sensitive (bool flag)
{
    gtk_widget_set_sensitive (widget_, flag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set widget visible
// @param flag Flag (true/false)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
label_impl::set_visible (bool flag)
{
    gtk_widget_set_visible (widget_, flag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set text
// @param text Text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
label_impl::set_text (const std::string &text)
{
    gtk_label_set_text (reinterpret_cast<GtkLabel *> (widget_), text.c_str ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set markup
// @param text Markup text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
label_impl::set_markup (const std::string &text)
{
    gtk_label_set_markup (reinterpret_cast<GtkLabel *> (widget_),
                          text.c_str ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set widget selectable
// @param flag Flag (true/false)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
label_impl::set_selectable (bool flag)
{
    gtk_label_set_selectable (reinterpret_cast<GtkLabel *> (widget_), flag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set horizontal alignment
// @param halign Alignment type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
label_impl::set_halign (halign_type halign)
{
    switch (halign)
    {
    case halign_type::left:
        gtk_widget_set_halign (widget_, GTK_ALIGN_START);
        break;
    case halign_type::center:
        gtk_widget_set_halign (widget_, GTK_ALIGN_CENTER);
        break;
    case halign_type::right:
        gtk_widget_set_halign (widget_, GTK_ALIGN_END);
        break;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set vertical alignment
// @param valign Alignment type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
label_impl::set_valign (valign_type valign)
{
    switch (valign)
    {
    case valign_type::top:
        gtk_widget_set_valign (widget_, GTK_ALIGN_START);
        break;
    case valign_type::center:
        gtk_widget_set_valign (widget_, GTK_ALIGN_CENTER);
        break;
    case valign_type::bottom:
        gtk_widget_set_valign (widget_, GTK_ALIGN_END);
        break;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set elide mode
// @param mode Elide mode
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
label_impl::set_elide_mode (elide_type mode)
{
    PangoEllipsizeMode pango_mode = PANGO_ELLIPSIZE_NONE;

    switch (mode)
    {
    case elide_type::none:
        pango_mode = PANGO_ELLIPSIZE_NONE;
        break;
    case elide_type::start:
        pango_mode = PANGO_ELLIPSIZE_START;
        break;
    case elide_type::middle:
        pango_mode = PANGO_ELLIPSIZE_MIDDLE;
        break;
    case elide_type::end:
        pango_mode = PANGO_ELLIPSIZE_END;
        break;
    }

    gtk_label_set_ellipsize (reinterpret_cast<GtkLabel *> (widget_),
                             pango_mode);
}

} // namespace mobius::extension::ui::gtk3
