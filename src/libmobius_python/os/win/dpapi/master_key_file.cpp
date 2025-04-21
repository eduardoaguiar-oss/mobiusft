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
//! \file master_key_file.cc C++ API <i>mobius.os.win.dpapi.master_key_file</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <pymobius.hpp>
#include "master_key_file.hpp"
#include "master_key.hpp"
#include "module.hpp"
#include "io/reader.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if object type is <i>master_key_file</i>
// @param pyobj Python object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_os_win_dpapi_master_key_file_check (PyObject *pyobj)
{
  return PyObject_IsInstance (pyobj, (PyObject *) &os_win_dpapi_master_key_file_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>master_key_file</i> Python object from C++ object
// @param obj C++ object
// @return new master_key_file object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_os_win_dpapi_master_key_file_to_pyobject (mobius::os::win::dpapi::master_key_file obj)
{
  PyObject *ret = _PyObject_New (&os_win_dpapi_master_key_file_t);

  if (ret)
    ((os_win_dpapi_master_key_file_o *) ret)->obj = new mobius::os::win::dpapi::master_key_file (obj);

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>master_key_file</i> C++ object from Python object
// @param pyobj Python object
// @return master_key_file object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::os::win::dpapi::master_key_file
pymobius_os_win_dpapi_master_key_file_from_pyobject (PyObject *pyobj)
{
  return * (reinterpret_cast <os_win_dpapi_master_key_file_o *>(pyobj)->obj);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>revision</i> Attribute getter
// @param self Object
// @return <i>revision</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_revision (os_win_dpapi_master_key_file_o *self)
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
tp_getter_guid (os_win_dpapi_master_key_file_o *self)
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
// @brief <i>flags</i> Attribute getter
// @param self Object
// @return <i>flags</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_flags (os_win_dpapi_master_key_file_o *self)
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
// @brief <i>master_key</i> Attribute getter
// @param self Object
// @return <i>master_key</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_master_key (os_win_dpapi_master_key_file_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = pymobius_os_win_dpapi_master_key_to_pyobject (self->obj->get_master_key ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>backup_key</i> Attribute getter
// @param self Object
// @return <i>backup_key</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_backup_key (os_win_dpapi_master_key_file_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = pymobius_os_win_dpapi_master_key_to_pyobject (self->obj->get_backup_key ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>credhist_guid</i> Attribute getter
// @param self Object
// @return <i>credhist_guid</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_credhist_guid (os_win_dpapi_master_key_file_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->get_credhist_guid ());
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
    (char *) "flags",
    (getter) tp_getter_flags,
    (setter) 0,
    (char *) "Master key file flags",
    NULL
  },
  {
    (char *) "master_key",
    (getter) tp_getter_master_key,
    (setter) 0,
    (char *) "Master key",
    NULL
  },
  {
    (char *) "backup_key",
    (getter) tp_getter_backup_key,
    (setter) 0,
    (char *) "Backup key",
    NULL
  },
  {
    (char *) "credhist_guid",
    (getter) tp_getter_credhist_guid,
    (setter) 0,
    (char *) "CREDHIST GUID",
    NULL
  },
  {NULL, NULL, NULL, NULL, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>master_key_file</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>master_key_file</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *args, PyObject *)
{
  // Parse input args
  mobius::io::reader arg_reader;

  try
    {
      arg_reader = pymobius_io_reader_from_pyobject (mobius::py::get_arg (args, 0));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // Create Python object
  os_win_dpapi_master_key_file_o *ret = reinterpret_cast <os_win_dpapi_master_key_file_o *> (type->tp_alloc (type, 0));

  if (ret)
    {
      try
        {
          ret->obj = new mobius::os::win::dpapi::master_key_file (arg_reader);
        }
      catch (const std::exception& e)
        {
          Py_DECREF (ret);
          mobius::py::set_runtime_error (e.what ());
          ret = nullptr;
        }
    }

  return reinterpret_cast <PyObject *> (ret);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>master_key_file</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (os_win_dpapi_master_key_file_o *self)
{
  delete self->obj;
  Py_TYPE (self)->tp_free ((PyObject*) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject os_win_dpapi_master_key_file_t =
{
  PyVarObject_HEAD_INIT (NULL, 0)
  "mobius.os.win.dpapi.master_key_file",   		// tp_name
  sizeof (os_win_dpapi_master_key_file_o), 		// tp_basicsize
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
  "master_key_file class",                 		// tp_doc
  0,                                       		// tp_traverse
  0,                                       		// tp_clear
  0,                                       		// tp_richcompare
  0,                                       		// tp_weaklistoffset
  0,                                       		// tp_iter
  0,                                       		// tp_iternext
  0,                                       		// tp_methods
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


