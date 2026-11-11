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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @file kff.cpp C++ API <i>mobius.core.kff.kff</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "kff.hpp"
#include <mobius/core/exception.inc>
#include <stdexcept>
#include "core/database/transaction.hpp"
#include "file.hpp"
#include <pymobius.hpp>
#include <pycall.hpp>

namespace mobius::py
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file type
// @param self Python self
// @param args Python arguments
// @return File type index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_file_type (PyObject *, PyObject *args)
{
    // Parse input args
    std::string arg_name;

    try
    {
        arg_name = mobius::py::get_arg<std::string> (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    return mobius::py::call_function (mobius::core::kff::get_file_type, arg_name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file type string
// @param self Python self
// @param args Python arguments
// @return File count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_file_type_string (PyObject *, PyObject *args)
{
    // Parse input args
    std::uint16_t arg_type;

    try
    {
        arg_type = mobius::py::get_arg<std::uint16_t> (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    return mobius::py::call_function (mobius::core::kff::get_file_type_string, arg_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash type index
// @param self Python self
// @param args Python arguments
// @return Hash type index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_hash_type (PyObject *, PyObject *args)
{
    // Parse input args
    std::string arg_name;

    try
    {
        arg_name = mobius::py::get_arg<std::string> (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    return mobius::py::call_function (mobius::core::kff::get_hash_type, arg_name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash type string
// @param self Python self
// @param args Python arguments
// @return Hash type string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_hash_type_string (PyObject *, PyObject *args)
{
    // Parse input args
    std::uint16_t arg_type;

    try
    {
        arg_type = mobius::py::get_arg<std::uint16_t> (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    return mobius::py::call_function (mobius::core::kff::get_hash_type_string, arg_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file count
// @param self Python self
// @param args Python arguments
// @return File count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_file_count (PyObject *, PyObject *)
{
    return mobius::py::call_function (mobius::core::kff::get_file_count);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get hash count
// @param self Python self
// @param args Python arguments
// @return Hash count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_hash_count (PyObject *, PyObject *)
{
    return mobius::py::call_function (mobius::core::kff::get_hash_count);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get alert count
// @param self Python self
// @param args Python arguments
// @return Alert count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_alert_count (PyObject *, PyObject *)
{
    return mobius::py::call_function (mobius::core::kff::get_alert_count);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get thumbnail count
// @param self Python self
// @param args Python arguments
// @return Thumbnail count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_thumbnail_count (PyObject *, PyObject *)
{
    return mobius::py::call_function (mobius::core::kff::get_thumbnail_count);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <b>has_size</b> function implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_has_size (PyObject *, PyObject *args)
{
    // Parse input args
    std::uint64_t arg_size;

    try
    {
        arg_size = mobius::py::get_arg<std::uint64_t> (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    return mobius::py::call_function (mobius::core::kff::has_size, arg_size);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>lookup</i> function implementation
// @param self Python self
// @param args Python arguments
// @return Lookup result
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_lookup (PyObject *, PyObject *args)
{
    // Parse input args
    std::uint16_t arg_hash_type;
    std::string arg_value;

    try
    {
        arg_hash_type = mobius::py::get_arg<std::uint16_t> (args, 0);
        arg_value = mobius::py::get_arg<std::string> (args, 1);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Execute C++ function
    return mobius::py::call_function (mobius::core::kff::lookup, arg_hash_type, arg_value);
}
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>new_transaction</i> function implementation
// @return new transaction object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_new_transaction (PyObject *, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_database_transaction_to_pyobject (mobius::core::kff::new_transaction ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

} // namespace mobius::py
