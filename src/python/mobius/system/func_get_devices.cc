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
//! \brief  C++ API module wrapper
//! \author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <pymobius.h>
#include <pylist.h>
#include "device.h"
#include <mobius/system/device_list.h>
#include <algorithm>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief function mobius.system.get_devices
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_get_devices (PyObject *, PyObject *args)
{
  // Parse input args
  std::string arg_subsystem;
  std::string arg_type;

  try
    {
      arg_subsystem = mobius::py::get_arg_as_std_string (args, 0, std::string ());
      arg_type = mobius::py::get_arg_as_std_string (args, 1, std::string ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // create list
  PyObject *ret = nullptr;

  // populate list
  try
    {
      mobius::system::device_list dev_list;

      std::vector <mobius::system::device> selected_dev_list;
      std::copy_if (
              dev_list.begin (),
              dev_list.end (),
              std::back_inserter (selected_dev_list),
              [arg_subsystem, arg_type] (const auto& dev) {
                    return (arg_subsystem.empty () || arg_subsystem == dev.get_subsystem ()) && (arg_type.empty () || arg_type == dev.get_type ()); }
      );

      ret = mobius::py::pylist_from_cpp_container (
               selected_dev_list,
               pymobius_system_device_to_pyobject
            );
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}
