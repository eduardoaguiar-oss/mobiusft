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
// @brief  C++ API module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "api_dataholder.hpp"
#include <pymobius.hpp>

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *api_dataholder_type = nullptr;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Getters and setters
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyGetSetDef tp_getset[] = {
    {"__dict__", PyObject_GenericGetDict, PyObject_GenericSetDict, "Type dictionary", nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief tp_dir implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_dir (api_dataholder_o *self, PyObject *)
{
    PyObject *dict = PyObject_GenericGetDict(reinterpret_cast<PyObject *> (self), nullptr);
    return PyDict_Keys (dict);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] = {
    {(char *) "__dir__", (PyCFunction) tp_dir, METH_VARARGS, "Dir function implementation"},
    {NULL, NULL, 0, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief api.dataholder: tp_dealloc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (api_dataholder_o *self)
{
    PyObject_GenericSetDict(reinterpret_cast<PyObject *> (self), nullptr, nullptr);
    Py_TYPE (self)->tp_free (reinterpret_cast<PyObject *> (self));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot api_dataholder_slots[] = {
    {Py_tp_new, reinterpret_cast<void *> (PyType_GenericNew)},
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("api_dataholder class")},
    {Py_tp_getset, reinterpret_cast<void *> (tp_getset)},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {Py_tp_getattro, reinterpret_cast<void *> (PyObject_GenericGetAttr)},
    {Py_tp_setattro, reinterpret_cast<void *> (PyObject_GenericSetAttr)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec api_dataholder_spec = {
    .name = "mobius.api_dataholder",
    .basicsize = sizeof (api_dataholder_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_MANAGED_DICT,
    .slots = api_dataholder_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.api_dataholder</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_api_dataholder_type ()
{
    // If type is already created, return it
    if (api_dataholder_type)
        return mobius::py::pytypeobject (api_dataholder_type);

    // Allocate type from spec
    api_dataholder_type = reinterpret_cast<PyTypeObject *> (
        PyType_FromSpec (&api_dataholder_spec)
    );

    // Create type
    mobius::py::pytypeobject type (api_dataholder_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief api.dataholder: create new object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
api_dataholder_o *
api_dataholder_new ()
{
    if (!api_dataholder_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("api_dataholder type is not initialized")
        );

    api_dataholder_o *self =
        (api_dataholder_o *) _PyObject_New (api_dataholder_type);

    return self;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set attribute
// @param obj object
// @param name name
// @param value value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
api_dataholder_setattr (
    api_dataholder_o *obj, const std::string &name, const std::string &value
)
{
    PyObject_GenericSetAttr (
        (PyObject *) obj, mobius::py::pystring_from_std_string (name),
        mobius::py::pystring_from_std_string (value)
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set attribute
// @param obj object
// @param name name
// @param value value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
api_dataholder_setattr (
    api_dataholder_o *obj, const std::string &name, std::int64_t value
)
{
    PyObject_GenericSetAttr (
        (PyObject *) obj, mobius::py::pystring_from_std_string (name),
        mobius::py::pylong_from_std_int64_t (value)
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set attribute
// @param obj object
// @param name name
// @param value value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
api_dataholder_setattr (
    api_dataholder_o *obj,
    const std::string &name,
    const mobius::core::datetime::datetime &value
)
{
    PyObject_GenericSetAttr (
        (PyObject *) obj, mobius::py::pystring_from_std_string (name),
        mobius::py::pydatetime_from_datetime (value)
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set attribute
// @param obj object
// @param name name
// @param value value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
api_dataholder_setattr (
    api_dataholder_o *obj, const std::string &name, PyObject *value
)
{
    PyObject_GenericSetAttr (
        (PyObject *) obj, mobius::py::pystring_from_std_string (name), value
    );
}
