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
//! \file evidence.cc C++ API <i>mobius.model.evidence</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <pymobius.hpp>
#include <pydict.hpp>
#include <pyset.hpp>
#include "evidence.hpp"
#include "case.hpp"
#include "item.hpp"
#include "pod/data.hpp"
#include <mobius/exception.inc>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>evidence</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_model_evidence_check (PyObject *value)
{
  return mobius::py::isinstance (value, &model_evidence_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>evidence</i> Python object from C++ object
// @param obj C++ object
// @return New evidence object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_model_evidence_to_pyobject (const mobius::framework::model::evidence& obj)
{
  return mobius::py::to_pyobject_nullable <model_evidence_o> (obj, &model_evidence_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>evidence</i> C++ object from Python object
// @param value Python value
// @return Evidence object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::framework::model::evidence
pymobius_model_evidence_from_pyobject (PyObject *value)
{
  return mobius::py::from_pyobject <model_evidence_o> (value, &model_evidence_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>item</i> Attribute getter
// @param self Object
// @return <i>item</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_item (model_evidence_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = pymobius_model_item_to_pyobject (self->obj->get_item ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>uid</i> Attribute getter
// @param self Object
// @return <i>uid</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_uid (model_evidence_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_int64_t (self->obj->get_uid ());
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
tp_getter_type (model_evidence_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->get_type ());
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
    (char *) "item",
    (getter) tp_getter_item,
    (setter) 0,
    (char *) "Case item",
    NULL
  },
  {
    (char *) "uid",
    (getter) tp_getter_uid,
    (setter) 0,
    (char *) "UID",
    NULL
  },
  {
    (char *) "type",
    (getter) tp_getter_type,
    (setter) 0,
    (char *) "Evidence type",
    NULL
  },
  {NULL, NULL, NULL, NULL, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>has_attribute</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_has_attribute (model_evidence_o *self, PyObject *args)
{
  // Parse input args
  std::string arg_id;

  try
    {
      arg_id = mobius::py::get_arg_as_std_string (args, 0);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->has_attribute (arg_id));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_attribute</i> method implementation
// @param self Object
// @param args Argument list
// @return Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_attribute (model_evidence_o *self, PyObject *args)
{
  // Parse input args
  std::string arg_id;

  try
    {
      arg_id = mobius::py::get_arg_as_std_string (args, 0);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = pymobius_pod_data_to_pyobject (self->obj->get_attribute (arg_id));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_attribute</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_attribute (model_evidence_o *self, PyObject *args)
{
  // Parse input args
  std::string arg_id;
  mobius::core::pod::data arg_value;

  try
    {
      arg_id = mobius::py::get_arg_as_std_string (args, 0);
      arg_value = mobius::py::get_arg_as_cpp (args, 1, pymobius_pod_data_from_pyobject);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // Execute C++ function
  try
    {
      self->obj->set_attribute (arg_id, arg_value);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
      return nullptr;
    }

  // return None
  return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_attributes</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_attributes (model_evidence_o *self, PyObject *args)
{
  // Parse input args
  std::map <std::string, mobius::core::pod::data> arg_attributes;

  try
    {
      arg_attributes = mobius::py::get_arg_as_cpp_map (
                          args,
                          0,
                          mobius::py::pystring_as_std_string,
                          pymobius_pod_data_from_pyobject
      );
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // Execute C++ function
  try
    {
      self->obj->set_attributes (arg_attributes);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
      return nullptr;
    }

  // return None
  return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>remove_attribute</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_remove_attribute (model_evidence_o *self, PyObject *args)
{
  // Parse input args
  std::string arg_id;

  try
    {
      arg_id = mobius::py::get_arg_as_std_string (args, 0);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // Execute C++ function
  try
    {
      self->obj->remove_attribute (arg_id);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
      return nullptr;
    }

  // return None
  return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_attribute</i> method implementation
// @param self Object
// @param args Argument list
// @return Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_attributes (model_evidence_o *self, PyObject *)
{
  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pydict_from_cpp_container (
              self->obj->get_attributes (),
              mobius::py::pystring_from_std_string,
              pymobius_pod_data_to_pyobject
            );
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>has_tag</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_has_tag (model_evidence_o *self, PyObject *args)
{
  // Parse input args
  std::string arg_name;

  try
    {
      arg_name = mobius::py::get_arg_as_std_string (args, 0);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->has_tag (arg_name));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_tag</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_tag (model_evidence_o *self, PyObject *args)
{
  // Parse input args
  std::string arg_name;

  try
    {
      arg_name = mobius::py::get_arg_as_std_string (args, 0);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // Execute C++ function
  try
    {
      self->obj->set_tag (arg_name);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
      return nullptr;
    }

  // return None
  return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_tag</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_tags (model_evidence_o *self, PyObject *args)
{
  // Parse input args
  std::set <std::string> arg_tags;

  try
    {
      arg_tags = mobius::py::get_arg_as_cpp_set (args, 0, mobius::py::pystring_as_std_string);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // Execute C++ function
  try
    {
      self->obj->set_tags (arg_tags);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
      return nullptr;
    }

  // return None
  return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>reset_tag</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_reset_tag (model_evidence_o *self, PyObject *args)
{
  // Parse input args
  std::string arg_name;

  try
    {
      arg_name = mobius::py::get_arg_as_std_string (args, 0);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // Execute C++ function
  try
    {
      self->obj->reset_tag (arg_name);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
      return nullptr;
    }

  // return None
  return mobius::py::pynone ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_tags</i> method implementation
// @param self Object
// @param args Argument list
// @return Set of evidence tags
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_tags (model_evidence_o *self, PyObject *)
{
  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pyset_from_cpp_container (
              self->obj->get_tags (),
              mobius::py::pystring_from_std_string
      );
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
    (char *) "has_attribute",
    (PyCFunction) tp_f_has_attribute,
    METH_VARARGS,
    "Check if attribute exists"
  },
  {
    (char *) "get_attribute",
    (PyCFunction) tp_f_get_attribute,
    METH_VARARGS,
    "Get attribute value"
  },
  {
    (char *) "set_attribute",
    (PyCFunction) tp_f_set_attribute,
    METH_VARARGS,
    "Set attribute value"
  },
  {
    (char *) "set_attributes",
    (PyCFunction) tp_f_set_attributes,
    METH_VARARGS,
    "Set attributes"
  },
  {
    (char *) "remove_attribute",
    (PyCFunction) tp_f_remove_attribute,
    METH_VARARGS,
    "Remove attribute"
  },
  {
    (char *) "get_attributes",
    (PyCFunction) tp_f_get_attributes,
    METH_VARARGS,
    "Get attributes as dict"
  },
  {
    (char *) "has_tag",
    (PyCFunction) tp_f_has_tag,
    METH_VARARGS,
    "Check if evidence has a given tag"
  },
  {
    (char *) "set_tag",
    (PyCFunction) tp_f_set_tag,
    METH_VARARGS,
    "Set tag"
  },
  {
    (char *) "set_tags",
    (PyCFunction) tp_f_set_tags,
    METH_VARARGS,
    "Set tags"
  },
  {
    (char *) "reset_tag",
    (PyCFunction) tp_f_reset_tag,
    METH_VARARGS,
    "Reset tag"
  },
  {
    (char *) "get_tags",
    (PyCFunction) tp_f_get_tags,
    METH_VARARGS,
    "Get tags"
  },
  {NULL, NULL, 0, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>evidence</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (model_evidence_o *self)
{
  delete self->obj;
  Py_TYPE (self)->tp_free ((PyObject*) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>evidence</i> getattro
// @param o Object
// @param name Attribute name
// @return Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getattro (PyObject *o, PyObject *name)
{
  PyObject *ret = nullptr;

  try
    {
      // search first for evidence.__dict__ internal values, such as tp_getset
      // and tp_methods entries
      ret = PyObject_GenericGetAttr (o, name);

      if (ret == nullptr)
        {
          mobius::py::reset_error ();

          // search item.attributes, using evidence.get_attribute (name)
          auto self = reinterpret_cast <model_evidence_o *> (o);
          auto s_name = mobius::py::pystring_as_std_string (name);

          if (self->obj->has_attribute (s_name))
            ret = pymobius_pod_data_to_pyobject (self->obj->get_attribute (s_name));

          else
            ret = mobius::py::pynone ();
        }
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>evidence</i> setattro
// @param o Object
// @param name Attribute name
// @param value Attribute value
// @return 0 if success, -1 if error
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static int
tp_setattro (PyObject *o, PyObject *name, PyObject *value)
{
  try
    {
      auto s_name = mobius::py::pystring_as_std_string (name);
      PyObject *attr = PyObject_GenericGetAttr (o, name);

      // internal attributes are read only
      if (attr != nullptr)
        {
          Py_DECREF (attr);

          if (value == nullptr)
            mobius::py::set_invalid_type_error ("cannot delete attribute '" + s_name + "'");
          else
            mobius::py::set_invalid_type_error ("cannot set attribute '" + s_name + "'");

          return -1;
        }

      // set attribute
      else
        {
          mobius::py::reset_error ();

          auto self = reinterpret_cast <model_evidence_o *>(o);

          if (value == nullptr)
            self->obj->remove_attribute (s_name);

          else
            {
              auto s_value = pymobius_pod_data_from_pyobject (value);
              self->obj->set_attribute (s_name, s_value);
            }
        }
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
      return -1;
    }

  return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject model_evidence_t =
{
  PyVarObject_HEAD_INIT (NULL, 0)
  "mobius.model.evidence",                 		// tp_name
  sizeof (model_evidence_o),               		// tp_basicsize
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
  tp_getattro,                             		// tp_getattro
  tp_setattro,                             		// tp_setattro
  0,                                       		// tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,		// tp_flags
  "Evidence object",                       		// tp_doc
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
  0,                                  		    // tp_new
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


