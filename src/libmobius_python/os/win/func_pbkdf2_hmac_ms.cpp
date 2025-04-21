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
// @brief  C++ API mobius.os.win module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <pymobius.hpp>
#include <mobius/os/win/pbkdf2_hmac_ms.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief pbkdf2_hmac_ms function
// @param self Function object
// @param args Argument list
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_pbkdf2_hmac_ms (PyObject *, PyObject *args)
{
  // parse input args
  std::string arg_hash_id;
  mobius::bytearray arg_key;
  mobius::bytearray arg_salt;
  std::uint32_t arg_iterations;
  std::uint16_t arg_dklen;

  try
    {
      arg_hash_id = mobius::py::get_arg_as_std_string (args, 0);
      arg_key = mobius::py::get_arg_as_bytearray (args, 1);
      arg_salt = mobius::py::get_arg_as_bytearray (args, 2);
      arg_iterations = mobius::py::get_arg_as_uint32_t (args, 3);
      arg_dklen = mobius::py::get_arg_as_uint16_t (args, 4);
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
      ret = mobius::py::pybytes_from_bytearray (
         mobius::os::win::pbkdf2_hmac_ms (
           arg_key,
           arg_salt,
           arg_iterations,
           arg_dklen,
           arg_hash_id)
           );
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}


