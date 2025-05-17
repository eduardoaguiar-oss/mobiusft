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
// @brief  C++ API mobius.core module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "module.hpp"
#include <pymobius.hpp>
#include "application.hpp"
#include "extension.hpp"
#include "log.hpp"
#include "resource.hpp"
#include "thread_guard.hpp"
#include "database/module.hpp"
#include "decoder/module.hpp"
#include "file_decoder/module.hpp"
#include "kff/module.hpp"
#include "system/module.hpp"
#include "turing/module.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Functions prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject* func_logf (PyObject *, PyObject *);
PyObject* func_set_logfile_path (PyObject *, PyObject *);
PyObject* func_add_resource (PyObject *, PyObject *);
PyObject* func_remove_resource (PyObject *, PyObject *);
PyObject* func_has_resource (PyObject *, PyObject *);
PyObject* func_get_resource (PyObject *, PyObject *);
PyObject* func_get_resources (PyObject *, PyObject *);
PyObject* func_get_resource_value (PyObject *, PyObject *);
PyObject* func_subscribe (PyObject *, PyObject *);
PyObject* func_unsubscribe (PyObject *, PyObject *);

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module methods
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef module_methods[] =
{
  {"add_resource", func_add_resource, METH_VARARGS, "Add resource"},
  {"get_resource", func_get_resource, METH_VARARGS, "Get resource"},
  {"get_resource_value", func_get_resource_value, METH_VARARGS, "Get resource value"},
  {"get_resources", func_get_resources, METH_VARARGS, "Get resources"},
  {"has_resource", func_has_resource, METH_VARARGS, "Check if resource exists"},
  {"logf", func_logf, METH_VARARGS, "Log event"},
  {"remove_resource", func_remove_resource, METH_VARARGS, "Remove resource"},
  {"set_logfile_path", func_set_logfile_path, METH_VARARGS, "Set log file path"},
  {"subscribe", func_subscribe, METH_VARARGS, "Subscribe to event"},
  {"unsubscribe", func_unsubscribe, METH_VARARGS, "Unsubscribe from event"},
  {nullptr, nullptr, 0, nullptr}, // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module definition structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyModuleDef module_def =
{
  PyModuleDef_HEAD_INIT,
  "mobius.core",
  "Mobius Forensic Toolkit mobius.core module",
  -1,
  module_methods,
  nullptr,
  nullptr,
  nullptr,
  nullptr
};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module initialization function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pymodule
new_core_module ()
{
  // Initialize module
  mobius::py::pymodule module (&module_def);

  // Add types
  module.add_type ("application", &core_application_t);
  module.add_type ("extension", &core_extension_t);
  module.add_type ("log", &core_log_t);
  module.add_type ("resource", &core_resource_t);
  module.add_type ("thread_guard", new_core_thread_guard_type ());

  // Build submodules
  module.add_submodule ("database", new_core_database_module ());
  module.add_submodule ("file_decoder", new_core_file_decoder_module ());
  module.add_submodule ("kff", new_core_kff_module ());
  module.add_submodule ("system", new_core_system_module ());
  module.add_submodule ("turing", new_core_turing_module ());

  // Return module
  return module;
}


