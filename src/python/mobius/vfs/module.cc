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
//! \brief  C++ API mobius.vfs module wrapper
//! \author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "module.h"
#include <pymobius.h>
#include "block.h"
#include "disk.h"
#include "filesystem.h"
#include "imagefile.h"
#include "vfs.h"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Function prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject* func_vfs_get_imagefile_implementations (PyObject *, PyObject *);
PyObject* func_vfs_new_disk_by_device_uid (PyObject *, PyObject *);
PyObject* func_vfs_new_disk_by_path (PyObject *, PyObject *);
PyObject* func_vfs_new_disk_by_url (PyObject *, PyObject *);
PyObject* func_vfs_new_imagefile_by_path (PyObject *, PyObject *);
PyObject* func_vfs_new_imagefile_by_url (PyObject *, PyObject *);
PyObject* func_vfs_new_imagefile_from_file (PyObject *, PyObject *);

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Module methods
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef module_methods[] =
{
  {
    "get_imagefile_implementations",
    func_vfs_get_imagefile_implementations,
    METH_VARARGS,
    "Get imagefile implementations available"
  },
  {
    "new_disk_by_device_uid",
    func_vfs_new_disk_by_device_uid,
    METH_VARARGS,
    "Create new disk by device UID"
  },
  {
    "new_disk_by_path",
    func_vfs_new_disk_by_path,
    METH_VARARGS,
    "Create new disk by file path"
  },
  {
    "new_disk_by_url",
    func_vfs_new_disk_by_url,
    METH_VARARGS,
    "Create new disk by file URL"
  },
  {
    "new_imagefile_by_path",
    func_vfs_new_imagefile_by_path,
    METH_VARARGS,
    "Create new imagefile by file path"
  },
  {
    "new_imagefile_by_url",
    func_vfs_new_imagefile_by_url,
    METH_VARARGS,
    "Create new imagefile by file URL"
  },
  {
    "new_imagefile_from_file",
    func_vfs_new_imagefile_from_file,
    METH_VARARGS,
    "Create new imagefile from file"
  },
  {
    NULL,
    NULL,
    0,
    NULL
  } // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Module definition structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyModuleDef module_def =
{
  PyModuleDef_HEAD_INIT,
  "mobius.vfs",
  "Mobius Forensic Toolkit mobius.vfs module",
  -1,
  module_methods,
  nullptr,
  nullptr,
  nullptr,
  nullptr
};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Create module
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pymodule
new_vfs_module ()
{
  // Initialize module
  mobius::py::pymodule module (&module_def);

  // Add types
  module.add_type ("block", &vfs_block_t);
  module.add_type ("disk", &vfs_disk_t);
  module.add_type ("filesystem", &vfs_filesystem_t);
  module.add_type ("imagefile", &vfs_imagefile_t);
  module.add_type ("vfs", &vfs_vfs_t);

  // Add constants
  module.add_constant ("STATUS_UNKNOWN", 1);

  // Return module
  return module;
}
