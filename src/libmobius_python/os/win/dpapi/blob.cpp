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
//! \file blob.cc C++ API <i>mobius.os.win.dpapi.blob</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <pymobius.hpp>
#include <pygil.hpp>
#include "blob.hpp"
#include "module.hpp"
#include "io/reader.hpp"
#include <mobius/io/bytearray_io.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if object is an instance of <i>mobius.os.win.dpapi.blob</i>
// @param pyobj Python object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_os_win_dpapi_blob_check (PyObject *pyobj)
{
  return PyObject_IsInstance (pyobj, (PyObject *) &os_win_dpapi_blob_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>blob</i> Python object from C++ object
// @param obj C++ object
// @return new blob object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_os_win_dpapi_blob_to_pyobject (mobius::os::win::dpapi::blob obj)
{
  PyObject *ret = _PyObject_New (&os_win_dpapi_blob_t);

  if (ret)
    ((os_win_dpapi_blob_o *) ret)->obj = new mobius::os::win::dpapi::blob (obj);

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>blob</i> C++ object from Python object
// @param value Python object
// @return blob object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::os::win::dpapi::blob
pymobius_os_win_dpapi_blob_from_pyobject (PyObject *value)
{
  if (!pymobius_os_win_dpapi_blob_check (value))
    throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG ("object must be an instance of mobius.os.win.dpapi.blob"));

  return * (reinterpret_cast <os_win_dpapi_blob_o *>(value)->obj);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>revision</i> Attribute getter
// @param self Object
// @return <i>revision</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_revision (os_win_dpapi_blob_o *self)
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
// @brief <i>provider_guid</i> Attribute getter
// @param self Object
// @return <i>provider_guid</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_provider_guid (os_win_dpapi_blob_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->get_provider_guid ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>master_key_revision</i> Attribute getter
// @param self Object
// @return <i>master_key_revision</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_master_key_revision (os_win_dpapi_blob_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_uint32_t (self->obj->get_master_key_revision ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>master_key_guid</i> Attribute getter
// @param self Object
// @return <i>master_key_guid</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_master_key_guid (os_win_dpapi_blob_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->get_master_key_guid ());
    }
  catch (const std::exception& e)
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
tp_getter_flags (os_win_dpapi_blob_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_uint32_t (self->obj->get_flags ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>description</i> Attribute getter
// @param self Object
// @return <i>description</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_description (os_win_dpapi_blob_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->get_description ());
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
tp_getter_cipher_id (os_win_dpapi_blob_o *self)
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
// @brief <i>key_length</i> Attribute getter
// @param self Object
// @return <i>key_length</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_key_length (os_win_dpapi_blob_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_uint32_t (self->obj->get_key_length ());
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
tp_getter_salt (os_win_dpapi_blob_o *self)
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
// @brief <i>hmac_key</i> Attribute getter
// @param self Object
// @return <i>hmac_key</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_hmac_key (os_win_dpapi_blob_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybytes_from_bytearray (self->obj->get_hmac_key ());
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
tp_getter_hash_id (os_win_dpapi_blob_o *self)
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
// @brief <i>hash_length</i> Attribute getter
// @param self Object
// @return <i>hash_length</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_hash_length (os_win_dpapi_blob_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_uint32_t (self->obj->get_hash_length ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>hmac_value</i> Attribute getter
// @param self Object
// @return <i>hmac_value</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_hmac_value (os_win_dpapi_blob_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybytes_from_bytearray (self->obj->get_hmac_value ());
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
tp_getter_cipher_text (os_win_dpapi_blob_o *self)
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
// @brief <i>plain_text</i> Attribute getter
// @param self Object
// @return <i>plain_text</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_plain_text (os_win_dpapi_blob_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybytes_from_bytearray (self->obj->get_plain_text ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>signature</i> Attribute getter
// @param self Object
// @return <i>signature</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_signature (os_win_dpapi_blob_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybytes_from_bytearray (self->obj->get_signature ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>signature_data</i> Attribute getter
// @param self Object
// @return <i>signature</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_signature_data (os_win_dpapi_blob_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybytes_from_bytearray (self->obj->get_signature_data ());
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
    NULL,
    (char *) "Struct revision",
    NULL
  },
  {
    (char *) "provider_guid",
    (getter) tp_getter_provider_guid,
    NULL,
    (char *) "Provider GUID",
    NULL
  },
  {
    (char *) "master_key_revision",
    (getter) tp_getter_master_key_revision,
    NULL,
    (char *) "Master key revision",
    NULL
  },
  {
    (char *) "master_key_guid",
    (getter) tp_getter_master_key_guid,
    NULL,
    (char *) "Master key GUID",
    NULL
  },
  {
    (char *) "flags",
    (getter) tp_getter_flags,
    NULL,
    (char *) "Flags",
    NULL
  },
  {
    (char *) "description",
    (getter) tp_getter_description,
    NULL,
    (char *) "Description",
    NULL
  },
  {
    (char *) "cipher_id",
    (getter) tp_getter_cipher_id,
    NULL,
    (char *) "Cipher algorithm ID",
    NULL
  },
  {
    (char *) "key_length",
    (getter) tp_getter_key_length,
    NULL,
    (char *) "Key length in bytes",
    NULL
  },
  {
    (char *) "salt",
    (getter) tp_getter_salt,
    NULL,
    (char *) "Salt",
    NULL
  },
  {
    (char *) "hmac_key",
    (getter) tp_getter_hmac_key,
    NULL,
    (char *) "HMAC key value",
    NULL
  },
  {
    (char *) "hash_id",
    (getter) tp_getter_hash_id,
    NULL,
    (char *) "Hash algorithm ID",
    NULL
  },
  {
    (char *) "hash_length",
    (getter) tp_getter_hash_length,
    NULL,
    (char *) "Hash length in bytes",
    NULL
  },
  {
    (char *) "hmac_value",
    (getter) tp_getter_hmac_value,
    NULL,
    (char *) "HMAC value",
    NULL
  },
  {
    (char *) "cipher_text",
    (getter) tp_getter_cipher_text,
    NULL,
    (char *) "Cipher text",
    NULL
  },
  {
    (char *) "plain_text",
    (getter) tp_getter_plain_text,
    NULL,
    (char *) "Plain text",
    NULL
  },
  {
    (char *) "signature",
    (getter) tp_getter_signature,
    NULL,
    (char *) "Signature",
    NULL
  },
  {
    (char *) "signature_data",
    (getter) tp_getter_signature_data,
    NULL,
    (char *) "Signature data",
    NULL
  },
  {NULL, NULL, NULL, NULL, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>test_key</i> method implementation
// @param self Object
// @param args Argument list
// @return None
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_test_key (os_win_dpapi_blob_o *self, PyObject *args)
{
  // parse input args
  mobius::bytearray arg_key;
  mobius::bytearray arg_entropy;

  try
    {
      arg_key = mobius::py::get_arg_as_bytearray (args, 0);
      arg_entropy = mobius::py::get_arg_as_bytearray (args, 1, mobius::bytearray ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      auto rc = mobius::py::GIL () (
                   self->obj->test_key (arg_key, arg_entropy)
                );
      ret = mobius::py::pybool_from_bool (rc);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>decrypt</i> method implementation
// @param self Object
// @param args Argument list
// @return None
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_decrypt (os_win_dpapi_blob_o *self, PyObject *args)
{
  // parse input args
  mobius::bytearray arg_key;
  mobius::bytearray arg_entropy;

  try
    {
      arg_key = mobius::py::get_arg_as_bytearray (args, 0);
      arg_entropy = mobius::py::get_arg_as_bytearray (args, 1, mobius::bytearray ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      auto rc = mobius::py::GIL () (
                   self->obj->decrypt (arg_key, arg_entropy)
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
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_decrypted (os_win_dpapi_blob_o *self, PyObject *)
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
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] =
{
  {
    (char *) "test_key",
    (PyCFunction) tp_f_test_key,
    METH_VARARGS,
    "Test if key is the right one"
  },
  {
    (char *) "decrypt",
    (PyCFunction) tp_f_decrypt,
    METH_VARARGS,
    "Decrypt BLOB using key"
  },
  {
    (char *) "is_decrypted",
    (PyCFunction) tp_f_is_decrypted,
    METH_VARARGS,
    "Check if BLOB is decrypted"
  },
  {NULL, NULL, 0, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>blob</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>blob</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *args, PyObject *)
{
  // parse input args
  mobius::io::reader arg_reader;

  try
    {
      arg_reader = mobius::py::get_arg_as_cpp (args, 0, pymobius_io_reader_from_pyobject);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // create object
  os_win_dpapi_blob_o *ret = reinterpret_cast <os_win_dpapi_blob_o *> (type->tp_alloc (type, 0));

  if (ret)
    {
      try
        {
          ret->obj = new mobius::os::win::dpapi::blob (arg_reader);
        }
      catch (const std::exception& e)
        {
          mobius::py::set_runtime_error (e.what ());
          Py_DECREF (ret);
          ret = nullptr;
        }
    }

  return reinterpret_cast <PyObject *> (ret);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>blob</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (os_win_dpapi_blob_o *self)
{
  delete self->obj;
  Py_TYPE (self)->tp_free ((PyObject*) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject os_win_dpapi_blob_t =
{
  PyVarObject_HEAD_INIT (NULL, 0)
  "mobius.os.win.dpapi.blob",              		// tp_name
  sizeof (os_win_dpapi_blob_o),            		// tp_basicsize
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
  "blob class",                            		// tp_doc
  0,                                       		// tp_traverse
  0,                                       		// tp_clear
  0,                                       		// tp_richcompare
  0,                                       		// tp_weaklistoffset
  0,                                       		// tp_iter
  0,                                       		// tp_iternext
  tp_methods,                                  		// tp_methods
  0,                                       		// tp_members
  tp_getset,                               		// tp_getset
  0,                                       		// tp_base
  0,                                       		// tp_dict
  0,                                       		// tp_descr_get
  0,                                       		// tp_descr_set
  0,                                       		// tp_dictoffset
  0,                                       		// tp_init
  0,                                       		// tp_alloc
  tp_new,                                  		// tp_new
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


