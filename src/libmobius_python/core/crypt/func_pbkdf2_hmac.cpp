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
#include <mobius/core/crypt/pkcs5.hpp>
#include <pygil.hpp>
#include <pymobius.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief pbkdf2_hmac function implementation
// @param self function object
// @param args argument list
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_pbkdf2_hmac (PyObject *, PyObject *args)
{
    // parse input args
    std::string arg_hash_id;
    mobius::core::bytearray arg_key;
    mobius::core::bytearray arg_salt;
    std::uint32_t arg_iterations;
    std::uint16_t arg_dklen;

    try
    {
        arg_hash_id = mobius::py::get_arg_as_std_string (args, 0);
        arg_key = mobius::py::get_arg_as_bytearray (args, 1);
        arg_salt = mobius::py::get_arg_as_bytearray (args, 2);
        arg_iterations = mobius::py::get_arg_as_uint32_t (args, 3);
        arg_dklen = mobius::py::get_arg_as_uint16_t (args, 4);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybytes_from_bytearray (
            mobius::py::GIL () (mobius::core::crypt::pbkdf2_hmac (
                arg_hash_id, arg_key, arg_salt, arg_iterations, arg_dklen)));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_io_error (e.what ());
    }

    return ret;
}
