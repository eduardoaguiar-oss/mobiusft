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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @file lnk.cc C++ API <i>mobius.core.decoder.lnk</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "lnk.hpp"
#include "core/io/reader.hpp"
#include <mobius/core/exception.inc>
#include <pymobius.hpp>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>lnk</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_decoder_lnk_check (PyObject *value)
{
    return PyObject_IsInstance (
        value, reinterpret_cast<PyObject *> (&core_decoder_lnk_t));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>lnk</i> Python object from C++ object
// @param obj C++ object
// @return New lnk object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_decoder_lnk_to_pyobject (const mobius::core::decoder::lnk &obj)
{
    PyObject *ret = _PyObject_New (&core_decoder_lnk_t);

    if (ret)
        ((core_decoder_lnk_o *) ret)->obj =
            new mobius::core::decoder::lnk (obj);

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>lnk</i> C++ object from Python object
// @param value Python value
// @return Lnk object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::decoder::lnk
pymobius_core_decoder_lnk_from_pyobject (PyObject *value)
{
    if (!pymobius_core_decoder_lnk_check (value))
        throw std::invalid_argument (MOBIUS_EXCEPTION_MSG (
            "object must be an instance of mobius.core.decoder.lnk"));

    return *(reinterpret_cast<core_decoder_lnk_o *> (value)->obj);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>header_size</i> Attribute getter
// @param self Object
// @return <i>header_size</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_header_size (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint32_t (
            self->obj->get_header_size ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>clsid</i> Attribute getter
// @param self Object
// @return <i>clsid</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_clsid (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_clsid ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>flags</i> Attribute getter
// @param self Object
// @return <i>flags</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_flags (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint32_t (self->obj->get_flags ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>file_attributes</i> Attribute getter
// @param self Object
// @return <i>file_attributes</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_file_attributes (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint32_t (
            self->obj->get_file_attributes ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>creation_time</i> Attribute getter
// @param self Object
// @return <i>creation_time</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_creation_time (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pydatetime_from_datetime (
            self->obj->get_creation_time ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>access_time</i> Attribute getter
// @param self Object
// @return <i>access_time</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_access_time (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pydatetime_from_datetime (
            self->obj->get_access_time ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>write_time</i> Attribute getter
// @param self Object
// @return <i>write_time</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_write_time (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret =
            mobius::py::pydatetime_from_datetime (self->obj->get_write_time ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>file_size</i> Attribute getter
// @param self Object
// @return <i>file_size</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_file_size (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret =
            mobius::py::pylong_from_std_uint32_t (self->obj->get_file_size ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>icon_index</i> Attribute getter
// @param self Object
// @return <i>icon_index</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_icon_index (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret =
            mobius::py::pylong_from_std_uint32_t (self->obj->get_icon_index ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>show_command</i> Attribute getter
// @param self Object
// @return <i>show_command</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_show_command (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint32_t (
            self->obj->get_show_command ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>hotkeys</i> Attribute getter
// @param self Object
// @return <i>hotkeys</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_hotkeys (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint16_t (self->obj->get_hotkeys ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>volume_label</i> Attribute getter
// @param self Object
// @return <i>volume_label</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_volume_label (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (
            self->obj->get_volume_label ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>drive_type</i> Attribute getter
// @param self Object
// @return <i>drive_type</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_drive_type (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret =
            mobius::py::pylong_from_std_uint32_t (self->obj->get_drive_type ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>drive_serial_number</i> Attribute getter
// @param self Object
// @return <i>drive_serial_number</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_drive_serial_number (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint32_t (
            self->obj->get_drive_serial_number ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>local_base_path</i> Attribute getter
// @param self Object
// @return <i>local_base_path</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_local_base_path (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (
            self->obj->get_local_base_path ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>common_path_suffix</i> Attribute getter
// @param self Object
// @return <i>common_path_suffix</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_common_path_suffix (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (
            self->obj->get_common_path_suffix ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>net_name</i> Attribute getter
// @param self Object
// @return <i>net_name</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_net_name (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_net_name ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>device_name</i> Attribute getter
// @param self Object
// @return <i>device_name</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_device_name (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (
            self->obj->get_device_name ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>network_provider_type</i> Attribute getter
// @param self Object
// @return <i>network_provider_type</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_network_provider_type (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint32_t (
            self->obj->get_network_provider_type ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>name</i> Attribute getter
// @param self Object
// @return <i>name</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_name (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_name ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>relative_path</i> Attribute getter
// @param self Object
// @return <i>relative_path</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_relative_path (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (
            self->obj->get_relative_path ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>working_dir</i> Attribute getter
// @param self Object
// @return <i>working_dir</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_working_dir (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (
            self->obj->get_working_dir ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>arguments</i> Attribute getter
// @param self Object
// @return <i>arguments</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_arguments (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret =
            mobius::py::pystring_from_std_string (self->obj->get_arguments ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>icon_location</i> Attribute getter
// @param self Object
// @return <i>icon_location</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_icon_location (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (
            self->obj->get_icon_location ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>netbios_name</i> Attribute getter
// @param self Object
// @return <i>netbios_name</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_netbios_name (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (
            self->obj->get_netbios_name ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>header_offset</i> Attribute getter
// @param self Object
// @return <i>header_offset</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_header_offset (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint64_t (
            self->obj->get_header_offset ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>link_target_id_list_offset</i> Attribute getter
// @param self Object
// @return <i>link_target_id_list_offset</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_link_target_id_list_offset (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint64_t (
            self->obj->get_link_target_id_list_offset ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>link_info_offset</i> Attribute getter
// @param self Object
// @return <i>link_info_offset</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_link_info_offset (core_decoder_lnk_o *self)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint64_t (
            self->obj->get_link_info_offset ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Getters and setters structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyGetSetDef tp_getset[] = {
    {(char *) "header_size", (getter) tp_getter_header_size, (setter) 0,
     (char *) "Header size in bytes", nullptr},
    {(char *) "clsid", (getter) tp_getter_clsid, (setter) 0, (char *) "CLSID",
     nullptr},
    {(char *) "flags", (getter) tp_getter_flags, (setter) 0, (char *) "Flags",
     nullptr},
    {(char *) "file_attributes", (getter) tp_getter_file_attributes, (setter) 0,
     (char *) "Link target attributes", nullptr},
    {(char *) "creation_time", (getter) tp_getter_creation_time, (setter) 0,
     (char *) "Link target creation time", nullptr},
    {(char *) "access_time", (getter) tp_getter_access_time, (setter) 0,
     (char *) "Link target access time", nullptr},
    {(char *) "write_time", (getter) tp_getter_write_time, (setter) 0,
     (char *) "Link target write time", nullptr},
    {(char *) "file_size", (getter) tp_getter_file_size, (setter) 0,
     (char *) "Link target file size (32 bits)", nullptr},
    {(char *) "icon_index", (getter) tp_getter_icon_index, (setter) 0,
     (char *) "Icon index within icon location", nullptr},
    {(char *) "show_command", (getter) tp_getter_show_command, (setter) 0,
     (char *) "Show command", nullptr},
    {(char *) "hotkeys", (getter) tp_getter_hotkeys, (setter) 0,
     (char *) "Hot keys", nullptr},
    {(char *) "volume_label", (getter) tp_getter_volume_label, (setter) 0,
     (char *) "Volume label", nullptr},
    {(char *) "drive_type", (getter) tp_getter_drive_type, (setter) 0,
     (char *) "Drive type", nullptr},
    {(char *) "drive_serial_number", (getter) tp_getter_drive_serial_number,
     (setter) 0, (char *) "Drive serial number", nullptr},
    {(char *) "local_base_path", (getter) tp_getter_local_base_path, (setter) 0,
     (char *) "Local base path", nullptr},
    {(char *) "common_path_suffix", (getter) tp_getter_common_path_suffix,
     (setter) 0, (char *) "Common path suffix", nullptr},
    {(char *) "net_name", (getter) tp_getter_net_name, (setter) 0,
     (char *) "Net name", nullptr},
    {(char *) "device_name", (getter) tp_getter_device_name, (setter) 0,
     (char *) "Device name", nullptr},
    {(char *) "network_provider_type", (getter) tp_getter_network_provider_type,
     (setter) 0, (char *) "Network provider type", nullptr},
    {(char *) "name", (getter) tp_getter_name, (setter) 0, (char *) "Name",
     nullptr},
    {(char *) "relative_path", (getter) tp_getter_relative_path, (setter) 0,
     (char *) "Relative path", nullptr},
    {(char *) "working_dir", (getter) tp_getter_working_dir, (setter) 0,
     (char *) "Working directory", nullptr},
    {(char *) "arguments", (getter) tp_getter_arguments, (setter) 0,
     (char *) "Command line arguments", nullptr},
    {(char *) "icon_location", (getter) tp_getter_icon_location, (setter) 0,
     (char *) "Icon location", nullptr},
    {(char *) "netbios_name", (getter) tp_getter_netbios_name, (setter) 0,
     (char *) "NetBIOS name", nullptr},
    {(char *) "header_offset", (getter) tp_getter_header_offset, (setter) 0,
     (char *) "Header section offset", nullptr},
    {(char *) "link_target_id_list_offset",
     (getter) tp_getter_link_target_id_list_offset, (setter) 0,
     (char *) "LinkTargetIDList section offset", nullptr},
    {(char *) "link_info_offset", (getter) tp_getter_link_info_offset,
     (setter) 0, (char *) "LinkInfo section offset", nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_target_readonly</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_target_readonly (core_decoder_lnk_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->is_target_readonly ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_target_hidden</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_target_hidden (core_decoder_lnk_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->is_target_hidden ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_target_system</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_target_system (core_decoder_lnk_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->is_target_system ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_target_directory</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_target_directory (core_decoder_lnk_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->is_target_directory ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_target_archive</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_target_archive (core_decoder_lnk_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->is_target_archive ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_target_temporary</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_target_temporary (core_decoder_lnk_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->is_target_temporary ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_target_sparse</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_target_sparse (core_decoder_lnk_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->is_target_sparse ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_target_reparse_point</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_target_reparse_point (core_decoder_lnk_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (
            self->obj->is_target_reparse_point ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_target_compressed</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_target_compressed (core_decoder_lnk_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->is_target_compressed ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_target_offline</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_target_offline (core_decoder_lnk_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->is_target_offline ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_target_content_indexed</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_target_content_indexed (core_decoder_lnk_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (
            self->obj->is_target_content_indexed ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_target_encrypted</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_target_encrypted (core_decoder_lnk_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->is_target_encrypted ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_run_in_separate_process</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_run_in_separate_process (core_decoder_lnk_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (
            self->obj->is_run_in_separate_process ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // Return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_run_as_user</i> method implementation
// @param self Object
// @param args Argument list
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_run_as_user (core_decoder_lnk_o *self, PyObject *)
{
    // Execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->is_run_as_user ());
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
    {(char *) "is_target_readonly", (PyCFunction) tp_f_is_target_readonly,
     METH_VARARGS, "Check if link target is readonly"},
    {(char *) "is_target_hidden", (PyCFunction) tp_f_is_target_hidden,
     METH_VARARGS, "Check if link target is hidden"},
    {(char *) "is_target_system", (PyCFunction) tp_f_is_target_system,
     METH_VARARGS, "Check if link target is system file"},
    {(char *) "is_target_directory", (PyCFunction) tp_f_is_target_directory,
     METH_VARARGS, "Check if link target is directory"},
    {(char *) "is_target_archive", (PyCFunction) tp_f_is_target_archive,
     METH_VARARGS, "Check if link target is archive"},
    {(char *) "is_target_temporary", (PyCFunction) tp_f_is_target_temporary,
     METH_VARARGS, "Check if link target is temporary"},
    {(char *) "is_target_sparse", (PyCFunction) tp_f_is_target_sparse,
     METH_VARARGS, "Check if link target is sparse file"},
    {(char *) "is_target_reparse_point",
     (PyCFunction) tp_f_is_target_reparse_point, METH_VARARGS,
     "Check if link target is reparse point"},
    {(char *) "is_target_compressed", (PyCFunction) tp_f_is_target_compressed,
     METH_VARARGS, "Check if link target is compressed"},
    {(char *) "is_target_offline", (PyCFunction) tp_f_is_target_offline,
     METH_VARARGS, "Check if link target is offline"},
    {(char *) "is_target_content_indexed",
     (PyCFunction) tp_f_is_target_content_indexed, METH_VARARGS,
     "Check if link target is content indexed"},
    {(char *) "is_target_encrypted", (PyCFunction) tp_f_is_target_encrypted,
     METH_VARARGS, "Check if link target is encrypted"},
    {(char *) "is_run_in_separate_process",
     (PyCFunction) tp_f_is_run_in_separate_process, METH_VARARGS,
     "Target run in separate process if set"},
    {(char *) "is_run_as_user", (PyCFunction) tp_f_is_run_as_user, METH_VARARGS,
     "Target run as a different user if set"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>lnk</i> Constructor
// @param type Type object
// @param args Argument list
// @param kwds Keywords dict
// @return new <i>lnk</i> object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_new (PyTypeObject *type, PyObject *args, PyObject *)
{
    // Parse input args
    mobius::core::io::reader arg_reader;

    try
    {
        arg_reader = mobius::py::get_arg_as_cpp (
            args, 0, pymobius_core_io_reader_from_pyobject);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_invalid_type_error (e.what ());
        return nullptr;
    }

    // Create Python object
    core_decoder_lnk_o *ret =
        reinterpret_cast<core_decoder_lnk_o *> (type->tp_alloc (type, 0));

    if (ret)
    {
        try
        {
            ret->obj = new mobius::core::decoder::lnk (arg_reader);
        }
        catch (const std::exception &e)
        {
            Py_DECREF (ret);
            mobius::py::set_runtime_error (e.what ());
            ret = nullptr;
        }
    }

    return reinterpret_cast<PyObject *> (ret);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>lnk</i> deallocator
// @param self Object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_decoder_lnk_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyTypeObject core_decoder_lnk_t = {
    PyVarObject_HEAD_INIT (nullptr, 0)        // header
    "mobius.core.decoder.lnk",                // tp_name
    sizeof (core_decoder_lnk_o),              // tp_basicsize
    0,                                        // tp_itemsize
    (destructor) tp_dealloc,                  // tp_dealloc
    0,                                        // tp_print
    0,                                        // tp_getattr
    0,                                        // tp_setattr
    0,                                        // tp_compare
    0,                                        // tp_repr
    0,                                        // tp_as_number
    0,                                        // tp_as_sequence
    0,                                        // tp_as_mapping
    0,                                        // tp_hash
    0,                                        // tp_call
    0,                                        // tp_str
    0,                                        // tp_getattro
    0,                                        // tp_setattro
    0,                                        // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    "lnk class",                              // tp_doc
    0,                                        // tp_traverse
    0,                                        // tp_clear
    0,                                        // tp_richcompare
    0,                                        // tp_weaklistoffset
    0,                                        // tp_iter
    0,                                        // tp_iternext
    tp_methods,                               // tp_methods
    0,                                        // tp_members
    tp_getset,                                // tp_getset
    0,                                        // tp_base
    0,                                        // tp_dict
    0,                                        // tp_descr_get
    0,                                        // tp_descr_set
    0,                                        // tp_dictoffset
    0,                                        // tp_init
    0,                                        // tp_alloc
    tp_new,                                   // tp_new
    0,                                        // tp_free
    0,                                        // tp_is_gc
    0,                                        // tp_bases
    0,                                        // tp_mro
    0,                                        // tp_cache
    0,                                        // tp_subclasses
    0,                                        // tp_weaklist
    0,                                        // tp_del
    0,                                        // tp_version_tag
    0,                                        // tp_finalize
};
