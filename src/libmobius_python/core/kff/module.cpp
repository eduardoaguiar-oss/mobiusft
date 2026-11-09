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
#include "hashset.hpp"
#include "kff.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module methods
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef module_methods[] = {
    {"get_alert_count", func_core_kff_get_alert_count, METH_NOARGS, "Get alert count"},
    {"get_file_count", func_core_kff_get_file_count, METH_NOARGS, "Get file count"},
    {"get_file_count_by_type", func_core_kff_get_file_count_by_type, METH_NOARGS, "Get file count by type"},
    {"get_hash_count", func_core_kff_get_hash_count, METH_NOARGS, "Get hash count"},
    {"get_hash_count_by_type", func_core_kff_get_hash_count_by_type, METH_NOARGS, "Get hash count by type"},
    {"get_thumbnail_count", func_core_kff_get_thumbnail_count, METH_NOARGS, "Get thumbnail count"},
    {"has_size", func_core_kff_has_size, METH_VARARGS, "Check if file has size"},
    {"lookup", func_core_kff_lookup, METH_VARARGS, "Lookup file by hash type and value"},
    {"new_transaction", func_core_kff_new_transaction, METH_NOARGS, "Create new transaction to KFF database"},
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
    module.add_type ("file", mobius::py::new_core_kff_file_type ());
    module.add_type ("kff", new_core_kff_kff_type ());
    module.add_type ("hashset", new_core_kff_hashset_type ());

    // Add constants
    module.add_constant("FILE_TYPE_REGULAR", mobius::core::kff::FILE_TYPE_REGULAR);
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
