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
// @file turing.cc C++ API <i>mobius.core.turing.turing</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "turing.hpp"
#include "core/database/transaction.hpp"
#include "module.hpp"
#include <pylist.hpp>
#include <pymobius.hpp>

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_turing_turing_type = nullptr;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create tuple from hash
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
PyTuple_from_hash (const std::tuple<std::string, std::string, std::string> &row)
{
    PyObject *ret = PyTuple_New (3);

    if (ret)
    {
        PyTuple_SetItem (
            ret, 0, mobius::py::pystring_from_std_string (std::get<0> (row)));
        PyTuple_SetItem (
            ret, 1, mobius::py::pystring_from_std_string (std::get<1> (row)));
        PyTuple_SetItem (
            ret, 2, mobius::py::pystring_from_std_string (std::get<2> (row)));
    }

    return ret;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>has_hash</i> method implementation
// @param self Object
// @param args Argument list
// @return True/False
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_has_hash (core_turing_turing_o *self, PyObject *args)
{
    // parse input args
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

    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (
            self->obj->has_hash (arg_type, arg_value));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_hash</i> method implementation
// @param self object
// @param args argument list
// @return new hash object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_hash (core_turing_turing_o *self, PyObject *args)
{
    // parse input args
    std::string arg_type;
    std::string arg_value;
    std::string arg_password;

    try
    {
        arg_type = mobius::py::get_arg_as_std_string (args, 0);
        arg_value = mobius::py::get_arg_as_std_string (args, 1);
        arg_password = mobius::py::get_arg_as_std_string (args, 2);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        self->obj->set_hash (arg_type, arg_value, arg_password);
        ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_hash_password</i> method implementation
// @param self Object
// @param args Argument list
// @return status, password
//
// status: 0 - not found
//         1 - found
//         2 - LM first half found
//         3 - LM second half found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_hash_password (core_turing_turing_o *self, PyObject *args)
{
    // parse input args
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

    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        auto p = self->obj->get_hash_password (arg_type, arg_value);
        ret = PyTuple_New (2);

        if (ret)
        {
            PyTuple_SetItem (
                ret, 0,
                mobius::py::pylong_from_int (static_cast<int> (p.first)));

            if (p.first == mobius::core::turing::turing::pwd_status::not_found)
                PyTuple_SetItem (ret, 1, mobius::py::pynone ());

            else
                PyTuple_SetItem (
                    ret, 1, mobius::py::pystring_from_std_string (p.second));
        }
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>remove_hashes</i> method implementation
// @param self object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_remove_hashes (core_turing_turing_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        self->obj->remove_hashes ();
        ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_hashes</i> method implementation
// @param self object
// @param args argument list
// @return hash list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_hashes (core_turing_turing_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylist_from_cpp_container (self->obj->get_hashes (),
                                                     PyTuple_from_hash);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>new_transaction</i> method implementation
// @param self object
// @param args argument list
// @return new transaction object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_new_transaction (core_turing_turing_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_database_transaction_to_pyobject (
            self->obj->new_transaction ());
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
    {"has_hash", (PyCFunction) tp_f_has_hash, METH_VARARGS,
     "Check if hash is set"},
    {"set_hash", (PyCFunction) tp_f_set_hash, METH_VARARGS,
     "Set hash type, value and password"},
    {"get_hash_password", (PyCFunction) tp_f_get_hash_password,
     METH_VARARGS, "Get password for a given hash"},
    {"remove_hashes", (PyCFunction) tp_f_remove_hashes, METH_VARARGS,
     "Remove all hashes from database"},
    {"get_hashes", (PyCFunction) tp_f_get_hashes, METH_VARARGS,
     "get all hashes from database"},
    {"new_transaction", (PyCFunction) tp_f_new_transaction,
     METH_VARARGS, "create new transaction"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>turing</i> constructor
// @param type type object
// @param args argument list
// @param kwds keywords dict
// @return new <i>turing</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *, PyObject *)
{
    core_turing_turing_o *self =
        (core_turing_turing_o *) type->tp_alloc (type, 0);

    if (self)
    {
        try
        {
            self->obj = new mobius::core::turing::turing ();
        }
        catch (const std::exception &e)
        {
            mobius::py::set_runtime_error (e.what ());
            Py_TYPE (self)->tp_free ((PyObject *) self);
            self = nullptr;
        }
    }

    return (PyObject *) self;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>turing</i> deallocator
// @param self object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_turing_turing_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_turing_turing_slots[] = {
    {Py_tp_new, reinterpret_cast<void *> (tp_new)},
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("core.turing.turing class")},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_turing_turing_spec = {
    .name = "mobius.core.turing.turing",
    .basicsize = sizeof (core_turing_turing_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = core_turing_turing_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.turing.turing</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_turing_turing_type ()
{
    // If type is already created, return it
    if (core_turing_turing_type)
        return mobius::py::pytypeobject (core_turing_turing_type);

    // Allocate type from spec
    core_turing_turing_type = reinterpret_cast<PyTypeObject *> (
        PyType_FromSpec (&core_turing_turing_spec)
    );

    // Create type
    mobius::py::pytypeobject type (core_turing_turing_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>core.turing.turing</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_turing_turing_check (PyObject *value)
{
    if (!core_turing_turing_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.turing.turing type is not initialized")
        );

    return mobius::py::isinstance (value, core_turing_turing_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.turing.turing</i> Python object from C++ object
// @param obj C++ object
// @return New core.turing.turing object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_turing_turing_to_pyobject (const mobius::core::turing::turing &obj)
{
    if (!core_turing_turing_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.turing.turing type is not initialized")
        );

    return mobius::py::to_pyobject<core_turing_turing_o> (
        obj, core_turing_turing_type
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.turing.turing</i> C++ object from Python object
// @param value Python value
// @return core.turing.turing object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::turing::turing
pymobius_core_turing_turing_from_pyobject (PyObject *value)
{
    if (!core_turing_turing_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.turing.turing type is not initialized")
        );

    return mobius::py::from_pyobject<core_turing_turing_o> (
        value, core_turing_turing_type
    );
}
