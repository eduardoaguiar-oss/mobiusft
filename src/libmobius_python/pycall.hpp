#ifndef LIBMOBIUS_PYTHON_PYCALL_HPP
#define LIBMOBIUS_PYTHON_PYCALL_HPP

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008-2026 Eduardo Aguiar
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
#define PY_SSIZE_T_CLEAN // PEP 353

#include <Python.h>
#include <functional>

namespace mobius::py
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Call C++ object member function with arguments, handling exceptions
// and converting the result to a Python object
//
// @tparam obj_type Type of the C++ object
// @tparam func_type Type of the C++ member function
// @tparam Args Types of the arguments to pass to the C++ function
// @param obj Pointer to the C++ object on which to call the member function
// @param f Pointer to the C++ member function to call
// @param args Arguments to pass to the C++ function
// @return Python object representing the result of the C++ function, including void
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename obj_type, typename func_type, typename... Args>
PyObject *
call_method (obj_type *obj, func_type &&f, Args &&...args)
{
    PyObject *ret = nullptr;

    try
    {
        using result_type = std::invoke_result_t<func_type, obj_type *, Args...>;

        if constexpr (std::is_void_v<result_type>)
        {
            (obj->*f) (std::forward<Args> (args)...);
            ret = pynone ();
        }
        else
        {
            ret = to_pyobject ((obj->*f) (std::forward<Args> (args)...));
        }
    }
    catch (const std::exception &e)
    {
        set_runtime_error (e.what ());
    }
    catch (...)
    {
        set_runtime_error ("unknown C++ exception");
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Call C++ function and convert result to Python object
// @tparam func_type Type of the C++ function
// @tparam Args Types of the arguments to pass to the C++ function
// @param f Pointer to the C++ function to call
// @param args Arguments to pass to the C++ function
// @return Python object representing the result of the C++ function, including void
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename func_type, typename... Args>
PyObject *
call_function (func_type &&f, Args &&...args)
{
    PyObject *ret = nullptr;

    try
    {
        using result_type = std::invoke_result_t<func_type, Args...>;

        if constexpr (std::is_void_v<result_type>)
        {
            f (std::forward<Args> (args)...);
            ret = pynone ();
        }
        else
        {
            ret = to_pyobject (f (std::forward<Args> (args)...));
        }
    }
    catch (const std::exception &e)
    {
        set_runtime_error (e.what ());
    }
    catch (...)
    {
        set_runtime_error ("unknown C++ exception");
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Generic getter for C++ member function
// @tparam obj_type Type of the C++ object
// @tparam method_type Type of the C++ member function
// @param obj C++ object or reference
// @param method C++ member function to call
// @return Python object representing the result of the C++ member function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename obj_type, typename method_type>
PyObject *
generic_getter (obj_type &&obj, method_type method) noexcept
{
    try
    {
        return mobius::py::to_pyobject (std::invoke (method, obj));
    }
    catch (const std::exception &e)
    {
        set_runtime_error (e.what ());
    }
    catch (...)
    {
        set_runtime_error ("unknown C++ exception");
    }

    return nullptr;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Generic setter for C++ member function
// @tparam T Type of the attribute
// @tparam obj_type Type of the Python wrapper object
// @tparam method_type Type of the C++ member function
// @tparam check_type Type of the Python type check function
// @param obj Python object wrapping the C++ object
// @param method C++ member function to set the attribute
// @param value Python object representing the new value
// @param pycheck Function to check the Python object type
// @return 0 on success, -1 on error
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T, typename obj_type, typename method_type, typename check_type>
int
generic_setter (obj_type &&obj, method_type method, PyObject *value, check_type pycheck)
{
    // If it is a delete operation, then return with error
    if (value == nullptr)
    {
        std::string msg = "cannot delete attribute";
        mobius::py::set_invalid_type_error (msg.c_str ());
        return -1;
    }

    // Check argument type
    if (!pycheck (value))
    {
        std::string msg = "invalid type for attribute";
        mobius::py::set_invalid_type_error (msg.c_str ());
        return -1;
    }

    // Set attribute
    try
    {
        std::invoke (method, obj, from_pyobject<T> (value));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return -1;
    }
    catch (...)
    {
        mobius::py::set_runtime_error ("unknown C++ exception");
        return -1;
    }

    // Return success
    return 0;
}

} // namespace mobius::py

#endif
