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
// @file widget.cc C++ API <i>mobius.core.ui.widget</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "widget.hpp"
#include <mobius/core/exception.inc>
#include <gtk/gtk.h> // PyGtk only interface for now
#include <pygobject.h>
#include <pymobius.hpp>
#include <pyobject.hpp>
#include <stdexcept>

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_ui_widget_type = nullptr;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_ui_widget</i> method implementation
// @param self Object
// @param args Argument list
// @return PyGObject
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_ui_widget (core_ui_widget_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = pygobject_new (G_OBJECT (self->obj->get_ui_widget<GtkWidget *> ()));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>show</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_show (core_ui_widget_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        self->obj->set_visible (true);
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
// @brief <i>hide</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_hide (core_ui_widget_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        self->obj->set_visible (false);
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
// @brief <i>set_visible</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_visible (core_ui_widget_o *self, PyObject *args)
{
    // Parse input args
    bool arg_flag;

    try
    {
        arg_flag = mobius::py::get_arg_as_bool (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->set_visible (arg_flag);
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
// @brief <i>set_sensitive</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_sensitive (core_ui_widget_o *self, PyObject *args)
{
    // Parse input args
    bool arg_flag;

    try
    {
        arg_flag = mobius::py::get_arg_as_bool (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->set_sensitive (arg_flag);
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
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] = {
    {"get_ui_widget", (PyCFunction) tp_f_get_ui_widget, METH_VARARGS, "Get low level implementation widget"},
    {"set_sensitive", (PyCFunction) tp_f_set_sensitive, METH_VARARGS, "Set widget sensitivity"},
    {"set_visible", (PyCFunction) tp_f_set_visible, METH_VARARGS, "Set widget visibility"},
    {"show", (PyCFunction) tp_f_show, METH_VARARGS, "Show widget"},
    {"hide", (PyCFunction) tp_f_hide, METH_VARARGS, "Hide widget"},
    {nullptr, nullptr, 0, nullptr}, // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>widget</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>widget</i> object
// @todo think about widget ownership
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *args, PyObject *)
{
    // Parse input args
    mobius::core::ui::widget arg_w;
    // bool arg_is_owner = false;

    try
    {
        if (mobius::py::get_arg_size (args) > 0)
            arg_w = mobius::py::get_arg_as_cpp (args, 0, pymobius_core_ui_widget_from_pyobject);

        // arg_is_owner = mobius::py::get_arg_as_bool (args, 1, false);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Create Python object
    core_ui_widget_o *ret = reinterpret_cast<core_ui_widget_o *> (type->tp_alloc (type, 0));

    if (ret)
    {
        try
        {
            ret->obj = new mobius::core::ui::widget (arg_w);
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
// @brief <i>widget</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_ui_widget_o *self)
{
    PyTypeObject *tp = Py_TYPE (self);
    delete self->obj;
    tp->tp_free (reinterpret_cast<PyObject *> (self));
    Py_DECREF (tp);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_ui_widget_slots[] = {
    {Py_tp_new, reinterpret_cast<void *> (tp_new)},
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("core.ui.widget class")},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_ui_widget_spec = {
    .name = "mobius.core.ui.widget",
    .basicsize = sizeof (core_ui_widget_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_IMMUTABLETYPE,
    .slots = core_ui_widget_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.ui.widget</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_ui_widget_type ()
{
    // If type is already created, return it
    if (core_ui_widget_type)
        return mobius::py::pytypeobject (core_ui_widget_type);

    // Initialize UI
    pygobject_init (3, 0, 0);

    // Allocate type from spec
    core_ui_widget_type = reinterpret_cast<PyTypeObject *> (PyType_FromSpec (&core_ui_widget_spec));

    // Create type
    mobius::py::pytypeobject type (core_ui_widget_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject *
get_core_ui_widget_type ()
{
    if (!core_ui_widget_type)
        new_core_ui_widget_type ();

    return core_ui_widget_type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>core.ui.widget</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_ui_widget_check (PyObject *value)
{
    if (!core_ui_widget_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.ui.widget type is not initialized"));

    return mobius::py::isinstance (value, core_ui_widget_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.ui.widget</i> Python object from C++ object
// @param obj C++ object
// @return New core.ui.widget object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_ui_widget_to_pyobject (const mobius::core::ui::widget &obj)
{
    if (!core_ui_widget_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.ui.widget type is not initialized"));

    return mobius::py::to_pyobject<core_ui_widget_o> (obj, core_ui_widget_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>widget</i> C++ object from Python object
// @param value Python value
// @return Widget object
//
// <i>value</i> can be of one of three types:
//    1. mobius.core.ui.widget
//    2. pygtk widget
//    3. any object with get_widget method returning mobius.core.ui.widget
//    object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::ui::widget
pymobius_core_ui_widget_from_pyobject (PyObject *value)
{
    mobius::core::ui::widget w;

    // Check if core.ui.widget type is initialized
    if (!core_ui_widget_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.ui.widget type is not initialized"));

    // if value is null of None, return
    if (!value || value == Py_None)
        return w;

    // type = mobius.core.ui.widget
    if (pymobius_core_ui_widget_check (value))
        w = mobius::py::from_pyobject<core_ui_widget_o> (value, core_ui_widget_type);

    // type = pygtk_widget
    else if (mobius::py::isinstance (value, &PyGObject_Type))
    {
        GtkWidget *wp = reinterpret_cast<GtkWidget *> (pygobject_get (value));

        if (!wp)
            throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("could not convert argument to GtkWidget"));

        w = mobius::core::ui::build_widget_from_low_level (wp);
    }

    // type = python object with get_widget method
    else
    {
        auto pyobj = mobius::py::pyobject (value, true);
        auto func = pyobj.get_attribute ("get_widget");

        if (!func || !func.is_callable ())
            throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("object has no get_widget method"));

        auto rc = func.call ();

        if (!pymobius_core_ui_widget_check (rc))
            throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid type returned by get_widget function"));

        w = mobius::py::from_pyobject<core_ui_widget_o> (rc, core_ui_widget_type);
    }

    return w;
}
