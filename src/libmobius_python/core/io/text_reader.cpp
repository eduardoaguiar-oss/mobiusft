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
#include "text_reader.hpp"
#include <mobius/core/exception.inc>
#include <mobius/core/io/bytearray_io.hpp>
#include <limits>
#include <pygil.hpp>
#include <pymobius.hpp>
#include <stdexcept>
#include "reader.hpp"

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_io_text_reader_type = nullptr;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Read all data from text_reader
// @param text_reader Pointer to text_reader object
// @return Data as C++ string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
_read_all (mobius::core::io::text_reader *text_reader)
{
    std::string ret;
    std::string data = text_reader->read (65536);

    while (!data.empty ())
    {
        ret += data;
        data = text_reader->read (65536);
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Read method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_read (core_io_text_reader_o *self, PyObject *args)
{
    std::uint64_t arg_size = 0;
    constexpr std::uint64_t max_size = std::numeric_limits<std::uint64_t>::max ();

    // parse input args
    try
    {
        arg_size = mobius::py::get_arg_as_uint64_t (args, 0, max_size);
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
        ret = mobius::py::to_pyobject (
            mobius::py::GIL () ((arg_size == max_size) ? _read_all (self->obj) : self->obj->read (arg_size))
        );
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
    {"read", (PyCFunction) tp_f_read, METH_VARARGS, "Read bytes from text_reader"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief tp_dealloc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_io_text_reader_o *self)
{
    PyTypeObject *tp = Py_TYPE (self);
    delete self->obj;
    tp->tp_free (reinterpret_cast<PyObject *> (self));
    Py_DECREF (tp);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>text_reader</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>text_reader</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *args, PyObject *)
{
    // Parse input args
    mobius::core::io::reader arg_reader;
    std::string arg_encoding;

    try
    {
        arg_reader = mobius::py::get_arg_as_cpp (args, 0, pymobius_core_io_reader_from_pyobject);
        arg_encoding = mobius::py::get_arg_as_std_string (args, 1, "UTF-8");
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Create Python object
    core_io_text_reader_o *ret = reinterpret_cast<core_io_text_reader_o *> (type->tp_alloc (type, 0));

    if (ret)
    {
        try
        {
            ret->obj = new mobius::core::io::text_reader (arg_reader, arg_encoding);
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_io_text_reader_slots[] = {
    {Py_tp_new, reinterpret_cast<void *> (tp_new)},
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("core.io.text_reader class")},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_io_text_reader_spec = {
    .name = "mobius.core.io.text_reader",
    .basicsize = sizeof (core_io_text_reader_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_IMMUTABLETYPE | Py_TPFLAGS_IMMUTABLETYPE,
    .slots = core_io_text_reader_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.io.text_reader</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_io_text_reader_type ()
{
    // If type is already created, return it
    if (core_io_text_reader_type)
        return mobius::py::pytypeobject (core_io_text_reader_type);

    // Allocate type from spec
    core_io_text_reader_type = reinterpret_cast<PyTypeObject *> (PyType_FromSpec (&core_io_text_reader_spec));

    // Create type
    mobius::py::pytypeobject type (core_io_text_reader_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>core.io.text_reader</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_io_text_reader_check (PyObject *value)
{
    if (!core_io_text_reader_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.io.text_reader type is not initialized"));

    return mobius::py::isinstance (value, core_io_text_reader_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.io.text_reader</i> Python object from C++ object
// @param obj C++ object
// @return New core.io.text_reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_io_text_reader_to_pyobject (const mobius::core::io::text_reader &obj)
{
    if (!core_io_text_reader_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.io.text_reader type is not initialized"));

    return mobius::py::to_pyobject<core_io_text_reader_o> (obj, core_io_text_reader_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.io.text_reader</i> C++ object from Python object
// @param value Python value
// @return core.io.text_reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::text_reader
pymobius_core_io_text_reader_from_pyobject (PyObject *value)
{
    if (!core_io_text_reader_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.io.text_reader type is not initialized"));

    return mobius::py::from_pyobject<core_io_text_reader_o> (value, core_io_text_reader_type);
}
