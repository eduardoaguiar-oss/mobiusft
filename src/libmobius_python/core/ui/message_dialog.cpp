// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025
// Eduardo Aguiar
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject core_ui_message_dialog_t = {
    PyVarObject_HEAD_INIT (nullptr, 0)        // header
    "mobius.core.ui.message_dialog",          // tp_name
    sizeof (core_ui_message_dialog_o),        // tp_basicsize
    0,                                        // tp_itemsize
    (destructor) tp_dealloc,                  // tp_dealloc
    0,                                        // tp_print
    0,                                        // tp_getattr
    0,                                        // tp_setattr
    0,                                        // tp_compare
    0,                                        // tp_repr
    0,                                        // tp_as_number
    0,                                        // tp_as_sequence
    0,                                        // tp_as_mapping
    0,                                        // tp_hash
    0,                                        // tp_call
    0,                                        // tp_str
    0,                                        // tp_getattro
    0,                                        // tp_setattro
    0,                                        // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    "Message Dialog class",                   // tp_doc
    0,                                        // tp_traverse
    0,                                        // tp_clear
    0,                                        // tp_richcompare
    0,                                        // tp_weaklistoffset
    0,                                        // tp_iter
    0,                                        // tp_iternext
    tp_methods,                               // tp_methods
    0,                                        // tp_members
    tp_getset,                                // tp_getset
    0,                                        // tp_base
    0,                                        // tp_dict
    0,                                        // tp_descr_get
    0,                                        // tp_descr_set
    0,                                        // tp_dictoffset
    0,                                        // tp_init
    0,                                        // tp_alloc
    tp_new,                                   // tp_new
    0,                                        // tp_free
    0,                                        // tp_is_gc
    0,                                        // tp_bases
    0,                                        // tp_mro
    0,                                        // tp_cache
    0,                                        // tp_subclasses
    0,                                        // tp_weaklist
    0,                                        // tp_del
    0,                                        // tp_version_tag
    0,                                        // tp_finalize
    nullptr,                                  // tp_vectorcall
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.ui.message_dialog</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_ui_message_dialog_type ()
{
    mobius::py::pytypeobject type (&core_ui_message_dialog_t);
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>message_dialog</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_ui_message_dialog_check (PyObject *value)
{
    return PyObject_IsInstance (
        value, reinterpret_cast<PyObject *> (&core_ui_message_dialog_t));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>message_dialog</i> Python object from C++ object
// @param obj C++ object
// @return New message_dialog object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_ui_message_dialog_to_pyobject (
    const mobius::core::ui::message_dialog &obj)
{
    PyObject *ret = _PyObject_New (&core_ui_message_dialog_t);

    if (ret)
        ((core_ui_message_dialog_o *) ret)->obj =
            new mobius::core::ui::message_dialog (obj);

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>message_dialog</i> C++ object from Python object
// @param value Python value
// @return Message_dialog object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::ui::message_dialog
pymobius_core_ui_message_dialog_from_pyobject (PyObject *value)
{
    if (!pymobius_core_ui_message_dialog_check (value))
        throw std::invalid_argument (MOBIUS_EXCEPTION_MSG (
            "object must be an instance of mobius.core.ui.message_dialog"));

    return *(reinterpret_cast<core_ui_message_dialog_o *> (value)->obj);
}
