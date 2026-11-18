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
// @file file.cpp
// @brief C++ API <i>mobius.core.kff.file</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "file.hpp"
#include <mobius/core/exception.inc>
#include <pycall.hpp>
#include <pymobius.hpp>
#include <stdexcept>
#include "../pod/data.hpp"

namespace mobius::py
{
namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_kff_file_type = nullptr;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_alert</i> method implementation
// @param self Object
// @param args Argument list
// @return True/False
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_alert (PyObject *self, PyObject *)
{
    return mobius::py::call_method (
        reinterpret_cast<core_kff_file_o *> (self)->obj,
        &mobius::core::kff::file::is_alert
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_type</i> method implementation
// @param self Object
// @param args Argument list
// @return File type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_type (PyObject *self, PyObject *)
{
    return mobius::py::call_method (
        reinterpret_cast<core_kff_file_o *> (self)->obj,
        &mobius::core::kff::file::get_type
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>has_size</i> method implementation
// @param self Object
// @param args Argument list
// @return True/False
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_has_size (PyObject *self, PyObject *)
{
    return mobius::py::call_method (
        reinterpret_cast<core_kff_file_o *> (self)->obj,
        &mobius::core::kff::file::has_size
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_size</i> method implementation
// @param self Object
// @param args Argument list
// @return File size
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_size (PyObject *self, PyObject *)
{
    return mobius::py::call_method (
        reinterpret_cast<core_kff_file_o *> (self)->obj,
        &mobius::core::kff::file::get_size
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_size</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_size (PyObject *self, PyObject *arg)
{
    // Parse input args
    std::uint64_t arg_size;

    try
    {
        arg_size = mobius::py::from_pyobject<std::uint64_t> (arg);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    return mobius::py::call_method (
        reinterpret_cast<core_kff_file_o *> (self)->obj,
        &mobius::core::kff::file::set_size,
        arg_size
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>has_hash</i> method implementation
// @param self Object
// @param args Argument list
// @return True/False
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_has_hash (PyObject *self, PyObject *arg)
{
    // Parse input args
    std::uint16_t arg_hash_type;

    try
    {
        arg_hash_type = mobius::py::from_pyobject<std::uint16_t> (arg);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    return mobius::py::call_method (
        reinterpret_cast<core_kff_file_o *> (self)->obj,
        &mobius::core::kff::file::has_hash,
        arg_hash_type
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_hash</i> method implementation
// @param self Object
// @param args Argument list
// @return Hash value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_hash (PyObject *self, PyObject *arg)
{
    // Parse input args
    std::uint16_t arg_hash_type;

    try
    {
        arg_hash_type = mobius::py::from_pyobject<std::uint16_t> (arg);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    return mobius::py::call_method (
        reinterpret_cast<core_kff_file_o *> (self)->obj,
        &mobius::core::kff::file::get_hash,
        arg_hash_type
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_hash</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_hash (PyObject *self, PyObject *args)
{
    // Parse input args
    std::uint16_t arg_hash_type;
    std::string arg_value;

    try
    {
        arg_hash_type = mobius::py::get_arg<std::uint16_t> (args, 0);
        arg_value = mobius::py::get_arg<std::string> (args, 1);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    return mobius::py::call_method (
        reinterpret_cast<core_kff_file_o *> (self)->obj,
        static_cast<void (mobius::core::kff::file::*) (std::uint16_t, const std::string &)> (
            &mobius::core::kff::file::set_hash
        ),
        arg_hash_type,
        arg_value
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>remove_hash</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_remove_hash (PyObject *self, PyObject *arg)
{
    // Parse input args
    std::uint16_t arg_hash_type;

    try
    {
        arg_hash_type = mobius::py::from_pyobject<std::uint16_t> (arg);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    return mobius::py::call_method (
        reinterpret_cast<core_kff_file_o *> (self)->obj,
        &mobius::core::kff::file::remove_hash,
        arg_hash_type
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>has_password</i> method implementation
// @param self Object
// @param args Argument list
// @return True if the file has the attribute, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_has_password (PyObject *self, PyObject *)
{
    return mobius::py::call_method (
        reinterpret_cast<core_kff_file_o *> (self)->obj,
        &mobius::core::kff::file::has_password
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_password</i> method implementation
// @param self Object
// @param args Argument list
// @return Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_password (PyObject *self, PyObject *)
{
    return mobius::py::call_method (
        reinterpret_cast<core_kff_file_o *> (self)->obj,
        &mobius::core::kff::file::get_password
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_password</i> method implementation
// @param self Object
// @param args Argument list
// @return None
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_password (PyObject *self, PyObject *arg)
{
    try
    {
        auto arg_value = mobius::py::from_pyobject<std::string> (arg);

        return mobius::py::call_method (
            reinterpret_cast<core_kff_file_o *> (self)->obj,
            &mobius::core::kff::file::set_password,
            arg_value
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return nullptr;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>remove_password</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_remove_password (PyObject *self, PyObject *)
{
    return mobius::py::call_method (
        reinterpret_cast<core_kff_file_o *> (self)->obj,
        &mobius::core::kff::file::remove_password
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>has_thumbnail</i> method implementation
// @param self Object
// @return True if the file has a thumbnail, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_has_thumbnail (PyObject *self, PyObject *)
{
    return mobius::py::call_method (
        reinterpret_cast<core_kff_file_o *> (self)->obj,
        &mobius::core::kff::file::has_thumbnail
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_thumbnail</i> method implementation
// @param self Object
// @param args Argument list
// @return Thumbnail data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_thumbnail (PyObject *self, PyObject *)
{
    return mobius::py::call_method (
        reinterpret_cast<core_kff_file_o *> (self)->obj,
        &mobius::core::kff::file::get_thumbnail
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_thumbnail</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_thumbnail (PyObject *self, PyObject *arg)
{
    // Parse input args
    mobius::core::bytearray arg_thumbnail;

    try
    {
        arg_thumbnail = mobius::py::from_pyobject<mobius::core::bytearray> (arg);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    return mobius::py::call_method (
        reinterpret_cast<core_kff_file_o *> (self)->obj,
        &mobius::core::kff::file::set_thumbnail,
        arg_thumbnail
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] = {
    {"is_alert", tp_f_is_alert, METH_NOARGS, "is_alert()\n--\n\nCheck if the file is an alert"},
    {"get_type", tp_f_get_type, METH_NOARGS, "get_type()\n--\n\nGet file type"},
    {"has_size", tp_f_has_size, METH_NOARGS, "has_size()\n--\n\nCheck if file has size"},
    {"get_size", tp_f_get_size, METH_NOARGS, "get_size()\n--\n\nGet file size"},
    {"set_size", tp_f_set_size, METH_O, "set_size(size)\n--\n\nSet file size"},
    {"has_hash", tp_f_has_hash, METH_O, "has_hash(hash_type)\n--\n\nCheck if file has a hash"},
    {"get_hash", tp_f_get_hash, METH_O, "get_hash(hash_type)\n--\n\nGet file hash"},
    {"set_hash", tp_f_set_hash, METH_VARARGS, "set_hash(hash_type,value)\n--\n\nSet file hash"},
    {"remove_hash", tp_f_remove_hash, METH_O, "remove_hash(hash_type)\n--\n\nRemove file hash"},
    {"has_password", tp_f_has_password, METH_NOARGS, "has_password()\n--\n\nCheck if file has a password"},
    {"get_password", tp_f_get_password, METH_NOARGS, "get_password()\n--\n\nGet file password"},
    {"set_password", tp_f_set_password, METH_O, "set_password(password)\n--\n\nSet file password"},
    {"remove_password", tp_f_remove_password, METH_NOARGS, "remove_password()\n--\n\nRemove file password"},
    {"has_thumbnail", tp_f_has_thumbnail, METH_NOARGS, "has_thumbnail()\n--\n\nCheck if file has a thumbnail"},
    {"get_thumbnail", tp_f_get_thumbnail, METH_NOARGS, "get_thumbnail()\n--\n\nGet thumbnail"},
    {"set_thumbnail", tp_f_set_thumbnail, METH_O, "set_thumbnail(thumbnail)\n--\n\nSet thumbnail"},
    {nullptr, nullptr, 0, nullptr}, // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>file</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_kff_file_o *self)
{
    PyTypeObject *tp = Py_TYPE (self);
    delete self->obj;
    tp->tp_free (reinterpret_cast<PyObject *> (self));
    Py_DECREF (tp);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_kff_file_slots[] = {
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("\n--\n\nKFF file")},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_kff_file_spec = {
    .name = "mobius.core.kff.file",
    .basicsize = sizeof (core_kff_file_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_IMMUTABLETYPE | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .slots = core_kff_file_slots,
};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get the global type object for core_kff_file
// @return PyTypeObject pointer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject *
get_core_kff_file_type ()
{
    if (!core_kff_file_type)
        core_kff_file_type = create_type (&core_kff_file_spec);

    return core_kff_file_type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>file</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_kff_file_check (PyObject *value)
{
    return mobius::py::isinstance (value, get_core_kff_file_type ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>file</i> Python object from C++ object
// @param obj C++ object
// @return New file object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
to_pyobject (const mobius::core::kff::file &obj)
{
    return mobius::py::to_pyobject_nullable<core_kff_file_o> (obj, get_core_kff_file_type ());
}

} // namespace mobius::py
