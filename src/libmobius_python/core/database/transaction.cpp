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
// @file transaction.cc C++ API <i>mobius.core.database.transaction</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "transaction.hpp"
#include <mobius/core/exception.inc>
#include <pymobius.hpp>
#include <stdexcept>
#include "module.hpp"

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_database_transaction_type = nullptr;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>commit</i> method implementation
// @param self object
// @param args argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_commit (core_database_transaction_o *self, PyObject *)
{
    if (self->obj == nullptr)
    {
        mobius::py::set_runtime_error ("Transaction has already ended");
        return nullptr;
    }

    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        self->obj->commit ();
        delete self->obj;
        self->obj = nullptr;
        ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>rollback</i> method implementation
// @param self object
// @param args argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_rollback (core_database_transaction_o *self, PyObject *)
{
    if (self->obj == nullptr)
    {
        mobius::py::set_runtime_error ("Transaction is already ended");
        return nullptr;
    }

    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        self->obj->rollback ();
        delete self->obj;
        self->obj = nullptr;
        ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] = {
    {"commit", (PyCFunction) tp_f_commit, METH_VARARGS, "commit operations"},
    {"rollback", (PyCFunction) tp_f_rollback, METH_VARARGS,
     "rollback operations"},
    {nullptr, nullptr, 0, nullptr}, // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>transaction</i> deallocator
// @param self object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_database_transaction_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_database_transaction_slots[] = {
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("nested transaction control class")},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_database_transaction_spec = {
    .name = "mobius.core.database.transaction",
    .basicsize = sizeof (core_database_transaction_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = core_database_transaction_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.database.transaction</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_database_transaction_type ()
{
    // If type is already created, return it
    if (core_database_transaction_type)
        return mobius::py::pytypeobject (core_database_transaction_type);

    // Allocate type from spec
    core_database_transaction_type = reinterpret_cast<PyTypeObject *> (
        PyType_FromSpec (&core_database_transaction_spec)
    );

    // Create type
    mobius::py::pytypeobject type (core_database_transaction_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>transaction</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_database_transaction_check (PyObject *value)
{
    if (!core_database_transaction_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("transaction type is not initialized")
        );

    return mobius::py::isinstance (value, core_database_transaction_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>transaction</i> Python object from C++ object
// @param obj C++ object
// @return New transaction object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_database_transaction_to_pyobject (
    const mobius::core::database::transaction &obj
)
{
    if (!core_database_transaction_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("transaction type is not initialized")
        );

    return mobius::py::to_pyobject<core_database_transaction_o> (
        obj, core_database_transaction_type
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>transaction</i> C++ object from Python object
// @param value Python value
// @return Transaction object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::database::transaction
pymobius_core_database_transaction_from_pyobject (PyObject *value)
{
    if (!core_database_transaction_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("transaction type is not initialized")
        );

    return mobius::py::from_pyobject<core_database_transaction_o> (
        value, core_database_transaction_type
    );
}
