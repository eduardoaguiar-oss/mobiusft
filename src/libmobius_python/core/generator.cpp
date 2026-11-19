// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// along with this program. If not, see <https://www.gnu.org/licenses/>.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @file generator.cpp
// @brief C++ API <i>mobius.core.generator</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "generator.hpp"
#include <pycall.hpp>
#include <stdexcept>

namespace mobius::py
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief tp_dealloc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (PyObject *self)
{
    auto py_self = reinterpret_cast<pymobius_core_generator_o *> (self);
    delete py_self->obj;
    py_self->obj = nullptr;

    PyTypeObject *type = Py_TYPE (self);
    type->tp_free (self);
    Py_DECREF (type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief tp_iternext
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_iternext (PyObject *self)
{
    auto py_self = reinterpret_cast<pymobius_core_generator_o *> (self);

    if (!py_self->obj)
        return nullptr; // exhausted: StopIteration

    try
    {
        PyObject *ret = py_self->obj->next ();

        if (!ret && !PyErr_Occurred ())
        {
            delete py_self->obj; // release statement / snapshot now
            py_self->obj = nullptr;
        }

        return ret;
    }
    catch (const std::exception &e)
    {
        PyErr_SetString (PyExc_RuntimeError, e.what ());
    }
    catch (...)
    {
        PyErr_SetString (PyExc_RuntimeError, "unknown C++ exception in generator");
    }

    return nullptr;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_generator_slots[] = {
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_iter, reinterpret_cast<void *> (PyObject_SelfIter)},
    {Py_tp_iternext, reinterpret_cast<void *> (tp_iternext)},
    {Py_tp_doc, const_cast<char *> ("Mobius generator (single-pass iterator)")},
    {0, nullptr},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_generator_spec = {
    .name = "mobius.core.generator",
    .basicsize = sizeof (pymobius_core_generator_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_IMMUTABLETYPE,
    .slots = core_generator_slots,
};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get the type object for core_generator
// @return PyTypeObject pointer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject *
get_core_generator_type ()
{
    // @brief Pointer to hold the heap-allocated type
    static PyTypeObject *core_generator_type = nullptr;

    if (!core_generator_type)
        core_generator_type = reinterpret_cast<PyTypeObject *> (PyType_FromSpec (&core_generator_spec))
;

    return core_generator_type;
}

} // namespace mobius::py
