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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief  C++ API mobius.core.vfs module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "module.hpp"
#include "block.hpp"
#include "disk.hpp"
#include "filesystem.hpp"
#include "imagefile.hpp"
#include "vfs.hpp"
#include <pymobius.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *func_vfs_get_imagefile_implementations (PyObject *, PyObject *);
PyObject *func_vfs_new_disk_by_device_uid (PyObject *, PyObject *);
PyObject *func_vfs_new_disk_by_path (PyObject *, PyObject *);
PyObject *func_vfs_new_disk_by_url (PyObject *, PyObject *);
PyObject *func_vfs_new_imagefile_by_path (PyObject *, PyObject *);
PyObject *func_vfs_new_imagefile_by_url (PyObject *, PyObject *);
PyObject *func_vfs_new_imagefile_from_file (PyObject *, PyObject *);

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module methods
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef module_methods[] = {
    {"get_imagefile_implementations", func_vfs_get_imagefile_implementations,
     METH_VARARGS, "Get imagefile implementations available"},
    {"new_disk_by_device_uid", func_vfs_new_disk_by_device_uid, METH_VARARGS,
     "Create new disk by device UID"},
    {"new_disk_by_path", func_vfs_new_disk_by_path, METH_VARARGS,
     "Create new disk by file path"},
    {"new_disk_by_url", func_vfs_new_disk_by_url, METH_VARARGS,
     "Create new disk by file URL"},
    {"new_imagefile_by_path", func_vfs_new_imagefile_by_path, METH_VARARGS,
     "Create new imagefile by file path"},
    {"new_imagefile_by_url", func_vfs_new_imagefile_by_url, METH_VARARGS,
     "Create new imagefile by file URL"},
    {"new_imagefile_from_file", func_vfs_new_imagefile_from_file, METH_VARARGS,
     "Create new imagefile from file"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module definition structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    "mobius.core.vfs",
    "Mobius Forensic Toolkit mobius.core.vfs module",
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
new_core_vfs_module ()
{
    // Initialize module
    mobius::py::pymodule module (&module_def);

    // Add types
    module.add_type ("block", &core_vfs_block_t);
    module.add_type ("disk", &core_vfs_disk_t);
    module.add_type ("filesystem", &core_vfs_filesystem_t);
    module.add_type ("imagefile", &core_vfs_imagefile_t);
    module.add_type ("vfs", &core_vfs_vfs_t);

    // Add constants
    module.add_constant ("STATUS_UNKNOWN", 1);

    // Return module
    return module;
}
