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
// @brief  C++ API mobius.framework module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "module.h"
#include "attribute.h"
#include "category.h"
#include "evidence_loader.h"
#include <pymobius.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Functions prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject* func_framework_has_config (PyObject *, PyObject *);
PyObject* func_framework_set_config (PyObject *, PyObject *);
PyObject* func_framework_get_config (PyObject *, PyObject *);
PyObject* func_framework_remove_config (PyObject *, PyObject *);
PyObject* func_framework_new_config_transaction (PyObject *, PyObject *);

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module methods
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef module_methods[] =
{
  {"get_category", func_framework_get_category, METH_VARARGS, "Get category"},
  {"new_category", func_framework_new_category, METH_VARARGS, "Create new category"},
  {"remove_category", func_framework_remove_category, METH_VARARGS, "Remove category"},
  {"get_categories", func_framework_get_categories, METH_VARARGS, "Get all categories"},
  {"remove_categories", func_framework_remove_categories, METH_VARARGS, "Remove all categories"},
  {"new_category_transaction", func_framework_new_category_transaction, METH_VARARGS, "Create new transaction object to category database"},
  {"has_config", func_framework_has_config, METH_VARARGS, "Check if config values exists"},
  {"set_config", func_framework_set_config, METH_VARARGS, "Set config value"},
  {"get_config", func_framework_get_config, METH_VARARGS, "Get config value"},
  {"remove_config", func_framework_remove_config, METH_VARARGS, "Remove config value"},
  {"new_config_transaction", func_framework_new_config_transaction, METH_VARARGS, "Create new transaction object to config database"},
  {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module definition structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyModuleDef module_def =
{
  PyModuleDef_HEAD_INIT,
  "mobius.framework",
  "Mobius Forensic Toolkit mobius.framework module",
  -1,
  module_methods,
  nullptr,
  nullptr,
  nullptr,
  nullptr
};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create module
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pymodule
new_framework_module ()
{
  // Initialize module
  mobius::py::pymodule module (&module_def);

  // Add types
  module.add_type ("attribute", new_framework_attribute_type ());
  module.add_type ("category", new_framework_category_type ());
  module.add_type ("evidence_loader", new_framework_evidence_loader_type ());

  // Add constants
  module.add_constant ("SCAN_TYPE_CANONICAL_FOLDERS", 1);
  module.add_constant ("SCAN_TYPE_ALL_FOLDERS", 2);
  module.add_constant ("SCAN_TYPE_ALL_FILES", 3);

  // Return module
  return module;
}


