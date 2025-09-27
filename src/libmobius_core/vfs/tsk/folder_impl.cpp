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
#include <mobius/core/exception.inc>
#include <mobius/core/io/entry.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/file_impl_null.hpp>
#include <mobius/core/io/folder.hpp>
#include <mobius/core/io/folder_impl_null.hpp>
#include <mobius/core/vfs/tsk/exception.hpp>
#include <mobius/core/vfs/tsk/file_impl.hpp>
#include <mobius/core/vfs/tsk/folder_impl.hpp>
#include <algorithm>
#include <stdexcept>
#include <tsk/libtsk.h>

namespace mobius::core::vfs::tsk
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize object
// @param fp Pointer to file structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::folder_impl (const fs_file &fp)
    : fs_file_ (fp),
      name_ (fs_file_.get_name ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get user name
// @return User name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
folder_impl::get_user_name () const
{
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return std::string (); // libtsk has no user name
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get group name
// @return Group name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
folder_impl::get_group_name () const
{
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return std::string (); // libtsk has no group name
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if folder is hidden
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
folder_impl::is_hidden () const
{
    if (!exists ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("folder does not exist")
        );

    return false; //! name_.empty () && name_[0] == '.';
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::create ()
{
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("cannot create folder"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clear folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::clear ()
{
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("cannot clear folder"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::remove ()
{
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("cannot remove folder"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Rename folder
// @param filename New filename
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder_impl::rename (const std::string &)
{
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("cannot rename folder"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Move folder
// @param impl Folder implementation pointer
// @return <b>true</b> if folder has been moved, <b>false</b> otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
folder_impl::move (folder_type)
{
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("cannot move folder"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get parent folder
// @return Parent folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::folder_type
folder_impl::get_parent () const
{
    fs_file parent = fs_file_.get_parent ();

    if (parent)
        return std::make_shared<folder_impl> (parent);

    return std::make_shared<mobius::core::io::folder_impl_null> ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get children
// @return Collection
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<mobius::core::io::entry>
folder_impl::get_children () const
{
    // Return cached children if already loaded
    if (children_loaded_)
        return children_;

    // Check if fs_file_ is valid
    if (!fs_file_)
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid folder"));

    // Create children from fs_file_ children
    auto children = fs_file_.get_children ();
    std::transform (
        children.begin(), children.end(),
        std::back_inserter (children_),
        [] (const fs_file &f)
        {
            if (f.get_type () == f.fs_file_type::folder)
                return mobius::core::io::entry (
                    mobius::core::io::folder (std::make_shared<folder_impl> (f))
                );

            return mobius::core::io::entry (
                mobius::core::io::file (std::make_shared<file_impl> (f))
            );
        }
    );

    // set children's paths
    auto path = get_path ();

    std::for_each (
        children_.begin (), children_.end (),
        [&path] (mobius::core::io::entry &e) { e.set_path (path + '/' + e.get_name ()); }
    );

    // set children loaded flag
    children_loaded_ = true;

    // Return children
    return children_;
}

} // namespace mobius::core::vfs::tsk
