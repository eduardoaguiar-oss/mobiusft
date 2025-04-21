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
#include "resource.hpp"
#include <pymobius.hpp>
#include <pylist.hpp>
#include <pyobject.hpp>
#include <mobius/core/resource.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <b>add_resource</b> function implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_add_resource (PyObject *, PyObject *args)
{
  // Parse input args
  std::string arg_id;
  std::string arg_description;
  mobius::py::pyobject arg_value;

  try
    {
      arg_id = mobius::py::get_arg_as_std_string (args, 0);
      arg_description = mobius::py::get_arg_as_std_string (args, 1);
      arg_value = mobius::py::pyobject (mobius::py::get_arg (args, 2), true);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // Execute C++ function
  try
    {
      mobius::core::add_resource (arg_id, arg_description, arg_value);
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
// @brief <b>remove_resource</b> function implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_remove_resource (PyObject *, PyObject *args)
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
      mobius::core::remove_resource (arg_id);
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
// @brief <b>has_resource</b> function implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_has_resource (PyObject *, PyObject *args)
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
      ret = mobius::py::pybool_from_bool (mobius::core::has_resource (arg_id));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
      return nullptr;
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <b>get_resource</b> function implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_get_resource (PyObject *, PyObject *args)
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
      ret = pymobius_core_resource_to_pyobject (mobius::core::get_resource (arg_id));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
      return nullptr;
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <b>get_resources</b> function implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_get_resources (PyObject *, PyObject *args)
{
  // Parse input args
  std::string arg_group_id;

  try
    {
      arg_group_id = mobius::py::get_arg_as_std_string (args, 0);
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
      ret = mobius::py::pylist_from_cpp_container (
               mobius::core::get_resources (arg_group_id),
               pymobius_core_resource_to_pyobject
            );
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
      return nullptr;
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <b>get_resource_value</b> function implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_get_resource_value (PyObject *, PyObject *args)
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
      ret = mobius::core::get_resource_value <mobius::py::pyobject> (arg_id).new_reference ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
      return nullptr;
    }

  return ret;
}



