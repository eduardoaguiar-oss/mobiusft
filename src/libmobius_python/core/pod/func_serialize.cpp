// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025
// Eduardo Aguiar
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
// @brief  C++ API mobius.core.pod module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "data.hpp"
#include "core/io/writer.hpp"
#include <mobius/core/exception.inc>
#include <pygil.hpp>
#include <pymobius.hpp>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Serialize data to bytearray
// @param args argument list
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
_serialize_to_bytearray (PyObject *args)
{
    // parse arguments
    mobius::core::pod::data arg_data;

    try
    {
        arg_data = mobius::py::get_arg_as_cpp (
            args, 0, pymobius_core_pod_data_from_pyobject);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybytes_from_bytearray (
            mobius::py::GIL () (mobius::core::pod::serialize (arg_data)));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Serialize data to writer
// @param args argument list
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
_serialize_to_writer (PyObject *args)
{
    // parse arguments
    mobius::core::io::writer arg_writer;
    mobius::core::pod::data arg_data;

    try
    {
        arg_writer = mobius::py::get_arg_as_cpp (
            args, 0, pymobius_core_io_writer_from_pyobject);
        arg_data = mobius::py::get_arg_as_cpp (
            args, 1, pymobius_core_pod_data_from_pyobject);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        {
            mobius::py::GIL gil;
            mobius::core::pod::serialize (arg_writer, arg_data);
        }
        ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <b>mobius.core.pod.serialize</b> function
// @param self function object
// @param args argument list
// @return Python object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_pod_serialize (PyObject *, PyObject *args)
{
    PyObject *ret = nullptr;

    auto arg_size = mobius::py::get_arg_size (args);

    if (arg_size == 1)
        ret = _serialize_to_bytearray (args);

    else if (arg_size == 2)
        ret = _serialize_to_writer (args);

    else
        mobius::py::set_invalid_type_error (
            MOBIUS_EXCEPTION_MSG ("invalid number of arguments").c_str ());

    return ret;
}
