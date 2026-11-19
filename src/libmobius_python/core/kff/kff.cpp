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
#include "file.hpp"
#include "core/database/transaction.hpp"
#include "core/generator.hpp"
#include <pycall.hpp>
#include <stdexcept>

namespace mobius::py
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file type
// @param self Python self
// @param arg Python argument
// @return File type index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_file_type (PyObject *, PyObject *arg)
{
    try
    {
        auto arg_name = mobius::py::get_arg<std::string> (arg);
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
// @param arg Python argument
// @return File count
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_file_type_string (PyObject *, PyObject *arg)
{
    try
    {
        auto arg_type = mobius::py::get_arg<std::uint16_t> (arg);
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
// @param arg Python argument
// @return Hash type index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_hash_type (PyObject *, PyObject *arg)
{
    try
    {
        auto arg_name = mobius::py::get_arg<std::string> (arg);
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
// @param arg Python argument
// @return Hash type string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_hash_type_string (PyObject *, PyObject *arg)
{
    try
    {
        auto arg_type = mobius::py::get_arg<std::uint16_t> (arg);
        return mobius::py::call_function (mobius::core::kff::get_hash_type_string, arg_type);
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
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
// @brief Get next file ID
// @param self Python self
// @param args Python arguments
// @return Next file ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_next_file_id (PyObject *, PyObject *)
{
    return mobius::py::call_function (mobius::core::kff::get_next_file_id);
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
// @param arg Python argument
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_has_size (PyObject *, PyObject *arg)
{
    try
    {
        auto arg_size = mobius::py::get_arg<std::uint64_t> (arg);
        return mobius::py::call_function (mobius::core::kff::has_size, arg_size);
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>iter_files</i> function implementation
// @return Generator for iterating over files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_kff_iter_files (PyObject *, PyObject *)
{
    try
    {
        return mobius::py::call_function (mobius::core::kff::iter_files);
    }
    catch (...)
    {
        return nullptr;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>iter_files_by_type</i> function implementation
// @return Generator for iterating over files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_kff_iter_files_by_type (PyObject *, PyObject *arg)
{
    try
    {
        auto arg_hash_type = mobius::py::get_arg<std::uint16_t> (arg);
        return mobius::py::call_function (mobius::core::kff::iter_files_by_type, arg_hash_type);
    }
    catch (...)
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
    catch (...)
    {
        return nullptr;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>new_file</i> function implementation
// @param self Python self
// @param args Python arguments
// @return new file object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_new_file (PyObject *, PyObject *args)
{
    try
    {
        auto arg_file_id = mobius::py::get_arg<std::uint64_t> (args, 0);
        auto arg_type = mobius::py::get_arg<std::uint16_t> (args, 1);
        auto arg_size = mobius::py::get_arg<std::int64_t> (args, 2, -1);
        return mobius::py::call_function (mobius::core::kff::new_file, arg_file_id, arg_type, arg_size);
    }
    catch (...)
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
    try
    {
        return mobius::py::call_function (mobius::core::kff::new_transaction);
    }
    catch (...)
    {
        return nullptr;
    }
}

} // namespace mobius::py
