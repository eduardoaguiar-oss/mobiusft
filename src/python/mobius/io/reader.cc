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
//! \brief C++ API module wrapper
//! \author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <pymobius.h>
#include <pygil.h>
#include "reader.h"
#include <mobius/exception.inc>
#include <mobius/io/bytearray_io.h>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Check if object type is <i>reader</i>
//! \param pyobj Python object
//! \return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_io_reader_check (PyObject *pyobj)
{
  return PyObject_IsInstance (pyobj, (PyObject *) &io_reader_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief create new object from C++ object
//! \param obj C++ object
//! \return new object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_io_reader_to_pyobject (mobius::io::reader obj)
{
  PyObject *ret = nullptr;

  if (obj)
    {
      ret = _PyObject_New (&io_reader_t);

      if (ret)
        ((io_reader_o *) ret)->obj = new mobius::io::reader (obj);
    }
  else
    {
      ret = mobius::py::pynone ();
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Create <i>reader</i> C++ object from Python object
//! \param pyobj Python object
//! \return reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::io::reader
pymobius_io_reader_from_pyobject (PyObject *pyobj)
{
  mobius::io::reader reader;

  if (pymobius_io_reader_check (pyobj))
    reader = * (reinterpret_cast <io_reader_o *>(pyobj)->obj);

  else if (mobius::py::pybytes_check (pyobj))
    {
      auto bytearray = mobius::py::pybytes_as_bytearray (pyobj);
      reader = mobius::io::new_bytearray_reader (bytearray);
    }

  else if (mobius::py::pystring_check (pyobj))
    {
      mobius::bytearray data (mobius::py::pystring_as_std_string (pyobj));
      reader = mobius::io::new_bytearray_reader (data);
    }

  else
    throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG ("argument must be either reader or bytes"));

  return reader;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief is_seekable getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_is_seekable (io_reader_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->is_seekable ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief is_rewindable getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_is_rewindable (io_reader_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->is_rewindable ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief size getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_size (io_reader_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_uint64_t (self->obj->get_size ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief getters and setters structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyGetSetDef tp_getset[] =
{
  {
    (char *) "is_seekable",
    (getter) tp_getter_is_seekable,
    (setter) 0,
    (char *) "check if reader is seekable", NULL
  },
  {
    (char *) "is_rewindable",
    (getter) tp_getter_is_rewindable,
    (setter) 0,
    (char *) "check if reader is rewindable", NULL
  },
  {
    (char *) "size",
    (getter) tp_getter_size,
    (setter) 0,
    (char *) "data size in bytes", NULL
  },
  {NULL, NULL, NULL, NULL, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Read method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_read (io_reader_o *self, PyObject *args)
{
  std::uint64_t arg_size = 0;

  // parse input args
  try
    {
      arg_size = mobius::py::get_arg_as_uint64_t (
                   args,
                   0,
                   self->obj->get_size () - self->obj->tell ()
                 );
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybytes_from_bytearray (
               mobius::py::GIL () (
                  self->obj->read (arg_size)
               )
            );
    }
  catch (const std::exception& e)
    {
      mobius::py::set_io_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief tell method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_tell (io_reader_o *self, PyObject *)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_int64_t (self->obj->tell ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief seek method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_seek (io_reader_o *self, PyObject *args)
{
  // parse input args
  std::int64_t arg_offset;
  std::uint32_t arg_whence = 0;

  try
    {
      arg_offset = mobius::py::get_arg_as_int64_t (args, 0);
      arg_whence = mobius::py::get_arg_as_uint32_t (args, 1, 0);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // convert 'whence' argument
  mobius::io::reader::whence_type w;

  if (arg_whence == 0)
    w = mobius::io::reader::whence_type::beginning;

  else if (arg_whence == 1)
    w = mobius::io::reader::whence_type::current;

  else if (arg_whence == 2)
    w = mobius::io::reader::whence_type::end;

  else
    {
      mobius::py::set_value_error ("invalid 'whence' argument");
      return nullptr;
    }

  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      self->obj->seek (arg_offset, w);
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_io_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief rewind method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_rewind (io_reader_o *self, PyObject *)
{
  PyObject *ret = nullptr;

  try
    {
      self->obj->rewind ();
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief skip method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_skip (io_reader_o *self, PyObject *args)
{
  // parse input args
  std::uint64_t arg_size;

  try
    {
      arg_size = mobius::py::get_arg_as_uint64_t (args, 0);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      self->obj->skip (arg_size);
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief <b>get_block_size</b> method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_block_size (io_reader_o *self, PyObject *)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_uint64_t (self->obj->tell ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] =
{
  {
    (char *) "read",
    (PyCFunction) tp_f_read,
    METH_VARARGS,
    "Read bytes from reader"
  },
  {
    (char *) "tell",
    (PyCFunction) tp_f_tell,
    METH_VARARGS,
    "Get current read position"
  },
  {
    (char *) "seek",
    (PyCFunction) tp_f_seek,
    METH_VARARGS,
    "Set current read position"
  },
  {
    (char *) "rewind",
    (PyCFunction) tp_f_rewind,
    METH_VARARGS,
    "Set current read position to the beginning of data"
  },
  {
    (char *) "skip",
    (PyCFunction) tp_f_skip,
    METH_VARARGS,
    "Set read position n bytes ahead"
  },
  {
    (char *) "get_block_size",
    (PyCFunction) tp_f_get_block_size,
    METH_VARARGS,
    "Get optimum block size"
  },
  {NULL, NULL, 0, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief tp_alloc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
io_reader_o *
io_reader_tp_alloc ()
{
  return (io_reader_o *) io_reader_t.tp_alloc (&io_reader_t, 0);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief tp_dealloc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (io_reader_o *self)
{
  delete self->obj;
  Py_TYPE (self)->tp_free ((PyObject*) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject io_reader_t =
{
  PyVarObject_HEAD_INIT (NULL, 0)
  "mobius.io.reader",               	           	// tp_name
  sizeof (io_reader_o),              	          	// tp_basicsize
  0,                                       		// tp_itemsize
  (destructor) tp_dealloc,  	                   	// tp_dealloc
  0,                                       		// tp_print
  0,                                       		// tp_getattr
  0,                                       		// tp_setattr
  0,                                       		// tp_compare
  0,                                       		// tp_repr
  0,                                       		// tp_as_number
  0,                                       		// tp_as_sequence
  0,                                       		// tp_as_mapping
  0,                                       		// tp_hash
  0,                                       		// tp_call
  0,                                       		// tp_str
  0,                                       		// tp_getattro
  0,                                       		// tp_setattro
  0,                                       		// tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,		// tp_flags
  "reader for mobius.io.file",       	          	// tp_doc
  0,                                       		// tp_traverse
  0,                                       		// tp_clear
  0,                                       		// tp_richcompare
  0,                                       		// tp_weaklistoffset
  0,                                       		// tp_iter
  0,                                       		// tp_iternext
  tp_methods,                                  		// tp_methods
  0,                                       		// tp_members
  tp_getset,                	                   	// tp_getset
  0,                             	              	// tp_base
  0,                                       		// tp_dict
  0,                                       		// tp_descr_get
  0,                                       		// tp_descr_set
  0,                                       		// tp_dictoffset
  0,                                       		// tp_init
  0,                                       		// tp_alloc
  0,                                    		// tp_new
  0,                                       		// tp_free
  0,                                       		// tp_is_gc
  0,                                       		// tp_bases
  0,                                       		// tp_mro
  0,                                       		// tp_cache
  0,                                       		// tp_subclasses
  0,                                       		// tp_weaklist
  0,                                       		// tp_del
  0,                                       		// tp_version_tag
  0,                                       		// tp_finalize
};
