#ifndef LIBMOBIUS_PYTHON_PYOBJECT_HPP
#define LIBMOBIUS_PYTHON_PYOBJECT_HPP

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
#include <pymobius.hpp>
#include <mobius/core/exception.inc>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace mobius::py
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Generic PyObject
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class pyobject
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  pyobject (PyObject *, bool = false);
  pyobject () noexcept = default;
  pyobject (pyobject&&) noexcept = default;
  pyobject (const pyobject&) noexcept = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  pyobject& operator= (const pyobject&) noexcept = default;
  pyobject& operator= (pyobject&&) noexcept = default;
  explicit operator bool () const noexcept;
  operator PyObject * () const noexcept;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bool is_callable () const noexcept;
  PyObject *new_reference () const noexcept;
  std::vector <std::string> dir () const;
  pyobject get_attribute (const std::string&) const;
  std::map <std::string, pyobject> get_attributes () const;
  std::string to_std_string () const;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Call function
  // @param args Function arguments
  // @return Value returned by function
  //! \todo Get error message
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  template <typename ...Args> pyobject
  call (Args... args) const
  {
    if (!is_callable ())
      throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("object is not callable"));

    PyObject *t = PyTuple_Pack (sizeof... (args), args...);
    if (!t)
      throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("could not build arguments"));

    PyObject *rc = PyObject_CallObject (*this, t);
    Py_DECREF (t);

    if (rc == nullptr)
      throw std::runtime_error (MOBIUS_EXCEPTION_MSG (get_error_message ()));

    return pyobject (rc);
  }

private:
  // @brief Implementation class forward declaration
  class impl;

  // @brief Implementation pointer
  std::shared_ptr <impl> impl_;
};

} // namespace mobius::py

#endif


