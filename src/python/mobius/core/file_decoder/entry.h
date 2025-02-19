#ifndef MOBIUS_PYTHON_CORE_FILE_DECODER_ENTRY_H
#define MOBIUS_PYTHON_CORE_FILE_DECODER_ENTRY_H

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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
#include <pytypeobject.h>
#include <mobius/core/file_decoder/entry.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Data structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
typedef struct
{
  PyObject_HEAD
  mobius::core::file_decoder::entry *obj;
} core_file_decoder_entry_o;


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject new_core_file_decoder_entry_type ();
bool pymobius_core_file_decoder_entry_check (PyObject *);
PyObject *pymobius_core_file_decoder_entry_to_pyobject (const mobius::core::file_decoder::entry&);
mobius::core::file_decoder::entry pymobius_core_file_decoder_entry_from_pyobject (PyObject *);

#endif
