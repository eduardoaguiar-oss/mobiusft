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
// @brief C++ API module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "line_reader.hpp"
#include <mobius/core/exception.inc>
#include <mobius/core/io/bytearray_io.hpp>
#include <limits>
#include <pygil.hpp>
#include <pymobius.hpp>
#include <stdexcept>
#include "reader.hpp"
#include "text_reader.hpp"

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_io_line_reader_type = nullptr;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Read method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_read (core_io_line_reader_o *self, PyObject *)
{
    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        std::string line;
        bool rc = mobius::py::GIL () (self->obj->read (line));

        if (rc)
            ret = mobius::py::pystring_from_std_string (line);

        else
            ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_io_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] = {
    {"read", (PyCFunction) tp_f_read, METH_VARARGS, "Read line"}, {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief tp_alloc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static core_io_line_reader_o *
tp_alloc ()
{
    return (core_io_line_reader_o *) core_io_line_reader_type->tp_alloc (core_io_line_reader_type, 0);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief tp_dealloc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_io_line_reader_o *self)
{
    PyTypeObject *tp = Py_TYPE (self);
    delete self->obj;
    tp->tp_free (reinterpret_cast<PyObject *> (self));
    Py_DECREF (tp);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>line_reader</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>line_reader</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *args, PyObject *)
{
    // Parse input args
    mobius::core::io::text_reader arg_text_reader;
    std::string arg_separator;

    try
    {
        // line_reader (text_reader, separator)
        if (mobius::py::check_arg (args, 0, pymobius_core_io_text_reader_check))
        {
            arg_text_reader = mobius::py::get_arg_as_cpp (args, 0, pymobius_core_io_text_reader_from_pyobject);
            arg_separator = mobius::py::get_arg_as_std_string (args, 1, "");
        }

        // line_reader (reader, encoding, separator)
        else
        {
            auto arg_reader = mobius::py::get_arg_as_cpp (args, 0, pymobius_core_io_reader_from_pyobject);
            auto arg_encoding = mobius::py::get_arg_as_std_string (args, 1, "UTF-8");

            arg_text_reader = mobius::core::io::text_reader (arg_reader, arg_encoding);
            arg_separator = mobius::py::get_arg_as_std_string (args, 2, "");
        }
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Create Python object
    core_io_line_reader_o *ret = reinterpret_cast<core_io_line_reader_o *> (type->tp_alloc (type, 0));

    if (ret)
    {
        try
        {
            ret->obj = new mobius::core::io::line_reader (arg_text_reader, arg_separator);
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
// @brief <i>line_reader</i> iterator creator
// @param self Self object
// @return New iterator
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_iter (PyObject *self)
{
    Py_INCREF (self);
    return self;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>line_reader</i> iterator next function
// @param self Self object
// @return New value or raise StopIteration
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_iternext (PyObject *self)
{
    PyObject *ret = nullptr;

    try
    {
        std::string line;

        if (reinterpret_cast<core_io_line_reader_o *> (self)->obj->read (line))
            ret = mobius::py::pystring_from_std_string (line);

        else
            mobius::py::set_stop_iteration ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_io_line_reader_slots[] = {
    {Py_tp_new, reinterpret_cast<void *> (tp_new)},
    {Py_tp_alloc, reinterpret_cast<void *> (tp_alloc)},
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("core.io.line_reader class")},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {Py_tp_iter, reinterpret_cast<void *> (tp_iter)},
    {Py_tp_iternext, reinterpret_cast<void *> (tp_iternext)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_io_line_reader_spec = {
    .name = "mobius.core.io.line_reader",
    .basicsize = sizeof (core_io_line_reader_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_IMMUTABLETYPE | Py_TPFLAGS_IMMUTABLETYPE,
    .slots = core_io_line_reader_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.io.line_reader</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_io_line_reader_type ()
{
    // If type is already created, return it
    if (core_io_line_reader_type)
        return mobius::py::pytypeobject (core_io_line_reader_type);

    // Allocate type from spec
    core_io_line_reader_type = reinterpret_cast<PyTypeObject *> (PyType_FromSpec (&core_io_line_reader_spec));

    // Create type
    mobius::py::pytypeobject type (core_io_line_reader_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>core.io.line_reader</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_io_line_reader_check (PyObject *value)
{
    if (!core_io_line_reader_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.io.line_reader type is not initialized"));

    return mobius::py::isinstance (value, core_io_line_reader_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.io.line_reader</i> Python object from C++ object
// @param obj C++ object
// @return New core.io.line_reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_io_line_reader_to_pyobject (const mobius::core::io::line_reader &obj)
{
    if (!core_io_line_reader_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.io.line_reader type is not initialized"));

    return mobius::py::to_pyobject<core_io_line_reader_o> (obj, core_io_line_reader_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.io.line_reader</i> C++ object from Python object
// @param value Python value
// @return core.io.line_reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::line_reader
pymobius_core_io_line_reader_from_pyobject (PyObject *value)
{
    if (!core_io_line_reader_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.io.line_reader type is not initialized"));

    return mobius::py::from_pyobject<core_io_line_reader_o> (value, core_io_line_reader_type);
}
