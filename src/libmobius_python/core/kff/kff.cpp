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
#include "core/database/transaction.hpp"
#include "file.hpp"
#include <pycall.hpp>
#include <stdexcept>

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
    try
    {
        auto arg_name = mobius::py::get_arg<std::string> (args, 0);
        return mobius::py::call_function (mobius::core::kff::get_file_type, arg_name);
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
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
    try
    {
        auto arg_type = mobius::py::get_arg<std::uint16_t> (args, 0);
        return mobius::py::call_function (mobius::core::kff::get_file_type_string, arg_type);
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
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
    try
    {
        auto arg_name = mobius::py::get_arg<std::string> (args, 0);
        return mobius::py::call_function (mobius::core::kff::get_hash_type, arg_name);
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
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
    try
    {
        auto arg_type = mobius::py::get_arg<std::uint16_t> (args, 0);
        return mobius::py::call_function (mobius::core::kff::get_hash_type_string, arg_type);
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
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
    try
    {
        auto arg_size = mobius::py::get_arg<std::uint64_t> (args, 0);
        return mobius::py::call_function (mobius::core::kff::has_size, arg_size);
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
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
    try
    {
        auto arg_hash_type = mobius::py::get_arg<std::uint16_t> (args, 0);
        auto arg_value = mobius::py::get_arg<std::string> (args, 1);
        return mobius::py::call_function (mobius::core::kff::lookup, arg_hash_type, arg_value);
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>new_transaction</i> function implementation
// @return new transaction object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_new_transaction (PyObject *, PyObject *)
{
    return mobius::py::call_function (mobius::core::kff::new_transaction);
}

} // namespace mobius::py
