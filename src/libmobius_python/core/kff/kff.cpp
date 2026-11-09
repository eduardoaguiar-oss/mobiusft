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
// @file kff.cc C++ API <i>mobius.core.kff.kff</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "kff.hpp"
#include <mobius/core/exception.inc>
#include <pydict.hpp>
#include <pylist.hpp>
#include <pymobius.hpp>
#include <stdexcept>
#include "core/database/connection_set.hpp"
#include "core/database/transaction.hpp"
#include "file.hpp"
#include "hashset.hpp"

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_kff_kff_type = nullptr;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>new_connection</i> method implementation
// @param self Object
// @param args Argument list
// @return new connection_set object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_new_connection (core_kff_kff_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_database_connection_set_to_pyobject (self->obj->new_connection ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>new_hashset</i> method implementation
// @param self Object
// @param args Argument list
// @return hash set object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_new_hashset (core_kff_kff_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_id;
    bool arg_is_alert;

    try
    {
        arg_id = mobius::py::get_arg_as_std_string (args, 0);
        arg_is_alert = mobius::py::get_arg_as_bool (args, 1);
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
        ret = pymobius_core_kff_hashset_to_pyobject (self->obj->new_hashset (arg_id, arg_is_alert));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>remove_hashset</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_remove_hashset (core_kff_kff_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_id;

    try
    {
        arg_id = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->remove_hashset (arg_id);
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
// @brief <i>get_hashsets</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_hashsets (core_kff_kff_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylist_from_cpp_pair_container (
            self->obj->get_hashsets (),
            mobius::py::pystring_from_std_string,
            pymobius_core_kff_hashset_to_pyobject
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>alert_lookup</i> method implementation
// @param self Object
// @param args Argument list
// @return Hash sets
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_alert_lookup (core_kff_kff_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_type;
    std::string arg_value;

    try
    {
        arg_type = mobius::py::get_arg_as_std_string (args, 0);
        arg_value = mobius::py::get_arg_as_std_string (args, 1);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylist_from_cpp_container (
            self->obj->alert_lookup (arg_type, arg_value),
            mobius::py::pystring_from_std_string
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>lookup</i> method implementation
// @param self Object
// @param args Argument list
// @return Hashsets
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_lookup (core_kff_kff_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_type;
    std::string arg_value;

    try
    {
        arg_type = mobius::py::get_arg_as_std_string (args, 0);
        arg_value = mobius::py::get_arg_as_std_string (args, 1);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (std::string (1, self->obj->lookup (arg_type, arg_value)));
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
    {"new_connection", (PyCFunction) tp_f_new_connection, METH_VARARGS, "create new connection set to KFF"},
    {"new_hashset", (PyCFunction) tp_f_new_hashset, METH_VARARGS, "Create hash set"},
    {"remove_hashset", (PyCFunction) tp_f_remove_hashset, METH_VARARGS, "Remove hash set"},
    {"get_hashsets", (PyCFunction) tp_f_get_hashsets, METH_VARARGS, "Get hash sets"},
    {"alert_lookup", (PyCFunction) tp_f_alert_lookup, METH_VARARGS, "Lookup hash in alert hash sets"},
    {"lookup", (PyCFunction) tp_f_lookup, METH_VARARGS, "Lookup hash in all hash sets"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief tp_new (default constructor)
// @param type type object
// @param args argument list
// @param kwds keywords dict
// @return new kff object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *, PyObject *)
{
    PyObject *ret = type->tp_alloc (type, 0);

    if (ret)
    {
        try
        {
            ((core_kff_kff_o *) ret)->obj = new mobius::core::kff::kff ();
        }
        catch (const std::exception &e)
        {
            Py_DECREF (ret);
            mobius::py::set_runtime_error (e.what ());
            ret = nullptr;
        }
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>kff</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_kff_kff_o *self)
{
    PyTypeObject *tp = Py_TYPE (self);
    delete self->obj;
    tp->tp_free (reinterpret_cast<PyObject *> (self));
    Py_DECREF (tp);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_kff_kff_slots[] = {
    {Py_tp_new, reinterpret_cast<void *> (tp_new)},
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("core.kff.kff class")},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_kff_kff_spec = {
    .name = "mobius.core.kff.kff",
    .basicsize = sizeof (core_kff_kff_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_IMMUTABLETYPE,
    .slots = core_kff_kff_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.kff.kff</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_kff_kff_type ()
{
    // If type is already created, return it
    if (core_kff_kff_type)
        return mobius::py::pytypeobject (core_kff_kff_type);

    // Allocate type from spec
    core_kff_kff_type = reinterpret_cast<PyTypeObject *> (PyType_FromSpec (&core_kff_kff_spec));

    // Create type
    mobius::py::pytypeobject type (core_kff_kff_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>core.kff.kff</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_kff_kff_check (PyObject *value)
{
    if (!core_kff_kff_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.kff.kff type is not initialized"));

    return mobius::py::isinstance (value, core_kff_kff_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.kff.kff</i> Python object from C++ object
// @param obj C++ object
// @return New core.kff.kff object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_kff_kff_to_pyobject (const mobius::core::kff::kff &obj)
{
    if (!core_kff_kff_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.kff.kff type is not initialized"));

    return mobius::py::to_pyobject<core_kff_kff_o> (obj, core_kff_kff_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.kff.kff</i> C++ object from Python object
// @param value Python value
// @return core.kff.kff object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::kff::kff
pymobius_core_kff_kff_from_pyobject (PyObject *value)
{
    if (!core_kff_kff_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.kff.kff type is not initialized"));

    return mobius::py::from_pyobject<core_kff_kff_o> (value, core_kff_kff_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file count
// @param self Python self
// @param args Python arguments
// @return File count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_file_count (PyObject *, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (mobius::core::kff::get_file_count ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash count
// @param self Python self
// @param args Python arguments
// @return Hash count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_hash_count (PyObject *, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (mobius::core::kff::get_hash_count ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get alert count
// @param self Python self
// @param args Python arguments
// @return Alert count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_alert_count (PyObject *, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (mobius::core::kff::get_alert_count ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get thumbnail count
// @param self Python self
// @param args Python arguments
// @return Thumbnail count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_thumbnail_count (PyObject *, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (mobius::core::kff::get_thumbnail_count ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file count by type
// @param self Python self
// @param args Python arguments
// @return Dictionary with file count by type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_file_count_by_type (PyObject *, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pydict_from_cpp_container (mobius::core::kff::get_file_count_by_type ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash count by type
// @param self Python self
// @param args Python arguments
// @return Dictionary with hash count by type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_hash_count_by_type (PyObject *, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pydict_from_cpp_container (mobius::core::kff::get_hash_count_by_type ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <b>has_size</b> function implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_has_size (PyObject *, PyObject *args)
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
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (mobius::core::kff::has_size (arg_size));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>lookup</i> function implementation
// @param self Python self
// @param args Python arguments
// @return Lookup result
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_lookup (PyObject *, PyObject *args)
{
    // Parse input args
    std::uint16_t arg_hash_type;
    std::string arg_value;

    try
    {
        arg_hash_type = mobius::py::get_arg_as_uint16_t (args, 0);
        arg_value = mobius::py::get_arg_as_std_string (args, 1);
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
        ret = mobius::py::to_pyobject (mobius::core::kff::lookup (arg_hash_type, arg_value));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    return ret;
}
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>new_transaction</i> function implementation
// @return new transaction object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_new_transaction (PyObject *, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_database_transaction_to_pyobject (mobius::core::kff::new_transaction ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}
