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
#include "registry_file.hpp"
#include "registry_key.hpp"
#include <pymobius.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_file: tp_new (default constructor)
// @param type type object
// @param args argument list
// @param kwds keywords dict
// @return new registry_file object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_os_win_registry_registry_file_to_pyobject (
    mobius::core::os::win::registry::registry_file r)
{
    PyObject *ret = _PyObject_New (&core_os_win_registry_registry_file_t);

    if (ret)
        ((core_os_win_registry_registry_file_o *) ret)->obj =
            new mobius::core::os::win::registry::registry_file (r);

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_file: tp_dealloc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_os_win_registry_registry_file_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_file: uid getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_uid (core_os_win_registry_registry_file_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint32_t (self->obj->get_uid ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_file: role getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_role (core_os_win_registry_registry_file_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_role ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_file: path getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_path (core_os_win_registry_registry_file_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_path ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_file: last_modification_time getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_last_modification_time (core_os_win_registry_registry_file_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pydatetime_from_datetime (
            self->obj->get_last_modification_time ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_file: root_key getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_root_key (core_os_win_registry_registry_file_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_os_win_registry_registry_key_to_pyobject (
            self->obj->get_root_key ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_file: getters and setters structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyGetSetDef tp_getset[] = {
    {(char *) "uid", (getter) tp_getter_uid, (setter) 0,
     (char *) "unique identifier", nullptr},
    {(char *) "role", (getter) tp_getter_role, (setter) 0, (char *) "file role",
     nullptr},
    {(char *) "path", (getter) tp_getter_path, (setter) 0,
     (char *) "original path", nullptr},
    {(char *) "last_modification_time",
     (getter) tp_getter_last_modification_time, (setter) 0,
     (char *) "last modification time", nullptr},
    {(char *) "root_key", (getter) tp_getter_root_key, (setter) 0,
     (char *) "root key", nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief registry_file: methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] = {
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief mobius.core.os.win.registry.registry_file: type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject core_os_win_registry_registry_file_t = {
    PyVarObject_HEAD_INIT (nullptr, 0)             // header
    "mobius.core.os.win.registry.registry_file",   // tp_name
    sizeof (core_os_win_registry_registry_file_o), // tp_basicsize
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
    "registry_file class",                         // tp_doc
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
