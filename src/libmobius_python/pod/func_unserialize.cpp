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
// @brief  C++ API mobius.pod module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <pymobius.hpp>
#include <pygil.hpp>
#include <io/reader.hpp>
#include "data.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <b>mobius.pod.unserialize</b> function
// @param self function object
// @param args argument list
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_pod_unserialize (PyObject *, PyObject *args)
{
  // parse arguments
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

  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      ret = pymobius_pod_data_to_pyobject (
               mobius::py::GIL () (
                  mobius::core::pod::unserialize (arg_reader)
               )
            );
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}


