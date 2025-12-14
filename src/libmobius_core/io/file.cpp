// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
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
#include <algorithm>
#include <mobius/core/exception.inc>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/file_impl_base.hpp>
#include <mobius/core/io/file_impl_null.hpp>
#include <mobius/core/io/local/file_impl.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/io/uri.hpp>
#include <stdexcept>

#ifdef SMBCLIENT_FOUND
#include <mobius/core/io/smb/file_impl.hpp>
#endif

namespace mobius::core::io
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Construct object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file::file () { impl_ = std::make_shared<file_impl_null> (); }

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Construct object
// @param impl Implementation object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file::file (const std::shared_ptr<file_impl_base> &impl)
    : impl_ (impl)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Copy file
// @param f File object (destination)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file::copy (file f) const
{
    auto reader = new_reader ();
    auto writer = f.new_writer ();

    std::uint64_t BLOCK_SIZE = 65536;
    auto size = get_size ();

    while (size > 0)
    {
        auto data = reader.read (std::min (BLOCK_SIZE, size));
        writer.write (data);
        size -= data.size ();
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Copy file to folder
// @param folder_out Folder object (dest)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file::copy (folder folder_out) const
{
    auto f = folder_out.new_file (get_name ());
    copy (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Move file
// @param f Destination file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file::move (file f)
{
    if (!impl_->move (f.impl_))
    {
        copy (f);
        remove ();
    }

    impl_->reload ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Move file to folder
// @param folder_out Folder object (dest)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file::move (folder folder_out)
{
    auto f = folder_out.new_file (get_name ());
    move (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file extension
// @return File extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
file::get_extension () const
{
    std::string extension;
    const std::string filename = get_name ();
    auto pos = filename.rfind ('.');

    if (pos != std::string::npos && pos != 0)
        extension = filename.substr (pos + 1);

    return extension;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get parent folder
// @return Parent folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder
file::get_parent () const
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
// @brief Create new sibling file object by name
// @param name Sibling file name
// @return Sibling file object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file
file::new_sibling_by_name (const std::string &name) const
{
    auto parent = get_parent ();

    return parent.new_file (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new sibling file object by extension
// @param ext Sibling extension
// @return Sibling file object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file
file::new_sibling_by_extension (const std::string &ext) const
{
    auto filename = get_name ();

    std::string name;
    auto pos = filename.rfind ('.');

    if (pos == std::string::npos)
        name = filename + '.' + ext;

    else
        name = filename.substr (0, pos) + '.' + ext;

    return new_sibling_by_name (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get streams
// @return Streams
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<stream>
file::get_streams () const
{
    auto impl_streams = impl_->get_streams ();

    std::vector<stream> streams (impl_streams.size ());

    std::transform (impl_streams.begin (), impl_streams.end (),
                    streams.begin (),
                    [] (const auto &s) { return stream (s); });

    return streams;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create file by path
// @param path file path
// @return file object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file
new_file_by_path (const std::string &path)
{
    return file (std::make_shared<local::file_impl> (path));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create file by URI
// @param url file URL
// @return file object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file
new_file_by_url (const std::string &url)
{
    mobius::core::io::uri u (url);

    if (u.get_scheme () == "file")
        return new_file_by_path (u.get_path ("utf-8"));

#ifdef SMBCLIENT_FOUND
    else if (u.get_scheme () == "smb")
        return file (std::make_shared<smb::file_impl> (url));
#endif

    else
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("unhandled file scheme"));
}

} // namespace mobius::core::io
