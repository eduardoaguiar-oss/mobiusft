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
// @file datasource_vfs.cc C++ API <i>mobius.core.datasource.datasource_vfs</i>
// class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "datasource_vfs.hpp"
#include "core/vfs/vfs.hpp"
#include "datasource.hpp"
#include <mobius/core/exception.inc>
#include <pymobius.hpp>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>datasource_vfs</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_datasource_datasource_vfs_check (PyObject *value)
{
    return mobius::py::isinstance (value, &core_datasource_datasource_vfs_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>datasource_vfs</i> Python object from C++ object
// @param obj C++ object
// @return New datasource_vfs object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_datasource_datasource_vfs_to_pyobject (
    const mobius::core::datasource::datasource_vfs &obj)
{
    return mobius::py::to_pyobject<core_datasource_datasource_vfs_o> (
        obj, &core_datasource_datasource_vfs_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>datasource_vfs</i> C++ object from Python object
// @param value Python value
// @return Datasource_vfs object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datasource::datasource_vfs
pymobius_core_datasource_datasource_vfs_from_pyobject (PyObject *value)
{
    return mobius::py::from_pyobject<core_datasource_datasource_vfs_o> (
        value, &core_datasource_datasource_vfs_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_vfs</i> method implementation
// @param self Object
// @param args Argument list
// @return VFS object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_vfs (core_datasource_datasource_vfs_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_vfs_vfs_to_pyobject (self->obj->get_vfs ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] = {
    {(char *) "get_vfs", (PyCFunction) tp_f_get_vfs, METH_VARARGS,
     "Get VFS object"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>datasource_vfs</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_datasource_datasource_vfs_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject core_datasource_datasource_vfs_t = {
    PyVarObject_HEAD_INIT (nullptr, 0)         // header
    "mobius.core.datasource.datasource_vfs",   // tp_name
    sizeof (core_datasource_datasource_vfs_o), // tp_basicsize
    0,                                         // tp_itemsize
    (destructor) tp_dealloc,                   // tp_dealloc
    0,                                         // tp_print
    0,                                         // tp_getattr
    0,                                         // tp_setattr
    0,                                         // tp_compare
    0,                                         // tp_repr
    0,                                         // tp_as_number
    0,                                         // tp_as_sequence
    0,                                         // tp_as_mapping
    0,                                         // tp_hash
    0,                                         // tp_call
    0,                                         // tp_str
    0,                                         // tp_getattro
    0,                                         // tp_setattro
    0,                                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  // tp_flags
    "VFS datasource class",                    // tp_doc
    0,                                         // tp_traverse
    0,                                         // tp_clear
    0,                                         // tp_richcompare
    0,                                         // tp_weaklistoffset
    0,                                         // tp_iter
    0,                                         // tp_iternext
    tp_methods,                                // tp_methods
    0,                                         // tp_members
    0,                                         // tp_getset
    &core_datasource_datasource_t,             // tp_base
    0,                                         // tp_dict
    0,                                         // tp_descr_get
    0,                                         // tp_descr_set
    0,                                         // tp_dictoffset
    0,                                         // tp_init
    0,                                         // tp_alloc
    0,                                         // tp_new
    0,                                         // tp_free
    0,                                         // tp_is_gc
    0,                                         // tp_bases
    0,                                         // tp_mro
    0,                                         // tp_cache
    0,                                         // tp_subclasses
    0,                                         // tp_weaklist
    0,                                         // tp_del
    0,                                         // tp_version_tag
    0,                                         // tp_finalize
};
