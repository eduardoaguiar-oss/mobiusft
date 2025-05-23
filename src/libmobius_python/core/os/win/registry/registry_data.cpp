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
// @brief C++ API module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "registry_data.hpp"
#include <pymobius.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_data: tp_new (default constructor)
// @param type type object
// @param args argument list
// @param kwds keywords dict
// @return new registry_data object or None if object is null
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_os_win_registry_registry_data_to_pyobject (
    mobius::core::os::win::registry::registry_data r)
{
    PyObject *ret = nullptr;

    if (r)
    {
        ret = _PyObject_New (&core_os_win_registry_registry_data_t);

        if (ret)
            ((core_os_win_registry_registry_data_o *) ret)->obj =
                new mobius::core::os::win::registry::registry_data (r);
    }
    else
        ret = mobius::py::pynone ();

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_data: tp_dealloc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_os_win_registry_registry_data_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_data: size getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_size (core_os_win_registry_registry_data_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint32_t (self->obj->get_size ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_data: type getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_type (core_os_win_registry_registry_data_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_int (
            static_cast<int> (self->obj->get_type ()));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_data: data getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_data (core_os_win_registry_registry_data_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybytes_from_bytearray (self->obj->get_data ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_data: getters and setters structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyGetSetDef tp_getset[] = {
    {(char *) "size", (getter) tp_getter_size, (setter) 0,
     (char *) "data size in bytes", nullptr},
    {(char *) "type", (getter) tp_getter_type, (setter) 0, (char *) "data type",
     nullptr},
    {(char *) "data", (getter) tp_getter_data, (setter) 0,
     (char *) "data buffer", nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_data: get_data_as_dword
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_data_as_dword (core_os_win_registry_registry_data_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint32_t (
            self->obj->get_data_as_dword ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_data: get_data_as_qword
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_data_as_qword (core_os_win_registry_registry_data_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint64_t (
            self->obj->get_data_as_qword ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_data: get_data_as_string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_data_as_string (core_os_win_registry_registry_data_o *self,
                         PyObject *args)
{
    // Parse input args
    std::string arg_encoding;

    try
    {
        arg_encoding = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (
            self->obj->get_data_as_string (arg_encoding));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_data: methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] = {
    {(char *) "get_data_as_dword", (PyCFunction) tp_f_get_data_as_dword,
     METH_VARARGS, "get data as dword"},
    {(char *) "get_data_as_qword", (PyCFunction) tp_f_get_data_as_qword,
     METH_VARARGS, "get data as qword"},
    {(char *) "get_data_as_string", (PyCFunction) tp_f_get_data_as_string,
     METH_VARARGS, "get data as string"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief mobius.core.os.win.registry.registry_data: type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject core_os_win_registry_registry_data_t = {
    PyVarObject_HEAD_INIT (nullptr, 0)             // header
    "mobius.core.os.win.registry.registry_data",   // tp_name
    sizeof (core_os_win_registry_registry_data_o), // tp_basicsize
    0,                                             // tp_itemsize
    (destructor) tp_dealloc,                       // tp_dealloc
    0,                                             // tp_print
    0,                                             // tp_getattr
    0,                                             // tp_setattr
    0,                                             // tp_compare
    0,                                             // tp_repr
    0,                                             // tp_as_number
    0,                                             // tp_as_sequence
    0,                                             // tp_as_mapping
    0,                                             // tp_hash
    0,                                             // tp_call
    0,                                             // tp_str
    0,                                             // tp_getattro
    0,                                             // tp_setattro
    0,                                             // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,      // tp_flags
    "registry_data class",                         // tp_doc
    0,                                             // tp_traverse
    0,                                             // tp_clear
    0,                                             // tp_richcompare
    0,                                             // tp_weaklistoffset
    0,                                             // tp_iter
    0,                                             // tp_iternext
    tp_methods,                                    // tp_methods
    0,                                             // tp_members
    tp_getset,                                     // tp_getset
    0,                                             // tp_base
    0,                                             // tp_dict
    0,                                             // tp_descr_get
    0,                                             // tp_descr_set
    0,                                             // tp_dictoffset
    0,                                             // tp_init
    0,                                             // tp_alloc
    0,                                             // tp_new
    0,                                             // tp_free
    0,                                             // tp_is_gc
    0,                                             // tp_bases
    0,                                             // tp_mro
    0,                                             // tp_cache
    0,                                             // tp_subclasses
    0,                                             // tp_weaklist
    0,                                             // tp_del
    0,                                             // tp_version_tag
    0,                                             // tp_finalize
};
