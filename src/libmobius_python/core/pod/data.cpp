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
// @file data.cc C++ API <i>mobius.core.pod.data</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "data.hpp"
#include <mobius/core/exception.inc>
#include <pylist.hpp>
#include <pymobius.hpp>
#include <pyobject.hpp>
#include <stdexcept>
#include <vector>
#include "map.hpp"

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_pod_data_type = nullptr;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create std::vector <data> from PyTuple
// @param py_value Python object
// @return C++ vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::vector<mobius::core::pod::data>
pymobius_core_pod_data_vector_from_pytuple (PyObject *py_value)
{
    std::vector<mobius::core::pod::data> v;

    Py_ssize_t siz = PyTuple_Size (py_value);

    for (Py_ssize_t i = 0; i < siz; i++)
    {
        PyObject *item = PyTuple_GetItem (py_value, i);

        if (!item)
            throw std::runtime_error (
                MOBIUS_EXCEPTION_MSG (mobius::py::get_error_message ())
            );

        v.push_back (pymobius_core_pod_data_from_pyobject (item));
    }

    return v;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create std::vector <data> from PySet
// @param py_value Python object
// @return C++ vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::vector<mobius::core::pod::data>
pymobius_core_pod_data_vector_from_pyset (PyObject *py_value)
{
    std::vector<mobius::core::pod::data> v;

    mobius::py::pyobject iter = PyObject_GetIter (py_value);
    if (!iter)
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG (mobius::py::get_error_message ())
        );

    mobius::py::pyobject item = PyIter_Next (iter);

    while (item)
    {
        v.push_back (pymobius_core_pod_data_from_pyobject (item));
        item = PyIter_Next (iter);
    }

    return v;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>type</i> Attribute getter
// @param self Object
// @return <i>type</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_type (core_pod_data_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_int (
            static_cast<int> (self->obj->get_type ())
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>value</i> Attribute getter
// @param self Object
// @return <i>value</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_value (core_pod_data_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_pod_data_to_pyobject (*(self->obj));
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
    {"type", (getter) tp_getter_type, (setter) 0, "Data type",
     nullptr},
    {"value", (getter) tp_getter_value, (setter) 0, "Value",
     nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>clone</i> method implementation
// @param self Object
// @param args Argument list
// @return New data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_clone (core_pod_data_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_pod_data_to_pyobject (self->obj->clone ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_null</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_null (core_pod_data_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->is_null ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_bool</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_bool (core_pod_data_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->is_bool ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_integer</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_integer (core_pod_data_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->is_integer ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_float</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_float (core_pod_data_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->is_float ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_datetime</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_datetime (core_pod_data_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->is_datetime ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_string</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_string (core_pod_data_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->is_string ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_bytearray</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_bytearray (core_pod_data_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->is_bytearray ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_list</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_list (core_pod_data_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->is_list ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_map</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_map (core_pod_data_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->is_map ());
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
    {"clone", (PyCFunction) tp_f_clone, METH_VARARGS,
     "Clone data object"},
    {"is_null", (PyCFunction) tp_f_is_null, METH_VARARGS,
     "Check if data is null"},
    {"is_bool", (PyCFunction) tp_f_is_bool, METH_VARARGS,
     "Check if data is boolean"},
    {"is_integer", (PyCFunction) tp_f_is_integer, METH_VARARGS,
     "Check if data is integer"},
    {"is_float", (PyCFunction) tp_f_is_float, METH_VARARGS,
     "Check if data is float"},
    {"is_datetime", (PyCFunction) tp_f_is_datetime, METH_VARARGS,
     "Check if data is datetime"},
    {"is_string", (PyCFunction) tp_f_is_string, METH_VARARGS,
     "Check if data is string"},
    {"is_bytearray", (PyCFunction) tp_f_is_bytearray, METH_VARARGS,
     "Check if data is bytearray"},
    {"is_list", (PyCFunction) tp_f_is_list, METH_VARARGS,
     "Check if data is list"},
    {"is_map", (PyCFunction) tp_f_is_map, METH_VARARGS,
     "Check if data is map"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>data</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_pod_data_o *self)
{
    PyTypeObject *tp = Py_TYPE (self);
    delete self->obj;
    tp->tp_free (reinterpret_cast<PyObject *> (self));
    Py_DECREF (tp);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_pod_data_slots[] = {
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("core.pod.data class")},
    {Py_tp_getset, reinterpret_cast<void *> (tp_getset)},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_pod_data_spec = {
    .name = "mobius.core.pod.data",
    .basicsize = sizeof (core_pod_data_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_IMMUTABLETYPE,
    .slots = core_pod_data_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.pod.data</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_pod_data_type ()
{
    // If type is already created, return it
    if (core_pod_data_type)
        return mobius::py::pytypeobject (core_pod_data_type);

    // Allocate type from spec
    core_pod_data_type = reinterpret_cast<PyTypeObject *> (
        PyType_FromSpec (&core_pod_data_spec)
    );

    // Create type
    mobius::py::pytypeobject type (core_pod_data_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get <i>mobius.core.pod.data</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject *
get_core_pod_data_type ()
{
    return core_pod_data_type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>core.pod.data</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_pod_data_check (PyObject *value)
{
    if (!core_pod_data_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.pod.data type is not initialized")
        );

    return mobius::py::isinstance (value, core_pod_data_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.pod.data</i> Python object from C++ object
// @param value C++ object
// @return New core.pod.data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
to_pyobject (const mobius::core::pod::data &value)
{
    if (!core_pod_data_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.pod.data type is not initialized")
        );

    PyObject *ret = nullptr;

    if (value.is_null ())
        ret = mobius::py::pynone ();

    else if (value.is_bool ())
        ret = mobius::py::to_pyobject (bool (value));

    else if (value.is_integer ())
        ret = mobius::py::pylong_from_std_int64_t (value.to_integer ());

    else if (value.is_float ())
        ret = mobius::py::pyfloat_from_cpp (value.to_float ());

    else if (value.is_datetime ())
        ret = mobius::py::pydatetime_from_datetime (value.to_datetime ());

    else if (value.is_string ())
        ret = mobius::py::to_pyobject (value.to_string ());

    else if (value.is_bytearray ())
        ret = mobius::py::pybytes_from_bytearray (
            value.to_bytearray ()
        );

    else if (value.is_list ())
        ret = mobius::py::pylist_from_cpp_container (
            value.to_list (),
            pymobius_core_pod_data_to_pyobject
        );

    else if (value.is_map ())
        ret =
            pymobius_core_pod_map_to_pyobject (value.to_map ());

    else
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("unknown mobius.core.pod.data type")
        );

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.pod.data</i> Python object from C++ object
// @param value C++ object
// @return New core.pod.data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_pod_data_to_pyobject (const mobius::core::pod::data &value)
{
    return to_pyobject (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.pod.data</i> C++ object from Python object
// @param value Python value
// @return core.pod.data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
pymobius_core_pod_data_from_pyobject (PyObject *value)
{
    if (!core_pod_data_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.pod.data type is not initialized")
        );

    mobius::core::pod::data data;

    if (pymobius_core_pod_data_check (value))
        data = *(reinterpret_cast<core_pod_data_o *> (value)->obj);

    else if (mobius::py::pynone_check (value))
        ;

    else if (mobius::py::pybool_check (value))
        data = mobius::core::pod::data (value == Py_True);

    else if (mobius::py::pylong_check (value))
        data = mobius::core::pod::data (
            mobius::py::pylong_as_std_int64_t (value)
        );

    else if (mobius::py::pyfloat_check (value))
        data = mobius::core::pod::data (PyFloat_AS_DOUBLE ((value)));

    else if (mobius::py::pydatetime_check (value))
        data = mobius::core::pod::data (
            mobius::py::pydatetime_as_datetime (value)
        );

    else if (mobius::py::pybytes_check (value))
        data = mobius::core::pod::data (
            mobius::py::pybytes_as_bytearray (value)
        );

    else if (mobius::py::pystring_check (value))
        data = mobius::core::pod::data (
            mobius::py::pystring_as_std_string (value)
        );

    else if (PyList_Check (value))
        data = mobius::py::pylist_to_cpp_container (
            value, pymobius_core_pod_data_from_pyobject
        );

    else if (PyTuple_Check (value))
        data = pymobius_core_pod_data_vector_from_pytuple (value);

    else if (PySet_Check (value))
        data = pymobius_core_pod_data_vector_from_pyset (value);

    else if (PyDict_Check (value))
        data = pymobius_core_pod_map_from_pyobject (value);

    else
        data = pymobius_core_pod_map_from_pyobject (value);

    return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create pure Python object from mobius.core.pod.data value
// @param value mobius.core.pod.data value
// @return New Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_pod_data_to_python (const mobius::core::pod::data &value)
{
    if (!core_pod_data_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.pod.data type is not initialized")
        );

    PyObject *ret = nullptr;

    if (value.is_list ())
        ret = mobius::py::pylist_from_cpp_container (
            std::vector<mobius::core::pod::data> (value),
            pymobius_core_pod_data_to_python
        );

    else if (value.is_map ())
        ret = pymobius_core_pod_map_to_python (mobius::core::pod::map (value));

    else
        ret = pymobius_core_pod_data_to_pyobject (value);

    return ret;
}
