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
#include <pymobius.h>
#include <pygil.h>
#include "datasource.h"
#include "vfs/vfs.h"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_datasource_from_vfs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_datasource_new_datasource_from_vfs (PyObject *, PyObject *args)
{
  // parse arguments
  mobius::vfs::vfs arg_vfs;

  try
    {
      arg_vfs = mobius::py::get_arg_as_cpp (args, 0, pymobius_vfs_vfs_from_pyobject);
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
      auto datasource = mobius::datasource::new_datasource_from_vfs (arg_vfs);
      ret = pymobius_datasource_datasource_to_pyobject (datasource);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_io_error (e.what ());
    }

  // create Python imagefile according to its type
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_datasource_by_imagefile_path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_datasource_new_datasource_by_imagefile_path (PyObject *, PyObject *args)
{
  // parse arguments
  std::string arg_path;

  try
    {
      arg_path = mobius::py::get_arg_as_std_string (args, 0);
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
      auto datasource = mobius::datasource::new_datasource_by_imagefile_path (arg_path);
      ret = pymobius_datasource_datasource_to_pyobject (datasource);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_io_error (e.what ());
    }

  // create Python imagefile according to its type
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_datasource_by_imagefile_url
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_datasource_new_datasource_by_imagefile_url (PyObject *, PyObject *args)
{
  // parse arguments
  std::string arg_url;

  try
    {
      arg_url = mobius::py::get_arg_as_std_string (args, 0);
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
      auto datasource = mobius::datasource::new_datasource_by_imagefile_url (arg_url);
      ret = pymobius_datasource_datasource_to_pyobject (datasource);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_io_error (e.what ());
    }

  // create Python imagefile according to its type
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_datasource_by_ufdr_path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_datasource_new_datasource_by_ufdr_path (PyObject *, PyObject *args)
{
  // parse arguments
  std::string arg_path;

  try
    {
      arg_path = mobius::py::get_arg_as_std_string (args, 0);
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
      auto datasource = mobius::datasource::new_datasource_by_ufdr_path (arg_path);
      ret = pymobius_datasource_datasource_to_pyobject (datasource);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_io_error (e.what ());
    }

  // create Python imagefile according to its type
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_datasource_by_ufdr_url
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_datasource_new_datasource_by_ufdr_url (PyObject *, PyObject *args)
{
  // parse arguments
  std::string arg_url;

  try
    {
      arg_url = mobius::py::get_arg_as_std_string (args, 0);
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
      auto datasource = mobius::datasource::new_datasource_by_ufdr_url (arg_url);
      ret = pymobius_datasource_datasource_to_pyobject (datasource);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_io_error (e.what ());
    }

  // create Python imagefile according to its type
  return ret;
}

