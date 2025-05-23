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
#include <Python.h>
#include "module.hpp"
#include <pymobius.hpp>
#include "pymodule.hpp"
#include <datetime.h>
#include <mobius/core/log.hpp>
#include "api_dataholder.hpp"
#include "core/module.hpp"
#include "framework/module.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module definition structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyModuleDef module_def =
{
  PyModuleDef_HEAD_INIT,
  "mobius",
  "Mobius Forensic Toolkit API wrapper",
  -1,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr
};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module initialisation function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyMODINIT_FUNC
PyInit_mobius (void)
{
  PyDateTime_IMPORT;
  PyObject *ret = nullptr;

  try
    {
      // Initialize module
      mobius::py::pymodule module (&module_def);

      // Add types
      module.add_type ("dataholder", &api_dataholder_t);

      // Build submodules
      module.add_submodule ("core", new_core_module ());
      module.add_submodule ("framework", new_framework_module ());

      // Create new reference to module object
      ret = module.new_reference ();
    }
  catch (const std::exception& e)
    {
      mobius::core::log log (__FILE__, __FUNCTION__);
      log.error (__LINE__, e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// These three PyDateTime functions must be located here, because
// PyDateTime_IMPORT creates a static variable, and as so, with local scope
// to this file only. These functions are declared into "pymobius.h"
// header file and can be used throughout the api_xxx.cc source files.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
namespace mobius::py
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if object is PyDateTime
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pydatetime_check (PyObject *obj)
{
  return PyDateTime_Check (obj);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create PyDateTime from mobius::core::datetime::datetime
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pydatetime_from_datetime (const mobius::core::datetime::datetime& dt)
{
  PyObject *ret = nullptr;

  if (dt)
    {
      auto d = dt.get_date ();
      auto t = dt.get_time ();

      ret = PyDateTime_FromDateAndTime (
              d.get_year (),
              d.get_month (),
              d.get_day (),
              t.get_hour (),
              t.get_minute (),
              t.get_second (), 0);
    }

  else
    ret = mobius::py::pynone ();

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create mobius::core::datetime::datetime from PyDateTime
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
pydatetime_as_datetime (PyObject *obj)
{
  mobius::core::datetime::datetime dt;

  if (!mobius::py::pynone_check (obj))
    dt = mobius::core::datetime::datetime (
           PyDateTime_GET_YEAR (obj),
           PyDateTime_GET_MONTH (obj),
           PyDateTime_GET_DAY (obj),
           PyDateTime_DATE_GET_HOUR (obj),
           PyDateTime_DATE_GET_MINUTE (obj),
           PyDateTime_DATE_GET_SECOND (obj));

  return dt;
}

} // namespace mobius::py


