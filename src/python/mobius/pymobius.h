#ifndef MOBIUS_PYTHON_PYMOBIUS_H
#define MOBIUS_PYTHON_PYMOBIUS_H

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
#include <mobius/bytearray.h>
#include <mobius/datetime/datetime.h>
#include <mobius/exception.inc>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <type_traits>

namespace mobius::py
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Parse function arguments
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t get_arg_size (PyObject *) noexcept;
PyObject *get_arg (PyObject *, std::uint32_t);
std::string get_arg_as_std_string (PyObject *, std::uint32_t);
mobius::bytearray get_arg_as_bytearray (PyObject *, std::uint32_t);
mobius::datetime::datetime get_arg_as_datetime (PyObject *, std::uint32_t);
bool get_arg_as_bool (PyObject *, std::uint32_t);
char get_arg_as_char (PyObject *, std::uint32_t);
int get_arg_as_int (PyObject *, std::uint32_t);
std::int32_t get_arg_as_int32_t (PyObject *, std::uint32_t);
std::int64_t get_arg_as_int64_t (PyObject *, std::uint32_t);
std::uint8_t get_arg_as_uint8_t (PyObject *, std::uint32_t);
std::uint16_t get_arg_as_uint16_t (PyObject *, std::uint32_t);
std::uint32_t get_arg_as_uint32_t (PyObject *, std::uint32_t);
std::uint64_t get_arg_as_uint64_t (PyObject *, std::uint32_t);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Parse function arguments (with default value)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string get_arg_as_std_string (PyObject *, std::uint32_t, const std::string&);
mobius::bytearray get_arg_as_bytearray (PyObject *, std::uint32_t, const mobius::bytearray&);
mobius::datetime::datetime get_arg_as_datetime (PyObject *, std::uint32_t, const mobius::datetime::datetime&);
bool get_arg_as_bool (PyObject *, std::uint32_t, bool);
char get_arg_as_char (PyObject *, std::uint32_t, char);
int get_arg_as_int (PyObject *, std::uint32_t, int);
std::int32_t get_arg_as_int32_t (PyObject *, std::uint32_t, std::int32_t);
std::int64_t get_arg_as_int64_t (PyObject *, std::uint32_t, std::int64_t);
std::uint8_t get_arg_as_uint8_t (PyObject *, std::uint32_t, std::uint8_t);
std::uint16_t get_arg_as_uint16_t (PyObject *, std::uint32_t, std::uint16_t);
std::uint32_t get_arg_as_uint32_t (PyObject *, std::uint32_t, std::uint32_t);
std::uint64_t get_arg_as_uint64_t (PyObject *, std::uint32_t, std::uint64_t);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Check argument type
//! \param args Function arguments
//! \param idx Argument number, starting in 0
//! \param f Check function
//! \return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename F> bool
check_arg (PyObject *args, std::uint32_t idx, F f)
{
  if (idx < get_arg_size (args))
    return f (get_arg (args, idx));

  throw std::out_of_range (MOBIUS_EXCEPTION_MSG ("argument index out of range"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get function argument and convert using a function
//! \param args Function arguments
//! \param idx Argument number, starting in 0
//! \param f Conversion function
//! \return C++ object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename F, typename D> decltype (auto)
get_arg_as_cpp (PyObject *args, std::uint32_t idx, F f, D d)
{
  if (idx < get_arg_size (args))
    return f (get_arg (args, idx));
  return d;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get function argument and convert using a function
//! \param args Function arguments
//! \param idx Argument number, starting in 0
//! \param f Conversion function
//! \return C++ object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename F> decltype (auto)
get_arg_as_cpp (PyObject *args, std::uint32_t idx, F f)
{
  return f (get_arg (args, idx));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Error handling
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void reset_error ();
void set_attribute_error (const std::string&);
void set_invalid_type_error (const std::string&);
void set_value_error (const std::string&);
void set_io_error (const std::string&);
void set_runtime_error (const std::string&);
void set_stop_iteration ();
std::string get_error_message ();

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Python reflection
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int get_line_number ();
std::string get_filename ();
std::string get_funcname ();

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Singleton types
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *pynone ();
PyObject *py_false ();
PyObject *py_true ();
PyObject *py_not_implemented ();

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// C++ <-> Python concrete types conversion
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool pynone_check (PyObject *);

bool pystring_check (PyObject *);
PyObject *pystring_from_std_string (const std::string&);
std::string pystring_as_std_string (PyObject *);

bool pybytes_check (PyObject *);
PyObject *pybytes_from_bytearray (const mobius::bytearray&);
PyObject *pybytes_from_char (char);
mobius::bytearray pybytes_as_bytearray (PyObject *);

bool pydatetime_check (PyObject *);
PyObject *pydatetime_from_datetime (const mobius::datetime::datetime&);
mobius::datetime::datetime pydatetime_as_datetime (PyObject *);

bool pybool_check (PyObject *);
PyObject *pybool_from_bool (bool);
bool pybool_as_bool (PyObject *);

bool pylong_check (PyObject *);
PyObject *pylong_from_int (int);
int pylong_as_int (PyObject *);

PyObject *pylong_from_std_int8_t (std::int8_t);
PyObject *pylong_from_std_int16_t (std::int16_t);
PyObject *pylong_from_std_int32_t (std::int32_t);
PyObject *pylong_from_std_int64_t (std::int64_t);
std::int8_t pylong_as_std_int8_t (PyObject *);
std::int16_t pylong_as_std_int16_t (PyObject *);
std::int32_t pylong_as_std_int32_t (PyObject *);
std::int64_t pylong_as_std_int64_t (PyObject *);

PyObject *pylong_from_std_uint8_t (std::uint8_t);
PyObject *pylong_from_std_uint16_t (std::uint16_t);
PyObject *pylong_from_std_uint32_t (std::uint32_t);
PyObject *pylong_from_std_uint64_t (std::uint64_t);
std::uint8_t pylong_as_std_uint8_t (PyObject *);
std::uint16_t pylong_as_std_uint16_t (PyObject *);
std::uint32_t pylong_as_std_uint32_t (PyObject *);
std::uint64_t pylong_as_std_uint64_t (PyObject *);

bool pyfloat_check (PyObject *);
double pyfloat_as_cpp (PyObject *);
PyObject *pyfloat_from_cpp (double);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// C++ <-> Python type conversion
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T>
bool
isinstance (PyObject *value, T *type)
{
  return PyObject_IsInstance (value, reinterpret_cast<PyObject *> (type));
}

template <typename O, typename T>
auto
from_pyobject (PyObject *value, T *type)
{
  if (isinstance (value, type))
    return * (reinterpret_cast <O *>(value)->obj);

  throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG ("object must be an instance of " + std::string (type->tp_name)));
}

template <typename O, typename Tcpp, typename T>
PyObject *
to_pyobject (const Tcpp& obj, T *type)
{
  PyObject *ret = _PyObject_New (type);

  if (ret)
    ((O *) ret)->obj = new Tcpp (obj);

  return ret;
}

template <typename O, typename Tcpp, typename T>
PyObject *
to_pyobject_nullable (const Tcpp& obj, T *type)
{
  PyObject *ret = nullptr;

  if (obj)
    {
      ret = _PyObject_New (type);

      if (ret)
        ((O *) ret)->obj = new Tcpp (obj);
    }

  else
    ret = mobius::py::pynone ();

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Check setter value
//! \param value Value object
//! \param attr_name Attribute name
//! \param pycheck Function to check value object
//! \return -1 if error, 0 if OK
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename F> void
check_setter_value (PyObject *value, const char* attr_name, F pycheck)
{
  // If it is a delete operation, then return with error
  if (value == nullptr)
    {
      std::string msg = "cannot delete '" + std::string (attr_name) + "' attribute";
      throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG (msg.c_str ()));
    }

  // Check argument type
  if (!pycheck (value))
    {
      std::string msg = "invalid type for '" + std::string (attr_name) + "' attribute";
      throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG (msg.c_str ()));
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Create new Python Tuple from C++ pair
//! \param p C++ pair
//! \param pyf1 Function to convert first value
//! \param pyf2 Function to convert second value
//! \return Python Tuple
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T, typename F1, typename F2> PyObject *
pytuple_from_cpp_pair (const T& p, F1 pyf1, F2 pyf2)
{
  PyObject *ret = PyTuple_New (2);

  if (ret)
    {
      PyTuple_SetItem (ret, 0, pyf1 (p.first));
      PyTuple_SetItem (ret, 1, pyf2 (p.second));
    }

  return ret;
}

} // namespace mobius::py

#endif
