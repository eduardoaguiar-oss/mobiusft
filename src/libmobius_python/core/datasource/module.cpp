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
// @brief  C++ API mobius.core.datasource module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "module.hpp"
#include "datasource.hpp"
#include "datasource_vfs.hpp"
#include "ufdr/module.hpp"
#include <pymobius.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *func_datasource_new_datasource_from_vfs (PyObject *, PyObject *);
PyObject *func_datasource_new_datasource_by_imagefile_path (PyObject *,
                                                            PyObject *);
PyObject *func_datasource_new_datasource_by_imagefile_url (PyObject *,
                                                           PyObject *);
PyObject *func_datasource_new_datasource_by_ufdr_path (PyObject *, PyObject *);
PyObject *func_datasource_new_datasource_by_ufdr_url (PyObject *, PyObject *);

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module methods
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef module_methods[] = {
    {"new_datasource_from_vfs", func_datasource_new_datasource_from_vfs,
     METH_VARARGS, "Create new datasource from VFS object"},
    {"new_datasource_by_imagefile_path",
     func_datasource_new_datasource_by_imagefile_path, METH_VARARGS,
     "Create new datasource by imagefile path"},
    {"new_datasource_by_imagefile_url",
     func_datasource_new_datasource_by_imagefile_url, METH_VARARGS,
     "Create new datasource by imagefile URL"},
    {"new_datasource_by_ufdr_path", func_datasource_new_datasource_by_ufdr_path,
     METH_VARARGS, "Create new datasource by UFDR file path"},
    {"new_datasource_by_ufdr_url", func_datasource_new_datasource_by_ufdr_url,
     METH_VARARGS, "Create new datasource by UFDR file URL"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module definition structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    "mobius.core.datasource",
    "Mobius Forensic Toolkit mobius.core.datasource module",
    -1,
    module_methods,
    nullptr,
    nullptr,
    nullptr,
    nullptr};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create module
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pymodule
new_core_datasource_module ()
{
    // Initialize module
    mobius::py::pymodule module (&module_def);

    // Add types
    module.add_type ("datasource", &core_datasource_datasource_t);
    module.add_type ("datasource_vfs", &core_datasource_datasource_vfs_t);

    // Build submodules
    module.add_submodule ("ufdr", new_core_datasource_ufdr_module ());

    // Return module
    return module;
}
