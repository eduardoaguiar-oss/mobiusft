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
//! \file item.cc C++ API <i>mobius.model.item</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <pymobius.hpp>
#include <pycallback.hpp>
#include <pylist.hpp>
#include <pydict.hpp>
#include <pyobject.hpp>
#include <functional>
#include "item.hpp"
#include "module.hpp"
#include "ant.hpp"
#include "case.hpp"
#include "event.hpp"
#include "evidence.hpp"
#include "database/connection.hpp"
#include "database/transaction.hpp"
#include "datasource/datasource.hpp"
#include "pod/data.hpp"
#include <mobius/crypt/hash.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>uid</i> attribute getter
// @param self object
// @return <i>uid</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_uid (model_item_o *self)
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
// @brief <i>category</i> attribute getter
// @param self object
// @return <i>category</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_category (model_item_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->get_category ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>case</i> attribute getter
// @param self object
// @return <i>case</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_case (model_item_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = pymobius_model_case_to_pyobject (self->obj->get_case ());
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
  {(char *) "case", (getter) tp_getter_case, nullptr, (char *) "Case object", nullptr},
  {(char *) "category", (getter) tp_getter_category, nullptr, (char *) "Category", nullptr},
  {(char *) "uid", (getter) tp_getter_uid, nullptr, (char *) "Unique ID", nullptr},
  {nullptr, nullptr, nullptr, nullptr, nullptr}, // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_child_count</i> method implementation
// @param self object
// @param args argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_child_count (model_item_o *self, PyObject *)
{
  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_int64_t (self->obj->get_child_count ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_children</i> method implementation
// @param self object
// @param args argument list
// @return children
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_children (model_item_o *self, PyObject *)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylist_from_cpp_container (
              self->obj->get_children (),
              pymobius_model_item_to_pyobject
            );
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_parent</i> method implementation
// @param self object
// @param args argument list
// @return parent, if exists
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_parent (model_item_o *self, PyObject *)
{
  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      ret = pymobius_model_item_to_pyobject (self->obj->get_parent ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>new_child</i> method implementation
// @param self object
// @param args argument list
// @return new item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_new_child (model_item_o *self, PyObject *args)
{
  // parse input args
  std::string arg_category;
  int arg_idx;

  try
    {
      arg_category = mobius::py::get_arg_as_std_string (args, 0);
      arg_idx = mobius::py::get_arg_as_int (args, 1, -1);
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
      ret = pymobius_model_item_to_pyobject (self->obj->new_child (arg_category, arg_idx));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>remove</i> method implementation
// @param self object
// @param args argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_remove (model_item_o *self, PyObject *)
{
  PyObject *ret = nullptr;

  try
    {
      self->obj->remove ();
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>move</i> method implementation
// @param self object
// @param args argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_move (model_item_o *self, PyObject *args)
{
  // parse input args
  int arg_idx;
  mobius::model::item arg_parent;

  try
    {
      arg_idx = mobius::py::get_arg_as_int (args, 0);
      arg_parent = mobius::py::get_arg_as_cpp (args, 1, pymobius_model_item_from_pyobject);
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
      self->obj->move (arg_idx, arg_parent);
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>expand_masks</i> method implementation
// @param self object
// @param args argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_expand_masks (model_item_o *self, PyObject *)
{
  PyObject *ret = nullptr;

  try
    {
      self->obj->expand_masks ();
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_data_path</i> method implementation
// @param self object
// @param args argument list
// @return full path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_data_path (model_item_o *self, PyObject *args)
{
  // parse input args
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

  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->get_data_path (arg_rpath));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>create_data_path</i> method implementation
// @param self object
// @param args argument list
// @return full path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_create_data_path (model_item_o *self, PyObject *args)
{
  // parse input args
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

  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->create_data_path (arg_rpath));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>has_attribute</i> method implementation
// @param self object
// @param args argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_has_attribute (model_item_o *self, PyObject *args)
{
  // parse input args
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

  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->has_attribute (arg_id));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_attribute</i> method implementation
// @param self object
// @param args argument list
// @return attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_attribute (model_item_o *self, PyObject *args)
{
  // parse input args
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

  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      ret = pymobius_pod_data_to_pyobject (self->obj->get_attribute (arg_id));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_attribute</i> method implementation
// @param self object
// @param args argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_attribute (model_item_o *self, PyObject *args)
{
  // parse input args
  std::string arg_id;
  mobius::pod::data arg_value;

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

  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      self->obj->set_attribute (arg_id, arg_value);
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>remove_attribute</i> method implementation
// @param self object
// @param args argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_remove_attribute (model_item_o *self, PyObject *args)
{
  // parse input args
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

  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      self->obj->remove_attribute (arg_id);
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_attributes</i> method implementation
// @param self object
// @param args argument list
// @return map containing attributes' IDs and values
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_attributes (model_item_o *self, PyObject *)
{
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

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>has_datasource</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_has_datasource (model_item_o *self, PyObject *)
{
  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->has_datasource ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_datasource</i> method implementation
// @param self Object
// @param args Argument list
// @return Datasource object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_datasource (model_item_o *self, PyObject *)
{
  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = pymobius_datasource_datasource_to_pyobject (self->obj->get_datasource ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_datasource</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_datasource (model_item_o *self, PyObject *args)
{
  // Parse input args
  mobius::datasource::datasource arg_datasource;

  try
    {
      arg_datasource = mobius::py::get_arg_as_cpp (args, 0, pymobius_datasource_datasource_from_pyobject);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // Execute C++ function
  try
    {
      self->obj->set_datasource (arg_datasource);
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
// @brief <i>remove_datasource</i> method implementation
// @param self object
// @param args argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_remove_datasource (model_item_o *self, PyObject *)
{
  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      self->obj->remove_datasource ();
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>has_ant</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_has_ant (model_item_o *self, PyObject *args)
{
  // parse input args
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

  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->has_ant (arg_id));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_ant</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_ant (model_item_o *self, PyObject *args)
{
  // parse input args
  std::string arg_id;
  std::string arg_name;
  std::string arg_version;

  try
    {
      arg_id = mobius::py::get_arg_as_std_string (args, 0);
      arg_name = mobius::py::get_arg_as_std_string (args, 1);
      arg_version = mobius::py::get_arg_as_std_string (args, 2);
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
      self->obj->set_ant (arg_id, arg_name, arg_version);
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>reset_ant</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_reset_ant (model_item_o *self, PyObject *args)
{
  // parse input args
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

  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      self->obj->reset_ant (arg_id);
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>remove_ants</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_remove_ants (model_item_o *self, PyObject *)
{
  PyObject *ret = nullptr;

  try
    {
      self->obj->remove_ants ();
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
      return nullptr;
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_ants</i> method implementation
// @param self Object
// @param args Argument list
// @return Executed ANTs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_ants (model_item_o *self, PyObject *)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylist_from_cpp_container (
              self->obj->get_ants (),
              pymobius_model_ant_to_pyobject
            );
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>new_evidence</i> method implementation
// @param self Object
// @param args Argument list
// @return New evidence object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_new_evidence (model_item_o *self, PyObject *args)
{
  // Parse input args
  std::string arg_type;

  try
    {
      arg_type = mobius::py::get_arg_as_std_string (args, 0);
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
      ret = pymobius_model_evidence_to_pyobject (self->obj->new_evidence (arg_type));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>add_evidence</i> method implementation
// @param self Object
// @param args Argument list
// @return New evidence object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_add_evidence (model_item_o *self, PyObject *args)
{
  // Parse input args
  mobius::model::evidence arg_evidence;

  try
    {
      arg_evidence = mobius::py::get_arg_as_cpp (args, 0, pymobius_model_evidence_from_pyobject);
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
      self->obj->add_evidence (arg_evidence);
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_evidences</i> method implementation
// @param self Object
// @param args Argument list
// @return None
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_evidences (model_item_o *self, PyObject *args)
{
  // Parse input args
  std::string arg_type;

  try
    {
      arg_type = mobius::py::get_arg_as_std_string (args, 0);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylist_from_cpp_container (
               self->obj->get_evidences (arg_type),
               pymobius_model_evidence_to_pyobject
            );
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>remove_evidences</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_remove_evidences (model_item_o *self, PyObject *args)
{
  // parse input args
  std::string arg_type;

  try
    {
      arg_type = mobius::py::get_arg_as_std_string (args, 0, std::string ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  PyObject *ret = nullptr;

  try
    {
      if (arg_type.empty ())
        self->obj->remove_evidences ();

      else
        self->obj->remove_evidences (arg_type);

      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>count_evidences</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_count_evidences (model_item_o *self, PyObject *args)
{
  // parse input args
  std::string arg_type;

  try
    {
      arg_type = mobius::py::get_arg_as_std_string (args, 0, std::string ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  PyObject *ret = nullptr;

  try
    {
      if (arg_type.empty ())
        {
          ret = mobius::py::pydict_from_cpp_container (
                  self->obj->count_evidences (),
                  mobius::py::pystring_from_std_string,
                  mobius::py::pylong_from_std_int64_t
                );
        }
      else
        ret = mobius::py::pylong_from_std_int64_t (self->obj->count_evidences (arg_type));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>add_event</i> method implementation
// @param self Object
// @param args Argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_add_event (model_item_o *self, PyObject *args)
{
  // Parse input args
  std::string arg_text;

  try
    {
      arg_text = mobius::py::get_arg_as_std_string (args, 0);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // Execute C++ function
  try
    {
      self->obj->add_event (arg_text);
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
// @brief <i>get_events</i> method implementation
// @param self Object
// @param args Argument list
// @return Events
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_events (model_item_o *self, PyObject *)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylist_from_cpp_container (
               self->obj->get_events (),
               pymobius_model_event_to_pyobject
            );
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>new_connection</i> method implementation
// @param self object
// @param args argument list
// @return new transaction object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_new_connection (model_item_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = pymobius_database_connection_to_pyobject (self->obj->new_connection ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>new_transaction</i> method implementation
// @param self object
// @param args argument list
// @return new transaction object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_new_transaction (model_item_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = pymobius_database_transaction_to_pyobject (self->obj->new_transaction ());
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
  {"get_child_count", (PyCFunction) tp_f_get_child_count, METH_VARARGS, "Get number of child items"},
  {"get_children", (PyCFunction) tp_f_get_children, METH_VARARGS, "Get children items"},
  {"get_parent", (PyCFunction) tp_f_get_parent, METH_VARARGS, "Get parent item"},
  {"new_child", (PyCFunction) tp_f_new_child, METH_VARARGS, "Create new child item"},
  {"remove", (PyCFunction) tp_f_remove, METH_VARARGS, "Remove item"},
  {"move", (PyCFunction) tp_f_move, METH_VARARGS, "Move item"},
  {"expand_masks", (PyCFunction) tp_f_expand_masks, METH_VARARGS, "Expand value masks using item's attributes"},
  {"get_data_path", (PyCFunction) tp_f_get_data_path, METH_VARARGS, "Get item data path"},
  {"create_data_path", (PyCFunction) tp_f_create_data_path, METH_VARARGS, "Create item data path"},
  {"has_attribute", (PyCFunction) tp_f_has_attribute, METH_VARARGS, "Check if attribute exists"},
  {"get_attribute", (PyCFunction) tp_f_get_attribute, METH_VARARGS, "Get attribute value"},
  {"set_attribute", (PyCFunction) tp_f_set_attribute, METH_VARARGS, "Set attribute value"},
  {"remove_attribute", (PyCFunction) tp_f_remove_attribute, METH_VARARGS, "Remove attribute"},
  {"get_attributes", (PyCFunction) tp_f_get_attributes, METH_VARARGS, "Get attributes"},
  {"has_datasource", (PyCFunction) tp_f_has_datasource, METH_VARARGS, "Check if item has datasource"},
  {"get_datasource", (PyCFunction) tp_f_get_datasource, METH_VARARGS, "Get datasource"},
  {"set_datasource", (PyCFunction) tp_f_set_datasource, METH_VARARGS, "Set datasource"},
  {"remove_datasource", (PyCFunction) tp_f_remove_datasource, METH_VARARGS, "Remove datasource"},
  {"has_ant", (PyCFunction) tp_f_has_ant, METH_VARARGS, "Check if ANT has been executed"},
  {"set_ant", (PyCFunction) tp_f_set_ant, METH_VARARGS, "Set ANT"},
  {"reset_ant", (PyCFunction) tp_f_reset_ant, METH_VARARGS, "Reset ANT"},
  {"remove_ants", (PyCFunction) tp_f_remove_ants, METH_VARARGS, "Remove ANTs"},
  {"get_ants", (PyCFunction) tp_f_get_ants, METH_VARARGS, "Get ANTs"},
  {"new_evidence", (PyCFunction) tp_f_new_evidence, METH_VARARGS, "Create new evidence"},
  {"add_evidence", (PyCFunction) tp_f_add_evidence, METH_VARARGS, "Add evidence"},
  {"get_evidences", (PyCFunction) tp_f_get_evidences, METH_VARARGS, "Get evidences by type"},
  {"remove_evidences", (PyCFunction) tp_f_remove_evidences, METH_VARARGS, "Remove evidences of a given type"},
  {"count_evidences", (PyCFunction) tp_f_count_evidences, METH_VARARGS, "Count evidences by type"},
  {"new_connection", (PyCFunction) tp_f_new_connection, METH_VARARGS, "Create new connection to case database"},
  {"new_transaction", (PyCFunction) tp_f_new_transaction, METH_VARARGS, "Create new transaction for case database"},
  {"add_event", (PyCFunction) tp_f_add_event, METH_VARARGS, "Add event"},
  {"get_events", (PyCFunction) tp_f_get_events, METH_VARARGS, "Get events"},
  {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>item</i> constructor
// @param type type object
// @param args argument list
// @param kwds keywords dict
// @return new <i>item</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *, PyObject *)
{
  model_item_o *self = (model_item_o *) type->tp_alloc (type, 0);

  if (self)
    self->obj = new mobius::model::item ();

  return (PyObject *) self;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>item</i> deallocator
// @param self object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (model_item_o *self)
{
  delete self->obj;
  Py_TYPE (self)->tp_free ((PyObject*) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>item</i> getattro
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
      // search first for item.__dict__ internal values, such as tp_getset
      // and tp_methods entries
      ret = PyObject_GenericGetAttr (o, name);

      if (ret == nullptr)
        {
          mobius::py::reset_error ();

          // search item.attributes, using item.get_attribute (name)
          auto self = reinterpret_cast <model_item_o *> (o);
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
// @brief <i>item</i> setattro
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

          auto self = reinterpret_cast <model_item_o *>(o);

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
// @brief <i>item</i> tp_richcompare
// @param py_a Item Object
// @param py_b Item Object
// @param op Operation
// @return Either Py_True or Py_False
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_richcompare (PyObject *py_a, PyObject *py_b, int op)
{
  PyObject *ret = nullptr;

  if (!pymobius_model_item_check (py_a) || !pymobius_model_item_check (py_b))
    ret = mobius::py::py_false ();

  else
    {
      auto a = * (reinterpret_cast <model_item_o *> (py_a)->obj);
      auto b = * (reinterpret_cast <model_item_o *> (py_b)->obj);
      bool rc = false;

      switch (op)
        {
        case Py_EQ:
          rc = (a == b);
          break;

        case Py_NE:
          rc = (a != b);
          break;

        case Py_LT:
          rc = (a < b);
          break;

        case Py_LE:
          rc = (a <= b);
          break;

        case Py_GT:
          rc = (a > b);
          break;

        case Py_GE:
          rc = (a >= b);
          break;
        }

      ret = rc ? mobius::py::py_true () : mobius::py::py_false ();
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>item</i> tp_hash
// @param self Item Object
// @return Item hash
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static Py_hash_t
tp_hash (model_item_o *self)
{
  // As negative value indicates error, return non-negative value
  return std::abs (
    static_cast <Py_hash_t> (
       std::hash <mobius::model::item> {} (*(self->obj))
    )
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject model_item_t =
{
  PyVarObject_HEAD_INIT (NULL, 0)
  "mobius.model.item",                     		// tp_name
  sizeof (model_item_o),                   		// tp_basicsize
  0,                                       		// tp_itemsize
  (destructor) tp_dealloc,                 		// tp_dealloc
  0,                                       		// tp_print
  0,                                  			// tp_getattr
  0,                               			// tp_setattr
  0,                                       		// tp_compare
  0,                                       		// tp_repr
  0,                                       		// tp_as_number
  0,                                       		// tp_as_sequence
  0,                                       		// tp_as_mapping
  reinterpret_cast <hashfunc> (tp_hash),		// tp_hash
  0,                                       		// tp_call
  0,                                       		// tp_str
  tp_getattro,                                 		// tp_getattro
  tp_setattro,                                		// tp_setattro
  0,                                       		// tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,		// tp_flags
  "item class",                            		// tp_doc
  0,                                       		// tp_traverse
  0,                                       		// tp_clear
  tp_richcompare,                             		// tp_richcompare
  0,                                       		// tp_weaklistoffset
  0,                                       		// tp_iter
  0,                                       		// tp_iternext
  tp_methods,                              		// tp_methods
  0,                                       		// tp_members
  tp_getset,             	    			// tp_getset
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.model.item</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_model_item_type ()
{
  mobius::py::pytypeobject type (&model_item_t);
  type.create ();

  return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>item</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_model_item_check (PyObject *value)
{
  return mobius::py::isinstance (value, &model_item_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>item</i> Python object from C++ object
// @param obj C++ object
// @return New item object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_model_item_to_pyobject (const mobius::model::item& obj)
{
  return mobius::py::to_pyobject_nullable <model_item_o> (obj, &model_item_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>item</i> C++ object from Python object
// @param value Python value
// @return Item object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::model::item
pymobius_model_item_from_pyobject (PyObject *value)
{
  return mobius::py::from_pyobject <model_item_o> (value, &model_item_t);
}

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <b>attribute-modified</b> event callback
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class attribute_modified_callback
{
public:
  attribute_modified_callback (PyObject *f)
    : f_ (f)
  {
  }

  void
  operator () (
    const mobius::model::item& item,
    const std::string& id,
    const mobius::pod::data& old_value,
    const mobius::pod::data& new_value)
  {
    f_.call (
      pymobius_model_item_to_pyobject (item),
      mobius::py::pystring_from_std_string (id),
      pymobius_pod_data_to_pyobject (old_value),
      pymobius_pod_data_to_pyobject (new_value)
    );
  }

private:
  mobius::py::pyobject f_;
};

mobius::py::callback <attribute_modified_callback> cb_1_ ("attribute-modified");

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <b>attribute-removed</b> event callback
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class attribute_removed_callback
{
public:
  attribute_removed_callback (PyObject *f)
    : f_ (f)
  {
  }

  void
  operator () (
    const mobius::model::item& item,
    const std::string& id,
    const mobius::pod::data& old_value)
  {
    f_.call (
      pymobius_model_item_to_pyobject (item),
      mobius::py::pystring_from_std_string (id),
      pymobius_pod_data_to_pyobject (old_value)
    );
  }

private:
  mobius::py::pyobject f_;
};

mobius::py::callback <attribute_removed_callback> cb_2_ ("attribute-removed");

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <b>datasource-modified</b> event callback
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class datasource_modified_callback
{
public:
  datasource_modified_callback (PyObject *f)
    : f_ (f)
  {
  }

  void
  operator () (
    const mobius::model::item& item,
    const mobius::datasource::datasource& datasource)
  {
    f_.call (
      pymobius_model_item_to_pyobject (item),
      pymobius_datasource_datasource_to_pyobject (datasource)
    );
  }

private:
  mobius::py::pyobject f_;
};

mobius::py::callback <datasource_modified_callback> cb_3_ ("datasource-modified");

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <b>datasource-removed</b> event callback
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class datasource_removed_callback
{
public:
  datasource_removed_callback (PyObject *f)
    : f_ (f)
  {
  }

  void
  operator () (
    const mobius::model::item& item)
  {
    f_.call (
      pymobius_model_item_to_pyobject (item)
    );
  }

private:
  mobius::py::pyobject f_;
};

mobius::py::callback <datasource_removed_callback> cb_4_ ("datasource-removed");

} // namespace


