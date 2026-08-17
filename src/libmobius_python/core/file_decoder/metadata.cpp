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
// @file metadata.cc C++ API <i>mobius.core.decoder.metadata</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "metadata.hpp"
#include <mobius/core/exception.inc>
#include <pylist.hpp>
#include <pymobius.hpp>
#include <stdexcept>
#include "core/pod/data.hpp"
#include "core/pod/map.hpp"

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_file_decoder_metadata_type = nullptr;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_value</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_value (core_file_decoder_metadata_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_group;
    std::string arg_name;
    mobius::core::pod::data arg_value;

    try
    {
        arg_group = mobius::py::get_arg_as_std_string (args, 0);
        arg_name = mobius::py::get_arg_as_std_string (args, 1);
        arg_value = mobius::py::get_arg_as_cpp (
            args, 2, pymobius_core_pod_data_from_pyobject
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->set_value (arg_group, arg_name, arg_value);
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
// @brief <i>get_value</i> method implementation
// @param self Object
// @param args Argument list
// @return Metadata value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_value (core_file_decoder_metadata_o *self, PyObject *args)
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
        ret = pymobius_core_pod_data_to_pyobject (
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
// @brief <i>get_groups</i> method implementation
// @param self Object
// @param args Argument list
// @return Groups IDs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_groups (core_file_decoder_metadata_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylist_from_cpp_container (
            self->obj->get_groups (), mobius::py::pystring_from_std_string
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
// @brief <i>get_group</i> method implementation
// @param self Object
// @param args Argument list
// @return Metadata
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_group (core_file_decoder_metadata_o *self, PyObject *args)
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
        ret = pymobius_core_pod_map_to_pyobject (
            self->obj->get_group (arg_group)
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
    {"set_value", (PyCFunction) tp_f_set_value, METH_VARARGS,
     "Set metadata value"},
    {"get_value", (PyCFunction) tp_f_get_value, METH_VARARGS,
     "Get metadata value"},
    {"get_groups", (PyCFunction) tp_f_get_groups, METH_VARARGS, "Get groups"},
    {"get_group", (PyCFunction) tp_f_get_group, METH_VARARGS,
     "Get group metadata"},
    {nullptr, nullptr, 0, nullptr}, // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>metadata</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>metadata</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *, PyObject *)
{
    core_file_decoder_metadata_o *ret =
        reinterpret_cast<core_file_decoder_metadata_o *> (
            type->tp_alloc (type, 0)
        );

    if (ret)
    {
        try
        {
            ret->obj = new mobius::core::file_decoder::metadata ();
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
// @brief <i>metadata</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_file_decoder_metadata_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_file_decoder_metadata_slots[] = {
    {Py_tp_new, reinterpret_cast<void *> (tp_new)},
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("core.file_decoder.metadata class")},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_file_decoder_metadata_spec = {
    .name = "mobius.core.file_decoder.metadata",
    .basicsize = sizeof (core_file_decoder_metadata_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = core_file_decoder_metadata_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.file_decoder.metadata</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_file_decoder_metadata_type ()
{
    // If type is already created, return it
    if (core_file_decoder_metadata_type)
        return mobius::py::pytypeobject (core_file_decoder_metadata_type);

    // Allocate type from spec
    core_file_decoder_metadata_type = reinterpret_cast<PyTypeObject *> (
        PyType_FromSpec (&core_file_decoder_metadata_spec)
    );

    // Create type
    mobius::py::pytypeobject type (core_file_decoder_metadata_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>core.file_decoder.metadata</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_file_decoder_metadata_check (PyObject *value)
{
    if (!core_file_decoder_metadata_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.file_decoder.metadata type is not initialized")
        );

    return mobius::py::isinstance (value, core_file_decoder_metadata_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.file_decoder.metadata</i> Python object from C++ object
// @param obj C++ object
// @return New core.file_decoder.metadata object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_file_decoder_metadata_to_pyobject (const mobius::core::file_decoder::metadata &obj)
{
    if (!core_file_decoder_metadata_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.file_decoder.metadata type is not initialized")
        );

    return mobius::py::to_pyobject<core_file_decoder_metadata_o> (
        obj, core_file_decoder_metadata_type
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.file_decoder.metadata</i> C++ object from Python object
// @param value Python value
// @return core.file_decoder.metadata object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::file_decoder::metadata
pymobius_core_file_decoder_metadata_from_pyobject (PyObject *value)
{
    if (!core_file_decoder_metadata_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.file_decoder.metadata type is not initialized")
        );

    return mobius::py::from_pyobject<core_file_decoder_metadata_o> (
        value, core_file_decoder_metadata_type
    );
}
