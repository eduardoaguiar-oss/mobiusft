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
#include "reader.hpp"
#include <mobius/core/exception.inc>
#include <mobius/core/io/bytearray_io.hpp>
#include <pygil.hpp>
#include <pymobius.hpp>
#include <stdexcept>

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_io_reader_type = nullptr;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief is_seekable getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_is_seekable (core_io_reader_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->is_seekable ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief is_rewindable getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_is_rewindable (core_io_reader_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->is_rewindable ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief size getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_size (core_io_reader_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint64_t (self->obj->get_size ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief getters and setters structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyGetSetDef tp_getset[] = {
    {"is_seekable",
     (getter) tp_getter_is_seekable,
     (setter) 0,
     "check if reader is seekable",
     nullptr},
    {"is_rewindable",
     (getter) tp_getter_is_rewindable,
     (setter) 0,
     "check if reader is rewindable",
     nullptr},
    {"size", (getter) tp_getter_size, (setter) 0, "data size in bytes", nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Read method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_read (core_io_reader_o *self, PyObject *args)
{
    std::uint64_t arg_size = 0;

    // parse input args
    try
    {
        arg_size = mobius::py::get_arg_as_uint64_t (args, 0, self->obj->get_size () - self->obj->tell ());
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
        ret = mobius::py::pybytes_from_bytearray (mobius::py::GIL () (self->obj->read (arg_size)));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_io_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief tell method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_tell (core_io_reader_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_int64_t (self->obj->tell ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief seek method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_seek (core_io_reader_o *self, PyObject *args)
{
    // parse input args
    std::int64_t arg_offset;
    std::uint32_t arg_whence = 0;

    try
    {
        arg_offset = mobius::py::get_arg_as_int64_t (args, 0);
        arg_whence = mobius::py::get_arg_as_uint32_t (args, 1, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // convert 'whence' argument
    mobius::core::io::reader::whence_type w;

    if (arg_whence == 0)
        w = mobius::core::io::reader::whence_type::beginning;

    else if (arg_whence == 1)
        w = mobius::core::io::reader::whence_type::current;

    else if (arg_whence == 2)
        w = mobius::core::io::reader::whence_type::end;

    else
    {
        mobius::py::set_value_error ("invalid 'whence' argument");
        return nullptr;
    }

    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        self->obj->seek (arg_offset, w);
        ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_io_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief rewind method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_rewind (core_io_reader_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        self->obj->rewind ();
        ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief skip method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_skip (core_io_reader_o *self, PyObject *args)
{
    // parse input args
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

    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        self->obj->skip (arg_size);
        ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <b>get_block_size</b> method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_block_size (core_io_reader_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint64_t (self->obj->tell ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] = {
    {"read", (PyCFunction) tp_f_read, METH_VARARGS, "Read bytes from reader"},
    {"tell", (PyCFunction) tp_f_tell, METH_VARARGS, "Get current read position"},
    {"seek", (PyCFunction) tp_f_seek, METH_VARARGS, "Set current read position"},
    {"rewind", (PyCFunction) tp_f_rewind, METH_VARARGS, "Set current read position to the beginning of data"},
    {"skip", (PyCFunction) tp_f_skip, METH_VARARGS, "Set read position n bytes ahead"},
    {"get_block_size", (PyCFunction) tp_f_get_block_size, METH_VARARGS, "Get optimum block size"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief tp_alloc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static core_io_reader_o *
tp_alloc ()
{
    return (core_io_reader_o *) core_io_reader_type->tp_alloc (core_io_reader_type, 0);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief tp_dealloc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_io_reader_o *self)
{
    PyTypeObject *tp = Py_TYPE (self);
    delete self->obj;
    tp->tp_free (reinterpret_cast<PyObject *> (self));
    Py_DECREF (tp);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_io_reader_slots[] = {
    {Py_tp_alloc, reinterpret_cast<void *> (tp_alloc)},
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("core.io.reader class")},
    {Py_tp_getset, reinterpret_cast<void *> (tp_getset)},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_io_reader_spec = {
    .name = "mobius.core.io.reader",
    .basicsize = sizeof (core_io_reader_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_IMMUTABLETYPE | Py_TPFLAGS_IMMUTABLETYPE,
    .slots = core_io_reader_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.io.reader</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_io_reader_type ()
{
    // If type is already created, return it
    if (core_io_reader_type)
        return mobius::py::pytypeobject (core_io_reader_type);

    // Allocate type from spec
    core_io_reader_type = reinterpret_cast<PyTypeObject *> (PyType_FromSpec (&core_io_reader_spec));

    // Create type
    mobius::py::pytypeobject type (core_io_reader_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>core.io.reader</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_io_reader_check (PyObject *value)
{
    if (!core_io_reader_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.io.reader type is not initialized"));

    return mobius::py::isinstance (value, core_io_reader_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.io.reader</i> Python object from C++ object
// @param obj C++ object
// @return New core.io.reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_io_reader_to_pyobject (const mobius::core::io::reader &obj)
{
    if (!core_io_reader_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.io.reader type is not initialized"));

    return mobius::py::to_pyobject_nullable<core_io_reader_o> (obj, core_io_reader_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.io.reader</i> C++ object from Python object
// @param value Python value
// @return core.io.reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::reader
pymobius_core_io_reader_from_pyobject (PyObject *value)
{
    if (!core_io_reader_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.io.reader type is not initialized"));

    mobius::core::io::reader reader;

    if (pymobius_core_io_reader_check (value))
        reader = *(reinterpret_cast<core_io_reader_o *> (value)->obj);

    else if (mobius::py::pybytes_check (value))
    {
        auto bytearray = mobius::py::pybytes_as_bytearray (value);
        reader = mobius::core::io::new_bytearray_reader (bytearray);
    }

    else if (mobius::py::pystring_check (value))
    {
        mobius::core::bytearray data (mobius::py::pystring_as_std_string (value));
        reader = mobius::core::io::new_bytearray_reader (data);
    }

    else
        throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("argument must be either reader or bytes"));

    return reader;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <b>mobius.core.io.new_slice_reader</b> function
// @param self function object
// @param args argument list
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_io_new_slice_reader (PyObject *, PyObject *args)
{
    // parse arguments
    mobius::core::io::reader arg_reader;
    std::int64_t arg_pos;
    std::int64_t arg_end = -1;

    try
    {
        arg_reader = mobius::py::get_arg_as_cpp (args, 0, pymobius_core_io_reader_from_pyobject);
        arg_pos = mobius::py::get_arg_as_int64_t (args, 1);
        arg_end = mobius::py::get_arg_as_int64_t (args, 2, -1);
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
        ret = pymobius_core_io_reader_to_pyobject (mobius::core::io::new_slice_reader (arg_reader, arg_pos, arg_end));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}
