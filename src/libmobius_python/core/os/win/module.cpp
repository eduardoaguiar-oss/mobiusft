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
// @brief  C++ API mobius.core.os module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "module.hpp"
#include "credential.hpp"
#include "dpapi/module.hpp"
#include "registry/module.hpp"
#include "trashbin/module.hpp"
#include <Python.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *func_hash_ie_entropy (PyObject *, PyObject *);
PyObject *func_hash_lm (PyObject *, PyObject *);
PyObject *func_hash_nt (PyObject *, PyObject *);
PyObject *func_hash_msdcc1 (PyObject *, PyObject *);
PyObject *func_hash_msdcc2 (PyObject *, PyObject *);
PyObject *func_pbkdf2_hmac_ms (PyObject *, PyObject *);

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module methods
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef module_methods[] = {
    {"hash_ie_entropy", func_hash_ie_entropy, METH_VARARGS,
     "Generate IE entropy hash"},
    {"hash_lm", func_hash_lm, METH_VARARGS, "Generate LM hash"},
    {"hash_nt", func_hash_nt, METH_VARARGS, "Generate NT hash"},
    {"hash_msdcc1", func_hash_msdcc1, METH_VARARGS, "Generate MSDCCv1 hash"},
    {"hash_msdcc2", func_hash_msdcc2, METH_VARARGS, "Generate MSDCCv2 hash"},
    {"pbkdf2_hmac_ms", func_pbkdf2_hmac_ms, METH_VARARGS,
     "Generate PBKDF2_HMAC key (MS version)"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module definition structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    "mobius.core.os.win",
    "Mobius Forensic Toolkit mobius.core.os.win module",
    -1,
    module_methods,
    nullptr,
    nullptr,
    nullptr,
    nullptr};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create mobius.core.os.win module
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pymodule
new_core_os_win_module ()
{
    // Initialize module
    mobius::py::pymodule module (&module_def);

    // Add types
    module.add_type ("credential", &core_os_win_credential_t);

    // Build submodules
    module.add_submodule ("dpapi", new_core_os_win_dpapi_module ());
    module.add_submodule ("registry", new_core_os_win_registry_module ());
    module.add_submodule ("trashbin", new_core_os_win_trashbin_module ());

    // Return module
    return module;
}
