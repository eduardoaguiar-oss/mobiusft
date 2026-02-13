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
// @file richtext.cpp C++ API <i>mobius.core.richtext</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "richtext.hpp"
#include <pylist.hpp>
#include <pymobius.hpp>
#include <stdexcept>
#include "pod/map.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>clear</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_clear (core_richtext_o *self, PyObject *)
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>size</i> method implementation
// @param self Object
// @param args Argument list
// @return None
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_size (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = PyLong_FromSize_t (self->obj->size ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_segments</i> method implementation
// @param self Object
// @param args Argument list
// @return None
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_segments (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    auto t = [] (const mobius::core::richtext::segment &s)
    { return std::make_pair (s.type, s.metadata); };

    try
    {
        ret = mobius::py::pylist_from_cpp_pair_transform (
            self->obj->get_segments (), mobius::py::pystring_from_std_string,
            pymobius_core_pod_map_to_python, t
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>begin_bold</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_begin_bold (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        self->obj->begin_bold ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>begin_italic</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_begin_italic (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        self->obj->begin_italic ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>begin_link</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_begin_link (core_richtext_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_url;

    try
    {
        arg_url = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception& e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->begin_link (arg_url);
    }
    catch (const std::exception& e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>begin_message_bubble</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_begin_message_bubble (core_richtext_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_timestamp;
    std::string arg_author;

    try
    {
        arg_timestamp = mobius::py::get_arg_as_std_string (args, 0);
        arg_author = mobius::py::get_arg_as_std_string (args, 1);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->begin_message_bubble (arg_timestamp, arg_author);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>begin_strikethrough</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_begin_strikethrough (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        self->obj->begin_strikethrough ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>begin_underline</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_begin_underline (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        self->obj->begin_underline ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>end_bold</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_end_bold (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        self->obj->end_bold ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>end_italic</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_end_italic (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        self->obj->end_italic ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>end_link</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_end_link (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        self->obj->end_link ();
    }
    catch (const std::exception& e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>end_message_bubble</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_end_message_bubble (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        self->obj->end_message_bubble ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>end_strikethrough</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_end_strikethrough (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        self->obj->end_strikethrough ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>end_underline</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_end_underline (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        self->obj->end_underline ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>add_text</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_add_text (core_richtext_o *self, PyObject *args)
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
        self->obj->add_text (arg_text);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>add_system_text</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_add_system_text (core_richtext_o *self, PyObject *args)
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
        self->obj->add_system_text (arg_text);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>add_newline</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_add_newline (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    try
    {
        self->obj->add_newline ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>add_emoji</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_add_emoji (core_richtext_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_id;

    try
    {
        arg_id = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->add_emoji (arg_id);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>add_flag</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_add_flag (core_richtext_o *self, PyObject *args)
{
    // Parse input args
    std::string arg_id;

    try
    {
        arg_id = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    try
    {
        self->obj->add_flag (arg_id);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }

    // return None
    return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>to_html</i> method implementation
// @param self Object
// @param args Argument list
// @return HTML markup text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_to_html (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->to_html ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>to_markdown</i> method implementation
// @param self Object
// @param args Argument list
// @return MarkDown markup text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_to_markdown (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->to_markdown ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>to_latex</i> method implementation
// @param self Object
// @param args Argument list
// @return LaTeX markup text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_to_latex (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->to_latex ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>to_pango</i> method implementation
// @param self Object
// @param args Argument list
// @return Pango markup text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_to_pango (core_richtext_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->to_pango ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] = {
    {"clear", (PyCFunction) tp_f_clear, METH_VARARGS, "Clear content"},
    {"size", (PyCFunction) tp_f_size, METH_VARARGS, "Get size"},
    {"get_segments", (PyCFunction) tp_f_get_segments, METH_VARARGS, "Get segments"},
    {"begin_bold", (PyCFunction) tp_f_begin_bold, METH_VARARGS, "Begin bold"},
    {"begin_italic", (PyCFunction) tp_f_begin_italic, METH_VARARGS, "Begin italic"},
    {"begin_link", (PyCFunction) tp_f_begin_link, METH_VARARGS, "Begin link"},
    {"begin_message_bubble", (PyCFunction) tp_f_begin_message_bubble, METH_VARARGS, "Add reply"},
    {"begin_strikethrough", (PyCFunction) tp_f_begin_strikethrough, METH_VARARGS, "Begin strikethrough"},
    {"begin_underline", (PyCFunction) tp_f_begin_underline, METH_VARARGS, "Begin underline"},
    {"end_bold", (PyCFunction) tp_f_end_bold, METH_VARARGS, "End bold text"},
    {"end_italic", (PyCFunction) tp_f_end_italic, METH_VARARGS, "End italic text"},
    {"end_link", (PyCFunction) tp_f_end_link, METH_VARARGS, "End link"},
    {"end_message_bubble", (PyCFunction) tp_f_end_message_bubble, METH_VARARGS, "End message bubble"},
    {"end_strikethrough", (PyCFunction) tp_f_end_strikethrough, METH_VARARGS, "End strikethrough"},
    {"end_underline", (PyCFunction) tp_f_end_underline, METH_VARARGS, "End underline"},
    {"add_text", (PyCFunction) tp_f_add_text, METH_VARARGS, "Add normal text"},
    {"add_system_text", (PyCFunction) tp_f_add_system_text, METH_VARARGS, "Add system text"},
    {"add_newline", (PyCFunction) tp_f_add_newline, METH_VARARGS, "Add newline"},
    {"add_emoji", (PyCFunction) tp_f_add_emoji, METH_VARARGS, "Add emoji char"},
    {"add_flag", (PyCFunction) tp_f_add_flag, METH_VARARGS, "Add flag char"},
    {"to_html", (PyCFunction) tp_f_to_html, METH_VARARGS, "Convert richtext to HTML"},
    {"to_markdown", (PyCFunction) tp_f_to_markdown, METH_VARARGS, "Convert richtext to Markdown"},
    {"to_latex", (PyCFunction) tp_f_to_latex, METH_VARARGS, "Convert richtext to LaTeX"},
    {"to_pango", (PyCFunction) tp_f_to_pango, METH_VARARGS, "Convert richtext to Pango"},
    {nullptr, nullptr, 0, nullptr}, // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>richtext</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>richtext</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *, PyObject *)
{
    core_richtext_o *ret =
        reinterpret_cast<core_richtext_o *> (type->tp_alloc (type, 0));

    if (ret)
    {
        try
        {
            ret->obj = new mobius::core::richtext ();
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>richtext</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_richtext_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyTypeObject core_richtext_t = {
    PyVarObject_HEAD_INIT (nullptr, 0)        // header
    "mobius.core.richtext",                   // tp_name
    sizeof (core_richtext_o),                 // tp_basicsize
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
    "RichText class",                         // tp_doc
    0,                                        // tp_traverse
    0,                                        // tp_clear
    0,                                        // tp_richcompare
    0,                                        // tp_weaklistoffset
    0,                                        // tp_iter
    0,                                        // tp_iternext
    tp_methods,                               // tp_methods
    0,                                        // tp_members
    0,                                        // tp_getset
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
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.richtext</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_richtext_type ()
{
    mobius::py::pytypeobject type (&core_richtext_t);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>richtext</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_richtext_check (PyObject *value)
{
    return mobius::py::isinstance (value, &core_richtext_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>richtext</i> Python object from C++ object
// @param obj C++ object
// @return New richtext object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_richtext_to_pyobject (const mobius::core::richtext &obj)
{
    return mobius::py::to_pyobject_nullable<core_richtext_o> (
        obj, &core_richtext_t
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>richtext</i> C++ object from Python object
// @param value Python value
// @return Richtext object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::richtext
pymobius_core_richtext_from_pyobject (PyObject *value)
{
    return mobius::py::from_pyobject<core_richtext_o> (value, &core_richtext_t);
}
