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
// @file sourcecode.cc C++ API <i>mobius.core.decoder.sourcecode</i> class
// wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "sourcecode.hpp"
#include <mobius/core/exception.inc>
#include <pymobius.hpp>
#include <stdexcept>
#include "core/io/reader.hpp"

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_decoder_sourcecode_type = nullptr;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get</i> method implementation
// @param self Object
// @param args Argument list
// @return Character
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get (core_decoder_sourcecode_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybytes_from_char (self->obj->get ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>peek</i> method implementation
// @param self Object
// @param args Argument list
// @return Character
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_peek (core_decoder_sourcecode_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybytes_from_char (self->obj->peek ());
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
    {(char *) "get", (PyCFunction) tp_f_get, METH_VARARGS,
     "Get next character"},
    {(char *) "peek", (PyCFunction) tp_f_peek, METH_VARARGS,
     "Peek next character"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>sourcecode</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>sourcecode</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *args, PyObject *)
{
    // Parse input args
    mobius::core::io::reader arg_reader;

    try
    {
        arg_reader = pymobius_core_io_reader_from_pyobject (
            mobius::py::get_arg (args, 0)
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Create Python object
    core_decoder_sourcecode_o *ret =
        reinterpret_cast<core_decoder_sourcecode_o *> (
            type->tp_alloc (type, 0)
        );

    if (ret)
    {
        try
        {
            ret->obj = new mobius::core::decoder::sourcecode (arg_reader);
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
// @brief <i>sourcecode</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_decoder_sourcecode_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_decoder_sourcecode_slots[] = {
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("sourcecode class")},
    {Py_tp_new, reinterpret_cast<void *> (tp_new)},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_decoder_sourcecode_spec = {
    .name = "mobius.core.decoder.sourcecode",
    .basicsize = sizeof (core_decoder_sourcecode_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = core_decoder_sourcecode_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.decoder.sourcecode</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_decoder_sourcecode_type ()
{
    // If type is already created, return it
    if (core_decoder_sourcecode_type)
        return mobius::py::pytypeobject (core_decoder_sourcecode_type);

    // Allocate type from spec
    core_decoder_sourcecode_type = reinterpret_cast<PyTypeObject *> (
        PyType_FromSpec (&core_decoder_sourcecode_spec)
    );

    // Create type
    mobius::py::pytypeobject type (core_decoder_sourcecode_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>sourcecode</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_decoder_sourcecode_check (PyObject *value)
{
    if (!core_decoder_sourcecode_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("sourcecode type is not initialized")
        );

    return mobius::py::isinstance (value, core_decoder_sourcecode_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>sourcecode</i> Python object from C++ object
// @param obj C++ object
// @return New sourcecode object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_decoder_sourcecode_to_pyobject (
    const mobius::core::decoder::sourcecode &obj
)
{
    if (!core_decoder_sourcecode_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("sourcecode type is not initialized")
        );

    return mobius::py::to_pyobject<core_decoder_sourcecode_o> (
        obj, core_decoder_sourcecode_type
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>sourcecode</i> C++ object from Python object
// @param value Python value
// @return Sourcecode object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::decoder::sourcecode
pymobius_core_decoder_sourcecode_from_pyobject (PyObject *value)
{
    if (!core_decoder_sourcecode_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("sourcecode type is not initialized")
        );

    return mobius::py::from_pyobject<core_decoder_sourcecode_o> (
        value, core_decoder_sourcecode_type
    );
}
