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
// @brief  C++ API module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "path.hpp"
#include <pymobius.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function join_path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_join_path (PyObject *, PyObject *args)
{
    // check arguments
    auto size = mobius::py::get_arg_size (args);

    if (size < 1)
    {
        mobius::py::set_value_error (
            "join_path must have at least one argument");
        return nullptr;
    }

    // parse arguments
    mobius::core::io::path path;

    for (std::uint32_t i = 0; i < size; i++)
    {
        PyObject *item = mobius::py::get_arg (args, i);
        mobius::core::io::path segment;

        if (pymobius_core_io_path_check (item))
            segment = pymobius_core_io_path_from_pyobject (item);

        else if (mobius::py::pystring_check (item))
            segment = mobius::core::io::path (
                mobius::py::pystring_as_std_string (item));

        else
        {
            mobius::py::set_value_error ("Invalid path segment");
            return nullptr;
        }

        if (i == 0)
            path = segment;
        else
            path = join (path, segment);
    }

    // return path
    return pymobius_core_io_path_to_pyobject (path);
}
