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
// @brief  C++ API module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <pymobius.hpp>
#include "api_dataholder.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief tp_dir implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_dir (api_dataholder_o *self, PyObject *)
{
  return PyDict_Keys (self->dict);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] =
{
  {
    (char *) "__dir__",
    (PyCFunction) tp_dir,
    METH_VARARGS,
    "dir function implementation"
  },
  {NULL, NULL, 0, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief api.dataholder: tp_dealloc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
api_dataholder_tp_dealloc (api_dataholder_o *self)
{
  Py_DECREF (self->dict);
  Py_TYPE (self)->tp_free ((PyObject*) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief api.dataholder: type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject api_dataholder_t =
{
  PyVarObject_HEAD_INIT (NULL, 0)
  "api.dataholder",                      		// tp_name
  sizeof (api_dataholder_o),             		// tp_basicsize
  0,                                          		// tp_itemsize
  (destructor) api_dataholder_tp_dealloc,		// tp_dealloc
  0,                                          		// tp_print
  0,                                          		// tp_getattr
  0,                                          		// tp_setattr
  0,                                          		// tp_compare
  0,                                          		// tp_repr
  0,                                          		// tp_as_number
  0,                                          		// tp_as_sequence
  0,                                          		// tp_as_mapping
  0,                                          		// tp_hash
  0,                                          		// tp_call
  0,                                          		// tp_str
  PyObject_GenericGetAttr,                    		// tp_getattro
  PyObject_GenericSetAttr,                     		// tp_setattro
  0,                                          		// tp_as_buffer
  Py_TPFLAGS_DEFAULT,   		                // tp_flags
  "generic dataholder class",                  		// tp_doc
  0,                                          		// tp_traverse
  0,                                          		// tp_clear
  0,                                          		// tp_richcompare
  0,                                          		// tp_weaklistoffset
  0,                                          		// tp_iter
  0,                                          		// tp_iternext
  tp_methods,                                  		// tp_methods
  0,                                          		// tp_members
  0,                                         		// tp_getset
  0,                                          		// tp_base
  0,                                          		// tp_dict
  0,                                          		// tp_descr_get
  0,
  offsetof (api_dataholder_o, dict),           		// tp_dictoffset
  0,                                          		// tp_init
  0,                                    		// tp_alloc
  PyType_GenericNew,                      		// tp_new
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief api.dataholder: create new object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
api_dataholder_o *
api_dataholder_new ()
{
  api_dataholder_o *self = (api_dataholder_o *) _PyObject_New (&api_dataholder_t);

  if (self)
    self->dict = PyDict_New ();

  return self;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set attribute
// @param obj object
// @param name name
// @param value value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
api_dataholder_setattr (api_dataholder_o *obj, const std::string& name, const std::string& value)
{
  PyObject_GenericSetAttr ((PyObject *) obj,
                           mobius::py::pystring_from_std_string (name),
                           mobius::py::pystring_from_std_string (value));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set attribute
// @param obj object
// @param name name
// @param value value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
api_dataholder_setattr (api_dataholder_o *obj, const std::string& name, std::int64_t value)
{
  PyObject_GenericSetAttr ((PyObject *) obj,
                           mobius::py::pystring_from_std_string (name),
                           mobius::py::pylong_from_std_int64_t (value));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set attribute
// @param obj object
// @param name name
// @param value value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
api_dataholder_setattr (api_dataholder_o *obj, const std::string& name, const mobius::datetime::datetime& value)
{
  PyObject_GenericSetAttr ((PyObject *) obj,
                           mobius::py::pystring_from_std_string (name),
                           mobius::py::pydatetime_from_datetime (value));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set attribute
// @param obj object
// @param name name
// @param value value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
api_dataholder_setattr (api_dataholder_o *obj, const std::string& name, PyObject *value)
{
  PyObject_GenericSetAttr ((PyObject *) obj,
                           mobius::py::pystring_from_std_string (name),
                           value);
}


