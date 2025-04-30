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
#include <mobius/core/system/dynamic_lib.hpp>
#include <mobius/core/exception.inc>
#include <dlfcn.h>
#include <stdexcept>

namespace mobius::core::system
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param filename Dynamic library path/filename (according to dlopen function)
// @param flag Open flag (according to dlopen function)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
dynamic_lib::dynamic_lib (const std::string& filename, bool lazy_binding)
{
  int flags = (lazy_binding) ? RTLD_LAZY : RTLD_NOW;
  handle_ = dlopen (filename.c_str (), flags);

  if (!handle_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG (dlerror ()));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
dynamic_lib::~dynamic_lib ()
{
  if (handle_)
    {
      dlclose (handle_);
      handle_ = nullptr;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get symbol from library
// @return Pointer to symbol or nullptr if symbol does not exist
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void *
dynamic_lib::get_symbol (const std::string& name) const
{
  if (!handle_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("dynamic library is not initialized"));

  return dlsym (handle_, name.c_str ());
}

} // namespace mobius::core::system


