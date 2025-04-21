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
//! \file qdatastream.cc C++ API <i>mobius.decoder.qdatastream</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <pymobius.hpp>
#include "qdatastream.hpp"
#include "module.hpp"
#include "io/reader.hpp"
#include "pod/data.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>eof</i> method implementation
// @param self object
// @param args argument list
// @return Boolean
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_eof (decoder_qdatastream_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (!bool (*self->obj));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qint8</i> method implementation
// @param self object
// @param args argument list
// @return 8-bits signed value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qint8 (decoder_qdatastream_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_int8_t (self->obj->get_qint8 ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qint16</i> method implementation
// @param self object
// @param args argument list
// @return 16-bits value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qint16 (decoder_qdatastream_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_int16_t (self->obj->get_qint16 ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qint32</i> method implementation
// @param self object
// @param args argument list
// @return 32-bits value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qint32 (decoder_qdatastream_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_int32_t (self->obj->get_qint32 ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qint64</i> method implementation
// @param self object
// @param args argument list
// @return 64-bits value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qint64 (decoder_qdatastream_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_int64_t (self->obj->get_qint64 ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_quint8</i> method implementation
// @param self object
// @param args argument list
// @return 8-bits value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_quint8 (decoder_qdatastream_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_uint8_t (self->obj->get_quint8 ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qstring</i> method implementation
// @param self object
// @param args argument list
// @return QString as std::string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qstring (decoder_qdatastream_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->get_qstring ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qbytearray</i> method implementation
// @param self object
// @param args argument list
// @return data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qbytearray (decoder_qdatastream_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybytes_from_bytearray (self->obj->get_qbytearray ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qdatetime</i> method implementation
// @param self object
// @param args argument list
// @return QDateTime as datetime
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qdatetime (decoder_qdatastream_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pydatetime_from_datetime (self->obj->get_qdatetime ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qimage</i> method implementation
// @param self object
// @param args argument list
// @return data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qimage (decoder_qdatastream_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybytes_from_bytearray (self->obj->get_qimage ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qpixmap</i> method implementation
// @param self object
// @param args argument list
// @return data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qpixmap (decoder_qdatastream_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybytes_from_bytearray (self->obj->get_qpixmap ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_qvariantap</i> method implementation
// @param self object
// @param args argument list
// @return data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_qvariant (decoder_qdatastream_o *self, PyObject *)
{
  // execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = pymobius_pod_data_to_pyobject (self->obj->get_qvariant ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] =
{
  {"eof", (PyCFunction) tp_f_eof, METH_VARARGS, "Return EOF indicator for stream"},
  {"get_qint8", (PyCFunction) tp_f_get_qint8, METH_VARARGS, "Get 8-bits signed value"},
  {"get_qint16", (PyCFunction) tp_f_get_qint16, METH_VARARGS, "Get 16-bits signed value"},
  {"get_qint32", (PyCFunction) tp_f_get_qint32, METH_VARARGS, "Get 32-bits signed value"},
  {"get_qint64", (PyCFunction) tp_f_get_qint64, METH_VARARGS, "Get 64-bits signed value"},
  {"get_quint8", (PyCFunction) tp_f_get_quint8, METH_VARARGS, "Get 8-bits unsigned value"},
  {"get_qstring", (PyCFunction) tp_f_get_qstring, METH_VARARGS, "Get QString"},
  {"get_qbytearray", (PyCFunction) tp_f_get_qbytearray, METH_VARARGS, "Get QBytearray"},
  {"get_qdatetime", (PyCFunction) tp_f_get_qdatetime, METH_VARARGS, "Get QDateTime"},
  {"get_qimage", (PyCFunction) tp_f_get_qimage, METH_VARARGS, "Get QImage data"},
  {"get_qpixmap", (PyCFunction) tp_f_get_qpixmap, METH_VARARGS, "Get QPixmap data"},
  {"get_qvariant", (PyCFunction) tp_f_get_qvariant, METH_VARARGS, "Get QVariant data"},
  {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>qdatastream</i> constructor
// @param type type object
// @param args argument list
// @param kwds keywords dict
// @return new <i>qdatastream</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *args, PyObject *)
{
  // Parse input args
  mobius::io::reader arg_reader;
  std::uint32_t version = mobius::decoder::qdatastream::QT_NEWEST;

  try
    {
      arg_reader = mobius::py::get_arg_as_cpp (args, 0, pymobius_io_reader_from_pyobject);
      version = mobius::py::get_arg_as_uint32_t (args, 1, mobius::decoder::qdatastream::QT_NEWEST);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_invalid_type_error (e.what ());
      return nullptr;
    }

  // Create Python object
  decoder_qdatastream_o *ret = reinterpret_cast <decoder_qdatastream_o *> (type->tp_alloc (type, 0));

  if (ret)
    {
      try
        {
          ret->obj = new mobius::decoder::qdatastream (arg_reader, version);
        }
      catch (const std::exception& e)
        {
          Py_DECREF (ret);
          mobius::py::set_runtime_error (e.what ());
          ret = nullptr;
        }
    }

  return reinterpret_cast <PyObject *> (ret);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>qdatastream</i> deallocator
// @param self object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (decoder_qdatastream_o *self)
{
  delete self->obj;
  Py_TYPE (self)->tp_free ((PyObject*) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyTypeObject decoder_qdatastream_t =
{
  PyVarObject_HEAD_INIT (NULL, 0)
  "mobius.decoder.qdatastream",            		// tp_name
  sizeof (decoder_qdatastream_o),          		// tp_basicsize
  0,                                       		// tp_itemsize
  (destructor) tp_dealloc,                 		// tp_dealloc
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
  "qdatastream class",                     		// tp_doc
  0,                                       		// tp_traverse
  0,                                       		// tp_clear
  0,                                       		// tp_richcompare
  0,                                       		// tp_weaklistoffset
  0,                                       		// tp_iter
  0,                                       		// tp_iternext
  tp_methods,                                 		// tp_methods
  0,                                       		// tp_members
  0,                                       		// tp_getset
  0,                                       		// tp_base
  0,                                       		// tp_dict
  0,                                       		// tp_descr_get
  0,                                       		// tp_descr_set
  0,                                       		// tp_dictoffset
  0,                                       		// tp_init
  0,                                       		// tp_alloc
  tp_new,                                  		// tp_new
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.decoder.qdatastream</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_decoder_qdatastream_type ()
{
  mobius::py::pytypeobject type (&decoder_qdatastream_t);
  type.create ();

  type.add_constant ("QT_1_0", mobius::decoder::qdatastream::QT_1_0);
  type.add_constant ("QT_2_0", mobius::decoder::qdatastream::QT_2_0);
  type.add_constant ("QT_2_1", mobius::decoder::qdatastream::QT_2_1);
  type.add_constant ("QT_3_0", mobius::decoder::qdatastream::QT_3_0);
  type.add_constant ("QT_3_1", mobius::decoder::qdatastream::QT_3_1);
  type.add_constant ("QT_3_3", mobius::decoder::qdatastream::QT_3_3);
  type.add_constant ("QT_4_0", mobius::decoder::qdatastream::QT_4_0);
  type.add_constant ("QT_4_1", mobius::decoder::qdatastream::QT_4_1);
  type.add_constant ("QT_4_2", mobius::decoder::qdatastream::QT_4_2);
  type.add_constant ("QT_4_3", mobius::decoder::qdatastream::QT_4_3);
  type.add_constant ("QT_4_4", mobius::decoder::qdatastream::QT_4_4);
  type.add_constant ("QT_4_5", mobius::decoder::qdatastream::QT_4_5);
  type.add_constant ("QT_4_6", mobius::decoder::qdatastream::QT_4_6);
  type.add_constant ("QT_4_7", mobius::decoder::qdatastream::QT_4_7);
  type.add_constant ("QT_4_8", mobius::decoder::qdatastream::QT_4_8);
  type.add_constant ("QT_4_9", mobius::decoder::qdatastream::QT_4_9);
  type.add_constant ("QT_5_0", mobius::decoder::qdatastream::QT_5_0);
  type.add_constant ("QT_5_1", mobius::decoder::qdatastream::QT_5_1);
  type.add_constant ("QT_5_2", mobius::decoder::qdatastream::QT_5_2);
  type.add_constant ("QT_5_3", mobius::decoder::qdatastream::QT_5_3);
  type.add_constant ("QT_5_4", mobius::decoder::qdatastream::QT_5_4);
  type.add_constant ("QT_5_5", mobius::decoder::qdatastream::QT_5_5);
  type.add_constant ("QT_5_6", mobius::decoder::qdatastream::QT_5_6);
  type.add_constant ("QT_5_7", mobius::decoder::qdatastream::QT_5_7);
  type.add_constant ("QT_5_8", mobius::decoder::qdatastream::QT_5_8);
  type.add_constant ("QT_5_9", mobius::decoder::qdatastream::QT_5_9);
  type.add_constant ("QT_5_10", mobius::decoder::qdatastream::QT_5_10);
  type.add_constant ("QT_5_11", mobius::decoder::qdatastream::QT_5_11);
  type.add_constant ("QT_5_12", mobius::decoder::qdatastream::QT_5_12);
  type.add_constant ("QT_5_13", mobius::decoder::qdatastream::QT_5_13);
  type.add_constant ("QT_5_14", mobius::decoder::qdatastream::QT_5_14);
  type.add_constant ("QT_5_15", mobius::decoder::qdatastream::QT_5_15);
  type.add_constant ("QT_6_0", mobius::decoder::qdatastream::QT_6_0);
  type.add_constant ("QT_NEWEST", mobius::decoder::qdatastream::QT_NEWEST);

  return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create <i>qdatastream</i> Python object from C++ object
// @param obj C++ object
// @return new qdatastream object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_decoder_qdatastream_to_pyobject (const mobius::decoder::qdatastream& obj)
{
  PyObject *ret = _PyObject_New (&decoder_qdatastream_t);

  if (ret)
    ((decoder_qdatastream_o *) ret)->obj = new mobius::decoder::qdatastream (obj);

  return ret;
}




