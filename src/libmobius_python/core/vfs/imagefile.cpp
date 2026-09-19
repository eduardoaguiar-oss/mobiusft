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
// @file imagefile.cc C++ API <i>mobius.core.vfs.imagefile</i> class wrapper
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "imagefile.hpp"
#include <pydict.hpp>
#include <pygil.hpp>
#include <pylist.hpp>
#include <pymobius.hpp>
#include "core/io/file.hpp"
#include "core/io/reader.hpp"
#include "core/io/writer.hpp"
#include "core/pod/data.hpp"
#include "core/resource.hpp"
#include "module.hpp"

namespace
{
// @brief Global pointer to hold the heap-allocated type
static PyTypeObject *core_vfs_imagefile_type = nullptr;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>type</i> attribute getter
// @param self object
// @return <i>type</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_type (core_vfs_imagefile_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pystring_from_std_string (self->obj->get_type ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>size</i> attribute getter
// @param self object
// @return <i>size</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_size (core_vfs_imagefile_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint64_t (self->obj->get_size ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>sectors</i> attribute getter
// @param self object
// @return <i>sectors</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_sectors (core_vfs_imagefile_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint64_t (self->obj->get_sectors ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>sector_size</i> attribute getter
// @param self object
// @return <i>sector_size</i> attribute
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getter_sector_size (core_vfs_imagefile_o *self, void *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylong_from_std_uint64_t (self->obj->get_sector_size ());
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
    {(char *) "type", (getter) tp_getter_type, (setter) 0, (char *) "type", nullptr},
    {(char *) "size", (getter) tp_getter_size, (setter) 0, (char *) "size", nullptr},
    {(char *) "sectors", (getter) tp_getter_sectors, (setter) 0, (char *) "number of sectors", nullptr},
    {(char *) "sector_size", (getter) tp_getter_sector_size, (setter) 0, (char *) "sector size", nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>is_available</i> method implementation
// @param self object
// @param args argument list
// @return new reader
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_is_available (core_vfs_imagefile_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pybool_from_bool (self->obj->is_available ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_attribute</i> method implementation
// @param self object
// @param args argument list
// @return attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_attribute (core_vfs_imagefile_o *self, PyObject *args)
{
    // parse input args
    std::string arg_id;

    try
    {
        arg_id = mobius::py::get_arg_as_std_string (args, 0);
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
        ret = pymobius_core_pod_data_to_pyobject (self->obj->get_attribute (arg_id));
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>set_attribute</i> method implementation
// @param self object
// @param args argument list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_set_attribute (core_vfs_imagefile_o *self, PyObject *args)
{
    // parse input args
    std::string arg_id;
    mobius::core::pod::data arg_value;

    try
    {
        arg_id = mobius::py::get_arg_as_std_string (args, 0);
        arg_value = mobius::py::get_arg_as_cpp (args, 1, pymobius_core_pod_data_from_pyobject);
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
        self->obj->set_attribute (arg_id, arg_value);
        ret = mobius::py::pynone ();
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>get_attributes</i> method implementation
// @param self object
// @param args argument list
// @return map containing attributes' IDs and values
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_get_attributes (core_vfs_imagefile_o *self, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pydict_from_cpp_container (
            self->obj->get_attributes (),
            mobius::py::pystring_from_std_string,
            pymobius_core_pod_data_to_pyobject
        );
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>new_reader</i> method implementation
// @param self object
// @param args argument list
// @return new reader
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_new_reader (core_vfs_imagefile_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_io_reader_to_pyobject (self->obj->new_reader ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>new_writer</i> method implementation
// @param self object
// @param args argument list
// @return new writer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_f_new_writer (core_vfs_imagefile_o *self, PyObject *)
{
    // execute C++ function
    PyObject *ret = nullptr;

    try
    {
        ret = pymobius_core_io_writer_to_pyobject (self->obj->new_writer ());
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    // return value
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Methods structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyMethodDef tp_methods[] = {
    {(char *) "is_available", (PyCFunction) tp_f_is_available, METH_VARARGS, "Check if imagefile is available"},
    {(char *) "get_attribute", (PyCFunction) tp_f_get_attribute, METH_VARARGS, "Get attribute value"},
    {(char *) "set_attribute", (PyCFunction) tp_f_set_attribute, METH_VARARGS, "Set attribute value"},
    {(char *) "get_attributes", (PyCFunction) tp_f_get_attributes, METH_VARARGS, "Get attributes"},
    {(char *) "new_reader", (PyCFunction) tp_f_new_reader, METH_VARARGS, "Create new reader"},
    {(char *) "new_writer", (PyCFunction) tp_f_new_writer, METH_VARARGS, "Create new writer"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>imagefile</i> deallocator
// @param self object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
tp_dealloc (core_vfs_imagefile_o *self)
{
    delete self->obj;
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>item</i> getattro
// @param o Object
// @param name Attribute name
// @return Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
tp_getattro (PyObject *o, PyObject *name)
{
    PyObject *ret = nullptr;

    try
    {
        // search first for item.__dict__ internal values, such as tp_getset
        // and tp_methods entries
        ret = PyObject_GenericGetAttr (o, name);

        if (ret == nullptr)
        {
            mobius::py::reset_error ();

            // search item.attributes, using item.get_attribute (name)
            auto self = reinterpret_cast<core_vfs_imagefile_o *> (o);
            auto s_name = mobius::py::pystring_as_std_string (name);
            ret = pymobius_core_pod_data_to_pyobject (self->obj->get_attribute (s_name));
        }
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>item</i> setattro
// @param o Object
// @param name Attribute name
// @param value Attribute value
// @return 0 if success, -1 if error
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static int
tp_setattro (PyObject *o, PyObject *name, PyObject *value)
{
    try
    {
        // check if it is a deletion operation
        auto s_name = mobius::py::pystring_as_std_string (name);

        if (value == nullptr)
        {
            mobius::py::set_invalid_type_error ("cannot delete attribute '" + s_name + "'");
            return -1;
        }

        // get attribute by name
        PyObject *attr = PyObject_GenericGetAttr (o, name);

        // internal attributes are read only
        if (attr != nullptr)
        {
            Py_DECREF (attr);

            if (value == nullptr)
                mobius::py::set_invalid_type_error ("cannot delete attribute '" + s_name + "'");
            else
                mobius::py::set_invalid_type_error ("cannot set attribute '" + s_name + "'");

            return -1;
        }

        // set attribute
        else
        {
            mobius::py::reset_error ();

            auto self = reinterpret_cast<core_vfs_imagefile_o *> (o);
            auto s_value = pymobius_core_pod_data_from_pyobject (value);
            self->obj->set_attribute (s_name, s_value);
        }
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
        return -1;
    }

    return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type Slots
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Slot core_vfs_imagefile_slots[] = {
    {Py_tp_dealloc, reinterpret_cast<void *> (tp_dealloc)},
    {Py_tp_doc, const_cast<char *> ("core.vfs.imagefile class")},
    {Py_tp_getset, reinterpret_cast<void *> (tp_getset)},
    {Py_tp_methods, reinterpret_cast<void *> (tp_methods)},
    {Py_tp_setattro, reinterpret_cast<void *> (tp_setattro)},
    {Py_tp_getattro, reinterpret_cast<void *> (tp_getattro)},
    {0, nullptr} // Sentinel
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Type specification
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyType_Spec core_vfs_imagefile_spec = {
    .name = "mobius.core.vfs.imagefile",
    .basicsize = sizeof (core_vfs_imagefile_o),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = core_vfs_imagefile_slots,
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>mobius.core.vfs.imagefile</i> type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::py::pytypeobject
new_core_vfs_imagefile_type ()
{
    // If type is already created, return it
    if (core_vfs_imagefile_type)
        return mobius::py::pytypeobject (core_vfs_imagefile_type);

    // Allocate type from spec
    core_vfs_imagefile_type = reinterpret_cast<PyTypeObject *> (PyType_FromSpec (&core_vfs_imagefile_spec));

    // Create type
    mobius::py::pytypeobject type (core_vfs_imagefile_type);
    type.create ();

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if value is an instance of <i>core.vfs.imagefile</i>
// @param value Python value
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
pymobius_core_vfs_imagefile_check (PyObject *value)
{
    if (!core_vfs_imagefile_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.vfs.imagefile type is not initialized")
        );

    return mobius::py::isinstance (value, core_vfs_imagefile_type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.vfs.imagefile</i> Python object from C++ object
// @param obj C++ object
// @return New core.vfs.imagefile object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymobius_core_vfs_imagefile_to_pyobject (const mobius::core::vfs::imagefile &obj)
{
    if (!core_vfs_imagefile_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.vfs.imagefile type is not initialized")
        );

    return mobius::py::to_pyobject<core_vfs_imagefile_o> (
        obj, core_vfs_imagefile_type
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create <i>core.vfs.imagefile</i> C++ object from Python object
// @param value Python value
// @return core.vfs.imagefile object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::vfs::imagefile
pymobius_core_vfs_imagefile_from_pyobject (PyObject *value)
{
    if (!core_vfs_imagefile_type)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("core.vfs.imagefile type is not initialized")
        );

    return mobius::py::from_pyobject<core_vfs_imagefile_o> (
        value, core_vfs_imagefile_type
    );
}

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create tuple from imagefile::info object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static PyObject *
PyTuple_from_imagefile_info (const mobius::core::resource &r)
{
    PyObject *ret = PyTuple_New (4);

    if (ret)
    {
        auto img_resource =
            r.get_value<mobius::core::vfs::imagefile_resource_type> ();

        PyTuple_SetItem (ret, 0,
                         mobius::py::pystring_from_std_string (r.get_id ()));
        PyTuple_SetItem (
            ret, 1,
            mobius::py::pystring_from_std_string (r.get_description ()));
        PyTuple_SetItem (ret, 2,
                         mobius::py::pystring_from_std_string (
                             img_resource.file_extensions));
        PyTuple_SetItem (
            ret, 3, mobius::py::pybool_from_bool (img_resource.is_writeable));
    }

    return ret;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function get_imagefile_implementations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_vfs_get_imagefile_implementations (PyObject *, PyObject *)
{
    PyObject *ret = nullptr;

    try
    {
        ret = mobius::py::pylist_from_cpp_container (
            mobius::core::get_resources ("vfs.imagefile"),
            PyTuple_from_imagefile_info);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_runtime_error (e.what ());
    }

    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_imagefile_by_path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_vfs_new_imagefile_by_path (PyObject *, PyObject *args)
{
    // parse arguments
    std::string arg_path;
    std::string arg_type = "autodetect";

    try
    {
        arg_path = mobius::py::get_arg_as_std_string (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_type = mobius::py::get_arg_as_std_string (args, 1);
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
        auto imagefile = mobius::py::GIL () (
            mobius::core::vfs::new_imagefile_by_path (arg_path, arg_type));
        ret = pymobius_core_vfs_imagefile_to_pyobject (imagefile);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_io_error (e.what ());
    }

    // create Python imagefile according to its type
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_imagefile_by_url
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_vfs_new_imagefile_by_url (PyObject *, PyObject *args)
{
    // parse arguments
    std::string arg_url;
    std::string arg_type = "autodetect";

    try
    {
        arg_url = mobius::py::get_arg_as_std_string (args, 0);

        if (mobius::py::get_arg_size (args) > 1)
            arg_type = mobius::py::get_arg_as_std_string (args, 1);
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
        auto imagefile = mobius::py::GIL () (
            mobius::core::vfs::new_imagefile_by_url (arg_url, arg_type));
        ret = pymobius_core_vfs_imagefile_to_pyobject (imagefile);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_io_error (e.what ());
    }

    // create Python imagefile according to its type
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Function new_imagefile_from_file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
func_vfs_new_imagefile_from_file (PyObject *, PyObject *args)
{
    // parse input args
    mobius::core::io::file arg_file;
    std::string arg_type;

    try
    {
        arg_file = mobius::py::get_arg_as_cpp (args, 0,
                                               pymobius_core_io_file_from_pyobject);
        arg_type = mobius::py::get_arg_as_std_string (args, 1, "autodetect");
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
        auto imagefile = mobius::py::GIL () (
            mobius::core::vfs::imagefile (arg_file, arg_type));
        ret = pymobius_core_vfs_imagefile_to_pyobject (imagefile);
    }
    catch (const std::exception &e)
    {
        mobius::py::set_io_error (e.what ());
    }

    // create Python imagefile according to its type
    return ret;
}
