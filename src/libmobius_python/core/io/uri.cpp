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
// @file uri.cc C++ API <i>mobius.core.io.uri</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "uri.hpp"
#include <mobius/core/exception.inc>
#include <pymobius.hpp>
#include <stdexcept>
#include "module.hpp"

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_io_uri_type = nullptr;

} // namespace

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
        ret = mobius::py::to_pyobject (self->obj->get_value ());
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
        ret = mobius::py::to_pyobject (self->obj->get_scheme ());
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
        ret = mobius::py::pystring_from_std_string (self->obj->get_authority ());
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
        arg_encoding = mobius::py::get_arg_as_std_string (args, 0, std::string ());
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
        ret = mobius::py::to_pyobject (self->obj->get_path (arg_encoding));
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
        arg_encoding = mobius::py::get_arg_as_std_string (args, 0, std::string ());
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
        ret = mobius::py::to_pyobject (self->obj->get_query (arg_encoding));
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
        arg_encoding = mobius::py::get_arg_as_std_string (args, 0, std::string ());
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
        ret = mobius::py::to_pyobject (self->obj->get_fragment (arg_encoding));
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
        ret = mobius::py::to_pyobject (self->obj->get_username ());
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
        ret = mobius::py::to_pyobject (self->obj->get_password ());
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
        ret = mobius::py::to_pyobject (self->obj->get_host ());
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
        ret = mobius::py::to_pyobject (self->obj->get_port ());
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
        ret = pymobius_core_io_uri_to_pyobject (self->obj->get_sibling_by_name (arg_filename));
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
        ret = pymobius_core_io_uri_to_pyobject (self->obj->get_sibling_by_extension (arg_extension));
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
        ret = pymobius_core_io_uri_to_pyobject (self->obj->get_child_by_name (arg_name));
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
        ret = pymobius_core_io_uri_to_pyobject (self->obj->get_child_by_path (arg_subpath));
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
        ret = mobius::py::to_pyobject (self->obj->get_filename ());
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
        ret = mobius::py::pystring_from_std_string (self->obj->get_extension ());
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
        ret = mobius::py::to_pyobject (self->obj->is_empty ());
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
        ret = mobius::py::to_pyobject (self->obj->is_relative ());
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
        ret = mobius::py::to_pyobject (self->obj->is_absolute ());
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
    {"get_value", (PyCFunction) tp_f_get_value, METH_VARARGS, "Get URI as string"},
    {"get_scheme", (PyCFunction) tp_f_get_scheme, METH_VARARGS, "Get scheme"},
    {"get_authority", (PyCFunction) tp_f_get_authority, METH_VARARGS, "Get authority"},
    {"get_path", (PyCFunction) tp_f_get_path, METH_VARARGS, "Get path"},
    {"get_query", (PyCFunction) tp_f_get_query, METH_VARARGS, "Get query"},
    {"get_fragment", (PyCFunction) tp_f_get_fragment, METH_VARARGS, "Get fragment"},
    {"get_username", (PyCFunction) tp_f_get_username, METH_VARARGS, "Get user name"},
    {"get_password", (PyCFunction) tp_f_get_password, METH_VARARGS, "Get password"},
    {"get_host", (PyCFunction) tp_f_get_host, METH_VARARGS, "Get host"},
    {"get_port", (PyCFunction) tp_f_get_port, METH_VARARGS, "Get port"},
    {"get_parent", (PyCFunction) tp_f_get_parent, METH_VARARGS, "Get parent URI"},
    {"get_sibling_by_name", (PyCFunction) tp_f_get_sibling_by_name, METH_VARARGS, "Get sibling URI by name"},
    {"get_sibling_by_extension",
     (PyCFunction) tp_f_get_sibling_by_extension,
     METH_VARARGS,
     "Get sibling URI by extension"},
    {"get_child_by_name", (PyCFunction) tp_f_get_child_by_name, METH_VARARGS, "Get child URI by name"},
    {"get_child_by_path", (PyCFunction) tp_f_get_child_by_path, METH_VARARGS, "Get child URI by path"},
    {"get_filename", (PyCFunction) tp_f_get_filename, METH_VARARGS, "Get filename"},
    {"get_extension", (PyCFunction) tp_f_get_extension, METH_VARARGS, "Get extension"},
    {"is_empty", (PyCFunction) tp_f_is_empty, METH_VARARGS, "Check if URI is empty"},
    {"is_relative", (PyCFunction) tp_f_is_relative, METH_VARARGS, "Check if URI is relative"},
    {"is_absolute", (PyCFunction) tp_f_is_absolute, METH_VARARGS, "Check if URI is absolute"},
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
    core_io_uri_o *ret = reinterpret_cast<core_io_uri_o *> (type->tp_alloc (type, 0));

    if (ret)
    {
        try
        {
            if (!arg_value.empty ())
                ret->obj = new mobius::core::io::uri (arg_value);

            else
                ret->obj = new mobius::core::io::uri (
                    arg_scheme,
                    arg_username,
                    arg_password,
                    arg_host,
                    arg_port,
                    arg_path,
                    arg_query,
                    arg_fragment
                );
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
    PyTypeObject *tp = Py_TYPE (self);
    delete self->obj;
    tp->tp_free (reinterpret_cast<PyObject *> (self));
    Py_DECREF (tp);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_io_uri_slots[] = {
    {Py_tp_new, reinterpret_cast<void *> (tp_new)},
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("core.io.uri()\n\n--\n\ncore.io.uri class")},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_io_uri_spec = {
    .name = "mobius.core.io.uri",
    .basicsize = sizeof (core_io_uri_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_IMMUTABLETYPE,
    .slots = core_io_uri_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.io.uri</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_io_uri_type ()
{
    // If type is already created, return it
    if (core_io_uri_type)
        return mobius::py::pytypeobject (core_io_uri_type);

    // Allocate type from spec
    core_io_uri_type = reinterpret_cast<PyTypeObject *> (PyType_FromSpec (&core_io_uri_spec));

    if (!core_io_uri_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("Failed to create mobius.core.io.uri type"));

    // Create type
    mobius::py::pytypeobject type (core_io_uri_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>core.io.uri</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_io_uri_check (PyObject *value)
{
    if (!core_io_uri_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.io.uri type is not initialized"));

    return mobius::py::isinstance (value, core_io_uri_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.io.uri</i> Python object from C++ object
// @param obj C++ object
// @return New core.io.uri object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_io_uri_to_pyobject (const mobius::core::io::uri &obj)
{
    if (!core_io_uri_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.io.uri type is not initialized"));

    return mobius::py::to_pyobject<core_io_uri_o> (obj, core_io_uri_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.io.uri</i> C++ object from Python object
// @param value Python value
// @return core.io.uri object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::uri
pymobius_core_io_uri_from_pyobject (PyObject *value)
{
    if (!core_io_uri_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.io.uri type is not initialized"));

    return mobius::py::from_pyobject<core_io_uri_o> (value, core_io_uri_type);
}
