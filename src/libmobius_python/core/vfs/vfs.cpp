// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025
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
// @file vfs.cc C++ API <i>mobius.core.vfs.vfs</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "vfs.hpp"
#include <mobius/core/exception.inc>
#include <pygil.hpp>
#include <pylist.hpp>
#include <pymobius.hpp>
#include <stdexcept>
#include "block.hpp"
#include "core/io/entry.hpp"
#include "core/pod/map.hpp"
#include "disk.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>vfs</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_vfs_vfs_check (PyObject *value)
{
    return mobius::py::isinstance (value, &core_vfs_vfs_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>vfs</i> Python object from C++ object
// @param obj C++ object
// @return New vfs object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_vfs_vfs_to_pyobject (const mobius::core::vfs::vfs &obj)
{
    return mobius::py::to_pyobject<core_vfs_vfs_o> (obj, &core_vfs_vfs_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>vfs</i> C++ object from Python object
// @param value Python value
// @return Vfs object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::vfs::vfs
pymobius_core_vfs_vfs_from_pyobject (PyObject *value)
{
    return mobius::py::from_pyobject<core_vfs_vfs_o> (value, &core_vfs_vfs_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_state</i> method implementation
// @param self Object
// @param args Argument list
// @return Object state
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_state (core_vfs_vfs_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_pod_map_to_pyobject (
            mobius::py::GIL () (self->obj->get_state ())
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>clear</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_clear (core_vfs_vfs_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        mobius::py::GIL GIL;
        self->obj->clear ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>rescan</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_rescan (core_vfs_vfs_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        mobius::py::GIL GIL;
        self->obj->rescan ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_available</i> method implementation
// @param self Object
// @param args Argument list
// @return True/False
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_available (core_vfs_vfs_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (
            mobius::py::GIL () (self->obj->is_available ())
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>add_disk</i> method implementation
// @param self Object
// @param args Argument list
// @return Datasource index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_add_disk (core_vfs_vfs_o *self, PyObject *args)
{
    // Parse input args
    mobius::core::vfs::disk arg_d;

    try
    {
        arg_d = mobius::py::get_arg_as_cpp (
            args, 0, pymobius_core_vfs_disk_from_pyobject
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = PyLong_FromSize_t (
            mobius::py::GIL () (self->obj->add_disk (arg_d))
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>remove_disk</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_remove_disk (core_vfs_vfs_o *self, PyObject *args)
{
    // Parse input args
    std::size_t arg_idx;

    try
    {
        arg_idx = mobius::py::get_arg_as_uint64_t (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        mobius::py::GIL GIL;
        self->obj->remove_disk (arg_idx);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_disks</i> method implementation
// @param self Object
// @param args Argument list
// @return Datasources
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_disks (core_vfs_vfs_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylist_from_cpp_container (
            mobius::py::GIL () (self->obj->get_disks ()),
            pymobius_core_vfs_disk_to_pyobject
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_blocks</i> method implementation
// @param self Object
// @param args Argument list
// @return Blocks
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_blocks (core_vfs_vfs_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylist_from_cpp_container (
            mobius::py::GIL () (self->obj->get_blocks ()),
            pymobius_core_vfs_block_to_pyobject
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_root_entries</i> method implementation
// @param self Object
// @param args Argument list
// @return Root entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_root_entries (core_vfs_vfs_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylist_from_cpp_container (
            mobius::py::GIL () (self->obj->get_root_entries ()),
            pymobius_core_io_entry_to_pyobject
        );
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
    {(char *) "get_state", (PyCFunction) tp_f_get_state, METH_VARARGS,
     "Get object state"},
    {(char *) "clear", (PyCFunction) tp_f_clear, METH_VARARGS, "Clear VFS"},
    {(char *) "rescan", (PyCFunction) tp_f_rescan, METH_VARARGS,
     "Rescan blocks and root entries"},
    {(char *) "is_available", (PyCFunction) tp_f_is_available, METH_VARARGS,
     "Check if VFS is available"},
    {(char *) "add_disk", (PyCFunction) tp_f_add_disk, METH_VARARGS,
     "Add disk"},
    {(char *) "remove_disk", (PyCFunction) tp_f_remove_disk, METH_VARARGS,
     "Remove disk"},
    {(char *) "get_disks", (PyCFunction) tp_f_get_disks, METH_VARARGS,
     "Get disks"},
    {(char *) "get_blocks", (PyCFunction) tp_f_get_blocks, METH_VARARGS,
     "Get blocks"},
    {(char *) "get_root_entries", (PyCFunction) tp_f_get_root_entries,
     METH_VARARGS, "Get root entries"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>vfs</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>vfs</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *, PyObject *)
{
    core_vfs_vfs_o *ret =
        reinterpret_cast<core_vfs_vfs_o *> (type->tp_alloc (type, 0));

    if (ret)
    {
        try
        {
            ret->obj = new mobius::core::vfs::vfs ();
        }
        catch (const std::exception &e)
        {
            Py_DECREF (ret);
            mobius::py::set_runtime_error (e.what ());
            ret = nullptr;
        }
    }

    return reinterpret_cast<PyObject *> (ret);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>vfs</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_vfs_vfs_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject core_vfs_vfs_t = {
    PyVarObject_HEAD_INIT (nullptr, 0)        // header
    "mobius.core.vfs.vfs",                    // tp_name
    sizeof (core_vfs_vfs_o),                  // tp_basicsize
    0,                                        // tp_itemsize
    (destructor) tp_dealloc,                  // tp_dealloc
    0,                                        // tp_print
    0,                                        // tp_getattr
    0,                                        // tp_setattr
    0,                                        // tp_compare
    0,                                        // tp_repr
    0,                                        // tp_as_number
    0,                                        // tp_as_sequence
    0,                                        // tp_as_mapping
    0,                                        // tp_hash
    0,                                        // tp_call
    0,                                        // tp_str
    0,                                        // tp_getattro
    0,                                        // tp_setattro
    0,                                        // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    "Virtual Filesystem (VFS) class",         // tp_doc
    0,                                        // tp_traverse
    0,                                        // tp_clear
    0,                                        // tp_richcompare
    0,                                        // tp_weaklistoffset
    0,                                        // tp_iter
    0,                                        // tp_iternext
    tp_methods,                               // tp_methods
    0,                                        // tp_members
    0,                                        // tp_getset
    0,                                        // tp_base
    0,                                        // tp_dict
    0,                                        // tp_descr_get
    0,                                        // tp_descr_set
    0,                                        // tp_dictoffset
    0,                                        // tp_init
    0,                                        // tp_alloc
    tp_new,                                   // tp_new
    0,                                        // tp_free
    0,                                        // tp_is_gc
    0,                                        // tp_bases
    0,                                        // tp_mro
    0,                                        // tp_cache
    0,                                        // tp_subclasses
    0,                                        // tp_weaklist
    0,                                        // tp_del
    0,                                        // tp_version_tag
    0,                                        // tp_finalize
};
