#ifndef LIBMOBIUS_PYTHON_CORE_KFF_KFF_HPP
#define LIBMOBIUS_PYTHON_CORE_KFF_KFF_HPP

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
#include <Python.h>
#include <mobius/core/kff/kff.hpp>
#include <pytypeobject.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Data structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
typedef struct
{
    PyObject_HEAD mobius::core::kff::kff *obj;
} core_kff_kff_o;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Helper functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject new_core_kff_kff_type ();
bool pymobius_core_kff_kff_check (PyObject *);
PyObject *pymobius_core_kff_kff_to_pyobject (const mobius::core::kff::kff &);
mobius::core::kff::kff pymobius_core_kff_kff_from_pyobject (PyObject *);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Functions prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *func_core_kff_get_file_count (PyObject *, PyObject *);
PyObject *func_core_kff_get_file_count_by_type (PyObject *, PyObject *);
PyObject *func_core_kff_get_hash_count (PyObject *, PyObject *);
PyObject *func_core_kff_get_hash_count_by_type (PyObject *, PyObject *);
PyObject *func_core_kff_get_alert_count (PyObject *, PyObject *);
PyObject *func_core_kff_get_thumbnail_count (PyObject *, PyObject *);
PyObject *func_core_kff_has_size (PyObject *, PyObject *);
PyObject *func_core_kff_new_transaction (PyObject *, PyObject *);



#endif
