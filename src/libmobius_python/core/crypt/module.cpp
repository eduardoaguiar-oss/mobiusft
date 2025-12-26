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
// @file C++ API <i>mobius.core.crypt</i> module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "module.hpp"
#include <pymobius.hpp>
#include "cipher.hpp"
#include "hash.hpp"
#include "hmac.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *func_hash_digest (PyObject *, PyObject *);
PyObject *func_new_cipher_cbc (PyObject *, PyObject *);
PyObject *func_new_cipher_cbc_cts (PyObject *, PyObject *);
PyObject *func_new_cipher_ctr (PyObject *, PyObject *);
PyObject *func_new_cipher_ecb (PyObject *, PyObject *);
PyObject *func_new_cipher_gcm (PyObject *, PyObject *);
PyObject *func_new_cipher_ige (PyObject *, PyObject *);
PyObject *func_new_cipher_ofb (PyObject *, PyObject *);
PyObject *func_new_cipher_poly1305 (PyObject *, PyObject *);
PyObject *func_new_cipher_stream (PyObject *, PyObject *);
PyObject *func_pbkdf1 (PyObject *, PyObject *);
PyObject *func_pbkdf2_hmac (PyObject *, PyObject *);
PyObject *func_rot13 (PyObject *, PyObject *);

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module methods
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef module_methods[] = {
    {"hash_digest", func_hash_digest, METH_VARARGS,
     "Generate hash digest for data"},
    {"new_cipher_cbc", func_new_cipher_cbc, METH_VARARGS,
     "Create new cipher object with CBC mode"},
    {"new_cipher_cbc_cts", func_new_cipher_cbc_cts, METH_VARARGS,
     "Create new cipher object with CBC/CTS mode"},
    {"new_cipher_ctr", func_new_cipher_ctr, METH_VARARGS,
     "Create new cipher object with CTR mode"},
    {"new_cipher_ecb", func_new_cipher_ecb, METH_VARARGS,
     "Create new cipher object with ECB mode"},
    {"new_cipher_gcm", func_new_cipher_gcm, METH_VARARGS,
     "Create new cipher object with GCM mode"},
    {"new_cipher_ige", func_new_cipher_ige, METH_VARARGS,
     "Create new cipher object with IGE mode"},
    {"new_cipher_ofb", func_new_cipher_ofb, METH_VARARGS,
     "Create new cipher object with OFB mode"},
    {"new_cipher_poly1305", func_new_cipher_poly1305, METH_VARARGS,
     "Create new cipher object with Poly1305 mode"},
    {"new_cipher_stream", func_new_cipher_stream, METH_VARARGS,
     "Create new stream cipher object"},
    {"pbkdf1", func_pbkdf1, METH_VARARGS, "Generate derived key using PBKDF1"},
    {"pbkdf2_hmac", func_pbkdf2_hmac, METH_VARARGS,
     "Generate derived key using PBKDF2_HMAC"},
    {"rot13", func_rot13, METH_VARARGS,
     "Encode/decode data with ROT-13 algorithm"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module definition structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    "mobius.core.crypt",
    "Mobius Forensic Toolkit mobius.core.crypt module",
    -1,
    module_methods,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create mobius.core.crypt module
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pymodule
new_core_crypt_module ()
{
    // Initialize module
    mobius::py::pymodule module (&module_def);

    // Add types
    module.add_type ("cipher", &core_crypt_cipher_t),
        module.add_type ("hash", &core_crypt_hash_t);
    module.add_type ("hmac", &core_crypt_hmac_t);

    // Return module
    return module;
}
