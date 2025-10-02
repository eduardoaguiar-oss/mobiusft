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
// @file box.cc C++ API <i>mobius.core.ui.box</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "box.hpp"
#include "widget.hpp"
#include <pymobius.hpp>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_spacing</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_spacing (core_ui_box_o *self, PyObject *args)
{
    // Parse input args
    std::uint32_t arg_siz;

    try
    {
        arg_siz = mobius::py::get_arg_as_uint32_t (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->set_spacing (arg_siz);
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
// @brief <i>set_border_width</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_border_width (core_ui_box_o *self, PyObject *args)
{
    // Parse input args
    std::uint32_t arg_siz;

    try
    {
        arg_siz = mobius::py::get_arg_as_uint32_t (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->set_border_width (arg_siz);
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
// @brief <i>add_child</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_add_child (core_ui_box_o *self, PyObject *args)
{
    // Parse input args
    mobius::core::ui::widget arg_w;
    mobius::core::ui::box::fill_type arg_filling;

    try
    {
        arg_w = mobius::py::get_arg_as_cpp (
            args, 0, pymobius_core_ui_widget_from_pyobject);
        arg_filling = static_cast<mobius::core::ui::box::fill_type> (
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
        self->obj->add_child (arg_w, arg_filling);
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
// @brief <i>add_filler</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_add_filler (core_ui_box_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        self->obj->add_filler ();
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
// @brief <i>remove_child</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_remove_child (core_ui_box_o *self, PyObject *args)
{
    // Parse input args
    mobius::core::ui::widget arg_w;

    try
    {
        arg_w = mobius::py::get_arg_as_cpp (
            args, 0, pymobius_core_ui_widget_from_pyobject);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->remove_child (arg_w);
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
// @brief <i>clear</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_clear (core_ui_box_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        self->obj->clear ();
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
    {"set_spacing", (PyCFunction) tp_f_set_spacing, METH_VARARGS,
     "Set spacing between widgets"},
    {"set_border_width", (PyCFunction) tp_f_set_border_width, METH_VARARGS,
     "Set border width"},
    {"add_child", (PyCFunction) tp_f_add_child, METH_VARARGS,
     "Add child widget"},
    {"add_filler", (PyCFunction) tp_f_add_filler, METH_VARARGS,
     "Add filler space"},
    {"remove_child", (PyCFunction) tp_f_remove_child, METH_VARARGS,
     "Remove child widget"},
    {"clear", (PyCFunction) tp_f_clear, METH_VARARGS, "Clear widget"},
    {nullptr, nullptr, 0, nullptr}, // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>box</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>box</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *args, PyObject *)
{
    // Parse input args
    mobius::core::ui::box::orientation_type arg_orientation;

    try
    {
        arg_orientation = static_cast<mobius::core::ui::box::orientation_type> (
            mobius::py::get_arg_as_int (args, 0));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Create Python object
    core_ui_box_o *ret =
        reinterpret_cast<core_ui_box_o *> (type->tp_alloc (type, 0));

    if (ret)
    {
        try
        {
            ret->obj = new mobius::core::ui::box (arg_orientation);
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
// @brief <i>box</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_ui_box_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyTypeObject core_ui_box_t = {
    PyVarObject_HEAD_INIT(nullptr, 0)        // initializer
    "mobius.core.ui.box",                    // tp_name
    sizeof(core_ui_box_o),                   // tp_basicsize
    0,                                       // tp_itemsize
    (destructor)tp_dealloc,                  // tp_dealloc
    0,                                       // tp_vectorcall_offset
    nullptr,                                 // tp_getattr
    nullptr,                                 // tp_setattr
    nullptr,                                 // tp_as_async
    nullptr,                                 // tp_repr
    nullptr,                                 // tp_as_number
    nullptr,                                 // tp_as_sequence
    nullptr,                                 // tp_as_mapping
    nullptr,                                 // tp_hash
    nullptr,                                 // tp_call
    nullptr,                                 // tp_str
    nullptr,                                 // tp_getattro
    nullptr,                                 // tp_setattro
    nullptr,                                 // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    "Box class",                             // tp_doc
    nullptr,                                 // tp_traverse
    nullptr,                                 // tp_clear
    nullptr,                                 // tp_richcompare
    0,                                       // tp_weaklistoffset
    nullptr,                                 // tp_iter
    nullptr,                                 // tp_iternext
    tp_methods,                              // tp_methods
    nullptr,                                 // tp_members
    nullptr,                                 // tp_getset
    get_ui_widget_type(),                    // tp_base
    nullptr,                                 // tp_dict
    nullptr,                                 // tp_descr_get
    nullptr,                                 // tp_descr_set
    0,                                       // tp_dictoffset
    nullptr,                                 // tp_init
    nullptr,                                 // tp_alloc
    tp_new,                                  // tp_new
    nullptr,                                 // tp_free
    nullptr,                                 // tp_is_gc
    nullptr,                                 // tp_bases
    nullptr,                                 // tp_mro
    nullptr,                                 // tp_cache
    nullptr,                                 // tp_subclasses
    nullptr,                                 // tp_weaklist
    nullptr,                                 // tp_del
    0,                                       // tp_version_tag
    nullptr,                                 // tp_finalize
    nullptr,                                 // tp_vectorcall (Python 3.9+)
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.ui.box</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_ui_box_type ()
{
    mobius::py::pytypeobject type (&core_ui_box_t);
    type.create ();

    type.add_constant ("orientation_vertical", 1);
    type.add_constant ("orientation_horizontal", 2);
    type.add_constant ("fill_none", 1);
    type.add_constant ("fill_with_space", 2);
    type.add_constant ("fill_with_widget", 3);

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>box</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_ui_box_check (PyObject *value)
{
    return mobius::py::isinstance (value, &core_ui_box_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>box</i> Python object from C++ object
// @param obj C++ object
// @return New box object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_ui_box_to_pyobject (const mobius::core::ui::box &obj)
{
    return mobius::py::to_pyobject<core_ui_box_o> (obj, &core_ui_box_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>box</i> C++ object from Python object
// @param value Python value
// @return Box object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::ui::box
pymobius_core_ui_box_from_pyobject (PyObject *value)
{
    return mobius::py::from_pyobject<core_ui_box_o> (value, &core_ui_box_t);
}
