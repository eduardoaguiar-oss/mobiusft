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
// @brief  C++ API module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "module.hpp"
#include <pymobius.hpp>
#include "file.hpp"
#include "folder.hpp"
#include "line_reader.hpp"
#include "path.hpp"
#include "reader.hpp"
#include "sequential_reader_adaptor.hpp"
#include "stream.hpp"
#include "tempfile.hpp"
#include "text_reader.hpp"
#include "text_writer.hpp"
#include "uri.hpp"
#include "walker.hpp"
#include "writer.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *func_get_current_folder (PyObject *, PyObject *);
PyObject *func_io_new_slice_reader (PyObject *, PyObject *);
PyObject *func_join_path (PyObject *, PyObject *);
PyObject *func_new_entry_by_path (PyObject *, PyObject *);
PyObject *func_new_entry_by_url (PyObject *, PyObject *);
PyObject *func_new_file_by_path (PyObject *, PyObject *);
PyObject *func_new_file_by_url (PyObject *, PyObject *);
PyObject *func_new_folder_by_path (PyObject *, PyObject *);
PyObject *func_new_folder_by_url (PyObject *, PyObject *);
PyObject *func_new_path_from_win (PyObject *, PyObject *);
PyObject *func_new_uri_from_path (PyObject *, PyObject *);
PyObject *func_set_tempdir_path (PyObject *, PyObject *);
PyObject *func_to_win_path (PyObject *, PyObject *);

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module methods
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef module_methods[] =
{
  {"get_current_folder", func_get_current_folder, METH_VARARGS, "Get current folder"},
  {"join_path", func_join_path, METH_VARARGS, "Join paths"},
  {"new_entry_by_path", func_new_entry_by_path, METH_VARARGS, "Create entry by path"},
  {"new_entry_by_url", func_new_entry_by_url, METH_VARARGS, "Create entry by URL"},
  {"new_file_by_path", func_new_file_by_path, METH_VARARGS, "Create file by path"},
  {"new_file_by_url", func_new_file_by_url, METH_VARARGS, "Create file by URL"},
  {"new_folder_by_path", func_new_folder_by_path, METH_VARARGS, "Create folder by path"},
  {"new_folder_by_url", func_new_folder_by_url, METH_VARARGS, "Create folder by URL"},
  {"new_path_from_win", func_new_path_from_win, METH_VARARGS, "Create path from Win path"},
  {"new_slice_reader", func_io_new_slice_reader, METH_VARARGS, "Create slice reader"},
  {"new_uri_from_path", func_new_uri_from_path, METH_VARARGS, "Create URI from path"},
  {"set_tempdir_path", func_set_tempdir_path, METH_VARARGS, "Set temporary directory path"},
  {"to_win_path", func_to_win_path, METH_VARARGS, "Convert path to Win path string"},
  {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Module definition structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyModuleDef module_def =
{
  PyModuleDef_HEAD_INIT,
  "mobius.io",
  "Mobius Forensic Toolkit mobius.io module",
  -1,
  module_methods,
  nullptr,
  nullptr,
  nullptr,
  nullptr
};

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create module
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pymodule
new_io_module ()
{
  // Initialize module
  mobius::py::pymodule module (&module_def);

  // Add types
  module.add_type ("file", &io_file_t);
  module.add_type ("folder", &io_folder_t);
  module.add_type ("line_reader", &io_line_reader_t);
  module.add_type ("path", &io_path_t);
  module.add_type ("reader", &io_reader_t);
  module.add_type ("sequential_reader_adaptor", &io_sequential_reader_adaptor_t);
  module.add_type ("stream", &io_stream_t);
  module.add_type ("tempfile", new_io_tempfile_type ());
  module.add_type ("text_reader", &io_text_reader_t);
  module.add_type ("text_writer", &io_text_writer_t);
  module.add_type ("uri", &io_uri_t);
  module.add_type ("walker", new_io_walker_type ());
  module.add_type ("writer", &io_writer_t);

  // Return module
  return module;
}


