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
// @file uri.cc C++ API <i>mobius.core.io.uri</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "uri.hpp"
#include "module.hpp"
#include <mobius/core/exception.inc>
#include <pymobius.hpp>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if object type is <i>uri</i>
// @param pyobj Python object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_io_uri_check (PyObject *pyobj)
{
    return PyObject_IsInstance (pyobj, (PyObject *) &core_io_uri_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>uri</i> Python object from C++ object
// @param obj C++ object
// @return new uri object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_io_uri_to_pyobject (mobius::core::io::uri obj)
{
    PyObject *ret = _PyObject_New (&core_io_uri_t);

    if (ret)
        ((core_io_uri_o *) ret)->obj = new mobius::core::io::uri (obj);

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>uri</i> C++ object from Python object
// @param pyobj Python object
// @return uri object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::uri
pymobius_core_io_uri_from_pyobject (PyObject *pyobj)
{
    if (!PyObject_IsInstance (pyobj, (PyObject *) &core_io_uri_t))
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("object type must be mobius.core.io.uri"));

    return *(reinterpret_cast<core_io_uri_o *> (pyobj)->obj);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_value</i> method implementation
// @param self Object
// @param args Argument list
// @return URI
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_value (core_io_uri_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_value ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_scheme</i> method implementation
// @param self Object
// @param args Argument list
// @return Scheme
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_scheme (core_io_uri_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_scheme ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_authority</i> method implementation
// @param self Object
// @param args Argument list
// @return Authority
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_authority (core_io_uri_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret =
            mobius::py::pystring_from_std_string (self->obj->get_authority ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_path</i> method implementation
// @param self Object
// @param args Argument list
// @return Path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_path (core_io_uri_o *self, PyObject *args)
{
    // parse input args
    std::string arg_encoding;

    try
    {
        arg_encoding =
            mobius::py::get_arg_as_std_string (args, 0, std::string ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (
            self->obj->get_path (arg_encoding));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_query</i> method implementation
// @param self Object
// @param args Argument list
// @return Query
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_query (core_io_uri_o *self, PyObject *args)
{
    // parse input args
    std::string arg_encoding;

    try
    {
        arg_encoding =
            mobius::py::get_arg_as_std_string (args, 0, std::string ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (
            self->obj->get_query (arg_encoding));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_fragment</i> method implementation
// @param self Object
// @param args Argument list
// @return Fragment
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_fragment (core_io_uri_o *self, PyObject *args)
{
    // parse input args
    std::string arg_encoding;

    try
    {
        arg_encoding =
            mobius::py::get_arg_as_std_string (args, 0, std::string ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (
            self->obj->get_fragment (arg_encoding));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_username</i> method implementation
// @param self Object
// @param args Argument list
// @return User name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_username (core_io_uri_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_username ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_password</i> method implementation
// @param self Object
// @param args Argument list
// @return Password
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_password (core_io_uri_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_password ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_host</i> method implementation
// @param self Object
// @param args Argument list
// @return Host
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_host (core_io_uri_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_host ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_port</i> method implementation
// @param self Object
// @param args Argument list
// @return Port
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_port (core_io_uri_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_port ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_parent</i> method implementation
// @param self Object
// @param args Argument list
// @return Directory
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_parent (core_io_uri_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_io_uri_to_pyobject (self->obj->get_parent ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_sibling_by_name</i> method implementation
// @param self Object
// @param args Argument list
// @return Directory
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_sibling_by_name (core_io_uri_o *self, PyObject *args)
{
    // parse input args
    std::string arg_filename;

    try
    {
        arg_filename = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_io_uri_to_pyobject (
            self->obj->get_sibling_by_name (arg_filename));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_sibling_by_extension</i> method implementation
// @param self Object
// @param args Argument list
// @return Directory
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_sibling_by_extension (core_io_uri_o *self, PyObject *args)
{
    // parse input args
    std::string arg_extension;

    try
    {
        arg_extension = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_io_uri_to_pyobject (
            self->obj->get_sibling_by_extension (arg_extension));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_child_by_name</i> method implementation
// @param self Object
// @param args Argument list
// @return Directory
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_child_by_name (core_io_uri_o *self, PyObject *args)
{
    // parse input args
    std::string arg_name;

    try
    {
        arg_name = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_io_uri_to_pyobject (
            self->obj->get_child_by_name (arg_name));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_child_by_path</i> method implementation
// @param self Object
// @param args Argument list
// @return Directory
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_child_by_path (core_io_uri_o *self, PyObject *args)
{
    // parse input args
    std::string arg_subpath;

    try
    {
        arg_subpath = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_io_uri_to_pyobject (
            self->obj->get_child_by_path (arg_subpath));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_filename</i> method implementation
// @param self Object
// @param args Argument list
// @return Filename
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_filename (core_io_uri_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_filename ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_extension</i> method implementation
// @param self Object
// @param args Argument list
// @return Get file extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_extension (core_io_uri_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret =
            mobius::py::pystring_from_std_string (self->obj->get_extension ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_empty</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_empty (core_io_uri_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->is_empty ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_relative</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_relative (core_io_uri_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->is_relative ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_absolute</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_absolute (core_io_uri_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->is_absolute ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] = {
    {(char *) "get_value", (PyCFunction) tp_f_get_value, METH_VARARGS,
     "Get URI as string"},
    {(char *) "get_scheme", (PyCFunction) tp_f_get_scheme, METH_VARARGS,
     "Get scheme"},
    {(char *) "get_authority", (PyCFunction) tp_f_get_authority, METH_VARARGS,
     "Get authority"},
    {(char *) "get_path", (PyCFunction) tp_f_get_path, METH_VARARGS,
     "Get path"},
    {(char *) "get_query", (PyCFunction) tp_f_get_query, METH_VARARGS,
     "Get query"},
    {(char *) "get_fragment", (PyCFunction) tp_f_get_fragment, METH_VARARGS,
     "Get fragment"},
    {(char *) "get_username", (PyCFunction) tp_f_get_username, METH_VARARGS,
     "Get user name"},
    {(char *) "get_password", (PyCFunction) tp_f_get_password, METH_VARARGS,
     "Get password"},
    {(char *) "get_host", (PyCFunction) tp_f_get_host, METH_VARARGS,
     "Get host"},
    {(char *) "get_port", (PyCFunction) tp_f_get_port, METH_VARARGS,
     "Get port"},
    {(char *) "get_parent", (PyCFunction) tp_f_get_parent, METH_VARARGS,
     "Get parent URI"},
    {(char *) "get_sibling_by_name", (PyCFunction) tp_f_get_sibling_by_name,
     METH_VARARGS, "Get sibling URI by name"},
    {(char *) "get_sibling_by_extension",
     (PyCFunction) tp_f_get_sibling_by_extension, METH_VARARGS,
     "Get sibling URI by extension"},
    {(char *) "get_child_by_name", (PyCFunction) tp_f_get_child_by_name,
     METH_VARARGS, "Get child URI by name"},
    {(char *) "get_child_by_path", (PyCFunction) tp_f_get_child_by_path,
     METH_VARARGS, "Get child URI by path"},
    {(char *) "get_filename", (PyCFunction) tp_f_get_filename, METH_VARARGS,
     "Get filename"},
    {(char *) "get_extension", (PyCFunction) tp_f_get_extension, METH_VARARGS,
     "Get extension"},
    {(char *) "is_empty", (PyCFunction) tp_f_is_empty, METH_VARARGS,
     "Check if URI is empty"},
    {(char *) "is_relative", (PyCFunction) tp_f_is_relative, METH_VARARGS,
     "Check if URI is relative"},
    {(char *) "is_absolute", (PyCFunction) tp_f_is_absolute, METH_VARARGS,
     "Check if URI is absolute"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>uri</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>uri</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *args, PyObject *)
{
    // Parse input args
    std::string arg_value;
    std::string arg_scheme;
    std::string arg_username;
    std::string arg_password;
    std::string arg_host;
    std::string arg_port;
    std::string arg_path;
    std::string arg_query;
    std::string arg_fragment;

    try
    {
        if (mobius::py::get_arg_size (args) == 1)
            arg_value = mobius::py::get_arg_as_std_string (args, 0);

        else
        {
            arg_scheme = mobius::py::get_arg_as_std_string (args, 0);
            arg_username = mobius::py::get_arg_as_std_string (args, 1);
            arg_password = mobius::py::get_arg_as_std_string (args, 2);
            arg_host = mobius::py::get_arg_as_std_string (args, 3);
            arg_port = mobius::py::get_arg_as_std_string (args, 4);
            arg_path = mobius::py::get_arg_as_std_string (args, 5);
            arg_query = mobius::py::get_arg_as_std_string (args, 6, "");
            arg_fragment = mobius::py::get_arg_as_std_string (args, 7, "");
        }
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // Create Python object
    core_io_uri_o *ret =
        reinterpret_cast<core_io_uri_o *> (type->tp_alloc (type, 0));

    if (ret)
    {
        try
        {
            if (!arg_value.empty ())
                ret->obj = new mobius::core::io::uri (arg_value);

            else
                ret->obj = new mobius::core::io::uri (
                    arg_scheme, arg_username, arg_password, arg_host, arg_port,
                    arg_path, arg_query, arg_fragment);
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
// @brief <i>uri</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_io_uri_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject core_io_uri_t = {
    PyVarObject_HEAD_INIT (nullptr, 0)        // header
    "mobius.core.io.uri",                     // tp_name
    sizeof (core_io_uri_o),                   // tp_basicsize
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
    "uri class",                              // tp_doc
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
