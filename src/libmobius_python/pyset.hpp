#ifndef LIBMOBIUS_PYTHON_PYSET_HPP
#define LIBMOBIUS_PYTHON_PYSET_HPP

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
#include <mobius/core/exception.inc>
#include <set>
#include <stdexcept>

namespace mobius::py
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new Python set from C++ set
// @param container C++ set
// @param pyfunc Function to convert C++ items to Python objects
// @return Python set object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename C, typename F> PyObject *
pyset_from_cpp_container (const C& container, F pyfunc)
{
  PyObject *ret = PySet_New (NULL);

  if (!ret)
    return nullptr;

  for (const auto& item : container)
    {
      PyObject *py_item = pyfunc (item);

      if (!py_item)
        {
          Py_CLEAR (ret);
          return nullptr;
        }

      if (PySet_Add (ret, py_item) == -1)
        {
          Py_CLEAR (ret);
          return nullptr;
        }

      Py_DECREF (py_item);
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new C++ set from Python set
// @param py_set Python set
// @param cppfunc Function convert from Python objects to C++ items
// @return set
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename F> auto
pyset_to_cpp_container (PyObject *py_set, F cppfunc) -> std::set <decltype (cppfunc (nullptr))>
{
  std::set <decltype (cppfunc (nullptr))> s;

  if (!PyAnySet_Check (py_set))
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("object is not a set"));

  PyObject *py_iter = PyObject_GetIter (py_set);
  if (!py_iter)
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("could not get iterator for set object"));

  PyObject *py_item;

  while ((py_item = PyIter_Next (py_iter)))
    {
      s.insert (cppfunc (py_item));
      Py_XDECREF (py_item);
    }

  Py_XDECREF (py_iter);

  return s;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get argument and convert to C++ vector, using a function
// @param args List of arguments
// @param idx Argument number, starting in 0
// @param f Conversion function
// @return C++ vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename F> decltype (auto)
get_arg_as_cpp_set (PyObject *args, std::uint32_t idx, F f)
{
  return pyset_to_cpp_container (get_arg (args, idx), f);
}

} // namespace mobius::py

#endif


