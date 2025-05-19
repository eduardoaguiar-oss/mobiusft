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
// @file master_key.cc C++ API <i>mobius.core.os.win.dpapi.master_key</i> class
// wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "master_key.hpp"
#include "module.hpp"
#include <pygil.hpp>
#include <pymobius.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if object type is <i>master_key</i>
// @param pyobj Python object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_os_win_dpapi_master_key_check (PyObject *pyobj)
{
    return PyObject_IsInstance (pyobj,
                                (PyObject *) &core_os_win_dpapi_master_key_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>master_key</i> Python object from C++ object
// @param obj C++ object
// @return new master_key object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_os_win_dpapi_master_key_to_pyobject (
    mobius::core::os::win::dpapi::master_key obj)
{
    PyObject *ret = nullptr;

    if (obj)
    {
        ret = _PyObject_New (&core_os_win_dpapi_master_key_t);

        if (ret)
            ((core_os_win_dpapi_master_key_o *) ret)->obj =
                new mobius::core::os::win::dpapi::master_key (obj);
    }
    else
        ret = mobius::py::pynone ();

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>master_key</i> C++ object from Python object
// @param value Python object
// @return master_key object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::os::win::dpapi::master_key
pymobius_core_os_win_dpapi_master_key_from_pyobject (PyObject *value)
{
    if (!pymobius_core_os_win_dpapi_master_key_check (value))
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("object must be an instance of "
                                  "mobius.core.os.win.dpapi.master_key"));

    return *(reinterpret_cast<core_os_win_dpapi_master_key_o *> (value)->obj);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>revision</i> Attribute getter
// @param self Object
// @return <i>revision</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_revision (core_os_win_dpapi_master_key_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint32_t (self->obj->get_revision ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>salt</i> Attribute getter
// @param self Object
// @return <i>salt</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_salt (core_os_win_dpapi_master_key_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybytes_from_bytearray (self->obj->get_salt ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>iterations</i> Attribute getter
// @param self Object
// @return <i>iterations</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_iterations (core_os_win_dpapi_master_key_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret =
            mobius::py::pylong_from_std_uint32_t (self->obj->get_iterations ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>hash_id</i> Attribute getter
// @param self Object
// @return <i>hash_id</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_hash_id (core_os_win_dpapi_master_key_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint32_t (self->obj->get_hash_id ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>cipher_id</i> Attribute getter
// @param self Object
// @return <i>cipher_id</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_cipher_id (core_os_win_dpapi_master_key_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret =
            mobius::py::pylong_from_std_uint32_t (self->obj->get_cipher_id ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>cipher_text</i> Attribute getter
// @param self Object
// @return <i>cipher_text</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_cipher_text (core_os_win_dpapi_master_key_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret =
            mobius::py::pybytes_from_bytearray (self->obj->get_cipher_text ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>plain_text</i> Attribute getter
// @param self Object
// @return <i>plain_text</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_plain_text (core_os_win_dpapi_master_key_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybytes_from_bytearray (self->obj->get_plain_text ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>flags</i> Attribute getter
// @param self Object
// @return <i>flags</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_flags (core_os_win_dpapi_master_key_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint32_t (self->obj->get_flags ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Getters and setters structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyGetSetDef tp_getset[] = {
    {(char *) "revision", (getter) tp_getter_revision, nullptr,
     (char *) "Struct revision", nullptr},
    {(char *) "salt", (getter) tp_getter_salt, nullptr, (char *) "Salt",
     nullptr},
    {(char *) "iterations", (getter) tp_getter_iterations, nullptr,
     (char *) "Number of key iterations", nullptr},
    {(char *) "hash_id", (getter) tp_getter_hash_id, nullptr,
     (char *) "Hash algorithm ID", nullptr},
    {(char *) "cipher_id", (getter) tp_getter_cipher_id, nullptr,
     (char *) "Cipher algorithm ID", nullptr},
    {(char *) "cipher_text", (getter) tp_getter_cipher_text, nullptr,
     (char *) "Cipher text", nullptr},
    {(char *) "plain_text", (getter) tp_getter_plain_text, nullptr,
     (char *) "Plain text", nullptr},
    {(char *) "flags", (getter) tp_getter_flags, nullptr,
     (char *) "Master key file flags", nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>decrypt_with_key</i> method implementation
// @param self Object
// @param args Argument list
// @return None
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_decrypt_with_key (core_os_win_dpapi_master_key_o *self, PyObject *args)
{
    // parse input args
    mobius::core::bytearray arg_key;

    try
    {
        arg_key = mobius::py::get_arg_as_bytearray (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (
            mobius::py::GIL () (self->obj->decrypt_with_key (arg_key)));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>decrypt_with_password_hash</i> method implementation
// @param self Object
// @param args Argument list
// @return None
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_decrypt_with_password_hash (core_os_win_dpapi_master_key_o *self,
                                 PyObject *args)
{
    // parse input args
    std::string arg_sid;
    mobius::core::bytearray arg_password_hash;

    try
    {
        arg_sid = mobius::py::get_arg_as_std_string (args, 0);
        arg_password_hash = mobius::py::get_arg_as_bytearray (args, 1);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (
            mobius::py::GIL () (self->obj->decrypt_with_password_hash (
                arg_sid, arg_password_hash)));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>decrypt_with_password</i> method implementation
// @param self Object
// @param args Argument list
// @return None
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_decrypt_with_password (core_os_win_dpapi_master_key_o *self,
                            PyObject *args)
{
    // parse input args
    std::string arg_sid;
    std::string arg_password;

    try
    {
        arg_sid = mobius::py::get_arg_as_std_string (args, 0);
        arg_password = mobius::py::get_arg_as_std_string (args, 1);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (mobius::py::GIL () (
            self->obj->decrypt_with_password (arg_sid, arg_password)));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_decrypted</i> method implementation
// @param self Object
// @param args Argument list
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_decrypted (core_os_win_dpapi_master_key_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->is_decrypted ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] = {
    {(char *) "decrypt_with_key", (PyCFunction) tp_f_decrypt_with_key,
     METH_VARARGS, "Decrypt master key using key"},
    {(char *) "decrypt_with_password_hash",
     (PyCFunction) tp_f_decrypt_with_password_hash, METH_VARARGS,
     "Decrypt master key using password hash"},
    {(char *) "decrypt_with_password", (PyCFunction) tp_f_decrypt_with_password,
     METH_VARARGS, "Decrypt master key using password"},
    {(char *) "is_decrypted", (PyCFunction) tp_f_is_decrypted, METH_VARARGS,
     "Check if master key is decrypted"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>master_key</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>master_key</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *, PyObject *)
{
    core_os_win_dpapi_master_key_o *self =
        (core_os_win_dpapi_master_key_o *) type->tp_alloc (type, 0);

    if (self)
        self->obj = new mobius::core::os::win::dpapi::master_key ();

    return (PyObject *) self;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>master_key</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_os_win_dpapi_master_key_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject core_os_win_dpapi_master_key_t = {
    PyVarObject_HEAD_INIT (nullptr, 0)        // header
    "mobius.core.os.win.dpapi.master_key",    // tp_name
    sizeof (core_os_win_dpapi_master_key_o),  // tp_basicsize
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
    "master_key class",                       // tp_doc
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
};
