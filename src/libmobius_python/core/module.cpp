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
#include "richtext.hpp"
#include "thread_guard.hpp"
#include "crypt/module.hpp"
#include "database/module.hpp"
#include "datasource/module.hpp"
#include "datetime/module.hpp"
#include "decoder/module.hpp"
#include "encoder/module.hpp"
#include "core/decoder/module.hpp"
#include "file_decoder/module.hpp"
#include "io/module.hpp"
#include "kff/module.hpp"
#include "os/module.hpp"
#include "pod/module.hpp"
#include "system/module.hpp"
#include "turing/module.hpp"
#include "ui/module.hpp"
#include "vfs/module.hpp"

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
  module.add_type ("richtext", new_core_richtext_type ());
  module.add_type ("thread_guard", new_core_thread_guard_type ());

  // Build submodules
  module.add_submodule ("crypt", new_core_crypt_module ());
  module.add_submodule ("database", new_core_database_module ());
  module.add_submodule ("datasource", new_core_datasource_module ());
  module.add_submodule ("datetime", new_core_datetime_module ());
  module.add_submodule ("decoder", new_core_decoder_module ());
  module.add_submodule ("encoder", new_core_encoder_module ());
  module.add_submodule ("file_decoder", new_core_file_decoder_module ());
  module.add_submodule ("io", new_core_io_module ());
  module.add_submodule ("kff", new_core_kff_module ());
  module.add_submodule ("os", new_core_os_module ());
  module.add_submodule ("pod", new_core_pod_module ());
  module.add_submodule ("system", new_core_system_module ());
  module.add_submodule ("turing", new_core_turing_module ());
  module.add_submodule ("ui", new_core_ui_module ());
  module.add_submodule ("vfs", new_core_vfs_module ());

  // Return module
  return module;
}


