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
#include "cipher.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_cipher_cbc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_new_cipher_cbc (PyObject *, PyObject *args)
{
  // parse arguments
  std::string arg_cipher_id;
  mobius::core::bytearray arg_key;
  mobius::core::bytearray arg_iv;

  try
    {
      arg_cipher_id = mobius::py::get_arg_as_std_string (args, 0);
      arg_key = mobius::py::get_arg_as_bytearray (args, 1);
      arg_iv = mobius::py::get_arg_as_bytearray (args, 2, mobius::core::bytearray {});
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
      auto cm =  mobius::core::crypt::new_cipher_cbc (arg_cipher_id, arg_key, arg_iv);
      ret = pymobius_crypt_cipher_to_pyobject (cm);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // create Python imagefile according to its type
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_cipher_cbc_cts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_new_cipher_cbc_cts (PyObject *, PyObject *args)
{
  // parse arguments
  std::string arg_cipher_id;
  mobius::core::bytearray arg_key;
  mobius::core::bytearray arg_iv;

  try
    {
      arg_cipher_id = mobius::py::get_arg_as_std_string (args, 0);
      arg_key = mobius::py::get_arg_as_bytearray (args, 1);
      arg_iv = mobius::py::get_arg_as_bytearray (args, 2, mobius::core::bytearray {});
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
      auto cm =  mobius::core::crypt::new_cipher_cbc_cts (arg_cipher_id, arg_key, arg_iv);
      ret = pymobius_crypt_cipher_to_pyobject (cm);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // create Python imagefile according to its type
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_cipher_ctr
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_new_cipher_ctr (PyObject *, PyObject *args)
{
  // parse arguments
  std::string arg_cipher_id;
  mobius::core::bytearray arg_key;
  mobius::core::bytearray arg_cv;

  try
    {
      arg_cipher_id = mobius::py::get_arg_as_std_string (args, 0);
      arg_key = mobius::py::get_arg_as_bytearray (args, 1);
      arg_cv = mobius::py::get_arg_as_bytearray (args, 2, mobius::core::bytearray {});
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
      auto cm =  mobius::core::crypt::new_cipher_ctr (arg_cipher_id, arg_key, arg_cv);
      ret = pymobius_crypt_cipher_to_pyobject (cm);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // create Python imagefile according to its type
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_cipher_ecb
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_new_cipher_ecb (PyObject *, PyObject *args)
{
  // parse arguments
  std::string arg_cipher_id;
  mobius::core::bytearray arg_key;

  try
    {
      arg_cipher_id = mobius::py::get_arg_as_std_string (args, 0);
      arg_key = mobius::py::get_arg_as_bytearray (args, 1);
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
      auto cm =  mobius::core::crypt::new_cipher_ecb (arg_cipher_id, arg_key);
      ret = pymobius_crypt_cipher_to_pyobject (cm);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // create Python imagefile according to its type
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_cipher_gcm
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_new_cipher_gcm (PyObject *, PyObject *args)
{
  // parse arguments
  std::string arg_cipher_id;
  mobius::core::bytearray arg_key;
  mobius::core::bytearray arg_iv;

  try
    {
      arg_cipher_id = mobius::py::get_arg_as_std_string (args, 0);
      arg_key = mobius::py::get_arg_as_bytearray (args, 1);
      arg_iv = mobius::py::get_arg_as_bytearray (args, 2, mobius::core::bytearray {});
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
      auto cm = mobius::core::crypt::new_cipher_gcm (arg_cipher_id, arg_key, arg_iv);
      ret = pymobius_crypt_cipher_to_pyobject (cm);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // create Python imagefile according to its type
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_cipher_ige
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_new_cipher_ige (PyObject *, PyObject *args)
{
  // parse arguments
  std::string arg_cipher_id;
  mobius::core::bytearray arg_key;
  mobius::core::bytearray arg_iv;

  try
    {
      arg_cipher_id = mobius::py::get_arg_as_std_string (args, 0);
      arg_key = mobius::py::get_arg_as_bytearray (args, 1);
      arg_iv = mobius::py::get_arg_as_bytearray (args, 2);
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
      auto cm =  mobius::core::crypt::new_cipher_ige (arg_cipher_id, arg_key, arg_iv);
      ret = pymobius_crypt_cipher_to_pyobject (cm);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // create Python imagefile according to its type
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_cipher_ofb
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_new_cipher_ofb (PyObject *, PyObject *args)
{
  // parse arguments
  std::string arg_cipher_id;
  mobius::core::bytearray arg_key;
  mobius::core::bytearray arg_iv;

  try
    {
      arg_cipher_id = mobius::py::get_arg_as_std_string (args, 0);
      arg_key = mobius::py::get_arg_as_bytearray (args, 1);
      arg_iv = mobius::py::get_arg_as_bytearray (args, 2, mobius::core::bytearray {});
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
      auto cm =  mobius::core::crypt::new_cipher_ofb (arg_cipher_id, arg_key, arg_iv);
      ret = pymobius_crypt_cipher_to_pyobject (cm);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // create Python imagefile according to its type
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_cipher_stream
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject*
func_new_cipher_stream (PyObject *, PyObject *args)
{
  // parse arguments
  std::string arg_cipher_id;
  mobius::core::bytearray arg_key;

  try
    {
      arg_cipher_id = mobius::py::get_arg_as_std_string (args, 0);
      arg_key = mobius::py::get_arg_as_bytearray (args, 1, mobius::core::bytearray {});
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
      auto cm =  mobius::core::crypt::new_cipher_stream (arg_cipher_id, arg_key);
      ret = pymobius_crypt_cipher_to_pyobject (cm);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // create Python imagefile according to its type
  return ret;
}

