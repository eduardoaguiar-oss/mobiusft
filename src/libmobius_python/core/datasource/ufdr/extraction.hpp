#ifndef LIBMOBIUS_PYTHON_CORE_DATASOURCE_UFDR_EXTRACTION_HPP
#define LIBMOBIUS_PYTHON_CORE_DATASOURCE_UFDR_EXTRACTION_HPP

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
#include <Python.h>
#include <mobius/core/datasource/ufdr/extraction.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Data structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
typedef struct
{
    PyObject_HEAD mobius::core::datasource::ufdr::extraction *obj;
} core_datasource_ufdr_extraction_o;

extern PyTypeObject core_datasource_ufdr_extraction_t;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Helper functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool pymobius_core_datasource_ufdr_extraction_check (PyObject *);
PyObject *pymobius_core_datasource_ufdr_extraction_to_pyobject (
    const mobius::core::datasource::ufdr::extraction &);
mobius::core::datasource::ufdr::extraction
pymobius_core_datasource_ufdr_extraction_from_pyobject (PyObject *);

#endif
