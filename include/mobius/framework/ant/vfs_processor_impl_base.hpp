#ifndef MOBIUS_FRAMEWORK_ANT_VFS_PROCESSOR_IMPL_BASE_HPP
#define MOBIUS_FRAMEWORK_ANT_VFS_PROCESSOR_IMPL_BASE_HPP

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <mobius/core/io/folder.hpp>

namespace mobius::framework::ant
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>vfs_processor_impl_base</i> class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class vfs_processor_impl_base
{
  public:
    virtual ~vfs_processor_impl_base () = default;
    virtual void on_folder (const mobius::core::io::folder &) = 0;
    virtual void on_complete () = 0;
};

} // namespace mobius::framework::ant

#endif
