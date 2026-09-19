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
// @file button.cc C++ API <i>mobius.core.ui.button</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "button.hpp"
#include <mobius/core/exception.inc>
#include <mobius/core/log.hpp>
#include <pyfunction.hpp>
#include <pygil.hpp>
#include <pymobius.hpp>
#include <stdexcept>
#include "icon.hpp"
#include "widget.hpp"

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_ui_button_type = nullptr;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Callback functor for clicked event
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class callback_clicked
{
  public:
    explicit callback_clicked (PyObject *f)
        : f_ (f)
    {
    }

    bool
    operator() ()
    {
        mobius::py::GIL_guard gil_guard;

        try
        {
            PyObject *py_rc = f_ ();
            if (!py_rc)
                return true;

            bool rc = mobius::py::pybool_as_bool (py_rc);
            Py_DECREF (py_rc);

            return rc;
        }
        catch (const std::exception &e)
        {
            mobius::core::log log (__FILE__, __FUNCTION__);
            log.error (__LINE__, e.what ());
        }

        return true;
    }

  private:
    mobius::py::function f_;
};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_text</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_text (core_ui_button_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_text;

    try
    {
        arg_text = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->set_text (arg_text);
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
// @brief <i>set_icon</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_icon (core_ui_button_o *self, PyObject *args)
{
    // Parse input args
    mobius::core::ui::icon arg_icon;

    try
    {
        arg_icon = mobius::py::get_arg_as_cpp (args, 0, pymobius_core_ui_icon_from_pyobject);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->set_icon (arg_icon);
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
// @brief <i>set_icon_by_name</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_icon_by_name (core_ui_button_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_name;

    try
    {
        arg_name = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->set_icon_by_name (arg_name);
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
// @brief <i>set_callback</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_callback (core_ui_button_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_event_id;
    PyObject *arg_f = nullptr;

    try
    {
        arg_event_id = mobius::py::get_arg_as_std_string (args, 0);
        arg_f = mobius::py::get_arg (args, 1);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        if (arg_event_id == "clicked")
            self->obj->set_callback (arg_event_id, callback_clicked (arg_f));

        else
        {
            mobius::py::set_invalid_type_error ("invalid event ID: " + arg_event_id);
            return nullptr;
        }
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
// @brief <i>reset_callback</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_reset_callback (core_ui_button_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_event_id;

    try
    {
        arg_event_id = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->reset_callback (arg_event_id);
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
    {"set_text", (PyCFunction) tp_f_set_text, METH_VARARGS, "Set text"},
    {"set_icon", (PyCFunction) tp_f_set_icon, METH_VARARGS, "Set button icon"},
    {"set_icon_by_name", (PyCFunction) tp_f_set_icon_by_name, METH_VARARGS, "Set icon by name"},
    {"set_callback", (PyCFunction) tp_f_set_callback, METH_VARARGS, "Set callback to event"},
    {"reset_callback", (PyCFunction) tp_f_reset_callback, METH_VARARGS, "Reset callback to event"},
    {nullptr, nullptr, 0, nullptr}, // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>button</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>button</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *, PyObject *)
{
    core_ui_button_o *ret = reinterpret_cast<core_ui_button_o *> (type->tp_alloc (type, 0));

    if (ret)
    {
        try
        {
            ret->obj = new mobius::core::ui::button ();
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
// @brief <i>button</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_ui_button_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_ui_button_slots[] = {
    {Py_tp_base, reinterpret_cast<void *> (get_core_ui_widget_type ())},
    {Py_tp_new, reinterpret_cast<void *> (tp_new)},
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("core.ui.button class")},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_ui_button_spec = {
    .name = "mobius.core.ui.button",
    .basicsize = sizeof (core_ui_button_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = core_ui_button_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.ui.button</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_ui_button_type ()
{
    // If type is already created, return it
    if (core_ui_button_type)
        return mobius::py::pytypeobject (core_ui_button_type);

    // Allocate type from spec
    core_ui_button_type = reinterpret_cast<PyTypeObject *> (PyType_FromSpec (&core_ui_button_spec));

    // Create type
    mobius::py::pytypeobject type (core_ui_button_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>core.ui.button</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_ui_button_check (PyObject *value)
{
    if (!core_ui_button_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.ui.button type is not initialized"));

    return mobius::py::isinstance (value, core_ui_button_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.ui.button</i> Python object from C++ object
// @param obj C++ object
// @return New core.ui.button object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_ui_button_to_pyobject (const mobius::core::ui::button &obj)
{
    if (!core_ui_button_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.ui.button type is not initialized"));

    return mobius::py::to_pyobject<core_ui_button_o> (obj, core_ui_button_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.ui.button</i> C++ object from Python object
// @param value Python value
// @return core.ui.button object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::ui::button
pymobius_core_ui_button_from_pyobject (PyObject *value)
{
    if (!core_ui_button_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.ui.button type is not initialized"));

    return mobius::py::from_pyobject<core_ui_button_o> (value, core_ui_button_type);
}
