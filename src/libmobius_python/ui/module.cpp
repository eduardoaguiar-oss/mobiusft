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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief  C++ API mobius.ui module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "module.hpp"
#include <pymobius.hpp>
#include "box.hpp"
#include "button.hpp"
#include "container.hpp"
#include "icon.hpp"
#include "label.hpp"
#include "message_dialog.hpp"
#include "stacked_container.hpp"
#include "widget.hpp"
#include "window.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject* func_ui_flush (PyObject *, PyObject *);
PyObject* func_ui_init (PyObject *, PyObject *);
PyObject* func_ui_set_implementation (PyObject *, PyObject *);
PyObject* func_ui_start (PyObject *, PyObject *);
PyObject* func_ui_stop (PyObject *, PyObject *);

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module methods
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef module_methods[] =
{
  {"flush", func_ui_flush, METH_VARARGS, "Flush UI events"},
  {"init", func_ui_start, METH_VARARGS, "Initialize user interface"},
  {"new_icon_by_name", func_ui_new_icon_by_name, METH_VARARGS, "Create new icon by name"},
  {"new_icon_by_path", func_ui_new_icon_by_path, METH_VARARGS, "Create new icon by file path"},
  {"new_icon_by_url", func_ui_new_icon_by_url, METH_VARARGS, "Create new icon by file URL"},
  {"new_icon_from_data", func_ui_new_icon_from_data, METH_VARARGS, "Create new icon from data"},
  {"set_icon_path", func_ui_set_icon_path, METH_VARARGS, "Set icon directory path"},
  {"set_implementation", func_ui_set_implementation, METH_VARARGS, "Set UI implementation"},
  {"start", func_ui_start, METH_VARARGS, "Start user interface"},
  {"stop", func_ui_stop, METH_VARARGS, "Stop user interface"},
  {nullptr, nullptr, 0, nullptr}, // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module definition structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyModuleDef module_def =
{
  PyModuleDef_HEAD_INIT,
  "mobius.ui",
  "Mobius Forensic Toolkit mobius.ui module",
  -1,
  module_methods,
  nullptr,
  nullptr,
  nullptr,
  nullptr
};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create mobius.ui module
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pymodule
new_ui_module ()
{
  // Initialize module
  mobius::py::pymodule module (&module_def);

  // Add types
  module.add_type ("widget", new_ui_widget_type ());
  module.add_type ("box", new_ui_box_type ());
  module.add_type ("button", new_ui_button_type ());
  module.add_type ("container", new_ui_container_type ());
  module.add_type ("icon", new_ui_icon_type ());
  module.add_type ("label", new_ui_label_type ());
  module.add_type ("message_dialog", new_ui_message_dialog_type ());
  module.add_type ("stacked_container", new_ui_stacked_container_type ());
  module.add_type ("window", new_ui_window_type ());

  // Return module
  return module;
}


