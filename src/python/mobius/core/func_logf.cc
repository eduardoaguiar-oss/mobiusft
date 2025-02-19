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
#include <mobius/core/log.h>
#include <mobius/string_functions.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief <b>logf</b> function implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_logf (PyObject *, PyObject *args)
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
      mobius::core::log log (mobius::py::get_filename (), mobius::py::get_funcname ());
      auto line_number = mobius::py::get_line_number ();

      if (mobius::string::startswith (arg_text, "ERR "))
        log.error (line_number, arg_text.substr (4));

      else if (mobius::string::startswith (arg_text, "WRN "))
        log.warning (line_number, arg_text.substr (4));

      else if (mobius::string::startswith (arg_text, "INF "))
        log.info (line_number, arg_text.substr (4));

      else if (mobius::string::startswith (arg_text, "DEV "))
        log.development (line_number, arg_text.substr (4));

      else if (mobius::string::startswith (arg_text, "DBG "))
        log.debug (line_number, arg_text.substr (4));
  
      else
        log.info (line_number, arg_text);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // return None
  return mobius::py::pynone ();
}
