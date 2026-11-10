#ifndef LIBMOBIUS_PYTHON_PYTUPLE_HPP
#define LIBMOBIUS_PYTHON_PYTUPLE_HPP

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
#define PY_SSIZE_T_CLEAN // PEP 353

#include <Python.h>
#include <utility>

namespace mobius::py
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new Python Tuple from C++ pair
// @param p C++ pair
// @return Python Tuple
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T>
PyObject *
pytuple_from_cpp_pair (const T &p)
{
    PyObject *ret = PyTuple_New (2);

    if (ret)
    {
        PyTuple_SetItem (ret, 0, to_pyobject (p.first));
        PyTuple_SetItem (ret, 1, to_pyobject (p.second));
    }

    return ret;
}

} // namespace mobius::py

#endif
