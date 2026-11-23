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
#include <pyfunction.hpp>
#include <pycall.hpp>
#include <stdexcept>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Logging function wrapper for Python log functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class log_function
{
  public:
    log_function () = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Constructor with optional Python log function
    // @param f Python callable object for logging
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    log_function (PyObject *f)
        : f_ (f)
    {
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Call the Python log function with a message
    // @param msg Message to log
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    operator() (const std::string &msg)
    {
        if (f_)
        {
            mobius::py::GIL_guard gil_guard;

            f_ (mobius::py::to_pyobject (msg));
        }
    }

  private:
    mobius::py::function f_;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get log function argument from Python args
// @param args Python argument tuple
// @param index Index of the argument
// @return log_function object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static log_function
_get_arg_log (PyObject *args, std::size_t index)
{
    if (index < mobius::py::get_arg_size (args))
        return log_function (mobius::py::get_arg (args, index));

    return log_function ();
}

} // namespace

namespace mobius::py
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Export collected data to a file
// @param self Python self
// @param path Path to the output file
// @param log Logging function or object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_export_collected_data (PyObject *, PyObject *args)
{
    try
    {
        auto arg_path = mobius::py::get_arg<std::string> (args, 0);
        auto arg_log = _get_arg_log (args, 1);

        mobius::py::GIL gil;
        return mobius::py::call_function (mobius::core::kff::export_collected_data, arg_path, arg_log);
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Export full KFF database to a file
// @param self Python self
// @param path Path to the output file
// @param log Logging function or object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_export_kff (PyObject *, PyObject *args)
{
    try
    {
        auto arg_path = mobius::py::get_arg<std::string> (args, 0);
        auto arg_log = _get_arg_log (args, 1);

        mobius::py::GIL gil;
        return mobius::py::call_function (mobius::core::kff::export_kff, arg_path, arg_log);
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
}

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
// @brief <i>get_file</i> function implementation
// @param self Python self
// @param arg Python argument
// @return new file object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_get_file (PyObject *, PyObject *arg)
{
    try
    {
        auto arg_file_id = mobius::py::get_arg<std::uint64_t> (arg);
        return mobius::py::call_function (mobius::core::kff::get_file, arg_file_id);
    }
    catch (...)
    {
        return nullptr;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Import collected data from a file
// @param self Python self
// @param path Path to the input file
// @param log Logging function or object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_import_collected_data (PyObject *, PyObject *args)
{
    try
    {
        auto arg_path = mobius::py::get_arg<std::string> (args, 0);
        auto arg_log = _get_arg_log (args, 1);

        mobius::py::GIL gil;
        return mobius::py::call_function (mobius::core::kff::import_collected_data, arg_path, arg_log);
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Import IPED hashes from a file
// @param self Python self
// @param path Path to the input file
// @param log Logging function or object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_import_iped_hashes (PyObject *, PyObject *args)
{
    try
    {
        auto arg_path = mobius::py::get_arg<std::string> (args, 0);
        auto arg_log = _get_arg_log (args, 1);

        mobius::py::GIL gil;
        return mobius::py::call_function (mobius::core::kff::import_iped_hashes, arg_path, arg_log);
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
func_core_kff_iter_files (PyObject *, PyObject *)
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
func_core_kff_iter_files_by_type (PyObject *, PyObject *arg)
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update file sizes from a file
// @param self Python self
// @param args Python arguments containing the path and log
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_update_file_sizes (PyObject *, PyObject *args)
{
    try
    {
        auto arg_path = mobius::py::get_arg<std::string> (args, 0);
        auto arg_log = _get_arg_log (args, 1);

        mobius::py::GIL gil;
        return mobius::py::call_function (mobius::core::kff::update_file_sizes, arg_path, arg_log);
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief update passwords from a file
// @param self Python self
// @param args Python arguments containing the path and log
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_core_kff_update_passwords (PyObject *, PyObject *args)
{
    try
    {
        auto arg_path = mobius::py::get_arg<std::string> (args, 0);
        auto arg_log = _get_arg_log (args, 1);

        mobius::py::GIL gil;
        return mobius::py::call_function (mobius::core::kff::update_passwords, arg_path, arg_log);
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
}


} // namespace mobius::py
