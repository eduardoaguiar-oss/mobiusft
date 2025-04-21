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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \file C++ API <i>mobius.decoder</i> module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "module.h"
#include <pymobius.h>
#include "data_decoder.h"
#include "inifile.h"
#include "lnk.h"
#include "mfc.h"
#include "qdatastream.h"
#include "sourcecode.h"
#include "tdf.h"
#include "sgml/module.h"
#include "xml/module.h"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *decoder_func_base16 (PyObject *, PyObject *);
PyObject *decoder_func_base32 (PyObject *, PyObject *);
PyObject *decoder_func_base32hex (PyObject *, PyObject *);
PyObject *decoder_func_base64 (PyObject *, PyObject *);
PyObject *decoder_func_base64url (PyObject *, PyObject *);
PyObject *decoder_func_btencode (PyObject *, PyObject *);
PyObject *decoder_func_get_filetype (PyObject *, PyObject *);

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module methods
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef module_methods[] =
{
  {"base16", decoder_func_base16, METH_VARARGS, "Decode base16 string"},
  {"base32", decoder_func_base32, METH_VARARGS, "Decode base32 string"},
  {"base32hex", decoder_func_base32hex, METH_VARARGS, "Decode base32hex string"},
  {"base64", decoder_func_base64, METH_VARARGS, "Decode base64 string"},
  {"base64url", decoder_func_base64url, METH_VARARGS, "Decode base64url string"},
  {"btencode", decoder_func_btencode, METH_VARARGS, "Decode BTencode data"},
  {"get_filetype", decoder_func_get_filetype, METH_VARARGS, "Get filetype from file content"},
  {nullptr,nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module definition structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyModuleDef module_def =
{
  PyModuleDef_HEAD_INIT,
  "mobius.decoder",
  "Mobius Forensic Toolkit mobius.decoder module",
  -1,
  module_methods,
  nullptr,
  nullptr,
  nullptr,
  nullptr
};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create mobius.decoder module
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pymodule
new_decoder_module ()
{
  // Initialize module
  mobius::py::pymodule module (&module_def);

  // Add types
  module.add_type ("data_decoder", &decoder_data_decoder_t);
  module.add_type ("inifile", new_decoder_inifile_type ());
  module.add_type ("lnk", &decoder_lnk_t);
  module.add_type ("mfc", &decoder_mfc_t);
  module.add_type ("qdatastream", new_decoder_qdatastream_type ());
  module.add_type ("sourcecode", &decoder_sourcecode_t);
  module.add_type ("tdf", &decoder_tdf_t);

  // Build submodules
  module.add_submodule ("sgml", new_decoder_sgml_module ());
  module.add_submodule ("xml", new_decoder_xml_module ());

  // Return module
  return module;
}




