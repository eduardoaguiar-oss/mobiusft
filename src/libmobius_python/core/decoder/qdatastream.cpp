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
// @file qdatastream.cc C++ API <i>mobius.core.decoder.qdatastream</i> class
// wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "qdatastream.hpp"
#include <mobius/core/exception.inc>
#include <pymobius.hpp>
#include <stdexcept>
#include "core/io/reader.hpp"
#include "core/pod/data.hpp"
#include "module.hpp"

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_decoder_qdatastream_type = nullptr;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>eof</i> method implementation
// @param self object
// @param args argument list
// @return Boolean
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_eof (core_decoder_qdatastream_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (!bool (*self->obj));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qint8</i> method implementation
// @param self object
// @param args argument list
// @return 8-bits signed value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qint8 (core_decoder_qdatastream_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_int8_t (self->obj->get_qint8 ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qint16</i> method implementation
// @param self object
// @param args argument list
// @return 16-bits value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qint16 (core_decoder_qdatastream_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_int16_t (self->obj->get_qint16 ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qint32</i> method implementation
// @param self object
// @param args argument list
// @return 32-bits value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qint32 (core_decoder_qdatastream_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_int32_t (self->obj->get_qint32 ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qint64</i> method implementation
// @param self object
// @param args argument list
// @return 64-bits value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qint64 (core_decoder_qdatastream_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_int64_t (self->obj->get_qint64 ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_quint8</i> method implementation
// @param self object
// @param args argument list
// @return 8-bits value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_quint8 (core_decoder_qdatastream_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint8_t (self->obj->get_quint8 ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qstring</i> method implementation
// @param self object
// @param args argument list
// @return QString as std::string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qstring (core_decoder_qdatastream_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_qstring ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qbytearray</i> method implementation
// @param self object
// @param args argument list
// @return data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qbytearray (core_decoder_qdatastream_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybytes_from_bytearray (self->obj->get_qbytearray ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qdatetime</i> method implementation
// @param self object
// @param args argument list
// @return QDateTime as datetime
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qdatetime (core_decoder_qdatastream_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret =
            mobius::py::pydatetime_from_datetime (self->obj->get_qdatetime ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qimage</i> method implementation
// @param self object
// @param args argument list
// @return data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qimage (core_decoder_qdatastream_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybytes_from_bytearray (self->obj->get_qimage ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qpixmap</i> method implementation
// @param self object
// @param args argument list
// @return data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qpixmap (core_decoder_qdatastream_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybytes_from_bytearray (self->obj->get_qpixmap ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qvariantap</i> method implementation
// @param self object
// @param args argument list
// @return data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qvariant (core_decoder_qdatastream_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_pod_data_to_pyobject (self->obj->get_qvariant ());
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
    {"eof", (PyCFunction) tp_f_eof, METH_VARARGS,
     "Return EOF indicator for stream"},
    {"get_qint8", (PyCFunction) tp_f_get_qint8, METH_VARARGS,
     "Get 8-bits signed value"},
    {"get_qint16", (PyCFunction) tp_f_get_qint16, METH_VARARGS,
     "Get 16-bits signed value"},
    {"get_qint32", (PyCFunction) tp_f_get_qint32, METH_VARARGS,
     "Get 32-bits signed value"},
    {"get_qint64", (PyCFunction) tp_f_get_qint64, METH_VARARGS,
     "Get 64-bits signed value"},
    {"get_quint8", (PyCFunction) tp_f_get_quint8, METH_VARARGS,
     "Get 8-bits unsigned value"},
    {"get_qstring", (PyCFunction) tp_f_get_qstring, METH_VARARGS,
     "Get QString"},
    {"get_qbytearray", (PyCFunction) tp_f_get_qbytearray, METH_VARARGS,
     "Get QBytearray"},
    {"get_qdatetime", (PyCFunction) tp_f_get_qdatetime, METH_VARARGS,
     "Get QDateTime"},
    {"get_qimage", (PyCFunction) tp_f_get_qimage, METH_VARARGS,
     "Get QImage data"},
    {"get_qpixmap", (PyCFunction) tp_f_get_qpixmap, METH_VARARGS,
     "Get QPixmap data"},
    {"get_qvariant", (PyCFunction) tp_f_get_qvariant, METH_VARARGS,
     "Get QVariant data"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>qdatastream</i> constructor
// @param type type object
// @param args argument list
// @param kwds keywords dict
// @return new <i>qdatastream</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *args, PyObject *)
{
    // Parse input args
    mobius::core::io::reader arg_reader;
    std::uint32_t version = mobius::core::decoder::qdatastream::QT_NEWEST;

    try
    {
        arg_reader = mobius::py::get_arg_as_cpp (
            args, 0, pymobius_core_io_reader_from_pyobject
        );
        version = mobius::py::get_arg_as_uint32_t (
            args, 1, mobius::core::decoder::qdatastream::QT_NEWEST
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Create Python object
    core_decoder_qdatastream_o *ret =
        reinterpret_cast<core_decoder_qdatastream_o *> (
            type->tp_alloc (type, 0)
        );

    if (ret)
    {
        try
        {
            ret->obj =
                new mobius::core::decoder::qdatastream (arg_reader, version);
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
// @brief <i>qdatastream</i> deallocator
// @param self object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_decoder_qdatastream_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_decoder_qdatastream_slots[] = {
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("qdatastream class")},
    {Py_tp_new, reinterpret_cast<void *> (tp_new)},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_decoder_qdatastream_spec = {
    .name = "mobius.core.decoder.qdatastream",
    .basicsize = sizeof (core_decoder_qdatastream_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = core_decoder_qdatastream_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.decoder.qdatastream</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_decoder_qdatastream_type ()
{
    // If type is already created, return it
    if (core_decoder_qdatastream_type)
        return mobius::py::pytypeobject (core_decoder_qdatastream_type);

    // Allocate type from spec
    core_decoder_qdatastream_type = reinterpret_cast<PyTypeObject *> (
        PyType_FromSpec (&core_decoder_qdatastream_spec)
    );

    // Create type
    mobius::py::pytypeobject type (core_decoder_qdatastream_type);
    type.create ();

    type.add_constant ("QT_1_0", mobius::core::decoder::qdatastream::QT_1_0);
    type.add_constant ("QT_2_0", mobius::core::decoder::qdatastream::QT_2_0);
    type.add_constant ("QT_2_1", mobius::core::decoder::qdatastream::QT_2_1);
    type.add_constant ("QT_3_0", mobius::core::decoder::qdatastream::QT_3_0);
    type.add_constant ("QT_3_1", mobius::core::decoder::qdatastream::QT_3_1);
    type.add_constant ("QT_3_3", mobius::core::decoder::qdatastream::QT_3_3);
    type.add_constant ("QT_4_0", mobius::core::decoder::qdatastream::QT_4_0);
    type.add_constant ("QT_4_1", mobius::core::decoder::qdatastream::QT_4_1);
    type.add_constant ("QT_4_2", mobius::core::decoder::qdatastream::QT_4_2);
    type.add_constant ("QT_4_3", mobius::core::decoder::qdatastream::QT_4_3);
    type.add_constant ("QT_4_4", mobius::core::decoder::qdatastream::QT_4_4);
    type.add_constant ("QT_4_5", mobius::core::decoder::qdatastream::QT_4_5);
    type.add_constant ("QT_4_6", mobius::core::decoder::qdatastream::QT_4_6);
    type.add_constant ("QT_4_7", mobius::core::decoder::qdatastream::QT_4_7);
    type.add_constant ("QT_4_8", mobius::core::decoder::qdatastream::QT_4_8);
    type.add_constant ("QT_4_9", mobius::core::decoder::qdatastream::QT_4_9);
    type.add_constant ("QT_5_0", mobius::core::decoder::qdatastream::QT_5_0);
    type.add_constant ("QT_5_1", mobius::core::decoder::qdatastream::QT_5_1);
    type.add_constant ("QT_5_2", mobius::core::decoder::qdatastream::QT_5_2);
    type.add_constant ("QT_5_3", mobius::core::decoder::qdatastream::QT_5_3);
    type.add_constant ("QT_5_4", mobius::core::decoder::qdatastream::QT_5_4);
    type.add_constant ("QT_5_5", mobius::core::decoder::qdatastream::QT_5_5);
    type.add_constant ("QT_5_6", mobius::core::decoder::qdatastream::QT_5_6);
    type.add_constant ("QT_5_7", mobius::core::decoder::qdatastream::QT_5_7);
    type.add_constant ("QT_5_8", mobius::core::decoder::qdatastream::QT_5_8);
    type.add_constant ("QT_5_9", mobius::core::decoder::qdatastream::QT_5_9);
    type.add_constant ("QT_5_10", mobius::core::decoder::qdatastream::QT_5_10);
    type.add_constant ("QT_5_11", mobius::core::decoder::qdatastream::QT_5_11);
    type.add_constant ("QT_5_12", mobius::core::decoder::qdatastream::QT_5_12);
    type.add_constant ("QT_5_13", mobius::core::decoder::qdatastream::QT_5_13);
    type.add_constant ("QT_5_14", mobius::core::decoder::qdatastream::QT_5_14);
    type.add_constant ("QT_5_15", mobius::core::decoder::qdatastream::QT_5_15);
    type.add_constant ("QT_6_0", mobius::core::decoder::qdatastream::QT_6_0);
    type.add_constant (
        "QT_NEWEST", mobius::core::decoder::qdatastream::QT_NEWEST
    );

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>qdatastream</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_decoder_qdatastream_check (PyObject *value)
{
    if (!core_decoder_qdatastream_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("qdatastream type is not initialized")
        );

    return mobius::py::isinstance (value, core_decoder_qdatastream_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>qdatastream</i> Python object from C++ object
// @param obj C++ object
// @return New qdatastream object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_decoder_qdatastream_to_pyobject (
    const mobius::core::decoder::qdatastream &obj
)
{
    if (!core_decoder_qdatastream_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("qdatastream type is not initialized")
        );

    return mobius::py::to_pyobject<core_decoder_qdatastream_o> (
        obj, core_decoder_qdatastream_type
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>qdatastream</i> C++ object from Python object
// @param value Python value
// @return Qdatastream object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::decoder::qdatastream
pymobius_core_decoder_qdatastream_from_pyobject (PyObject *value)
{
    if (!core_decoder_qdatastream_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("qdatastream type is not initialized")
        );

    return mobius::py::from_pyobject<core_decoder_qdatastream_o> (
        value, core_decoder_qdatastream_type
    );
}
