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
//! \file data.cc C++ API <i>mobius.pod.data</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <pymobius.hpp>
#include <pylist.hpp>
#include <pyobject.hpp>
#include "data.hpp"
#include "map.hpp"
#include <mobius/core/exception.inc>
#include <stdexcept>
#include <vector>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create std::vector <data> from PyTuple
// @param py_value Python object
// @return C++ vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::vector <mobius::core::pod::data>
pymobius_pod_data_vector_from_pytuple (PyObject *py_value)
{
  std::vector <mobius::core::pod::data> v;

  Py_ssize_t siz = PyTuple_Size (py_value);

  for (Py_ssize_t i = 0;i < siz;i++)
    {
      PyObject *item = PyTuple_GetItem (py_value, i);

      if (!item)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG (mobius::py::get_error_message ()));

      v.push_back (pymobius_pod_data_from_pyobject (item));
    }

  return v;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create std::vector <data> from PySet
// @param py_value Python object
// @return C++ vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::vector <mobius::core::pod::data>
pymobius_pod_data_vector_from_pyset (PyObject *py_value)
{
  std::vector <mobius::core::pod::data> v;

  mobius::py::pyobject iter = PyObject_GetIter (py_value);
  if (!iter)
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG (mobius::py::get_error_message ()));

  mobius::py::pyobject item = PyIter_Next (iter);

  while (item)
    {
      v.push_back (pymobius_pod_data_from_pyobject (item));
      item = PyIter_Next (iter);
    }

  return v;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if object type is an instance of <i>mobius.pod.data</i>
// @param pyobj Python object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_pod_data_check (PyObject *pyobj)
{
  return PyObject_IsInstance (pyobj, (PyObject *) &pod_data_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create mobius.pod.data value from Python object
// @param py_value Python object
// @return mobius.pod.data value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
pymobius_pod_data_from_pyobject (PyObject *py_value)
{
  mobius::core::pod::data data;

  if (pymobius_pod_data_check (py_value))
    data = * (reinterpret_cast <pod_data_o *>(py_value)->obj);

  else if (mobius::py::pynone_check (py_value))
    ;

  else if (mobius::py::pybool_check (py_value))
    data = mobius::core::pod::data (py_value == Py_True);

  else if (mobius::py::pylong_check (py_value))
    data = mobius::core::pod::data (mobius::py::pylong_as_std_int64_t (py_value));

  else if (mobius::py::pyfloat_check (py_value))
    data = mobius::core::pod::data (PyFloat_AS_DOUBLE ( (py_value)));

  else if (mobius::py::pydatetime_check (py_value))
    data = mobius::core::pod::data (mobius::py::pydatetime_as_datetime (py_value));

  else if (mobius::py::pybytes_check (py_value))
    data = mobius::core::pod::data (mobius::py::pybytes_as_bytearray (py_value));

  else if (mobius::py::pystring_check (py_value))
    data = mobius::core::pod::data (mobius::py::pystring_as_std_string (py_value));

  else if (PyList_Check (py_value))
    data = mobius::py::pylist_to_cpp_container (py_value, pymobius_pod_data_from_pyobject);

  else if (PyTuple_Check (py_value))
    data = pymobius_pod_data_vector_from_pytuple (py_value);

  else if (PySet_Check (py_value))
    data = pymobius_pod_data_vector_from_pyset (py_value);

  else if (PyDict_Check (py_value))
    data = pymobius_pod_map_from_pyobject (py_value);

  else
    data = pymobius_pod_map_from_pyobject (py_value);

  return data;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create Python object from mobius.pod.data value
// @param value mobius.pod.data value
// @return New Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_pod_data_to_pyobject (const mobius::core::pod::data& value)
{
  PyObject *ret = nullptr;

  if (value.is_null ())
    ret = mobius::py::pynone ();

  else if (value.is_bool ())
    ret = mobius::py::pybool_from_bool (bool (value));

  else if (value.is_integer ())
    ret = mobius::py::pylong_from_std_int64_t (std::int64_t (value));

  else if (value.is_float ())
    ret = mobius::py::pyfloat_from_cpp (static_cast <long double> (value));

  else if (value.is_datetime ())
    ret = mobius::py::pydatetime_from_datetime (mobius::core::datetime::datetime (value));

  else if (value.is_string ())
    ret = mobius::py::pystring_from_std_string (std::string (value));

  else if (value.is_bytearray ())
    ret = mobius::py::pybytes_from_bytearray (mobius::core::bytearray (value));

  else if (value.is_list ())
    ret = mobius::py::pylist_from_cpp_container (
             std::vector <mobius::core::pod::data> (value),
             pymobius_pod_data_to_pyobject
          );

  else if (value.is_map ())
    ret = pymobius_pod_map_to_pyobject (mobius::core::pod::map (value));

  else
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("unknown mobius.pod.data type"));

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create pure Python object from mobius.pod.data value
// @param value mobius.pod.data value
// @return New Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_pod_data_to_python (const mobius::core::pod::data& value)
{
  PyObject *ret = nullptr;

  if (value.is_list ())
    ret = mobius::py::pylist_from_cpp_container (
            std::vector <mobius::core::pod::data> (value),
            pymobius_pod_data_to_python
          );

  else if (value.is_map ())
    ret = pymobius_pod_map_to_python (mobius::core::pod::map (value));

  else
    ret = pymobius_pod_data_to_pyobject (value);

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>type</i> Attribute getter
// @param self Object
// @return <i>type</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_type (pod_data_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_int (static_cast <int> (self->obj->get_type ()));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>value</i> Attribute getter
// @param self Object
// @return <i>value</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_value (pod_data_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = pymobius_pod_data_to_pyobject (*(self->obj));
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
    (char *) "type",
    (getter) tp_getter_type,
    (setter) 0,
    (char *) "Data type",
    NULL
  },
  {
    (char *) "value",
    (getter) tp_getter_value,
    (setter) 0,
    (char *) "Value",
    NULL
  },
  {NULL, NULL, NULL, NULL, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>clone</i> method implementation
// @param self Object
// @param args Argument list
// @return New data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_clone (pod_data_o *self, PyObject *)
{
  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = pymobius_pod_data_to_pyobject (self->obj->clone ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_null</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_null (pod_data_o *self, PyObject *)
{
  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->is_null ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_bool</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_bool (pod_data_o *self, PyObject *)
{
  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->is_bool ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_integer</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_integer (pod_data_o *self, PyObject *)
{
  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->is_integer ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_float</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_float (pod_data_o *self, PyObject *)
{
  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->is_float ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_datetime</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_datetime (pod_data_o *self, PyObject *)
{
  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->is_datetime ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_string</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_string (pod_data_o *self, PyObject *)
{
  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->is_string ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_bytearray</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_bytearray (pod_data_o *self, PyObject *)
{
  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->is_bytearray ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_list</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_list (pod_data_o *self, PyObject *)
{
  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->is_list ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_map</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_map (pod_data_o *self, PyObject *)
{
  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->is_map ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] =
{
  {
    (char *) "clone",
    (PyCFunction) tp_f_clone,
    METH_VARARGS,
    "Clone data object"
  },
  {
    (char *) "is_null",
    (PyCFunction) tp_f_is_null,
    METH_VARARGS,
    "Check if data is null"
  },
  {
    (char *) "is_bool",
    (PyCFunction) tp_f_is_bool,
    METH_VARARGS,
    "Check if data is boolean"
  },
  {
    (char *) "is_integer",
    (PyCFunction) tp_f_is_integer,
    METH_VARARGS,
    "Check if data is integer"
  },
  {
    (char *) "is_float",
    (PyCFunction) tp_f_is_float,
    METH_VARARGS,
    "Check if data is float"
  },
  {
    (char *) "is_datetime",
    (PyCFunction) tp_f_is_datetime,
    METH_VARARGS,
    "Check if data is datetime"
  },
  {
    (char *) "is_string",
    (PyCFunction) tp_f_is_string,
    METH_VARARGS,
    "Check if data is string"
  },
  {
    (char *) "is_bytearray",
    (PyCFunction) tp_f_is_bytearray,
    METH_VARARGS,
    "Check if data is bytearray"
  },
  {
    (char *) "is_list",
    (PyCFunction) tp_f_is_list,
    METH_VARARGS,
    "Check if data is list"
  },
  {
    (char *) "is_map",
    (PyCFunction) tp_f_is_map,
    METH_VARARGS,
    "Check if data is map"
  },
  {NULL, NULL, 0, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>data</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (pod_data_o *self)
{
  delete self->obj;
  Py_TYPE (self)->tp_free ((PyObject*) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject pod_data_t =
{
  PyVarObject_HEAD_INIT (NULL, 0)
  "mobius.pod.data",                       		// tp_name
  sizeof (pod_data_o),                     		// tp_basicsize
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
  "data class",                            		// tp_doc
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


