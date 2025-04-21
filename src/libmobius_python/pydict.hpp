#ifndef LIBMOBIUS_PYTHON_PYDICT_HPP
#define LIBMOBIUS_PYTHON_PYDICT_HPP

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
#define PY_SSIZE_T_CLEAN        // PEP 353

#include <Python.h>
#include <mobius/exception.inc>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <map>

namespace mobius::py
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new Python dict from C++ container
// @param container C++ container
// @param pyfk Function to convert C++ keys to Python objects
// @param pyfv Function to convert C++ values to Python objects
// @return Python dict
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename C, typename FK, typename FV> PyObject *
pydict_from_cpp_container (const C& container, FK pyfk, FV pyfv)
{
  PyObject *ret = PyDict_New ();

  if (!ret)
    return nullptr;

  for (const auto& p : container)
    {
      PyObject *py_key = pyfk (p.first);

      if (!py_key)
        {
          Py_CLEAR (ret);
          return nullptr;
        }

      PyObject *py_value = pyfv (p.second);

      if (!py_value)
        {
          Py_DECREF (py_key);
          Py_CLEAR (ret);
          return nullptr;
        }

      PyDict_SetItem (ret, py_key, py_value);
      Py_DECREF (py_key);
      Py_DECREF (py_value);
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new C++ map from Python dict
// @param py_dict Python dict
// @param cppfunc Function to Python objects to C++ items
// @return Vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename F1, typename F2> auto
pydict_to_cpp_container (PyObject *py_dict, F1 cppfunc1, F2 cppfunc2) -> std::map <decltype (cppfunc1 (nullptr)), decltype (cppfunc2 (nullptr))>
{
  std::map <decltype (cppfunc1 (nullptr)), decltype (cppfunc2 (nullptr))> m;

  if (!PyDict_Check (py_dict))
    throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG ("argument is not a dict"));

  PyObject *key, *value;
  Py_ssize_t pos = 0;

  while (PyDict_Next (py_dict, &pos, &key, &value))
    m[cppfunc1 (key)] = cppfunc2 (value);

  return m;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get argument and convert to C++ map, using two functions
// @param args List of arguments
// @param idx Argument number, starting in 0
// @param f1 Conversion function 1
// @param f2 Conversion function 2
// @return C++ map
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename F1, typename F2> decltype (auto)
get_arg_as_cpp_map (PyObject *args, std::uint32_t idx, F1 f1, F2 f2)
{
  return pydict_to_cpp_container (get_arg (args, idx), f1, f2);
}

} // namespace mobius::py

#endif


