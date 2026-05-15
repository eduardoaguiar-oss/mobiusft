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
// @file inifile.cc C++ API <i>mobius.core.decoder.inifile</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "inifile.hpp"
#include <mobius/core/exception.inc>
#include <pymobius.hpp>
#include <pydict.hpp>
#include <pygil.hpp>
#include <stdexcept>
#include "core/io/reader.hpp"

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_decoder_inifile_type = nullptr;

} // namespace 

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_case_sensitive</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_case_sensitive (core_decoder_inifile_o *self, PyObject *args)
{
    // Parse input args
    bool arg_flag;

    try
    {
        arg_flag = mobius::py::get_arg_as_bool (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->set_case_sensitive (arg_flag);
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
// @brief <i>set_comment_char</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_comment_char (core_decoder_inifile_o *self, PyObject *args)
{
    // Parse input args
    char arg_c;

    try
    {
        arg_c = mobius::py::get_arg_as_char (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->set_comment_char (arg_c);
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
// @brief <i>set_value_char</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_value_char (core_decoder_inifile_o *self, PyObject *args)
{
    // Parse input args
    char arg_c;

    try
    {
        arg_c = mobius::py::get_arg_as_char (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->set_value_char (arg_c);
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
// @brief <i>has_value</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_has_value (core_decoder_inifile_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_group;
    std::string arg_name;

    try
    {
        arg_group = mobius::py::get_arg_as_std_string (args, 0);
        arg_name = mobius::py::get_arg_as_std_string (args, 1);
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
        ret = mobius::py::pybool_from_bool (
            self->obj->has_value (arg_group, arg_name)
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
// @brief <i>get_value</i> method implementation
// @param self Object
// @param args Argument list
// @return Value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_value (core_decoder_inifile_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_group;
    std::string arg_name;

    try
    {
        arg_group = mobius::py::get_arg_as_std_string (args, 0);
        arg_name = mobius::py::get_arg_as_std_string (args, 1);
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
        ret = mobius::py::pystring_from_std_string (
            self->obj->get_value (arg_group, arg_name)
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
// @brief <i>get_values</i> method implementation
// @param self Object
// @param args Argument list
// @return Map of key/value pairs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_values (core_decoder_inifile_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_group;

    try
    {
        arg_group = mobius::py::get_arg_as_std_string (args, 0);
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
        ret = mobius::py::pydict_from_cpp_container (
            self->obj->get_values (arg_group),
            mobius::py::pystring_from_std_string,
            mobius::py::pystring_from_std_string
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
    {"set_case_sensitive", (PyCFunction) tp_f_set_case_sensitive, METH_VARARGS,
     "Set group and key to be case sensitive or not"},
    {"set_comment_char", (PyCFunction) tp_f_set_comment_char, METH_VARARGS,
     "Set char used to start a comment"},
    {"set_value_char", (PyCFunction) tp_f_set_value_char, METH_VARARGS,
     "Set char used to separate key and value"},
    {"has_value", (PyCFunction) tp_f_has_value, METH_VARARGS,
     "Check if inifile has a given value"},
    {"get_value", (PyCFunction) tp_f_get_value, METH_VARARGS, "Get value"},
    {"get_values", (PyCFunction) tp_f_get_values, METH_VARARGS,
     "Get map of key/value pairs for a given group"},
    {nullptr, nullptr, 0, nullptr}, // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>inifile</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>inifile</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *args, PyObject *)
{
    // Parse input args
    mobius::core::io::reader arg_reader;
    std::string arg_encoding;
    std::string arg_separator;

    try
    {
        arg_reader = mobius::py::get_arg_as_cpp (
            args, 0, pymobius_core_io_reader_from_pyobject
        );
        arg_encoding = mobius::py::get_arg_as_std_string (args, 1, "UTF-8");
        arg_separator = mobius::py::get_arg_as_std_string (args, 2, {});
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Create Python object
    core_decoder_inifile_o *ret =
        reinterpret_cast<core_decoder_inifile_o *> (type->tp_alloc (type, 0));

    if (ret)
    {
        try
        {
            ret->obj = mobius::py::GIL () (new mobius::core::decoder::inifile (
                arg_reader, arg_encoding, arg_separator
            ));
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
// @brief <i>inifile</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_decoder_inifile_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_decoder_inifile_slots[] = {
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("inifile class")},
    {Py_tp_new, reinterpret_cast<void *> (tp_new)},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_decoder_inifile_spec = {
    .name = "mobius.core.decoder.inifile",
    .basicsize = sizeof (core_decoder_inifile_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = core_decoder_inifile_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.decoder.inifile</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_decoder_inifile_type ()
{
    // If type is already created, return it
    if (core_decoder_inifile_type)
        return mobius::py::pytypeobject (core_decoder_inifile_type);

    // Allocate type from spec
    core_decoder_inifile_type = reinterpret_cast<PyTypeObject *> (
        PyType_FromSpec (&core_decoder_inifile_spec)
    );

    // Create type
    mobius::py::pytypeobject type (core_decoder_inifile_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>inifile</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_decoder_inifile_check (PyObject *value)
{
    if (!core_decoder_inifile_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("inifile type is not initialized")
        );

    return mobius::py::isinstance (value, core_decoder_inifile_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>inifile</i> Python object from C++ object
// @param obj C++ object
// @return New inifile object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_decoder_inifile_to_pyobject (
    const mobius::core::decoder::inifile &obj
)
{
    if (!core_decoder_inifile_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("inifile type is not initialized")
        );

    return mobius::py::to_pyobject<core_decoder_inifile_o> (
        obj, core_decoder_inifile_type
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>inifile</i> C++ object from Python object
// @param value Python value
// @return Inifile object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::decoder::inifile
pymobius_core_decoder_inifile_from_pyobject (PyObject *value)
{
    if (!core_decoder_inifile_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("inifile type is not initialized")
        );

    return mobius::py::from_pyobject<core_decoder_inifile_o> (
        value, core_decoder_inifile_type
    );
}
