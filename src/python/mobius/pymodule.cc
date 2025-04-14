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
#include "pymodule.h"
#include <Python.h>
#include <stdexcept>
#include <mobius/exception.inc>

namespace mobius::py
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief pymodule implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class pymodule::impl
{
public:

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors and destructor
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl (const impl&) = delete;
  impl (impl&&) = delete;
  impl (PyModuleDef *);
  ~impl ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl& operator= (const impl&) = delete;
  impl& operator= (impl&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get pointer
  // @return PyObject pointer
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  PyObject *
  pointer ()
  {
    return obj_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Create new reference
  // @return PyObject pointer
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  PyObject *
  new_reference () const
  {
    Py_INCREF (obj_);
    return obj_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void add_type (const std::string&, PyTypeObject *);
  void add_type (const std::string&, pytypeobject);
  void add_submodule (const std::string&, const pymodule&);
  void add_constant (const std::string&, int);

private:
  // @brief Python object
  PyObject *obj_ = nullptr;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param p Pointer to PyModuleDef structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
pymodule::impl::impl (PyModuleDef *p)
{
    // create module object
    obj_ = PyModule_Create (p);
    if (!obj_)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("could not create Python module '" + std::string (p->m_name) + "'"));
    
    // register module into sys.modules
    PyObject *sys_modules = PyImport_GetModuleDict ();
    if (!sys_modules)
        throw std::runtime_error ("Failed to get sys.modules");
        
    if (PyDict_SetItemString(sys_modules, p->m_name, obj_) < 0)
        throw std::runtime_error ("Failed to register " + std::string (p->m_name) + " in sys.modules");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
pymodule::impl::~impl ()
{
  if (obj_)
    Py_DECREF (obj_);

  obj_ = nullptr;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add type to module
// @param name Type name
// @param type Type object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
pymodule::impl::add_type (const std::string& name, PyTypeObject *type)
{
  if (PyType_Ready (type) < 0)
   throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("could not build type"));

  Py_INCREF (type);
  if (PyModule_AddObject (obj_, name.c_str (), reinterpret_cast <PyObject *> (type)) < 0)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("error adding type '" + name + "' to module"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add type to module
// @param name Type name
// @param type Type object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
pymodule::impl::add_type (const std::string& name, pytypeobject type)
{
  type.create ();

  Py_INCREF (type);
  if (PyModule_AddObject (obj_, name.c_str (), static_cast <PyObject *> (type)) < 0)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("error adding type '" + name + "' to module"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add submodule to module
// @param name Submodule name
// @param module Submodule object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
pymodule::impl::add_submodule (const std::string& name, const pymodule& module)
{
  PyObject *subobj = module.new_reference ();

  if (PyModule_AddObject (obj_, name.c_str (), subobj) < 0)
    {
      Py_DECREF (subobj);
      throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("error adding submodule '" + name + "' to module"));
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add constant to module
// @param name Name
// @param value Value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
pymodule::impl::add_constant (const std::string& name, int value)
{
  if (PyModule_AddIntConstant (obj_, name.c_str (), value) < 0)
    {
      throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("error adding constant '" + name + "' to module"));
    }
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param p Pointer to PyModuleDef structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
pymodule::pymodule (PyModuleDef *p)
 : impl_ (std::make_shared <impl> (p))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if object is valid
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
pymodule::operator bool () const noexcept
{
  return impl_->pointer () != nullptr;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get PyObject pointer
// @return PyObject pointer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
pymodule::operator PyObject * () const noexcept
{
  return impl_->pointer ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new reference to module
// @return Reference
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PyObject *
pymodule::new_reference () const
{
  return impl_->new_reference ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add type to module
// @param name Type name
// @param type Type object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
pymodule::add_type (const std::string& name, PyTypeObject *type)
{
  return impl_->add_type (name, type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add type to module
// @param name Type name
// @param type Type object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
pymodule::add_type (const std::string& name, pytypeobject type)
{
  return impl_->add_type (name, type);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add submodule to module
// @param name Submodule name
// @param module Submodule object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
pymodule::add_submodule (const std::string& name, const pymodule& module)
{
  impl_->add_submodule (name, module);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add constant to module
// @param name Name
// @param value Value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
pymodule::add_constant (const std::string& name, int value)
{
  impl_->add_constant (name, value);
}

} // namespace mobius::py


