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
#include "message_dialog_impl.h"
#include <mobius/exception.inc>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <algorithm>
#include <list>
#include <stdexcept>
#include <string>

namespace mobius::extension::ui::gtk3
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default buttons' text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::vector <std::string> button_text_ =
{
  "",
  "gtk-ok",
  "gtk-cancel",
  "gtk-yes",
  "gtk-no",
  "gtk-open",
  "gtk-save",
  "gtk-close",
  "gtk-discard",
  "gtk-apply",
  "Reset",
  "Restore Defaults",
  "gtk-help",
  "Save All",
  "Yes to All",
  "No to All",
  "Abort",
  "Retry",
  "Ignore"
};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param t Message type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
message_dialog_impl::message_dialog_impl (type t)
 : type_ (t)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add default button
// @param button_id Button ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
message_dialog_impl::response_type
message_dialog_impl::add_button (button button_id)
{
  auto response_id = static_cast <response_type> (button_id);
  const std::string text = button_text_[response_id];
  _add_button (text, response_id);

  return response_id;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add button
// @param text Button text
// @return Response ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
message_dialog_impl::response_type
message_dialog_impl::add_button (const std::string& text)
{
  auto response_id = next_response_id_++;
  _add_button (text, response_id);

  return response_id;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set default response
// @param response_id Response ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_dialog_impl::set_default_response (response_type response_id)
{
  default_response_id_ = response_id;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Run dialog
// @return Response ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
message_dialog_impl::response_type
message_dialog_impl::run ()
{
  // set gtk_type
  GtkMessageType gtk_type;

  switch (type_)
    {
      case type::info: gtk_type = GTK_MESSAGE_INFO; break;
      case type::question: gtk_type = GTK_MESSAGE_QUESTION; break;
      case type::warning: gtk_type = GTK_MESSAGE_WARNING; break;
      case type::error: gtk_type = GTK_MESSAGE_ERROR; break;
      default: gtk_type = GTK_MESSAGE_OTHER; break;
    }

  // create dialog
  GtkWidget *dialog = gtk_message_dialog_new (
    nullptr,
    GTK_DIALOG_MODAL,
    gtk_type,
    GTK_BUTTONS_NONE,
    "%s",
    text_.c_str ()
  );

  // set informative text, if necessary
  if (!informative_text_.empty ())
    gtk_message_dialog_format_secondary_text (
      GTK_MESSAGE_DIALOG (dialog),
      "%s", informative_text_.c_str ()
    );

  // add buttons
  for (auto& p : buttons_)
    gtk_dialog_add_button (GTK_DIALOG (dialog), p.first.c_str (), p.second);

  // set default response, if necessary
  if (default_response_id_)
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), default_response_id_);

  // run dialog
  int rc = gtk_dialog_run (GTK_DIALOG (dialog));

  // destroy
  gtk_widget_destroy (dialog);

  return rc;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add button to list of buttons
// @param text Button text
// @param response_id Response ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_dialog_impl::_add_button (
  const std::string& text,
  response_type response_id
)
{
  // check if response_id is already used
  if (std::find_if (buttons_.begin (),
	            buttons_.end (),
	            [response_id] (auto p) { return p.second == response_id; }
	           ) != buttons_.end ())
    throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG ("response_id already used"));

  // add button
  buttons_.emplace_back (text, response_id);
}

} // namespace mobius::extension::ui::gtk3


