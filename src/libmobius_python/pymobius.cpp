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

#include <pymobius.hpp>
#include <frameobject.h>
#include <bytesobject.h>
#include <mobius/exception.inc>
#include <stdexcept>

namespace mobius::py
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument list size
// @param args Argument tuple
// @return Number of items
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
get_arg_size (PyObject *args) noexcept
{
  return PyTuple_Size (args);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument
// @param args Argument tuple
// @param idx Argument index
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
get_arg (PyObject *args, std::uint32_t idx)
{
  if (idx < PyTuple_Size (args))
    return PyTuple_GetItem (args, idx);

  throw std::out_of_range ("argument index out of range");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as string
// @param args Argument tuple
// @param idx Argument index
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_arg_as_std_string (PyObject *args, std::uint32_t idx)
{
  return pystring_as_std_string (get_arg (args, idx));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as string (with default value)
// @param args Argument tuple
// @param idx Argument index
// @param value Default value
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_arg_as_std_string (PyObject *args, std::uint32_t idx, const std::string& value)
{
  if (idx < get_arg_size (args))
    return get_arg_as_std_string (args, idx);

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as bytearray
// @param args Argument tuple
// @param idx Argument index
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
get_arg_as_bytearray (PyObject *args, std::uint32_t idx)
{
  return pybytes_as_bytearray (get_arg (args, idx));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as bytearray (with default value)
// @param args Argument tuple
// @param idx Argument index
// @param value Default value
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
get_arg_as_bytearray (PyObject *args, std::uint32_t idx, const mobius::bytearray& value)
{
  if (idx < get_arg_size (args))
    return get_arg_as_bytearray (args, idx);

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as datetime
// @param args Argument tuple
// @param idx Argument index
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
get_arg_as_datetime (PyObject *args, std::uint32_t idx)
{
  PyObject *arg = get_arg (args, idx);

  if (!pydatetime_check (arg))
    throw std::invalid_argument ("function arg is not datetime");

  return pydatetime_as_datetime (arg);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as datetime (with default value)
// @param args Argument tuple
// @param idx Argument index
// @param value Default value
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
get_arg_as_datetime (
  PyObject *args,
  std::uint32_t idx,
  const mobius::core::datetime::datetime& value)
{
  if (idx < get_arg_size (args))
    return get_arg_as_datetime (args, idx);

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as bool
// @param args Argument tuple
// @param idx Argument index
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
get_arg_as_bool (PyObject *args, std::uint32_t idx)
{
  return pybool_as_bool (get_arg (args, idx));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as bool (with default value)
// @param args Argument tuple
// @param idx Argument index
// @param value Default value
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
get_arg_as_bool (PyObject *args, std::uint32_t idx, bool value)
{
  if (idx < get_arg_size (args))
    return pybool_as_bool (get_arg (args, idx));

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as char
// @param args Argument tuple
// @param idx Argument index
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
char
get_arg_as_char (PyObject *args, std::uint32_t idx)
{
  Py_ssize_t siz = 0;
  const char *buffer = PyUnicode_AsUTF8AndSize (get_arg (args, idx), &siz);

  if (buffer)
    {
      if (siz != 1)
        throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG ("invalid char"));

      return buffer[0];
    }

  else
    throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG ("invalid Python string"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as char
// @param args Argument tuple
// @param idx Argument index
// @param c Default value
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
char
get_arg_as_char (PyObject *args, std::uint32_t idx, char c)
{
  if (idx < get_arg_size (args))
    return get_arg_as_char (args, idx);

  return c;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as int
// @param args Argument tuple
// @param idx Argument index
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
get_arg_as_int (PyObject *args, std::uint32_t idx)
{
  return PyLong_AsLong (get_arg (args, idx));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as int (with default value)
// @param args Argument tuple
// @param idx Argument index
// @param value Default value
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
get_arg_as_int (PyObject *args, std::uint32_t idx, int value)
{
  if (idx < get_arg_size (args))
    return PyLong_AsLong (get_arg (args, idx));

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as std::int32_t
// @param args Argument tuple
// @param idx Argument index
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int32_t
get_arg_as_int32_t (PyObject *args, std::uint32_t idx)
{
  return pylong_as_std_int32_t (get_arg (args, idx));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as std::int32_t (with default value)
// @param args Argument tuple
// @param idx Argument index
// @param value Default value
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int32_t
get_arg_as_int32_t (PyObject *args, std::uint32_t idx, std::int32_t value)
{
  if (idx < get_arg_size (args))
    return pylong_as_std_int32_t (get_arg (args, idx));

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as std::int64_t
// @param args Argument tuple
// @param idx Argument index
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int64_t
get_arg_as_int64_t (PyObject *args, std::uint32_t idx)
{
  return pylong_as_std_int64_t (get_arg (args, idx));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as std::int64_t (with default value)
// @param args Argument tuple
// @param idx Argument index
// @param value Default value
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int64_t
get_arg_as_int64_t (PyObject *args, std::uint32_t idx, std::int64_t value)
{
  if (idx < get_arg_size (args))
    return pylong_as_std_int64_t (get_arg (args, idx));

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as std::uint8_t
// @param args Argument tuple
// @param idx Argument index
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint8_t
get_arg_as_uint8_t (PyObject *args, std::uint32_t idx)
{
  return pylong_as_std_uint8_t (get_arg (args, idx));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as std::uint8_t (with default value)
// @param args Argument tuple
// @param idx Argument index
// @param value Default value
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint8_t
get_arg_as_uint8_t (PyObject *args, std::uint32_t idx, std::uint8_t value)
{
  if (idx < get_arg_size (args))
    return get_arg_as_uint8_t (args, idx);

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as std::uint16_t
// @param args Argument tuple
// @param idx Argument index
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint16_t
get_arg_as_uint16_t (PyObject *args, std::uint32_t idx)
{
  return pylong_as_std_uint16_t (get_arg (args, idx));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as std::uint16_t (with default value)
// @param args Argument tuple
// @param idx Argument index
// @param value Default value
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint16_t
get_arg_as_uint16_t (PyObject *args, std::uint32_t idx, std::uint16_t value)
{
  if (idx < get_arg_size (args))
    return get_arg_as_uint16_t (args, idx);

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as std::uint32_t
// @param args Argument tuple
// @param idx Argument index
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
get_arg_as_uint32_t (PyObject *args, std::uint32_t idx)
{
  return pylong_as_std_uint32_t (get_arg (args, idx));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as std::uint32_t (with default value)
// @param args Argument tuple
// @param idx Argument index
// @param value Default value
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
get_arg_as_uint32_t (PyObject *args, std::uint32_t idx, std::uint32_t value)
{
  if (idx < get_arg_size (args))
    return get_arg_as_uint32_t (args, idx);

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as std::uint64_t
// @param args Argument tuple
// @param idx Argument index
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
get_arg_as_uint64_t (PyObject *args, std::uint32_t idx)
{
  return pylong_as_std_uint64_t (get_arg (args, idx));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get function argument as std::uint64_t (with default value)
// @param args Argument tuple
// @param idx Argument index
// @param value Default value
// @return Argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
get_arg_as_uint64_t (PyObject *args, std::uint32_t idx, std::uint64_t value)
{
  if (idx < get_arg_size (args))
    return get_arg_as_uint64_t (args, idx);

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset error
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
reset_error ()
{
  PyErr_Clear ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set AttributeError
// @param msg Error message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
set_attribute_error (const std::string& msg)
{
  PyErr_SetString (PyExc_AttributeError, msg.c_str ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set TypeError
// @param msg Error message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
set_invalid_type_error (const std::string& msg)
{
  PyErr_SetString (PyExc_TypeError, msg.c_str ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set ValueError
// @param msg Error message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
set_value_error (const std::string& msg)
{
  PyErr_SetString (PyExc_ValueError, msg.c_str ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set IOError
// @param msg Error message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
set_io_error (const std::string& msg)
{
  PyErr_SetString (PyExc_IOError, msg.c_str ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set RuntimeError
// @param msg Error message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
set_runtime_error (const std::string& msg)
{
  PyErr_SetString (PyExc_RuntimeError, msg.c_str ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set StopIteration
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
set_stop_iteration ()
{
  PyErr_SetNone (PyExc_StopIteration);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get error message
// @return Error message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_error_message ()
{
  PyObject *exc_type, *exc_value, *exc_traceback;
  PyErr_Fetch (&exc_type, &exc_value, &exc_traceback);
  PyErr_NormalizeException (&exc_type, &exc_value, &exc_traceback);

  std::string msg = mobius::py::pystring_as_std_string (PyObject_Str (exc_type))
    + ' ' + mobius::py::pystring_as_std_string (PyObject_Str (exc_value));

  auto p_traceback = reinterpret_cast <PyTracebackObject *> (exc_traceback);

  while (p_traceback)
    {
      msg += "\nFile: \"" + mobius::py::pystring_as_std_string (PyObject_Str (p_traceback->tb_frame->f_code->co_filename)) + "\", line " + std::to_string (p_traceback->tb_lineno);

      p_traceback = p_traceback->tb_next;
    }

  Py_XDECREF (exc_type);
  Py_XDECREF (exc_value);
  Py_XDECREF (exc_traceback);

  return msg;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get current line number
// @return Line number
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
get_line_number ()
{
  int line = -1;

  PyFrameObject *frame = PyEval_GetFrame ();

  if (frame)
    line = PyFrame_GetLineNumber (frame);

  return line;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get current file name
// @return File name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_filename ()
{
  std::string filename;

  PyFrameObject *frame = PyEval_GetFrame ();
  if (frame && frame->f_code && frame->f_code->co_filename)
    filename = mobius::py::pystring_as_std_string (frame->f_code->co_filename);

  return filename;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get current function name
// @return Function name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_funcname ()
{
  std::string funcname;

  PyFrameObject *frame = PyEval_GetFrame ();
  if (frame && frame->f_code && frame->f_code->co_name)
    funcname = mobius::py::pystring_as_std_string (frame->f_code->co_name);

  return funcname;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new instance of Py_None
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pynone ()
{
  Py_INCREF (Py_None);
  return Py_None;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new instance of Py_True
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
py_true ()
{
  Py_INCREF (Py_True);
  return Py_True;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new instance of Py_False
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
py_false ()
{
  Py_INCREF (Py_False);
  return Py_False;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new instance of Py_NotImplemented
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
py_not_implemented ()
{
  Py_INCREF (Py_NotImplemented);
  return Py_NotImplemented;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if object is Py_None
// @param value Python object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pynone_check (PyObject *value)
{
  return value == Py_None;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check string
// @param value Python object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pystring_check (PyObject *value)
{
  return PyUnicode_Check (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create PyUnicode from std::string
// @param s C++ string
// @return Python Unicode object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pystring_from_std_string (const std::string& s)
{
  auto ret = PyUnicode_DecodeUTF8 (s.c_str (), s.size (), "replace");

  if (!ret)
    throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG ("invalid C++ UTF-8 string"));

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create std::string from PyUnicode
// @param value Python String
// @return C++ string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
pystring_as_std_string (PyObject *value)
{
  const char *buffer = PyUnicode_AsUTF8 (value);
  std::string str;

  if (buffer)
    str = buffer;

  else
    throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG ("invalid Python string"));

  return str;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check PyBytes
// @param value Python object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pybytes_check (PyObject *value)
{
  return PyBytes_Check (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create PyBytes from mobius::bytearray
// @param array Bytearray
// @return Python Bytes object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pybytes_from_bytearray (const mobius::bytearray& array)
{
  return PyBytes_FromStringAndSize ((const char *) array.data (), array.size ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create mobius::bytearray from PyBytes
// @param value Python Object
// @return Bytearray
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::bytearray
pybytes_as_bytearray (PyObject *value)
{
  char *buffer;
  Py_ssize_t length;

  if (PyBytes_AsStringAndSize (value, &buffer, &length) == -1)
    throw std::runtime_error (mobius::MOBIUS_EXCEPTION_MSG ("invalid bytes"));

  return mobius::bytearray (reinterpret_cast <const std::uint8_t *> (buffer), length);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create PyBytes from char
// @param c Char
// @return Python Bytes object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pybytes_from_char (char c)
{
  char buffer[2] = {c, 0};
  return PyBytes_FromStringAndSize (buffer, 1);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check PyBool
// @param value Python object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pybool_check (PyObject *value)
{
  return PyBool_Check (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create PyBytes from mobius::bytearray
// @param b Bool value
// @return Python Bool object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pybool_from_bool (bool b)
{
  PyObject *ret = b ? Py_True : Py_False;
  Py_INCREF (ret);

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create bool from PyBool
// @param value Python Object
// @return bool value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pybool_as_bool (PyObject *value)
{
  return value == Py_True;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check PyLong
// @param value Python object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pylong_check (PyObject *value)
{
  return PyLong_Check (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create PyLong from std::int8_t
// @param value Value
// @return Python Unicode object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pylong_from_std_int8_t (std::int8_t value)
{
  return PyLong_FromLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create std::int8_t from PyLong
// @param value Python Object
// @return std::int8_t
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int8_t
pylong_as_std_int8_t (PyObject *value)
{
  return PyLong_AsLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create PyLong from std::int16_t
// @param value Value
// @return Python Unicode object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pylong_from_std_int16_t (std::int16_t value)
{
  return PyLong_FromLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create std::uint16_t from PyLong
// @param value Python Object
// @return std::uint16_t
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int16_t
pylong_as_std_int16_t (PyObject *value)
{
  return PyLong_AsLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create PyLong from std::int32_t
// @param value Value
// @return Python Unicode object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pylong_from_std_int32_t (std::int32_t value)
{
  return PyLong_FromLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create std::uint32_t from PyLong
// @param value Python Object
// @return std::uint32_t
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int32_t
pylong_as_std_int32_t (PyObject *value)
{
  return PyLong_AsLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create PyLong from std::uint8_t
// @param value Value
// @return Python Long object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pylong_from_std_uint8_t (std::uint8_t value)
{
  return PyLong_FromUnsignedLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create PyLong from std::uint16_t
// @param value Value
// @return Python Long object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pylong_from_std_uint16_t (std::uint16_t value)
{
  return PyLong_FromUnsignedLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create PyLong from std::uint32_t
// @param value Value
// @return Python Long object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pylong_from_std_uint32_t (std::uint32_t value)
{
  return PyLong_FromUnsignedLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create std::uint8_t from PyLong
// @param value Python Object
// @return std::uint8_t
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint8_t
pylong_as_std_uint8_t (PyObject *value)
{
  return PyLong_AsUnsignedLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create std::uint16_t from PyLong
// @param value Python Object
// @return std::uint16_t
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint16_t
pylong_as_std_uint16_t (PyObject *value)
{
  return PyLong_AsUnsignedLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create std::uint32_t from PyLong
// @param value Python Object
// @return std::uint32_t
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
pylong_as_std_uint32_t (PyObject *value)
{
  return PyLong_AsUnsignedLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create PyLong from std::int64_t
// @param value Value
// @return Python Unicode object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pylong_from_std_int64_t (std::int64_t value)
{
  return PyLong_FromLongLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create std::uint64_t from PyLong
// @param value Python Object
// @return std::uint64_t
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::int64_t
pylong_as_std_int64_t (PyObject *value)
{
  return PyLong_AsLongLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create PyLong from std::uint64_t
// @param value Value
// @return Python Unicode object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pylong_from_std_uint64_t (std::uint64_t value)
{
  return PyLong_FromUnsignedLongLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create std::uint64_t from PyLong
// @param value Python Object
// @return std::uint64_t
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
pylong_as_std_uint64_t (PyObject *value)
{
  std::uint64_t v = PyLong_AsUnsignedLongLong (value);

  if (PyErr_Occurred ())
    throw std::runtime_error (get_error_message ());

  return v;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create PyLong from int
// @param value Value
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pylong_from_int (int value)
{
  return PyLong_FromLongLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create int from PyLong
// @param value Python Object
// @return int
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
pylong_as_int (PyObject *value)
{
  return PyLong_AsLong (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check PyFloat
// @param value Python object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pyfloat_check (PyObject *value)
{
  return PyFloat_Check (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create double value from PyFloat
// @param value Python Object
// @return double value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
double
pyfloat_as_cpp (PyObject *value)
{
  return PyFloat_AS_DOUBLE (value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create PyFloat from double
// @param value Value
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pyfloat_from_cpp (double value)
{
  return PyFloat_FromDouble (value);
}

} // namespace mobius::py


