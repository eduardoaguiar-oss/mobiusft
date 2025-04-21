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
// @brief C++ API module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <pymobius.hpp>
#include "application.hpp"
#include "pod/data.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief application: tp_new (default constructor)
// @param type type object
// @param args argument list
// @param kwds keywords dict
// @return new application object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *, PyObject *)
{
  PyObject *ret = type->tp_alloc (type, 0);

  if (ret)
    {
      try
        {
          ((core_application_o *) ret)->obj = new mobius::core::application ();
        }
      catch (const std::exception& e)
        {
          Py_DECREF (ret);
          mobius::py::set_io_error (e.what ());
          ret = nullptr;
        }
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Application: tp_dealloc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_application_o *self)
{
  delete self->obj;
  Py_TYPE (self)->tp_free ((PyObject*) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Application: name getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_name (core_application_o *self)
{
  return mobius::py::pystring_from_std_string (self->obj->get_name ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Application: version getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_version (core_application_o *self)
{
  return mobius::py::pystring_from_std_string (self->obj->get_version ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Application: title getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_title (core_application_o *self)
{
  return mobius::py::pystring_from_std_string (self->obj->get_title ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Application: copyright getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_copyright (core_application_o *self)
{
  return mobius::py::pystring_from_std_string (self->obj->get_copyright ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief application: getters and setters structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyGetSetDef tp_getset[] =
{
  {
    (char *) "name",
    (getter) tp_getter_name,
    (setter) 0,
    (char *) "application name", NULL
  },
  {
    (char *) "version",
    (getter) tp_getter_version,
    (setter) 0,
    (char *) "application version", NULL
  },
  {
    (char *) "title",
    (getter) tp_getter_title,
    (setter) 0,
    (char *) "application title", NULL
  },
  {
    (char *) "copyright",
    (getter) tp_getter_copyright,
    (setter) 0,
    (char *) "application copyright notice", NULL
  },
  {NULL, NULL, NULL, NULL, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief application: get_config_path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_config_path (core_application_o *self, PyObject *args)
{
  // Parse input args
  std::string arg_rpath;

  try
    {
      arg_rpath = mobius::py::get_arg_as_std_string (args, 0);
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
      ret = mobius::py::pystring_from_std_string (self->obj->get_config_path (arg_rpath));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_io_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief application: get_cache_path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_cache_path (core_application_o *self, PyObject *args)
{
  // Parse input args
  std::string arg_rpath;

  try
    {
      arg_rpath = mobius::py::get_arg_as_std_string (args, 0);
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
      ret = mobius::py::pystring_from_std_string (self->obj->get_cache_path (arg_rpath));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief application: get_data_path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_data_path (core_application_o *self, PyObject *args)
{
  // Parse input args
  std::string arg_rpath;

  try
    {
      arg_rpath = mobius::py::get_arg_as_std_string (args, 0);
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
      ret = mobius::py::pystring_from_std_string (self->obj->get_data_path (arg_rpath));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief application: start
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_start (core_application_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      self->obj->start ();
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief application: stop
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_stop (core_application_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      self->obj->stop ();
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief application: methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] =
{
  {
    (char *) "get_config_path",
    (PyCFunction) tp_f_get_config_path,
    METH_VARARGS,
    "get config path"
  },
  {
    (char *) "get_cache_path",
    (PyCFunction) tp_f_get_cache_path,
    METH_VARARGS,
    "get cache path"
  },
  {
    (char *) "get_data_path",
    (PyCFunction) tp_f_get_data_path,
    METH_VARARGS,
    "get data path"
  },
  {
    (char *) "start",
    (PyCFunction) tp_f_start,
    METH_VARARGS,
    "start application"
  },
  {
    (char *) "stop",
    (PyCFunction) tp_f_stop,
    METH_VARARGS,
    "stop application"
  },
  {NULL, NULL, 0, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief application: type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject core_application_t =
{
  PyVarObject_HEAD_INIT (NULL, 0)
  "mobius.core.application",                   		// tp_name
  sizeof (core_application_o),              		// tp_basicsize
  0,                                       		// tp_itemsize
  (destructor) tp_dealloc,  	                   	// tp_dealloc
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
  "application class",                     		// tp_doc
  0,                                       		// tp_traverse
  0,                                       		// tp_clear
  0,                                       		// tp_richcompare
  0,                                       		// tp_weaklistoffset
  0,                                       		// tp_iter
  0,                                       		// tp_iternext
  tp_methods,                              		// tp_methods
  0,                                       		// tp_members
  tp_getset,                	                   	// tp_getset
  0,                             	              	// tp_base
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
  0,							// tp_finalize
};


