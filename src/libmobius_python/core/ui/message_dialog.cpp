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
// @file message_dialog.cc C++ API <i>mobius.core.ui.message_dialog</i> class
// wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "message_dialog.hpp"
#include <mobius/core/exception.inc>
#include <pymobius.hpp>
#include <stdexcept>

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_ui_message_dialog_type = nullptr;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>type</i> Attribute getter
// @param self Object
// @return <i>type</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_type (core_ui_message_dialog_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_int (
            static_cast<int> (self->obj->get_type ()));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>title</i> Attribute getter
// @param self Object
// @return <i>title</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_title (core_ui_message_dialog_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_title ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>title</i> Attribute setter
// @param self Object
// @param value New value
// @return 0 on success, -1 on error
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static int
tp_setter_title (core_ui_message_dialog_o *self, PyObject *value, void *)
{
    // Check value
    try
    {
        mobius::py::check_setter_value (value, "title",
                                        mobius::py::pystring_check);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return -1;
    }

    // Set title attribute
    try
    {
        self->obj->set_title (mobius::py::pystring_as_std_string (value));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return -1;
    }

    // Return success
    return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>text</i> Attribute getter
// @param self Object
// @return <i>text</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_text (core_ui_message_dialog_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_text ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>text</i> Attribute setter
// @param self Object
// @param value New value
// @return 0 on success, -1 on error
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static int
tp_setter_text (core_ui_message_dialog_o *self, PyObject *value, void *)
{
    // Check value
    try
    {
        mobius::py::check_setter_value (value, "text",
                                        mobius::py::pystring_check);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return -1;
    }

    // Set text attribute
    try
    {
        self->obj->set_text (mobius::py::pystring_as_std_string (value));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return -1;
    }

    // Return success
    return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>informative_text</i> Attribute getter
// @param self Object
// @return <i>informative_text</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_informative_text (core_ui_message_dialog_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (
            self->obj->get_informative_text ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>informative_text</i> Attribute setter
// @param self Object
// @param value New value
// @return 0 on success, -1 on error
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static int
tp_setter_informative_text (core_ui_message_dialog_o *self, PyObject *value,
                            void *)
{
    // Check value
    try
    {
        mobius::py::check_setter_value (value, "informative_text",
                                        mobius::py::pystring_check);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return -1;
    }

    // Set informative_text attribute
    try
    {
        self->obj->set_informative_text (
            mobius::py::pystring_as_std_string (value));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return -1;
    }

    // Return success
    return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Getters and setters structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyGetSetDef tp_getset[] = {
    {(char *) "type", (getter) tp_getter_type, (setter) 0,
     (char *) "Message type", nullptr},
    {(char *) "title", (getter) tp_getter_title, (setter) tp_setter_title,
     (char *) "Window's title", nullptr},
    {(char *) "text", (getter) tp_getter_text, (setter) tp_setter_text,
     (char *) "Text", nullptr},
    {(char *) "informative_text", (getter) tp_getter_informative_text,
     (setter) tp_setter_informative_text, (char *) "Informative text", nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>add_button</i> method implementation
// @param self Object
// @param args Argument list
// @return Response ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_add_button (core_ui_message_dialog_o *self, PyObject *args)
{
    // Parse input args
    PyObject *obj = nullptr;

    try
    {
        obj = mobius::py::get_arg (args, 0);
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
        if (mobius::py::pylong_check (obj))
        {
            auto button_id =
                static_cast<mobius::core::ui::message_dialog::button> (
                    mobius::py::pylong_as_int (obj));
            ret =
                mobius::py::pylong_from_int (self->obj->add_button (button_id));
        }
        else if (mobius::py::pystring_check (obj))
        {
            auto text = mobius::py::pystring_as_std_string (obj);
            ret = mobius::py::pylong_from_int (self->obj->add_button (text));
        }
        else
            mobius::py::set_invalid_type_error ("invalid argument");
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_default_response</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_default_response (core_ui_message_dialog_o *self, PyObject *args)
{
    // Parse input args
    mobius::core::ui::message_dialog::response_type arg_response_id;

    try
    {
        arg_response_id =
            static_cast<mobius::core::ui::message_dialog::response_type> (
                mobius::py::get_arg_as_int (args, 0));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->set_default_response (arg_response_id);
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
// @brief <i>run</i> method implementation
// @param self Object
// @param args Argument list
// @return Response ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_run (core_ui_message_dialog_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_int (self->obj->run ());
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
    {(char *) "add_button", (PyCFunction) tp_f_add_button, METH_VARARGS,
     "Add button"},
    {(char *) "set_default_response", (PyCFunction) tp_f_set_default_response,
     METH_VARARGS, "Set default response"},
    {(char *) "run", (PyCFunction) tp_f_run, METH_VARARGS, "Run dialog"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>message_dialog</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>message_dialog</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *args, PyObject *)
{
    // Parse input args
    mobius::core::ui::message_dialog::type arg_type;

    try
    {
        arg_type = static_cast<mobius::core::ui::message_dialog::type> (
            mobius::py::get_arg_as_int (args, 0));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Create Python object
    core_ui_message_dialog_o *ret =
        reinterpret_cast<core_ui_message_dialog_o *> (type->tp_alloc (type, 0));

    if (ret)
    {
        try
        {
            ret->obj = new mobius::core::ui::message_dialog (arg_type);
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
// @brief <i>message_dialog</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_ui_message_dialog_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_ui_message_dialog_slots[] = {
    {Py_tp_new, reinterpret_cast<void *> (tp_new)},
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("core.ui.message_dialog class")},
    {Py_tp_getset, reinterpret_cast<void *> (tp_getset)},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_ui_message_dialog_spec = {
    .name = "mobius.core.ui.message_dialog",
    .basicsize = sizeof (core_ui_message_dialog_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = core_ui_message_dialog_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.ui.message_dialog</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_ui_message_dialog_type ()
{
    // If type is already created, return it
    if (core_ui_message_dialog_type)
        return mobius::py::pytypeobject (core_ui_message_dialog_type);

    // Allocate type from spec
    core_ui_message_dialog_type = reinterpret_cast<PyTypeObject *> (
        PyType_FromSpec (&core_ui_message_dialog_spec)
    );

    // Create type
    mobius::py::pytypeobject type (core_ui_message_dialog_type);
    type.create ();

    type.add_constant ("type_info", 1);
    type.add_constant ("type_warning", 2);
    type.add_constant ("type_question", 3);
    type.add_constant ("type_error", 4);

    type.add_constant ("button_ok", 1);
    type.add_constant ("button_cancel", 2);
    type.add_constant ("button_yes", 3);
    type.add_constant ("button_no", 4);
    type.add_constant ("button_open", 5);
    type.add_constant ("button_save", 6);
    type.add_constant ("button_close", 7);
    type.add_constant ("button_discard", 8);
    type.add_constant ("button_apply", 9);
    type.add_constant ("button_reset", 10);
    type.add_constant ("button_restore_defaults", 11);
    type.add_constant ("button_help", 12);
    type.add_constant ("button_save_all", 13);
    type.add_constant ("button_yes_to_all", 14);
    type.add_constant ("button_no_to_all", 15);
    type.add_constant ("button_abort", 16);
    type.add_constant ("button_retry", 17);
    type.add_constant ("button_ignore", 18);

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>core.ui.message_dialog</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_ui_message_dialog_check (PyObject *value)
{
    if (!core_ui_message_dialog_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.ui.message_dialog type is not initialized")
        );

    return mobius::py::isinstance (value, core_ui_message_dialog_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.ui.message_dialog</i> Python object from C++ object
// @param obj C++ object
// @return New core.ui.message_dialog object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_ui_message_dialog_to_pyobject (const mobius::core::ui::message_dialog &obj)
{
    if (!core_ui_message_dialog_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.ui.message_dialog type is not initialized")
        );

    return mobius::py::to_pyobject<core_ui_message_dialog_o> (
        obj, core_ui_message_dialog_type
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.ui.message_dialog</i> C++ object from Python object
// @param value Python value
// @return core.ui.message_dialog object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::ui::message_dialog
pymobius_core_ui_message_dialog_from_pyobject (PyObject *value)
{
    if (!core_ui_message_dialog_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.ui.message_dialog type is not initialized")
        );

    return mobius::py::from_pyobject<core_ui_message_dialog_o> (
        value, core_ui_message_dialog_type
    );
}
