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
// @brief C++ API module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <pymobius.h>
#include <pylist.h>
#include "folder.h"
#include "file.h"
#include "entry.h"
#include "stream.h"
#include "pod/data.h"
#include "pod/map.h"
#include <mobius/exception.inc>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if object type is <i>folder</i>
// @param pyobj Python object
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_io_folder_check (PyObject *pyobj)
{
  return PyObject_IsInstance (pyobj, (PyObject *) &io_folder_t);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new object from C++ object
// @param obj C++ object
// @return new object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_io_folder_to_pyobject (mobius::io::folder obj)
{
  PyObject *ret = nullptr;

  if (obj)
    {
      ret = _PyObject_New (&io_folder_t);

      if (ret)
        ((io_folder_o *) ret)->obj = new mobius::io::folder (obj);
    }
  else
    {
      ret = mobius::py::pynone ();
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>folder</i> C++ object from Python object
// @param pyobj Python object
// @return folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::io::folder
pymobius_io_folder_from_pyobject (PyObject *pyobj)
{
  if (!PyObject_IsInstance (pyobj, (PyObject *) &io_folder_t))
    throw std::invalid_argument (mobius::MOBIUS_EXCEPTION_MSG ("object type must be mobius.io.folder"));

  return * (reinterpret_cast <io_folder_o *>(pyobj)->obj);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief name getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_name (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->get_name ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief name setter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static int
tp_setter_name (io_folder_o *self, PyObject *value, void *)
{
  if (value == nullptr)
    {
      mobius::py::set_invalid_type_error ("cannot delete 'name' attribute");
      return -1;
    }

  if (!mobius::py::pystring_check (value))
    {
      mobius::py::set_invalid_type_error ("invalid type for 'name' attribute");
      return -1;
    }

  // set path
  try
    {
      self->obj->set_name (mobius::py::pystring_as_std_string (value));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
      return -1;
    }

  return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief short_name getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_short_name (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->get_short_name ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief path getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_path (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->get_path ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief path setter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static int
tp_setter_path (io_folder_o *self, PyObject *value, void *)
{
  if (value == nullptr)
    {
      mobius::py::set_invalid_type_error ("cannot delete 'path' attribute");
      return -1;
    }

  if (!mobius::py::pystring_check (value))
    {
      mobius::py::set_invalid_type_error ("invalid type for 'path' attribute");
      return -1;
    }

  // set path
  try
    {
      self->obj->set_path (mobius::py::pystring_as_std_string (value));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
      return -1;
    }

  return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief inode getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_inode (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_std_uint64_t (self->obj->get_inode ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief size getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_size (io_folder_o *self)
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
// @brief user_id getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_user_id (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_int (static_cast <int> (self->obj->get_user_id ()));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief user_name getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_user_name (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->get_user_name ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief group_id getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_group_id (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_int (static_cast <int> (self->obj->get_group_id ()));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief group_name getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_group_name (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->get_group_name ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief permissions getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_permissions (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylong_from_int (static_cast <int> (self->obj->get_permissions ()));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief access_time getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_access_time (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pydatetime_from_datetime (self->obj->get_access_time ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief modification_time getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_modification_time (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pydatetime_from_datetime (self->obj->get_modification_time ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief metadata_time getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_metadata_time (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pydatetime_from_datetime (self->obj->get_metadata_time ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief creation_time getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_creation_time (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pydatetime_from_datetime (self->obj->get_creation_time ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief deletion_time getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_deletion_time (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pydatetime_from_datetime (self->obj->get_deletion_time ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief backup_time getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_backup_time (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pydatetime_from_datetime (self->obj->get_backup_time ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief getters and setters structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyGetSetDef tp_getset[] =
{
  {
    (char *) "name",
    (getter) tp_getter_name,
    (setter) tp_setter_name,
    (char *) "file name", NULL
  },
  {
    (char *) "short_name",
    (getter) tp_getter_short_name,
    (setter) 0,
    (char *) "short file name", NULL
  },
  {
    (char *) "path",
    (getter) tp_getter_path,
    (setter) tp_setter_path,
    (char *) "path", NULL
  },
  {
    (char *) "inode",
    (getter) tp_getter_inode,
    (setter) 0,
    (char *) "inode", NULL
  },
  {
    (char *) "size",
    (getter) tp_getter_size,
    (setter) 0,
    (char *) "folder size in bytes", NULL
  },
  {
    (char *) "user_id",
    (getter) tp_getter_user_id,
    (setter) 0,
    (char *) "owner's user ID", NULL
  },
  {
    (char *) "user_name",
    (getter) tp_getter_user_name,
    (setter) 0,
    (char *) "owner's user name", NULL
  },
  {
    (char *) "group_id",
    (getter) tp_getter_group_id,
    (setter) 0,
    (char *) "group ID", NULL
  },
  {
    (char *) "group_name",
    (getter) tp_getter_group_name,
    (setter) 0,
    (char *) "group name", NULL
  },
  {
    (char *) "permissions",
    (getter) tp_getter_permissions,
    (setter) 0,
    (char *) "access permission mask", NULL
  },
  {
    (char *) "access_time",
    (getter) tp_getter_access_time,
    (setter) 0,
    (char *) "last access date/time", NULL
  },
  {
    (char *) "modification_time",
    (getter) tp_getter_modification_time,
    (setter) 0,
    (char *) "last data modification date/time", NULL
  },
  {
    (char *) "metadata_time",
    (getter) tp_getter_metadata_time,
    (setter) 0,
    (char *) "last metadata modification date/time", NULL
  },
  {
    (char *) "creation_time",
    (getter) tp_getter_creation_time,
    (setter) 0,
    (char *) "creation date/time", NULL
  },
  {
    (char *) "deletion_time",
    (getter) tp_getter_deletion_time,
    (setter) 0,
    (char *) "deletion date/time", NULL
  },
  {
    (char *) "backup_time",
    (getter) tp_getter_backup_time,
    (setter) 0,
    (char *) "backup date/time", NULL
  },
  {NULL, NULL, NULL, NULL, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief exists method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_exists (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->exists ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief is_deleted method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_deleted (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->is_deleted ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief is_reallocated method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_reallocated (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->is_reallocated ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief is_hidden method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_hidden (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->is_hidden ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief is_browseable method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_browseable (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (self->obj->is_browseable ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief is_folder method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_folder (io_folder_o *)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (true);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief is_file method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_file (io_folder_o *)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pybool_from_bool (false);
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get_extension method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_extension (io_folder_o *self)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pystring_from_std_string (self->obj->get_extension ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get_parent method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_parent (io_folder_o *self, PyObject *)
{
  PyObject *ret = nullptr;

  try
    {
      ret = pymobius_io_folder_to_pyobject (self->obj->get_parent ());
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get_children method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_children (io_folder_o *self, PyObject *)
{
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylist_from_cpp_container (
               self->obj->get_children (),
               pymobius_io_entry_to_pyobject
            );
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get_child_by_name method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_child_by_name (io_folder_o *self, PyObject *args)
{
  // parse input args
  std::string arg_name;
  bool arg_case_sensitive;

  try
    {
      arg_name = mobius::py::get_arg_as_std_string (args, 0);
      arg_case_sensitive = mobius::py::get_arg_as_bool (args, 1, true);
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
      ret = pymobius_io_entry_to_pyobject (
              self->obj->get_child_by_name (arg_name, arg_case_sensitive)
            );
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get_child_by_path method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_child_by_path (io_folder_o *self, PyObject *args)
{
  // parse input args
  std::string arg_name;
  bool arg_case_sensitive;

  try
    {
      arg_name = mobius::py::get_arg_as_std_string (args, 0);
      arg_case_sensitive = mobius::py::get_arg_as_bool (args, 1, true);
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
      ret = pymobius_io_entry_to_pyobject (
              self->obj->get_child_by_path (arg_name, arg_case_sensitive)
            );
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get_children_by_name method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_children_by_name (io_folder_o *self, PyObject *args)
{
  // parse input args
  std::string arg_name;
  bool arg_case_sensitive;

  try
    {
      arg_name = mobius::py::get_arg_as_std_string (args, 0);
      arg_case_sensitive = mobius::py::get_arg_as_bool (args, 1, true);
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
      ret = mobius::py::pylist_from_cpp_container (
               self->obj->get_children_by_name (arg_name, arg_case_sensitive),
               pymobius_io_entry_to_pyobject
            );
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief new_file method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_new_file (io_folder_o *self, PyObject *args)
{
  // parse input args
  std::string arg_name;

  try
    {
      arg_name = mobius::py::get_arg_as_std_string (args, 0);
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
      ret = pymobius_io_file_to_pyobject (self->obj->new_file (arg_name));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief new_folder method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_new_folder (io_folder_o *self, PyObject *args)
{
  // parse input args
  std::string arg_name;

  try
    {
      arg_name = mobius::py::get_arg_as_std_string (args, 0);
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
      ret = pymobius_io_folder_to_pyobject (self->obj->new_folder (arg_name));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_create (io_folder_o *self, PyObject *)
{
  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      self->obj->create ();
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief clear folder content
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_clear (io_folder_o *self, PyObject *)
{
  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      self->obj->clear ();
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief reload method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_reload (io_folder_o *self, PyObject *)
{
  PyObject *ret = nullptr;

  try
    {
      self->obj->reload ();
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief copy method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_copy (io_folder_o *self, PyObject *args)
{
  // parse input args
  mobius::io::folder arg_dst;

  try
    {
      arg_dst = pymobius_io_folder_from_pyobject (mobius::py::get_arg (args, 0));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_value_error (e.what ());
      return nullptr;
    }

  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      self->obj->copy (arg_dst);
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief move method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_move (io_folder_o *self, PyObject *args)
{
  // parse input args
  mobius::io::folder arg_dst;

  try
    {
      arg_dst = pymobius_io_folder_from_pyobject (mobius::py::get_arg (args, 0));
    }
  catch (const std::exception& e)
    {
      mobius::py::set_value_error (e.what ());
      return nullptr;
    }

  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      self->obj->move (arg_dst);
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief remove folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_remove (io_folder_o *self, PyObject *)
{
  // execute C++ code
  PyObject *ret = nullptr;

  try
    {
      self->obj->remove ();
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief rename method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_rename (io_folder_o *self, PyObject *args)
{
  // parse input args
  std::string arg_name;

  try
    {
      arg_name = mobius::py::get_arg_as_std_string (args, 0);
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
      self->obj->rename (arg_name);
      ret = mobius::py::pynone ();
    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_streams</i> method implementation
// @param self Object
// @param args Argument list
// @return Streams
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_streams (io_folder_o *self, PyObject *)
{
  // Execute C++ function
  PyObject *ret = nullptr;

  try
    {
      ret = mobius::py::pylist_from_cpp_container (
               self->obj->get_streams (),
               pymobius_io_stream_to_pyobject
            );

    }
  catch (const std::exception& e)
    {
      mobius::py::set_runtime_error (e.what ());
    }

  // Return value
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] =
{
  {
    (char *) "exists",
    (PyCFunction) tp_f_exists,
    METH_VARARGS,
    "Check if folder exists"
  },
  {
    (char *) "is_deleted",
    (PyCFunction) tp_f_is_deleted,
    METH_VARARGS,
    "Check if folder is deleted"
  },
  {
    (char *) "is_reallocated",
    (PyCFunction) tp_f_is_reallocated,
    METH_VARARGS,
    "Check if folder is reallocated"
  },
  {
    (char *) "is_hidden",
    (PyCFunction) tp_f_is_hidden,
    METH_VARARGS,
    "Check if folder is hidden"
  },
  {
    (char *) "is_browseable",
    (PyCFunction) tp_f_is_browseable,
    METH_VARARGS,
    "Check if folder is browseable"
  },
  {
    (char *) "is_folder",
    (PyCFunction) tp_f_is_folder,
    METH_VARARGS,
    "Check if entry is a folder"
  },
  {
    (char *) "is_file",
    (PyCFunction) tp_f_is_file,
    METH_VARARGS,
    "Check if entry is a file"
  },
  {
    (char *) "get_extension",
    (PyCFunction) tp_f_get_extension,
    METH_VARARGS,
    "Get folder extension"
  },
  {
    (char *) "get_parent",
    (PyCFunction) tp_f_get_parent,
    METH_VARARGS,
    "Get parent folder"
  },
  {
    (char *) "get_children",
    (PyCFunction) tp_f_get_children,
    METH_VARARGS,
    "Get children"
  },
  {
    (char *) "get_child_by_name",
    (PyCFunction) tp_f_get_child_by_name,
    METH_VARARGS,
    "Get child by name"
  },
  {
    (char *) "get_child_by_path",
    (PyCFunction) tp_f_get_child_by_path,
    METH_VARARGS,
    "Get child by path"
  },
  {
    (char *) "get_children_by_name",
    (PyCFunction) tp_f_get_children_by_name,
    METH_VARARGS,
    "Get children by name"
  },
  {
    (char *) "new_file",
    (PyCFunction) tp_f_new_file,
    METH_VARARGS,
    "Create new file object"
  },
  {
    (char *) "new_folder",
    (PyCFunction) tp_f_new_folder,
    METH_VARARGS,
    "Create new folder object"
  },
  {
    (char *) "create",
    (PyCFunction) tp_f_create,
    METH_VARARGS,
    "Create folder"
  },
  {
    (char *) "clear",
    (PyCFunction) tp_f_clear,
    METH_VARARGS,
    "Clear folder content"
  },
  {
    (char *) "reload",
    (PyCFunction) tp_f_reload,
    METH_VARARGS,
    "Reload folder info"
  },
  {
    (char *) "copy",
    (PyCFunction) tp_f_copy,
    METH_VARARGS,
    "Copy folder"
  },
  {
    (char *) "move",
    (PyCFunction) tp_f_move,
    METH_VARARGS,
    "Move folder"
  },
  {
    (char *) "remove",
    (PyCFunction) tp_f_remove,
    METH_VARARGS,
    "Remove folder"
  },
  {
    (char *) "rename",
    (PyCFunction) tp_f_rename,
    METH_VARARGS,
    "Rename folder"
  },
  {
    (char *) "get_streams",
    (PyCFunction) tp_f_get_streams,
    METH_VARARGS,
    "Get streams"
  },
  {NULL, NULL, 0, NULL} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief tp_dealloc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (io_folder_o *self)
{
  delete self->obj;
  Py_TYPE (self)->tp_free ((PyObject*) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject io_folder_t =
{
  PyVarObject_HEAD_INIT (NULL, 0)
  "mobius.io.folder",               	           	// tp_name
  sizeof (io_folder_o),              	          	// tp_basicsize
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
  "folder class with support for URL's",       		// tp_doc
  0,                                       		// tp_traverse
  0,                                       		// tp_clear
  0,                                       		// tp_richcompare
  0,                                       		// tp_weaklistoffset
  0,                                       		// tp_iter
  0,                                       		// tp_iternext
  tp_methods,                                 		// tp_methods
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


