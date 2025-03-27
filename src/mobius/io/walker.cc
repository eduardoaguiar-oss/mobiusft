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
#include "walker.h"
#include <mobius/core/log.h>
#include <mobius/string_functions.h>
#include <stdexcept>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get files from entries
// @param entries Entries
// @return Files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::vector <mobius::io::file>
_get_files_from_entries (const std::vector <mobius::io::entry>& entries)
{
  std::vector <mobius::io::file> files;

  std::for_each (
      entries.begin (),
      entries.end (),
      [&files](const mobius::io::entry& e){
          if (e.is_file ())
            files.push_back (e.get_file ());
      }
  );

  return files;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folders from entries
// @param entries Entries
// @return Folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::vector <mobius::io::folder>
_get_folders_from_entries (const std::vector <mobius::io::entry>& entries)
{
  std::vector <mobius::io::folder> folders;

  std::for_each (
      entries.begin (),
      entries.end (),
      [&folders](const mobius::io::entry& e){
          if (e.is_folder ())
            folders.push_back (e.get_folder ());
      }
  );

  return folders;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Walk through path, getting entries that match
// @param folder Folder object
// @param rpath Relative path
// @param cmp Compare function
// @return Entries that match compare function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::entry>
_walk (
  const mobius::io::folder& folder,
  const std::string& rpath,
  std::function <bool(const std::string&, const std::string&)> cmp
)
{
  std::vector <mobius::io::entry> entries;

  mobius::io::walker w (folder);

  auto pos = rpath.find ('/');

  // last level = filename
  if (pos == std::string::npos)
    {
      auto f = [cmp,rpath](const mobius::io::entry& e){ return cmp (rpath, e.get_name ()); };
      entries = w.get_entries_by_predicate (f);
    }

  // intermediate level = folder name
  else
    {
      std::string first_rpath = rpath.substr (0, pos);
      std::string remaining_rpath = rpath.substr (pos + 1);
      auto f = [cmp,first_rpath](const mobius::io::folder& fld){ return cmp (first_rpath, fld.get_name ()); };

      for (const auto& child : w.get_folders_by_predicate (f))
        {
          auto child_entries = _walk (child, remaining_rpath, cmp);

          entries.insert (entries.end (),
                          std::make_move_iterator (child_entries.begin ()),
                          std::make_move_iterator (child_entries.end ())
                         );
        }
    }

  return entries;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Walk folder recursively, getting entries that match predicate function
// @param folder Folder object
// @param f Predicate function
// @return Entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::entry>
_find (
  const mobius::io::folder& folder,
  std::function <bool(const mobius::io::entry&)> f
)
{
  std::vector <mobius::io::entry> entries;
  mobius::io::walker w (folder);

  // get entries that match predicate function
  auto child_entries = w.get_entries_by_predicate (f);

  entries.insert (entries.end (),
                  std::make_move_iterator (child_entries.begin ()),
                  std::make_move_iterator (child_entries.end ())
                 );

  // navigate through subfolders
  for (const auto& fld : w.get_folders ())
    {
      auto child_entries = _find (fld, f);

      entries.insert (entries.end (),
                      std::make_move_iterator (child_entries.begin ()),
                      std::make_move_iterator (child_entries.end ())
                     );
    }

  return entries;
}

} // namespace
namespace mobius::io
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief walker implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class walker::impl
{
public:

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl (const impl&) = delete;
  impl (impl&&) = delete;
  explicit impl (const mobius::io::folder&);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl& operator= (const impl&) = delete;
  impl& operator= (impl&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Set ignore reallocated flag
  // @param f Flag (true/false)
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_ignore_reallocated (bool f)
  {
    flag_ignore_reallocated_ = f;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Set case sensitive flag
  // @param f Flag (true/false)
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_case_sensitive (bool f)
  {
    flag_case_sensitive_ = f;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::vector <mobius::io::entry> get_entries () const;
  std::vector <mobius::io::entry> get_entries_by_name (const std::string&) const;
  std::vector <mobius::io::entry> get_entries_by_path (const std::string&) const;
  std::vector <mobius::io::entry> get_entries_by_pattern (const std::string&) const;
  std::vector <mobius::io::entry> get_entries_by_predicate (std::function <bool (const mobius::io::entry&)>) const;
  std::vector <mobius::io::entry> find_entries (std::function <bool (const mobius::io::entry&)>) const;
  std::vector <mobius::io::file> get_files () const;
  std::vector <mobius::io::file> get_files_by_name (const std::string&) const;
  std::vector <mobius::io::file> get_files_by_path (const std::string&) const;
  std::vector <mobius::io::file> get_files_by_pattern (const std::string&) const;
  std::vector <mobius::io::file> get_files_by_predicate (std::function <bool (const mobius::io::file&)>) const;
  std::vector <mobius::io::file> find_files (std::function <bool (const mobius::io::file&)>) const;
  std::vector <mobius::io::folder> get_folders () const;
  std::vector <mobius::io::folder> get_folders_by_name (const std::string&) const;
  std::vector <mobius::io::folder> get_folders_by_path (const std::string&) const;
  std::vector <mobius::io::folder> get_folders_by_pattern (const std::string&) const;
  std::vector <mobius::io::folder> get_folders_by_predicate (std::function <bool (const mobius::io::folder&)>) const;
  std::vector <mobius::io::folder> find_folders (std::function <bool (const mobius::io::folder&)>) const;

private:
  // @brief Base folder
  const mobius::io::folder folder_;

  // @brief Flag to ignore reallocated entries
  bool flag_ignore_reallocated_ = true;

  // @brief Flag to compare file names with case sensitivity or not
  bool flag_case_sensitive_ = false;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param folder Base folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
walker::impl::impl (const mobius::io::folder& folder)
    : folder_ (folder)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get entries
// @return Entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::entry>
walker::impl::get_entries () const
{
  std::vector <mobius::io::entry> entries;

  auto f = flag_ignore_reallocated_ ?
           [](const mobius::io::entry& e){ return !e.is_reallocated (); } :
           [](const mobius::io::entry&){ return true; };

  try
    {
      auto folder_entries = folder_.get_children ();

      std::copy_if (
         folder_entries.begin (),
         folder_entries.end (),
         std::back_inserter (entries),
         f
      );
    }
  catch (const std::exception& e)
    {
      mobius::core::log log (__FILE__, __FUNCTION__);
      log.warning (__LINE__, e.what ());
    }

  return entries;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get entries by name
// @param name Name
// @return Entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::entry>
walker::impl::get_entries_by_name (const std::string& name) const
{
  auto cmp = (flag_case_sensitive_) ?
               mobius::string::case_sensitive_match :
               mobius::string::case_insensitive_match;

  auto f = [cmp,&name](const auto& e){
      return cmp (name, e.get_name ());
  };

  return get_entries_by_predicate (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get entries by path
// @param path Relative path
// @return Entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::entry>
walker::impl::get_entries_by_path (const std::string& path) const
{
  auto cmp = (flag_case_sensitive_) ?
               mobius::string::case_sensitive_match :
               mobius::string::case_insensitive_match;

  return _walk (folder_, path, cmp);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get entries by pattern
// @param pattern fnmatch function pattern
// @return Entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::entry>
walker::impl::get_entries_by_pattern (const std::string& pattern) const
{
  auto cmp = (flag_case_sensitive_) ?
               mobius::string::fnmatch :
               mobius::string::case_insensitive_fnmatch;

  return _walk (folder_, pattern, cmp);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get entries by predicate function
// @param f Predicate function
// @return Entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::entry>
walker::impl::get_entries_by_predicate (std::function <bool (const mobius::io::entry&)> f) const
{
  std::vector <mobius::io::entry> entries;

  auto folder_entries = get_entries ();

  std::copy_if (
      std::make_move_iterator (folder_entries.begin ()),
      std::make_move_iterator (folder_entries.end ()),
      std::back_inserter (entries),
      f
  );

  return entries;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Find entries recursively by predicate function
// @param f Predicate function
// @return Entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::entry>
walker::impl::find_entries (std::function <bool (const mobius::io::entry&)> f) const
{
  return _find (folder_, f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get files
// @return Files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::file>
walker::impl::get_files () const
{
  return _get_files_from_entries (get_entries ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get files by name
// @param name Name
// @return Files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::file>
walker::impl::get_files_by_name (const std::string& name) const
{
  return _get_files_from_entries (get_entries_by_name (name));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get files by path
// @param path Relative path
// @return Files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::file>
walker::impl::get_files_by_path (const std::string& path) const
{
  return _get_files_from_entries (get_entries_by_path (path));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get files by pattern
// @param pattern fnmatch function pattern
// @return Files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::file>
walker::impl::get_files_by_pattern (const std::string& pattern) const
{
  return _get_files_from_entries (get_entries_by_pattern (pattern));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get files by predicate function
// @param f Predicate function
// @return Files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::file>
walker::impl::get_files_by_predicate (std::function <bool (const mobius::io::file&)> f) const
{
  auto entry_f = [f](const mobius::io::entry& e){
      return e.is_file () && f (e.get_file ());
  };

  return _get_files_from_entries (get_entries_by_predicate (entry_f));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Find files recursively by predicate function
// @param f Predicate function
// @return Files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::file>
walker::impl::find_files (std::function <bool (const mobius::io::file&)> f) const
{
  auto entry_f = [f](const mobius::io::entry& e){
      return e.is_file () && f (e.get_file ());
  };

  return _get_files_from_entries (find_entries (entry_f));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folders
// @return Folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::folder>
walker::impl::get_folders () const
{
  return _get_folders_from_entries (get_entries ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folders by name
// @param name Name
// @return Folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::folder>
walker::impl::get_folders_by_name (const std::string& name) const
{
  return _get_folders_from_entries (get_entries_by_name (name));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folders by path
// @param path Relative path
// @return Folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::folder>
walker::impl::get_folders_by_path (const std::string& path) const
{
  return _get_folders_from_entries (get_entries_by_path (path));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folders by pattern
// @param pattern fnmatch function pattern
// @return Folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::folder>
walker::impl::get_folders_by_pattern (const std::string& pattern) const
{
  return _get_folders_from_entries (get_entries_by_pattern (pattern));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folders by predicate function
// @param f Predicate function
// @return Folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::folder>
walker::impl::get_folders_by_predicate (std::function <bool (const mobius::io::folder&)> f) const
{
  auto entry_f = [f](const mobius::io::entry& e){
      return e.is_folder () && f (e.get_folder ());
  };

  return _get_folders_from_entries (get_entries_by_predicate (entry_f));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Find folders recursively by predicate function
// @param f Predicate function
// @return Folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::folder>
walker::impl::find_folders (std::function <bool (const mobius::io::folder&)> f) const
{
  auto entry_f = [f](const mobius::io::entry& e){
      return e.is_folder () && f (e.get_folder ());
  };

  return _get_folders_from_entries (find_entries (entry_f));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param folder Base folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
walker::walker (const mobius::io::folder& folder)
  : impl_ (std::make_shared <impl> (folder))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set ignore reallocated flag
// @param flag Flag (true/false)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
walker::set_ignore_reallocated (bool flag)
{
  impl_->set_ignore_reallocated (flag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set case sensitive flag
// @param flag Flag (true/false)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
walker::set_case_sensitive (bool flag)
{
  impl_->set_case_sensitive (flag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get entries
// @return Entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::entry>
walker::get_entries () const
{
  return impl_->get_entries ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get entries by name
// @param name Name
// @return Entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::entry>
walker::get_entries_by_name (const std::string& name) const
{
  return impl_->get_entries_by_name (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get entries by path
// @param path Relative path
// @return Entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::entry>
walker::get_entries_by_path (const std::string& path) const
{
  return impl_->get_entries_by_path (path);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get entries by pattern
// @param pattern fnmatch function pattern
// @return Entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::entry>
walker::get_entries_by_pattern (const std::string& pattern) const
{
  return impl_->get_entries_by_pattern (pattern);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get entries by predicate function
// @param f Predicate function
// @return Entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::entry>
walker::get_entries_by_predicate (std::function <bool (const mobius::io::entry&)> f) const
{
  return impl_->get_entries_by_predicate (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Find entries recursively by predicate function
// @param f Predicate function
// @return Entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::entry>
walker::find_entries (std::function <bool (const mobius::io::entry&)> f) const
{
  return impl_->find_entries (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get files
// @return Files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::file>
walker::get_files () const
{
  return impl_->get_files ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get files by name
// @param name Name
// @return Files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::file>
walker::get_files_by_name (const std::string& name) const
{
  return impl_->get_files_by_name (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get files by path
// @param path Relative path
// @return Files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::file>
walker::get_files_by_path (const std::string& path) const
{
  return impl_->get_files_by_path (path);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get files by pattern
// @param pattern fnmatch function pattern
// @return Files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::file>
walker::get_files_by_pattern (const std::string& pattern) const
{
  return impl_->get_files_by_pattern (pattern);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get files by predicate function
// @param f Predicate function
// @return Files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::file>
walker::get_files_by_predicate (std::function <bool (const mobius::io::file&)> f) const
{
  return impl_->get_files_by_predicate (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Find files recursively by predicate function
// @param f Predicate function
// @return Files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::file>
walker::find_files (std::function <bool (const mobius::io::file&)> f) const
{
  return impl_->find_files (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folders
// @return Folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::folder>
walker::get_folders () const
{
  return impl_->get_folders ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folders by name
// @param name Name
// @return Folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::folder>
walker::get_folders_by_name (const std::string& name) const
{
  return impl_->get_folders_by_name (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folders by path
// @param path Relative path
// @return Folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::folder>
walker::get_folders_by_path (const std::string& path) const
{
  return impl_->get_folders_by_path (path);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folders by pattern
// @param pattern fnmatch function pattern
// @return Folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::folder>
walker::get_folders_by_pattern (const std::string& pattern) const
{
  return impl_->get_folders_by_pattern (pattern);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folders by predicate function
// @param f Predicate function
// @return Folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::folder>
walker::get_folders_by_predicate (std::function <bool (const mobius::io::folder&)> f) const
{
  return impl_->get_folders_by_predicate (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Find folders recursively by predicate function
// @param f Predicate function
// @return Folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::folder>
walker::find_folders (std::function <bool (const mobius::io::folder&)> f) const
{
  return impl_->find_folders (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file by name
// @param name Name
// @return File
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::io::file
walker::get_file_by_name (const std::string& name) const
{
  mobius::io::file tmp_f;

  for (const auto& f : get_files_by_name (name))
    {
      if (!f.is_deleted ())
        return f;

      if (!tmp_f)
        tmp_f = f;
    }

  return tmp_f;
}


} // namespace mobius::io


