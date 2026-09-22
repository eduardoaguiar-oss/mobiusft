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
// @file map.cc C++ API <i>mobius.core.pod.map</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "map.hpp"
#include <mobius/core/exception.inc>
#include <mobius/core/string_functions.hpp>
#include <pydict.hpp>
#include <pylist.hpp>
#include <pymobius.hpp>
#include <stdexcept>
#include "api_dataholder.hpp"
#include "data.hpp"
#include "pyobject.hpp"

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_pod_map_type = nullptr;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create Python object from POD map
// @param value POD map
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
map_to_object (const mobius::core::pod::map &value)
{
    api_dataholder_o *data = api_dataholder_new ();

    for (const auto &p : value)
    {
        if (p.first != ".object")
            api_dataholder_setattr (
                data, p.first, pymobius_core_pod_data_to_pyobject (p.second)
            );
    }

    return reinterpret_cast<PyObject *> (data);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>map</i> from Python object
// @param obj Python object
// @return POD map
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static mobius::core::pod::map
map_from_object (PyObject *obj)
{
    mobius::core::pod::map map;
    mobius::py::pyobject py_obj (obj, true);

    for (const auto &[key, value] : py_obj.get_attributes ())
    {
        if (
            !mobius::core::string::startswith (key, "__") &&
            value &&                 // not null
            !value.is_callable () && // eliminate functions
            obj != value             // eliminate self references
        )
            map.set (key, pymobius_core_pod_data_from_pyobject (value));
    }

    map.set (".object", mobius::core::pod::data ());

    return map;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_size</i> method implementation
// @param self Object
// @param args Argument list
// @return Number of items
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_size (core_pod_map_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->get_size ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>contains</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_contains (core_pod_map_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_key;

    try
    {
        arg_key = mobius::py::get_arg_as_std_string (args, 0);
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
        ret = mobius::py::to_pyobject (self->obj->contains (arg_key));
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
// @return Data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get (core_pod_map_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_key;
    mobius::core::pod::data arg_varg;

    try
    {
        arg_key = mobius::py::get_arg_as_std_string (args, 0);
        arg_varg = mobius::py::get_arg_as_cpp (
            args, 1, pymobius_core_pod_data_from_pyobject,
            mobius::core::pod::data ()
        );
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
            self->obj->get (arg_key, arg_varg)
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
// @brief <i>set</i> method implementation
// @param self Object
// @param args Argument list
// @return None
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set (core_pod_map_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_key;
    mobius::core::pod::data arg_value;

    try
    {
        arg_key = mobius::py::get_arg_as_std_string (args, 0);
        arg_value = mobius::py::get_arg_as_cpp (
            args, 1, pymobius_core_pod_data_from_pyobject
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
        self->obj->set (arg_key, arg_value);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>remove</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_remove (core_pod_map_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_key;

    try
    {
        arg_key = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->remove (arg_key);
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
// @brief <i>update</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_update (core_pod_map_o *self, PyObject *args)
{
    // Parse input args
    mobius::core::pod::map arg_map;

    try
    {
        arg_map = mobius::py::get_arg_as_cpp (
            args, 0, pymobius_core_pod_map_from_pyobject
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
        self->obj->update (arg_map);
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
// @brief <i>to_python</i> method implementation
// @param self Object
// @param args Argument list
// @return Data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_to_python (core_pod_map_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pydict_from_cpp_container (
            *self->obj, mobius::py::pystring_from_std_string,
            pymobius_core_pod_data_to_pyobject
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_values</i> method implementation
// @param self Object
// @param args Argument list
// @return Data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_values (core_pod_map_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylist_from_cpp_pair_container (
            *self->obj, mobius::py::pystring_from_std_string,
            pymobius_core_pod_data_to_pyobject
        );
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
    {"get_size", (PyCFunction) tp_f_get_size, METH_VARARGS,
     "Get map size"},
    {"contains", (PyCFunction) tp_f_contains, METH_VARARGS,
     "Check if map contains a given key"},
    {"get", (PyCFunction) tp_f_get, METH_VARARGS, "Get item"},
    {"set", (PyCFunction) tp_f_set, METH_VARARGS, "Set item"},
    {"remove", (PyCFunction) tp_f_remove, METH_VARARGS, "Remove item"},
    {"update", (PyCFunction) tp_f_update, METH_VARARGS,
     "Update with data from another map"},
    {"to_python", (PyCFunction) tp_f_to_python, METH_VARARGS,
     "Convert map to Python dict"},
    {"get_values", (PyCFunction) tp_f_get_values, METH_VARARGS,
     "Get items"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>map</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>map</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *, PyObject *args, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        mobius::core::pod::map map;

        if (mobius::py::get_arg_size (args) > 0)
            map = pymobius_core_pod_map_from_pyobject (
                mobius::py::get_arg (args, 0)
            );

        ret = pymobius_core_pod_map_to_pyobject (map);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>map</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_pod_map_o *self)
{
    PyTypeObject *tp = Py_TYPE (self);
    delete self->obj;
    tp->tp_free (reinterpret_cast<PyObject *> (self));
    Py_DECREF (tp);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_pod_map_slots[] = {
    {Py_tp_base, reinterpret_cast<void *> (new_core_pod_data_type ().get())},
    {Py_tp_new, reinterpret_cast<void *> (tp_new)},
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("core.pod.map class")},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_pod_map_spec = {
    .name = "mobius.core.pod.map",
    .basicsize = sizeof (core_pod_map_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_IMMUTABLETYPE,
    .slots = core_pod_map_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.pod.map</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_pod_map_type ()
{
    // If type is already created, return it
    if (core_pod_map_type)
        return mobius::py::pytypeobject (core_pod_map_type);

    // Allocate type from spec
    core_pod_map_type =
        reinterpret_cast<PyTypeObject *> (PyType_FromSpec (&core_pod_map_spec));

    // Create type
    mobius::py::pytypeobject type (core_pod_map_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>core.pod.map</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_pod_map_check (PyObject *value)
{
    if (!core_pod_map_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.pod.map type is not initialized")
        );

    return mobius::py::isinstance (value, core_pod_map_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>map</i> Python object from C++ object
// @param map POD map object
// @return new map object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_pod_map_to_pyobject (const mobius::core::pod::map &map)
{
    if (!core_pod_map_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.pod.map type is not initialized")
        );

    PyObject *ret = nullptr;

    if (map.contains (".object"))
        ret = map_to_object (map);

    else
        ret = mobius::py::to_pyobject<core_pod_map_o> (map, core_pod_map_type);

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create pure Python object from C++ object
// @param map POD map object
// @return Dict object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_pod_map_to_python (const mobius::core::pod::map &map)
{
    if (!core_pod_map_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.pod.map type is not initialized")
        );

    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pydict_from_cpp_container (
            map, mobius::py::pystring_from_std_string,
            pymobius_core_pod_data_to_python
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>map</i> C++ object from Python object
// @param py_value Python object
// @return map object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::map
pymobius_core_pod_map_from_pyobject (PyObject *py_value)
{
    if (!core_pod_map_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.pod.map type is not initialized")
        );

    mobius::core::pod::map map;

    if (pymobius_core_pod_map_check (py_value))
        map = *(reinterpret_cast<core_pod_map_o *> (py_value)->obj);

    else if (PyDict_Check (py_value))
    {
        PyObject *key, *value;
        Py_ssize_t pos = 0;

        while (PyDict_Next (py_value, &pos, &key, &value))
        {
            auto cpp_key = mobius::py::pystring_as_std_string (key);
            auto cpp_value = pymobius_core_pod_data_from_pyobject (value);

            map.set (cpp_key, cpp_value);
        }
    }

    // Read attributes from Python object
    else
        map = map_from_object (py_value);

    return map;
}
