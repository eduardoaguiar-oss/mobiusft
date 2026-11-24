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
// @brief  C++ API mobius.core.kff module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <pymobius.hpp>
#include "module.hpp"
#include "file.hpp"
#include "kff.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module methods
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef module_methods[] = {
    {"export_collected_data", mobius::py::func_core_kff_export_collected_data, METH_VARARGS, "Export collected data to a file"},
    {"export_database", mobius::py::func_core_kff_export_database, METH_VARARGS, "Export full KFF database to a file"},
    {"get_alert_count", mobius::py::func_core_kff_get_alert_count, METH_NOARGS, "Get alert count"},
    {"get_file", mobius::py::func_core_kff_get_file, METH_O, "Get file by ID"},
    {"get_file_count", mobius::py::func_core_kff_get_file_count, METH_NOARGS, "Get file count"},
    {"get_file_type", mobius::py::func_core_kff_get_file_type, METH_O, "Get file type index"},
    {"get_file_type_string", mobius::py::func_core_kff_get_file_type_string, METH_O, "Get file type string"},
    {"get_hash_count", mobius::py::func_core_kff_get_hash_count, METH_NOARGS, "Get hash count"},
    {"get_hash_type", mobius::py::func_core_kff_get_hash_type, METH_O, "Get hash type index"},
    {"get_hash_type_string", mobius::py::func_core_kff_get_hash_type_string, METH_O, "Get hash type string"},
    {"get_next_file_id", mobius::py::func_core_kff_get_next_file_id, METH_NOARGS, "Get next file ID"},
    {"get_thumbnail_count", mobius::py::func_core_kff_get_thumbnail_count, METH_NOARGS, "Get thumbnail count"},
    {"has_size", mobius::py::func_core_kff_has_size, METH_O, "Check if file has size"},
    {"iter_files", mobius::py::func_core_kff_iter_files, METH_NOARGS, "Iterate over files"},
    {"iter_files_by_type", mobius::py::func_core_kff_iter_files_by_type, METH_O, "Iterate over files by hash type"},
    {"lookup", mobius::py::func_core_kff_lookup, METH_VARARGS, "Lookup file by hash type and value"},
    {"merge_collected_data", mobius::py::func_core_kff_merge_collected_data, METH_VARARGS, "Merge collected data from a file"},
    {"merge_file_sizes", mobius::py::func_core_kff_merge_file_sizes, METH_VARARGS, "Merge file sizes from a file"},
    {"merge_passwords", mobius::py::func_core_kff_merge_passwords, METH_VARARGS, "Merge passwords from a file"},
    {"new_file", mobius::py::func_core_kff_new_file, METH_VARARGS, "Create new file"},
    {"new_transaction", mobius::py::func_core_kff_new_transaction, METH_NOARGS, "Create new transaction to KFF database"},
    {"populate_from_database", mobius::py::func_core_kff_populate_from_database, METH_VARARGS, "Populate the KFF database from a database file"},
    {"populate_from_iped_hashes", mobius::py::func_core_kff_populate_from_iped_hashes, METH_VARARGS, "Import IPED hashes from a file"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module definition structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    "mobius.core.kff",
    "Mobius Forensic Toolkit mobius.core.kff module",
    -1,
    module_methods,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create module
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pymodule
new_core_kff_module ()
{
    // Initialize module
    mobius::py::pymodule module (&module_def);

    // Add types
    module.add_type ("file", mobius::py::get_core_kff_file_type ());

    // Add constants
    module.add_constant("FILE_TYPE_INFO", mobius::core::kff::FILE_TYPE_INFO);
    module.add_constant("FILE_TYPE_IGNORED", mobius::core::kff::FILE_TYPE_IGNORED);
    module.add_constant("FILE_TYPE_ALERT_CSAM", mobius::core::kff::FILE_TYPE_ALERT_CSAM);
    module.add_constant("FILE_TYPE_ALERT_MALWARE", mobius::core::kff::FILE_TYPE_ALERT_MALWARE);
    module.add_constant("HASH_TYPE_ED2K", mobius::core::kff::HASH_TYPE_ED2K);
    module.add_constant("HASH_TYPE_MD5", mobius::core::kff::HASH_TYPE_MD5);
    module.add_constant("HASH_TYPE_SHA1", mobius::core::kff::HASH_TYPE_SHA1);
    module.add_constant("HASH_TYPE_SHA2_256", mobius::core::kff::HASH_TYPE_SHA2_256);
    module.add_constant("HASH_TYPE_SHA2_512", mobius::core::kff::HASH_TYPE_SHA2_512);

    // Return module
    return module;
}
