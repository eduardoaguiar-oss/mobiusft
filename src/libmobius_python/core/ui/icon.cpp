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
#include <pymobius.hpp>
#include <stdexcept>
#include "widget.hpp"

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_ui_icon_type = nullptr;

} // namespace

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
    mobius::core::ui::icon::size_type arg_size = mobius::core::ui::icon::size_type::toolbar;

    try
    {
        arg_name = mobius::py::get_arg_as_std_string (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_size = static_cast<mobius::core::ui::icon::size_type> (mobius::py::get_arg_as_int (args, 1));
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
    mobius::core::ui::icon::size_type arg_size = mobius::core::ui::icon::size_type::toolbar;

    try
    {
        arg_path = mobius::py::get_arg_as_std_string (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_size = static_cast<mobius::core::ui::icon::size_type> (mobius::py::get_arg_as_int (args, 1));
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
    mobius::core::ui::icon::size_type arg_size = mobius::core::ui::icon::size_type::toolbar;

    try
    {
        arg_url = mobius::py::get_arg_as_std_string (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_size = static_cast<mobius::core::ui::icon::size_type> (mobius::py::get_arg_as_int (args, 1));
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
    mobius::core::ui::icon::size_type arg_size = mobius::core::ui::icon::size_type::toolbar;

    try
    {
        arg_data = mobius::py::get_arg_as_bytearray (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_size = static_cast<mobius::core::ui::icon::size_type> (mobius::py::get_arg_as_int (args, 1));
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
    {"set_icon_by_name", (PyCFunction) tp_f_set_icon_by_name, METH_VARARGS, "Set icon by name"},
    {"set_icon_by_path", (PyCFunction) tp_f_set_icon_by_path, METH_VARARGS, "Set icon by path"},
    {"set_icon_by_url", (PyCFunction) tp_f_set_icon_by_url, METH_VARARGS, "Set icon by URL"},
    {"set_icon_from_data", (PyCFunction) tp_f_set_icon_from_data, METH_VARARGS, "Set icon from data"},
    {nullptr, nullptr, 0, nullptr}, // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>icon</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_ui_icon_o *self)
{
    PyTypeObject *tp = Py_TYPE (self);
    delete self->obj;
    tp->tp_free (reinterpret_cast<PyObject *> (self));
    Py_DECREF (tp);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_ui_icon_slots[] = {
    {Py_tp_base, reinterpret_cast<void *> (get_core_ui_widget_type ())},
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("core.ui.icon class")},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_ui_icon_spec = {
    .name = "mobius.core.ui.icon",
    .basicsize = sizeof (core_ui_icon_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_IMMUTABLETYPE,
    .slots = core_ui_icon_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.ui.icon</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_ui_icon_type ()
{
    // If type is already created, return it
    if (core_ui_icon_type)
        return mobius::py::pytypeobject (core_ui_icon_type);

    // Allocate type from spec
    core_ui_icon_type = reinterpret_cast<PyTypeObject *> (PyType_FromSpec (&core_ui_icon_spec));

    // Create type
    mobius::py::pytypeobject type (core_ui_icon_type);
    type.create ();

    type.add_constant ("size_menu", 16);
    type.add_constant ("size_toolbar", 24);
    type.add_constant ("size_dnd", 32);
    type.add_constant ("size_dialog", 48);
    type.add_constant ("size_large", 64);
    type.add_constant ("size_extra_large", 128);

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>core.ui.icon</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_ui_icon_check (PyObject *value)
{
    if (!core_ui_icon_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.ui.icon type is not initialized"));

    return mobius::py::isinstance (value, core_ui_icon_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.ui.icon</i> Python object from C++ object
// @param obj C++ object
// @return New core.ui.icon object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_ui_icon_to_pyobject (const mobius::core::ui::icon &obj)
{
    if (!core_ui_icon_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.ui.icon type is not initialized"));

    return mobius::py::to_pyobject<core_ui_icon_o> (obj, core_ui_icon_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.ui.icon</i> C++ object from Python object
// @param value Python value
// @return core.ui.icon object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::ui::icon
pymobius_core_ui_icon_from_pyobject (PyObject *value)
{
    if (!core_ui_icon_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.ui.icon type is not initialized"));

    return mobius::py::from_pyobject<core_ui_icon_o> (value, core_ui_icon_type);
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
    mobius::core::ui::icon::size_type arg_size = mobius::core::ui::icon::size_type::toolbar;

    try
    {
        arg_name = mobius::py::get_arg_as_std_string (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_size = static_cast<mobius::core::ui::icon::size_type> (mobius::py::get_arg_as_int (args, 1));
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
        ret = pymobius_core_ui_icon_to_pyobject (mobius::core::ui::new_icon_by_name (arg_name, arg_size));
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
    mobius::core::ui::icon::size_type arg_size = mobius::core::ui::icon::size_type::toolbar;

    try
    {
        arg_path = mobius::py::get_arg_as_std_string (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_size = static_cast<mobius::core::ui::icon::size_type> (mobius::py::get_arg_as_int (args, 1));
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
        ret = pymobius_core_ui_icon_to_pyobject (mobius::core::ui::new_icon_by_path (arg_path, arg_size));
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
    mobius::core::ui::icon::size_type arg_size = mobius::core::ui::icon::size_type::toolbar;

    try
    {
        arg_url = mobius::py::get_arg_as_std_string (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_size = static_cast<mobius::core::ui::icon::size_type> (mobius::py::get_arg_as_int (args, 1));
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
        ret = pymobius_core_ui_icon_to_pyobject (mobius::core::ui::new_icon_by_url (arg_url, arg_size));
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
    mobius::core::ui::icon::size_type arg_size = mobius::core::ui::icon::size_type::toolbar;

    try
    {
        arg_data = mobius::py::get_arg_as_bytearray (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_size = static_cast<mobius::core::ui::icon::size_type> (mobius::py::get_arg_as_int (args, 1));
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
        ret = pymobius_core_ui_icon_to_pyobject (mobius::core::ui::new_icon_from_data (arg_data, arg_size));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return icon
    return ret;
}
