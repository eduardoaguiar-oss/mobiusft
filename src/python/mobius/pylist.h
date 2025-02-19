#ifndef MOBIUS_PYTHON_PYLIST_H
#define MOBIUS_PYTHON_PYLIST_H

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

#include "pymobius.h"
#include <mobius/exception.inc>
#include <stdexcept>

namespace mobius::py
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Create new Python list from C++ container
//! \param container C++ container
//! \param pyfunc Function to convert C++ items to Python objects
//! \return Python list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename C, typename F> PyObject *
pylist_from_cpp_container (const C& container, F pyfunc)
{
  PyObject *ret = PyList_New (0);

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

      PyList_Append (ret, py_item);
      Py_DECREF (py_item);
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Create new C++ vector from Python list
//! \param container C++ container
//! \param cppfunc Function to Python objects to C++ items
//! \return Vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename F> auto
pylist_to_cpp_container (PyObject *list, F cppfunc) -> std::vector <decltype (cppfunc (nullptr))>
{
  std::vector <decltype (cppfunc (nullptr))> v;

  if (!PyList_Check (list))
    throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG ("object is not a list"));

  Py_ssize_t size = PyList_Size (list);
  if (size == -1)
    throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG (get_error_message ()));

  for (Py_ssize_t i = 0; i < size; i++)
    {
      PyObject *py_item = PyList_GetItem (list, i);

      if (!py_item)
        throw std::runtime_error (mobius::MOBIUS_EXCEPTION_MSG (get_error_message ()));

      v.push_back (cppfunc (py_item));
    }

  return v;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Create new Python list from C++ pair container
//! \param container C++ container
//! \param pyf1 Function to convert first value
//! \param pyf2 Function to convert second value
//! \return Python list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename C, typename F1, typename F2> PyObject *
pylist_from_cpp_pair_container (const C& container, F1 pyf1, F2 pyf2)
{
  PyObject *ret = PyList_New (0);

  if (!ret)
    return nullptr;

  for (const auto& p : container)
    {
      PyObject *py_item = PyTuple_New (2);

      if (!py_item)
        {
          Py_CLEAR (ret);
          return nullptr;
        }

      PyTuple_SetItem (py_item, 0, pyf1 (p.first));
      PyTuple_SetItem (py_item, 1, pyf2 (p.second));

      PyList_Append (ret, py_item);
      Py_DECREF (py_item);
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get argument and convert to C++ vector, using a function
//! \param args List of arguments
//! \param idx Argument number, starting in 0
//! \param f Conversion function
//! \return C++ vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename F> decltype (auto)
get_arg_as_cpp_vector (PyObject *args, std::uint32_t idx, F f)
{
  return pylist_to_cpp_container (get_arg (args, idx), f);
}

} // namespace mobius::py

#endif
