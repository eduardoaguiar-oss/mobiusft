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
// @file sequential_reader_adaptor.cc C++ API
// <i>mobius.core.io.sequential_reader_adaptor</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "sequential_reader_adaptor.hpp"
#include "reader.hpp"
#include <mobius/core/exception.inc>
#include <pymobius.hpp>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>sequential_reader_adaptor</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_io_sequential_reader_adaptor_check (PyObject *value)
{
    return PyObject_IsInstance (
        value,
        reinterpret_cast<PyObject *> (&core_io_sequential_reader_adaptor_t));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>sequential_reader_adaptor</i> Python object from C++ object
// @param obj C++ object
// @return New sequential_reader_adaptor object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_io_sequential_reader_adaptor_to_pyobject (
    const mobius::core::io::sequential_reader_adaptor &obj)
{
    PyObject *ret = _PyObject_New (&core_io_sequential_reader_adaptor_t);

    if (ret)
        ((core_io_sequential_reader_adaptor_o *) ret)->obj =
            new mobius::core::io::sequential_reader_adaptor (obj);

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>sequential_reader_adaptor</i> C++ object from Python object
// @param value Python value
// @return Sequential_reader_adaptor object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::sequential_reader_adaptor
pymobius_core_io_sequential_reader_adaptor_from_pyobject (PyObject *value)
{
    if (!pymobius_core_io_sequential_reader_adaptor_check (value))
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("object must be an instance of "
                                  "mobius.core.io.sequential_reader_adaptor"));

    return *(
        reinterpret_cast<core_io_sequential_reader_adaptor_o *> (value)->obj);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>block_size</i> Attribute getter
// @param self Object
// @return <i>block_size</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_block_size (core_io_sequential_reader_adaptor_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret =
            mobius::py::pylong_from_std_uint64_t (self->obj->get_block_size ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Getters and setters structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyGetSetDef tp_getset[] = {
    {(char *) "block_size", (getter) tp_getter_block_size, (setter) 0,
     (char *) "Read ahead block size in bytes", nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>peek</i> method implementation
// @param self Object
// @param args Argument list
// @return Byte read
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_peek (core_io_sequential_reader_adaptor_o *self, PyObject *args)
{
    // Parse input args
    std::uint64_t arg_size;

    try
    {
        arg_size = mobius::py::get_arg_as_uint64_t (args, 0, 1);
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
        if (arg_size == 1)
            ret = mobius::py::pylong_from_std_uint8_t (self->obj->peek ());

        else
            ret =
                mobius::py::pybytes_from_bytearray (self->obj->peek (arg_size));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get</i> method implementation
// @param self Object
// @param args Argument list
// @return Byte read
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get (core_io_sequential_reader_adaptor_o *self, PyObject *args)
{
    // Parse input args
    std::uint64_t arg_size;

    try
    {
        arg_size = mobius::py::get_arg_as_uint64_t (args, 0, 1);
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
        if (arg_size == 1)
            ret = mobius::py::pylong_from_std_uint8_t (self->obj->get ());

        else
            ret =
                mobius::py::pybytes_from_bytearray (self->obj->get (arg_size));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>skip</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_skip (core_io_sequential_reader_adaptor_o *self, PyObject *args)
{
    // Parse input args
    std::uint64_t arg_size;

    try
    {
        arg_size = mobius::py::get_arg_as_uint64_t (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->skip (arg_size);
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
// @brief <i>tell</i> method implementation
// @param self Object
// @param args Argument list
// @return Current reading position from the beginning of the reader
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_tell (core_io_sequential_reader_adaptor_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint64_t (self->obj->tell ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>eof</i> method implementation
// @param self Object
// @param args Argument list
// @return True/False
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_eof (core_io_sequential_reader_adaptor_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->eof ());
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
    {(char *) "peek", (PyCFunction) tp_f_peek, METH_VARARGS,
     "Peek one or more bytes, without moving the reading position"},
    {(char *) "get", (PyCFunction) tp_f_get, METH_VARARGS,
     "Get one or more bytes"},
    {(char *) "skip", (PyCFunction) tp_f_skip, METH_VARARGS,
     "Skip size bytes forward"},
    {(char *) "tell", (PyCFunction) tp_f_tell, METH_VARARGS,
     "Get current reading position"},
    {(char *) "eof", (PyCFunction) tp_f_eof, METH_VARARGS,
     "Check if end-of-file (EOF) is reached"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>sequential_reader_adaptor</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>sequential_reader_adaptor</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *args, PyObject *)
{
    // Parse input args
    mobius::core::io::reader arg_reader;
    std::uint64_t arg_block_size;

    try
    {
        arg_reader = mobius::py::get_arg_as_cpp (
            args, 0, pymobius_core_io_reader_from_pyobject);
        arg_block_size = mobius::py::get_arg_as_uint64_t (args, 1, 65536);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Create Python object
    core_io_sequential_reader_adaptor_o *ret =
        reinterpret_cast<core_io_sequential_reader_adaptor_o *> (
            type->tp_alloc (type, 0));

    if (ret)
    {
        try
        {
            ret->obj = new mobius::core::io::sequential_reader_adaptor (
                arg_reader, arg_block_size);
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
// @brief <i>sequential_reader_adaptor</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_io_sequential_reader_adaptor_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject core_io_sequential_reader_adaptor_t = {
    PyVarObject_HEAD_INIT (nullptr, 0)            // header
    "mobius.core.io.sequential_reader_adaptor",   // tp_name
    sizeof (core_io_sequential_reader_adaptor_o), // tp_basicsize
    0,                                            // tp_itemsize
    (destructor) tp_dealloc,                      // tp_dealloc
    0,                                            // tp_print
    0,                                            // tp_getattr
    0,                                            // tp_setattr
    0,                                            // tp_compare
    0,                                            // tp_repr
    0,                                            // tp_as_number
    0,                                            // tp_as_sequence
    0,                                            // tp_as_mapping
    0,                                            // tp_hash
    0,                                            // tp_call
    0,                                            // tp_str
    0,                                            // tp_getattro
    0,                                            // tp_setattro
    0,                                            // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,     // tp_flags
    "sequential_reader_adaptor class",            // tp_doc
    0,                                            // tp_traverse
    0,                                            // tp_clear
    0,                                            // tp_richcompare
    0,                                            // tp_weaklistoffset
    0,                                            // tp_iter
    0,                                            // tp_iternext
    tp_methods,                                   // tp_methods
    0,                                            // tp_members
    tp_getset,                                    // tp_getset
    0,                                            // tp_base
    0,                                            // tp_dict
    0,                                            // tp_descr_get
    0,                                            // tp_descr_set
    0,                                            // tp_dictoffset
    0,                                            // tp_init
    0,                                            // tp_alloc
    tp_new,                                       // tp_new
    0,                                            // tp_free
    0,                                            // tp_is_gc
    0,                                            // tp_bases
    0,                                            // tp_mro
    0,                                            // tp_cache
    0,                                            // tp_subclasses
    0,                                            // tp_weaklist
    0,                                            // tp_del
    0,                                            // tp_version_tag
    0,                                            // tp_finalize
};
