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
// @file icon.cc C++ API <i>mobius.core.ui.icon</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "icon.hpp"
#include "widget.hpp"
#include <pymobius.hpp>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_icon_by_name</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_icon_by_name (core_ui_icon_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_name;
    mobius::core::ui::icon::size_type arg_size =
        mobius::core::ui::icon::size_type::toolbar;

    try
    {
        arg_name = mobius::py::get_arg_as_std_string (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_size = static_cast<mobius::core::ui::icon::size_type> (
                mobius::py::get_arg_as_int (args, 1));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->set_icon_by_name (arg_name, arg_size);
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
// @brief <i>set_icon_by_path</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_icon_by_path (core_ui_icon_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_path;
    mobius::core::ui::icon::size_type arg_size =
        mobius::core::ui::icon::size_type::toolbar;

    try
    {
        arg_path = mobius::py::get_arg_as_std_string (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_size = static_cast<mobius::core::ui::icon::size_type> (
                mobius::py::get_arg_as_int (args, 1));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->set_icon_by_path (arg_path, arg_size);
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
// @brief <i>set_icon_by_url</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_icon_by_url (core_ui_icon_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_url;
    mobius::core::ui::icon::size_type arg_size =
        mobius::core::ui::icon::size_type::toolbar;

    try
    {
        arg_url = mobius::py::get_arg_as_std_string (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_size = static_cast<mobius::core::ui::icon::size_type> (
                mobius::py::get_arg_as_int (args, 1));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->set_icon_by_url (arg_url, arg_size);
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
// @brief <i>set_icon_from_data</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_icon_from_data (core_ui_icon_o *self, PyObject *args)
{
    // Parse input args
    mobius::core::bytearray arg_data;
    mobius::core::ui::icon::size_type arg_size =
        mobius::core::ui::icon::size_type::toolbar;

    try
    {
        arg_data = mobius::py::get_arg_as_bytearray (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_size = static_cast<mobius::core::ui::icon::size_type> (
                mobius::py::get_arg_as_int (args, 1));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->set_icon_from_data (arg_data, arg_size);
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
    {"set_icon_by_name", (PyCFunction) tp_f_set_icon_by_name, METH_VARARGS,
     "Set icon by name"},
    {"set_icon_by_path", (PyCFunction) tp_f_set_icon_by_path, METH_VARARGS,
     "Set icon by path"},
    {"set_icon_by_url", (PyCFunction) tp_f_set_icon_by_url, METH_VARARGS,
     "Set icon by URL"},
    {"set_icon_from_data", (PyCFunction) tp_f_set_icon_from_data, METH_VARARGS,
     "Set icon from data"},
    {nullptr, nullptr, 0, nullptr}, // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>icon</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_ui_icon_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyTypeObject core_ui_icon_t = {
    PyVarObject_HEAD_INIT (nullptr, 0)        // header
    "mobius.core.ui.icon",                    // tp_name
    sizeof (core_ui_icon_o),                  // tp_basicsize
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
    "Icon class",                             // tp_doc
    0,                                        // tp_traverse
    0,                                        // tp_clear
    0,                                        // tp_richcompare
    0,                                        // tp_weaklistoffset
    0,                                        // tp_iter
    0,                                        // tp_iternext
    tp_methods,                               // tp_methods
    0,                                        // tp_members
    0,                                        // tp_getset
    get_ui_widget_type (),                    // tp_base
    0,                                        // tp_dict
    0,                                        // tp_descr_get
    0,                                        // tp_descr_set
    0,                                        // tp_dictoffset
    0,                                        // tp_init
    0,                                        // tp_alloc
    0,                                        // tp_new
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
// @brief Create <i>mobius.core.ui.icon</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_ui_icon_type ()
{
    mobius::py::pytypeobject type (&core_ui_icon_t);
    type.create ();

    type.add_constant ("size_menu", 16);
    type.add_constant ("size_toolbar", 24);
    type.add_constant ("size_dnd", 32);
    type.add_constant ("size_dialog", 48);
    type.add_constant ("size_large", 64);
    type.add_constant ("size_extra_large", 128);

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>icon</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_ui_icon_check (PyObject *value)
{
    return mobius::py::isinstance (value, &core_ui_icon_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>icon</i> Python object from C++ object
// @param obj C++ object
// @return New icon object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_ui_icon_to_pyobject (const mobius::core::ui::icon &obj)
{
    return mobius::py::to_pyobject<core_ui_icon_o> (obj, &core_ui_icon_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>icon</i> C++ object from Python object
// @param value Python value
// @return Icon object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::ui::icon
pymobius_core_ui_icon_from_pyobject (PyObject *value)
{
    return mobius::py::from_pyobject<core_ui_icon_o> (value, &core_ui_icon_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_icon_path</i> function
// @param self Function object
// @param args Argument list
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_ui_set_icon_path (PyObject *, PyObject *args)
{
    // parse input args
    std::string arg_path;

    try
    {
        arg_path = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        mobius::core::ui::set_icon_path (arg_path);
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
// @brief <i>new_icon_by_name</i> function
// @param self Function object
// @param args Argument list
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_ui_new_icon_by_name (PyObject *, PyObject *args)
{
    // parse input args
    std::string arg_name;
    mobius::core::ui::icon::size_type arg_size =
        mobius::core::ui::icon::size_type::toolbar;

    try
    {
        arg_name = mobius::py::get_arg_as_std_string (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_size = static_cast<mobius::core::ui::icon::size_type> (
                mobius::py::get_arg_as_int (args, 1));
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
        ret = pymobius_core_ui_icon_to_pyobject (
            mobius::core::ui::new_icon_by_name (arg_name, arg_size));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return icon
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>new_icon_by_path</i> function
// @param self Function object
// @param args Argument list
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_ui_new_icon_by_path (PyObject *, PyObject *args)
{
    // parse input args
    std::string arg_path;
    mobius::core::ui::icon::size_type arg_size =
        mobius::core::ui::icon::size_type::toolbar;

    try
    {
        arg_path = mobius::py::get_arg_as_std_string (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_size = static_cast<mobius::core::ui::icon::size_type> (
                mobius::py::get_arg_as_int (args, 1));
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
        ret = pymobius_core_ui_icon_to_pyobject (
            mobius::core::ui::new_icon_by_path (arg_path, arg_size));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return icon
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>new_icon_by_url</i> function
// @param self Function object
// @param args Argument list
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_ui_new_icon_by_url (PyObject *, PyObject *args)
{
    // parse input args
    std::string arg_url;
    mobius::core::ui::icon::size_type arg_size =
        mobius::core::ui::icon::size_type::toolbar;

    try
    {
        arg_url = mobius::py::get_arg_as_std_string (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_size = static_cast<mobius::core::ui::icon::size_type> (
                mobius::py::get_arg_as_int (args, 1));
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
        ret = pymobius_core_ui_icon_to_pyobject (
            mobius::core::ui::new_icon_by_url (arg_url, arg_size));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return icon
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>new_icon_from_data</i> function
// @param self Function object
// @param args Argument list
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_ui_new_icon_from_data (PyObject *, PyObject *args)
{
    // parse input args
    mobius::core::bytearray arg_data;
    mobius::core::ui::icon::size_type arg_size =
        mobius::core::ui::icon::size_type::toolbar;

    try
    {
        arg_data = mobius::py::get_arg_as_bytearray (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_size = static_cast<mobius::core::ui::icon::size_type> (
                mobius::py::get_arg_as_int (args, 1));
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
        ret = pymobius_core_ui_icon_to_pyobject (
            mobius::core::ui::new_icon_from_data (arg_data, arg_size));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return icon
    return ret;
}
