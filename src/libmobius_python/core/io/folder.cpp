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
// @brief C++ API module wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "folder.hpp"
#include <mobius/core/exception.inc>
#include <pygil.hpp>
#include <pylist.hpp>
#include <pymobius.hpp>
#include <stdexcept>
#include "core/pod/data.hpp"
#include "core/pod/map.hpp"
#include "entry.hpp"
#include "file.hpp"
#include "stream.hpp"

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_io_folder_type = nullptr;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief name getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_name (core_io_folder_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->get_name ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief name setter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static int
tp_setter_name (core_io_folder_o *self, PyObject *value, void *)
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
    catch (const std::exception &e)
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
tp_getter_short_name (core_io_folder_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_short_name ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief path getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_path (core_io_folder_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->get_path ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief path setter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static int
tp_setter_path (core_io_folder_o *self, PyObject *value, void *)
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
    catch (const std::exception &e)
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
tp_getter_inode (core_io_folder_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->get_inode ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief size getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_size (core_io_folder_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->get_size ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief user_id getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_user_id (core_io_folder_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_int (static_cast<int> (self->obj->get_user_id ()));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief user_name getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_user_name (core_io_folder_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_user_name ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief group_id getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_group_id (core_io_folder_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_int (static_cast<int> (self->obj->get_group_id ()));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief group_name getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_group_name (core_io_folder_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_group_name ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief permissions getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_permissions (core_io_folder_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_int (static_cast<int> (self->obj->get_permissions ()));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief access_time getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_access_time (core_io_folder_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->get_access_time ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief modification_time getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_modification_time (core_io_folder_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->get_modification_time ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief metadata_time getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_metadata_time (core_io_folder_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->get_metadata_time ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief creation_time getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_creation_time (core_io_folder_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->get_creation_time ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief deletion_time getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_deletion_time (core_io_folder_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->get_deletion_time ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief backup_time getter
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_backup_time (core_io_folder_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->get_backup_time ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief getters and setters structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyGetSetDef tp_getset[] = {
    {"name", (getter) tp_getter_name, (setter) tp_setter_name, "file name", nullptr},
    {"short_name", (getter) tp_getter_short_name, (setter) 0, "short file name", nullptr},
    {"path", (getter) tp_getter_path, (setter) tp_setter_path, "path", nullptr},
    {"inode", (getter) tp_getter_inode, (setter) 0, "inode", nullptr},
    {"size", (getter) tp_getter_size, (setter) 0, "folder size in bytes", nullptr},
    {"user_id", (getter) tp_getter_user_id, (setter) 0, "owner's user ID", nullptr},
    {"user_name", (getter) tp_getter_user_name, (setter) 0, "owner's user name", nullptr},
    {"group_id", (getter) tp_getter_group_id, (setter) 0, "group ID", nullptr},
    {"group_name", (getter) tp_getter_group_name, (setter) 0, "group name", nullptr},
    {"permissions", (getter) tp_getter_permissions, (setter) 0, "access permission mask", nullptr},
    {"access_time", (getter) tp_getter_access_time, (setter) 0, "last access date/time", nullptr},
    {"modification_time",
     (getter) tp_getter_modification_time,
     (setter) 0,
     "last data modification date/time",
     nullptr},
    {"metadata_time", (getter) tp_getter_metadata_time, (setter) 0, "last metadata modification date/time", nullptr},
    {"creation_time", (getter) tp_getter_creation_time, (setter) 0, "creation date/time", nullptr},
    {"deletion_time", (getter) tp_getter_deletion_time, (setter) 0, "deletion date/time", nullptr},
    {"backup_time", (getter) tp_getter_backup_time, (setter) 0, "backup date/time", nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief exists method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_exists (core_io_folder_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (mobius::py::GIL () (self->obj->exists ()));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief is_deleted method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_deleted (core_io_folder_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->is_deleted ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief is_reallocated method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_reallocated (core_io_folder_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->is_reallocated ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief is_hidden method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_hidden (core_io_folder_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->is_hidden ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief is_browseable method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_browseable (core_io_folder_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (self->obj->is_browseable ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief is_folder method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_folder (core_io_folder_o *, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (true);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief is_file method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_file (core_io_folder_o *, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::to_pyobject (false);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get_extension method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_extension (core_io_folder_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_extension ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get_parent method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_parent (core_io_folder_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_io_folder_to_pyobject (self->obj->get_parent ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get_children method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_children (core_io_folder_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylist_from_cpp_container (
            mobius::py::GIL () (self->obj->get_children ()),
            pymobius_core_io_entry_to_pyobject
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get_child_by_name method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_child_by_name (core_io_folder_o *self, PyObject *args)
{
    // parse input args
    std::string arg_name;
    bool arg_case_sensitive;

    try
    {
        arg_name = mobius::py::get_arg_as_std_string (args, 0);
        arg_case_sensitive = mobius::py::get_arg_as_bool (args, 1, true);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_io_entry_to_pyobject (
            mobius::py::GIL () (self->obj->get_child_by_name (arg_name, arg_case_sensitive))
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get_child_by_path method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_child_by_path (core_io_folder_o *self, PyObject *args)
{
    // parse input args
    std::string arg_name;
    bool arg_case_sensitive;

    try
    {
        arg_name = mobius::py::get_arg_as_std_string (args, 0);
        arg_case_sensitive = mobius::py::get_arg_as_bool (args, 1, true);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_io_entry_to_pyobject (
            mobius::py::GIL () (self->obj->get_child_by_path (arg_name, arg_case_sensitive))
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get_children_by_name method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_children_by_name (core_io_folder_o *self, PyObject *args)
{
    // parse input args
    std::string arg_name;
    bool arg_case_sensitive;

    try
    {
        arg_name = mobius::py::get_arg_as_std_string (args, 0);
        arg_case_sensitive = mobius::py::get_arg_as_bool (args, 1, true);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylist_from_cpp_container (
            mobius::py::GIL () (self->obj->get_children_by_name (arg_name, arg_case_sensitive)),
            pymobius_core_io_entry_to_pyobject
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief new_file method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_new_file (core_io_folder_o *self, PyObject *args)
{
    // parse input args
    std::string arg_name;

    try
    {
        arg_name = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_io_file_to_pyobject (self->obj->new_file (arg_name));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief new_folder method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_new_folder (core_io_folder_o *self, PyObject *args)
{
    // parse input args
    std::string arg_name;

    try
    {
        arg_name = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_io_folder_to_pyobject (self->obj->new_folder (arg_name));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief create folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_create (core_io_folder_o *self, PyObject *)
{
    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        mobius::py::GIL gil;
        self->obj->create ();
        ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief clear folder content
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_clear (core_io_folder_o *self, PyObject *)
{
    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        mobius::py::GIL gil;
        self->obj->clear ();
        ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief reload method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_reload (core_io_folder_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        mobius::py::GIL gil;
        self->obj->reload ();
        ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief copy method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_copy (core_io_folder_o *self, PyObject *args)
{
    // parse input args
    mobius::core::io::folder arg_dst;

    try
    {
        arg_dst = pymobius_core_io_folder_from_pyobject (mobius::py::get_arg (args, 0));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_value_error (e.what ());
        return nullptr;
    }

    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        mobius::py::GIL gil;
        self->obj->copy (arg_dst);
        ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief move method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_move (core_io_folder_o *self, PyObject *args)
{
    // parse input args
    mobius::core::io::folder arg_dst;

    try
    {
        arg_dst = pymobius_core_io_folder_from_pyobject (mobius::py::get_arg (args, 0));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_value_error (e.what ());
        return nullptr;
    }

    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        mobius::py::GIL gil;
        self->obj->move (arg_dst);
        ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief remove folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_remove (core_io_folder_o *self, PyObject *)
{
    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        mobius::py::GIL gil;
        self->obj->remove ();
        ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief rename method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_rename (core_io_folder_o *self, PyObject *args)
{
    // parse input args
    std::string arg_name;

    try
    {
        arg_name = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        mobius::py::GIL gil;
        self->obj->rename (arg_name);
        ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
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
tp_f_get_streams (core_io_folder_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylist_from_cpp_container (
            mobius::py::GIL () (self->obj->get_streams ()),
            pymobius_core_io_stream_to_pyobject
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] = {
    {"exists", (PyCFunction) tp_f_exists, METH_VARARGS, "Check if folder exists"},
    {"is_deleted", (PyCFunction) tp_f_is_deleted, METH_VARARGS, "Check if folder is deleted"},
    {"is_reallocated", (PyCFunction) tp_f_is_reallocated, METH_VARARGS, "Check if folder is reallocated"},
    {"is_hidden", (PyCFunction) tp_f_is_hidden, METH_VARARGS, "Check if folder is hidden"},
    {"is_browseable", (PyCFunction) tp_f_is_browseable, METH_VARARGS, "Check if folder is browseable"},
    {"is_folder", (PyCFunction) tp_f_is_folder, METH_VARARGS, "Check if entry is a folder"},
    {"is_file", (PyCFunction) tp_f_is_file, METH_VARARGS, "Check if entry is a file"},
    {"get_extension", (PyCFunction) tp_f_get_extension, METH_VARARGS, "Get folder extension"},
    {"get_parent", (PyCFunction) tp_f_get_parent, METH_VARARGS, "Get parent folder"},
    {"get_children", (PyCFunction) tp_f_get_children, METH_VARARGS, "Get children"},
    {"get_child_by_name", (PyCFunction) tp_f_get_child_by_name, METH_VARARGS, "Get child by name"},
    {"get_child_by_path", (PyCFunction) tp_f_get_child_by_path, METH_VARARGS, "Get child by path"},
    {"get_children_by_name", (PyCFunction) tp_f_get_children_by_name, METH_VARARGS, "Get children by name"},
    {"new_file", (PyCFunction) tp_f_new_file, METH_VARARGS, "Create new file object"},
    {"new_folder", (PyCFunction) tp_f_new_folder, METH_VARARGS, "Create new folder object"},
    {"create", (PyCFunction) tp_f_create, METH_VARARGS, "Create folder"},
    {"clear", (PyCFunction) tp_f_clear, METH_VARARGS, "Clear folder content"},
    {"reload", (PyCFunction) tp_f_reload, METH_VARARGS, "Reload folder info"},
    {"copy", (PyCFunction) tp_f_copy, METH_VARARGS, "Copy folder"},
    {"move", (PyCFunction) tp_f_move, METH_VARARGS, "Move folder"},
    {"remove", (PyCFunction) tp_f_remove, METH_VARARGS, "Remove folder"},
    {"rename", (PyCFunction) tp_f_rename, METH_VARARGS, "Rename folder"},
    {"get_streams", (PyCFunction) tp_f_get_streams, METH_VARARGS, "Get streams"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief tp_dealloc
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_io_folder_o *self)
{
    PyTypeObject *tp = Py_TYPE (self);
    delete self->obj;
    tp->tp_free (reinterpret_cast<PyObject *> (self));
    Py_DECREF (tp);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_io_folder_slots[] = {
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("core.io.folder class")},
    {Py_tp_getset, reinterpret_cast<void *> (tp_getset)},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_io_folder_spec = {
    .name = "mobius.core.io.folder",
    .basicsize = sizeof (core_io_folder_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_IMMUTABLETYPE | Py_TPFLAGS_IMMUTABLETYPE,
    .slots = core_io_folder_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.io.folder</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_io_folder_type ()
{
    // If type is already created, return it
    if (core_io_folder_type)
        return mobius::py::pytypeobject (core_io_folder_type);

    // Allocate type from spec
    core_io_folder_type = reinterpret_cast<PyTypeObject *> (PyType_FromSpec (&core_io_folder_spec));

    // Create type
    mobius::py::pytypeobject type (core_io_folder_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>core.io.folder</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_io_folder_check (PyObject *value)
{
    if (!core_io_folder_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.io.folder type is not initialized"));

    return mobius::py::isinstance (value, core_io_folder_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.io.folder</i> Python object from C++ object
// @param obj C++ object
// @return New core.io.folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_io_folder_to_pyobject (const mobius::core::io::folder &obj)
{
    if (!core_io_folder_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.io.folder type is not initialized"));

    return mobius::py::to_pyobject_nullable<core_io_folder_o> (obj, core_io_folder_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.io.folder</i> C++ object from Python object
// @param value Python value
// @return core.io.folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::folder
pymobius_core_io_folder_from_pyobject (PyObject *value)
{
    if (!core_io_folder_type)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("core.io.folder type is not initialized"));

    return mobius::py::from_pyobject<core_io_folder_o> (value, core_io_folder_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_folder_by_path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_new_folder_by_path (PyObject *, PyObject *args)
{
    // parse arguments
    std::string arg_path;

    try
    {
        arg_path = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_io_folder_to_pyobject (mobius::core::io::new_folder_by_path (arg_path));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_folder_by_url
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_new_folder_by_url (PyObject *, PyObject *args)
{
    // parse arguments
    std::string arg_url;

    try
    {
        arg_url = mobius::py::get_arg_as_std_string (args, 0);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_io_folder_to_pyobject (mobius::core::io::new_folder_by_url (arg_url));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function get_current_folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_get_current_folder (PyObject *, PyObject *)
{
    // execute C++ code
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_io_folder_to_pyobject (mobius::core::io::get_current_folder ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}
