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

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_vfs_vfs_type = nullptr;

} // namespace

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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_vfs_vfs_slots[] = {
    {Py_tp_new, reinterpret_cast<void *> (tp_new)},
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("core.vfs.vfs class")},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_vfs_vfs_spec = {
    .name = "mobius.core.vfs.vfs",
    .basicsize = sizeof (core_vfs_vfs_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = core_vfs_vfs_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.vfs.vfs</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_vfs_vfs_type ()
{
    // If type is already created, return it
    if (core_vfs_vfs_type)
        return mobius::py::pytypeobject (core_vfs_vfs_type);

    // Allocate type from spec
    core_vfs_vfs_type = reinterpret_cast<PyTypeObject *> (
        PyType_FromSpec (&core_vfs_vfs_spec)
    );

    // Create type
    mobius::py::pytypeobject type (core_vfs_vfs_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>core.vfs.vfs</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_vfs_vfs_check (PyObject *value)
{
    if (!core_vfs_vfs_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.vfs.vfs type is not initialized")
        );

    return mobius::py::isinstance (value, core_vfs_vfs_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.vfs.vfs</i> Python object from C++ object
// @param obj C++ object
// @return New core.vfs.vfs object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_vfs_vfs_to_pyobject (const mobius::core::vfs::vfs &obj)
{
    if (!core_vfs_vfs_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.vfs.vfs type is not initialized")
        );

    return mobius::py::to_pyobject<core_vfs_vfs_o> (
        obj, core_vfs_vfs_type
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.vfs.vfs</i> C++ object from Python object
// @param value Python value
// @return core.vfs.vfs object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::vfs::vfs
pymobius_core_vfs_vfs_from_pyobject (PyObject *value)
{
    if (!core_vfs_vfs_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.vfs.vfs type is not initialized")
        );

    return mobius::py::from_pyobject<core_vfs_vfs_o> (
        value, core_vfs_vfs_type
    );
}
