// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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
//! \file credhist_entry.cc C++ API <i>mobius.os.win.dpapi.credhist_entry</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <pymobius.h>
#include <pygil.h>
#include "credhist_entry.h"
#include "module.h"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if object type is <i>credhist_entry</i>
// @param pyobj Python object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_os_win_dpapi_credhist_entry_check (PyObject *pyobj)
{
  return PyObject_IsInstance (pyobj, (PyObject *) &os_win_dpapi_credhist_entry_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>credhist_entry</i> Python object from C++ object
// @param obj C++ object
// @return new credhist_entry object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_os_win_dpapi_credhist_entry_to_pyobject (mobius::os::win::dpapi::credhist_entry obj)
{
  PyObject *ret = _PyObject_New (&os_win_dpapi_credhist_entry_t);

  if (ret)
    ((os_win_dpapi_credhist_entry_o *) ret)->obj = new mobius::os::win::dpapi::credhist_entry (obj);

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>credhist_entry</i> C++ object from Python object
// @param value Python object
// @return credhist_entry object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::os::win::dpapi::credhist_entry
pymobius_os_win_dpapi_credhist_entry_from_pyobject (PyObject *value)
{
  if (!pymobius_os_win_dpapi_credhist_entry_check (value))
    throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG ("object must be an instance of mobius.os.win.dpapi.credhist_entry"));

  return * (reinterpret_cast <os_win_dpapi_credhist_entry_o *>(value)->obj);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>revision</i> Attribute getter
// @param self Object
// @return <i>revision</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_revision (os_win_dpapi_credhist_entry_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_uint32_t (self->obj->get_revision ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>guid</i> Attribute getter
// @param self Object
// @return <i>guid</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_guid (os_win_dpapi_credhist_entry_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->get_guid ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>type</i> Attribute getter
// @param self Object
// @return <i>type</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_type (os_win_dpapi_credhist_entry_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_uint32_t (self->obj->get_type ());
    }
  catch (const std::exception& e)
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
tp_getter_hash_id (os_win_dpapi_credhist_entry_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_uint32_t (self->obj->get_hash_id ());
    }
  catch (const std::exception& e)
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
tp_getter_iterations (os_win_dpapi_credhist_entry_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_uint32_t (self->obj->get_iterations ());
    }
  catch (const std::exception& e)
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
tp_getter_cipher_id (os_win_dpapi_credhist_entry_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_uint32_t (self->obj->get_cipher_id ());
    }
  catch (const std::exception& e)
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
tp_getter_salt (os_win_dpapi_credhist_entry_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybytes_from_bytearray (self->obj->get_salt ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>sid</i> Attribute getter
// @param self Object
// @return <i>sid</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_sid (os_win_dpapi_credhist_entry_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->get_sid ());
    }
  catch (const std::exception& e)
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
tp_getter_cipher_text (os_win_dpapi_credhist_entry_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybytes_from_bytearray (self->obj->get_cipher_text ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>hash_sha1</i> Attribute getter
// @param self Object
// @return <i>hash_sha1</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_hash_sha1 (os_win_dpapi_credhist_entry_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybytes_from_bytearray (self->obj->get_hash_sha1 ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>hash_ntlm</i> Attribute getter
// @param self Object
// @return <i>hash_ntlm</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_hash_ntlm (os_win_dpapi_credhist_entry_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybytes_from_bytearray (self->obj->get_hash_ntlm ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Getters and setters structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyGetSetDef tp_getset[] =
{
  {
    (char *) "revision",
    (getter) tp_getter_revision,
    (setter) 0,
    (char *) "Struct revision",
    NULL
  },
  {
    (char *) "guid",
    (getter) tp_getter_guid,
    (setter) 0,
    (char *) "GUID",
    NULL
  },
  {
    (char *) "type",
    (getter) tp_getter_type,
    (setter) 0,
    (char *) "Type",
    NULL
  },
  {
    (char *) "hash_id",
    (getter) tp_getter_hash_id,
    (setter) 0,
    (char *) "Hash algorithm ID",
    NULL
  },
  {
    (char *) "iterations",
    (getter) tp_getter_iterations,
    (setter) 0,
    (char *) "Number of key iterations",
    NULL
  },
  {
    (char *) "cipher_id",
    (getter) tp_getter_cipher_id,
    (setter) 0,
    (char *) "Cipher algorithm ID",
    NULL
  },
  {
    (char *) "salt",
    (getter) tp_getter_salt,
    (setter) 0,
    (char *) "Salt",
    NULL
  },
  {
    (char *) "sid",
    (getter) tp_getter_sid,
    (setter) 0,
    (char *) "SID",
    NULL
  },
  {
    (char *) "cipher_text",
    (getter) tp_getter_cipher_text,
    (setter) 0,
    (char *) "Cipher text",
    NULL
  },
  {
    (char *) "hash_sha1",
    (getter) tp_getter_hash_sha1,
    (setter) 0,
    (char *) "SHA1 hash value",
    NULL
  },
  {
    (char *) "hash_ntlm",
    (getter) tp_getter_hash_ntlm,
    (setter) 0,
    (char *) "NTLM hash value",
    NULL
  },
  {NULL, NULL, NULL, NULL, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>decrypt_with_key</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_decrypt_with_key (os_win_dpapi_credhist_entry_o *self, PyObject *args)
{
  // parse input args
  mobius::bytearray arg_key;

  try
    {
      arg_key = mobius::py::get_arg_as_bytearray (args, 0);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      auto rc = mobius::py::GIL () (
                   self->obj->decrypt_with_key (arg_key)
                );
      ret = mobius::py::pybool_from_bool (rc);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>decrypt_with_password_hash</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_decrypt_with_password_hash (os_win_dpapi_credhist_entry_o *self, PyObject *args)
{
  // parse input args
  mobius::bytearray arg_password_hash;

  try
    {
      arg_password_hash = mobius::py::get_arg_as_bytearray (args, 0);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      auto rc = mobius::py::GIL () (
                   self->obj->decrypt_with_password_hash (arg_password_hash)
                );

      ret = mobius::py::pybool_from_bool (rc);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>decrypt_with_password</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_decrypt_with_password (os_win_dpapi_credhist_entry_o *self, PyObject *args)
{
  // parse input args
  std::string arg_password;

  try
    {
      arg_password = mobius::py::get_arg_as_std_string (args, 0);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      auto rc = mobius::py::GIL () (
                   self->obj->decrypt_with_password (arg_password)
                );
      ret = mobius::py::pybool_from_bool (rc);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_decrypted</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_decrypted (os_win_dpapi_credhist_entry_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->is_decrypted ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
      return nullptr;
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] =
{
  {
    (char *) "decrypt_with_key",
    (PyCFunction) tp_f_decrypt_with_key,
    METH_VARARGS,
    "Decrypt entry using key"
  },
  {
    (char *) "decrypt_with_password_hash",
    (PyCFunction) tp_f_decrypt_with_password_hash,
    METH_VARARGS,
    "Decrypt entry using password hash"
  },
  {
    (char *) "decrypt_with_password",
    (PyCFunction) tp_f_decrypt_with_password,
    METH_VARARGS,
    "Decrypt entry using password"
  },
  {
    (char *) "is_decrypted",
    (PyCFunction) tp_f_is_decrypted,
    METH_VARARGS,
    "Check if entry is decrypted"
  },
  {NULL, NULL, 0, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>credhist_entry</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (os_win_dpapi_credhist_entry_o *self)
{
  delete self->obj;
  Py_TYPE (self)->tp_free ((PyObject*) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject os_win_dpapi_credhist_entry_t =
{
  PyVarObject_HEAD_INIT (NULL, 0)
  "mobius.os.win.dpapi.credhist_entry",    		// tp_name
  sizeof (os_win_dpapi_credhist_entry_o),  		// tp_basicsize
  0,                                       		// tp_itemsize
  (destructor) tp_dealloc,                 		// tp_dealloc
  0,                                       		// tp_print
  0,                                       		// tp_getattr
  0,                                       		// tp_setattr
  0,                                       		// tp_compare
  0,                                       		// tp_repr
  0,                                       		// tp_as_number
  0,                                       		// tp_as_sequence
  0,                                       		// tp_as_mapping
  0,                                       		// tp_hash
  0,                                       		// tp_call
  0,                                       		// tp_str
  0,                                       		// tp_getattro
  0,                                       		// tp_setattro
  0,                                       		// tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,		// tp_flags
  "credhist_entry class",                  		// tp_doc
  0,                                       		// tp_traverse
  0,                                       		// tp_clear
  0,                                       		// tp_richcompare
  0,                                       		// tp_weaklistoffset
  0,                                       		// tp_iter
  0,                                       		// tp_iternext
  tp_methods,                              		// tp_methods
  0,                                       		// tp_members
  tp_getset,                               		// tp_getset
  0,                                       		// tp_base
  0,                                       		// tp_dict
  0,                                       		// tp_descr_get
  0,                                       		// tp_descr_set
  0,                                       		// tp_dictoffset
  0,                                       		// tp_init
  0,                                       		// tp_alloc
  0,                                  			// tp_new
  0,                                       		// tp_free
  0,                                       		// tp_is_gc
  0,                                       		// tp_bases
  0,                                       		// tp_mro
  0,                                       		// tp_cache
  0,                                       		// tp_subclasses
  0,                                       		// tp_weaklist
  0,                                       		// tp_del
  0,                                       		// tp_version_tag
  0,                                       		// tp_finalize
};


