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
#include <mobius/core/exception.inc>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/folder.hpp>
#include <mobius/core/io/folder_impl_null.hpp>
#include <mobius/core/io/local/folder_impl.hpp>
#include <mobius/core/io/local/get_current_folder.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/io/uri.hpp>
#include <mobius/core/string_functions.hpp>
#include <functional>
#include <memory>
#include <stdexcept>
#include <algorithm>

#ifdef SMBCLIENT_FOUND
#include <mobius/core/io/smb/folder_impl.hpp>
#endif

namespace mobius::core::io
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Construct object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder::folder ()
{
    impl_ = std::make_shared<folder_impl_null> ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Construct object
// @param impl Implementation object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder::folder (const std::shared_ptr<folder_impl_base> &impl)
    : impl_ (impl)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new file object
// @return File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file
folder::new_file (const std::string &name) const
{
    auto e = get_child_by_name (name);

    if (e.is_file ())
        return e.get_file ();

    return {};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new folder object
// @return Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder
folder::new_folder (const std::string &name) const
{
    auto e = get_child_by_name (name);

    if (e.is_folder ())
        return e.get_folder ();

    return {};
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folder extension
// @return Folder extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
folder::get_extension () const
{
    std::string extension;
    const std::string name = get_name ();
    auto pos = name.rfind ('.');

    if (pos != std::string::npos && pos != 0)
        extension = name.substr (pos + 1);

    return extension;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Copy folder
// @param dst Folder object (destination)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder::copy (folder dst) const
{
    dst.create ();

    for (const auto &entry : get_children ())
    {
        if (entry.is_file ())
        {
            auto src_file = entry.get_file ();
            auto dst_file = dst.new_file (src_file.get_name ());
            src_file.copy (dst_file);
        }

        else if (entry.is_folder ())
        {
            auto src_folder = entry.get_folder ();
            auto dst_folder = dst.new_folder (src_folder.get_name ());
            src_folder.copy (dst_folder);
        }

        else
            throw std::invalid_argument (
                MOBIUS_EXCEPTION_MSG ("unhandled entry")
            );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Move folder
// @param dst Destination folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
folder::move (folder dst)
{
    if (!impl_->move (dst.impl_))
    {
        copy (dst);
        remove ();
    }

    impl_->reload ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get parent folder
// @return Parent folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder
folder::get_parent () const
{
    auto parent = folder (impl_->get_parent ());

    if (parent)
    {
        mobius::core::io::path path (get_path ());
        parent.set_path (path.get_dirname ());
    }

    return parent;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get child by name
// @param name Name
// @param cs Case sensitive flag
// @return Child, if found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::entry
folder::get_child_by_name (const std::string &name, bool cs) const
{
    mobius::core::io::entry entry;
    std::function<bool (const std::string &, const std::string &)> comp =
        (cs) ? mobius::core::string::case_sensitive_match
             : mobius::core::string::case_insensitive_match;

    // search child
    for (const auto &child : get_children ())
    {
        if (comp (name, child.get_name ()))
        {
            if (child.is_deleted ())
                entry = child;

            else
                return child;
        }
    }

    return entry;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get child by path
// @param path Relative path
// @param cs Case sensitive flag
// @return Child, if found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::entry
folder::get_child_by_path (const std::string &path, bool cs) const
{
    std::string::size_type pos = 0;
    std::string::size_type end = path.find ('/');
    mobius::core::io::folder c_folder = *this;

    while (end != std::string::npos)
    {
        const std::string name = path.substr (pos, end - pos);
        auto e = c_folder.get_child_by_name (name, cs);

        if (e.is_folder ())
            c_folder = e.get_folder ();

        else
            return mobius::core::io::entry ();

        pos = end + 1;
        end = path.find ('/', pos);
    }

    const std::string name = path.substr (pos);
    return c_folder.get_child_by_name (name, cs);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get children by name
// @param name Children name
// @param cs Case sensitive flag
// @return Children
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<entry>
folder::get_children_by_name (const std::string &name, bool cs) const
{
    std::vector<entry> children;
    std::function<bool (const std::string &, const std::string &)> comp =
        (cs) ? mobius::core::string::case_sensitive_match
             : mobius::core::string::case_insensitive_match;

    for (const auto &child : get_children ())
    {
        if (comp (name, child.get_name ()))
            children.push_back (child);
    }

    return children;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get streams
// @return Streams
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<stream>
folder::get_streams () const
{
    auto impl_streams = impl_->get_streams ();

    std::vector<stream> streams (impl_streams.size ());

    std::transform (
        impl_streams.begin (), impl_streams.end (), streams.begin (),
        [] (const auto &s) { return stream (s); }
    );

    return streams;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create folder by path
// @param path folder path
// @return Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder
new_folder_by_path (const std::string &path)
{
    return folder (std::make_shared<local::folder_impl> (path));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create folder by URL
// @param url folder URL
// @return Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder
new_folder_by_url (const std::string &url)
{
    mobius::core::io::uri u (url);

    if (u.get_scheme () == "file")
        return new_folder_by_path (u.get_path ("utf-8"));

#ifdef SMBCLIENT_FOUND
    else if (u.get_scheme () == "smb")
        return folder (std::make_shared<smb::folder_impl> (url));
#endif

    else
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("unhandled folder scheme")
        );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get current folder
// @return Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder
get_current_folder ()
{
    return local::get_current_folder ();
}

} // namespace mobius::core::io
