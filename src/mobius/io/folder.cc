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
#include "folder.h"
#include "file.h"
#include "folder_impl_null.h"
#include "local/folder_impl.h"
#include "local/get_current_folder.h"
#include "path.h"
#include "uri.h"
#include <mobius/exception.inc>
#include <mobius/string_functions.h>
#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>
#include <config.h>

#ifdef HAVE_SMB_SUPPORT
#include "smb/folder_impl.h"
#endif

namespace mobius::io
{
namespace
{
using entry_impl = folder_impl_base::entry_impl;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Collection implementation for folder entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class collection_impl : public mobius::collection_impl_base <mobius::io::entry>
{
public:
  using pointer_type = std::shared_ptr <collection_impl_base <entry_impl>>;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  collection_impl (const pointer_type&, const std::string&);
  bool get (mobius::io::entry&) override;
  void reset () override;

private:
  pointer_type impl_;
  std::string path_;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize object
// @param impl Implementation object
// @param path Collection base folder path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
collection_impl::collection_impl (const pointer_type& impl, const std::string& path)
  : impl_ (impl),
    path_ (path)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folder entry
// @param e Entry reference
// @return true/false if entry was found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
collection_impl::get (mobius::io::entry& e)
{
  bool rc = false;
  folder_impl_base::entry_impl eimpl;

  if (impl_->get (eimpl))
    {
      if (eimpl.folder_p)
        e = entry (folder (eimpl.folder_p));

      else if (eimpl.file_p)
        e = entry (file (eimpl.file_p));

      else
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid entry_impl"));

      e.set_path (path_ + '/' + e.get_name ());
      rc = true;
    }

  return rc;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset collection
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
collection_impl::reset ()
{
  impl_->reset ();
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Construct object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder::folder ()
{
  impl_ = std::make_shared <folder_impl_null> ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Construct object
// @param impl Implementation object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder::folder (std::shared_ptr <folder_impl_base> impl)
  : impl_ (impl)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new file object
// @return File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file
folder::new_file (const std::string& name) const
{
  return file (impl_->new_file (name));
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

  for (const auto& entry : get_children ())
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
        throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("unhandled entry"));
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
      mobius::io::path path (get_path ());
      parent.set_path (path.get_dirname ());
    }

  return parent;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get children
// @return Child entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder::children_type
folder::get_children () const
{
  return children_type (
           std::make_shared <collection_impl> (
              impl_->get_children (),
              get_path ())
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get child by name
// @param name Name
// @param cs Case sensitive flag
// @return Child, if found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::io::entry
folder::get_child_by_name (const std::string& name, bool cs) const
{
  mobius::io::entry entry;
  std::function <bool (const std::string&, const std::string&)> comp = (cs) ? mobius::string::case_sensitive_match : mobius::string::case_insensitive_match;

  // search child
  for (const auto& child : get_children ())
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
mobius::io::entry
folder::get_child_by_path (const std::string& path, bool cs) const
{
  std::string::size_type pos = 0;
  std::string::size_type end = path.find ('/');
  mobius::io::folder c_folder = *this;

  while (end != std::string::npos)
    {
      const std::string name = path.substr (pos, end - pos);
      auto e = c_folder.get_child_by_name (name, cs);

      if (e.is_folder ())
        c_folder = e.get_folder ();

      else
        return mobius::io::entry ();

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
std::vector <entry>
folder::get_children_by_name (const std::string& name, bool cs) const
{
  std::vector <entry> children;
  std::function <bool (const std::string&, const std::string&)> comp = (cs) ? mobius::string::case_sensitive_match : mobius::string::case_insensitive_match;

  for (const auto& child : get_children ())
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
std::vector <stream>
folder::get_streams () const
{
  auto impl_streams = impl_->get_streams ();

  std::vector <stream> streams (impl_streams.size ());

  std::transform (impl_streams.begin (),
                  impl_streams.end (),
                  streams.begin (),
                  [](const auto& s){ return stream (s); }
                 );

  return streams;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create folder by path
// @param path folder path
// @return Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder
new_folder_by_path (const std::string& path)
{
  return folder (std::make_shared <local::folder_impl> (path));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create folder by URL
// @param url folder URL
// @return Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder
new_folder_by_url (const std::string& url)
{
  mobius::io::uri u (url);

  if (u.get_scheme () == "file")
    return new_folder_by_path (u.get_path ("utf-8"));

#ifdef HAVE_SMB_SUPPORT
  else if (u.get_scheme () == "smb")
    return folder (std::make_shared <smb::folder_impl> (url));
#endif

  else
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("unhandled folder scheme"));
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

} // namespace mobius::io


