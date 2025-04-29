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
#include <mobius/core/collection_impl_base.hpp>
#include <mobius/exception.inc>
#include <mobius/io/file_impl_null.h>
#include <mobius/io/folder_impl_null.h>
#include <mobius/core/vfs/tsk/folder_impl.hpp>
#include <mobius/core/vfs/tsk/file_impl.hpp>
#include <mobius/core/vfs/tsk/exception.hpp>
#include <tsk/libtsk.h>
#include <stdexcept>

namespace mobius::core::vfs::tsk
{
namespace
{
using entry_impl = mobius::io::folder_impl_base::entry_impl;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Collection implementation for folder entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class collection_impl_folder : public mobius::core::collection_impl_base <entry_impl>
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors and destructor
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  explicit collection_impl_folder (const fs_file&);
  collection_impl_folder (const collection_impl_folder&) = delete;
  collection_impl_folder (collection_impl_folder&&) = delete;
  ~collection_impl_folder ();

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Assignment operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  collection_impl_folder& operator= (const collection_impl_folder&) = delete;
  collection_impl_folder& operator= (collection_impl_folder&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bool get (entry_impl&) override;
  void reset () override;

private:
  fs_file fs_file_;
  TSK_FS_DIR *fs_dir_p_ = nullptr;
  std::uint32_t pos_ = 0;
  std::uint32_t count_ = 0;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize object
// @param f fs_file object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
collection_impl_folder::collection_impl_folder (const fs_file& f)
  : fs_file_ (f)
{
  if (!f)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid folder"));

  fs_dir_p_ = tsk_fs_dir_open_meta (
                 fs_file_.get_pointer ()->fs_info,
                 fs_file_.get_inode ()
              );

  if (!fs_dir_p_)
    throw std::runtime_error (TSK_EXCEPTION_MSG);

  pos_ = 0;
  count_ = tsk_fs_dir_getsize (fs_dir_p_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destroy object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
collection_impl_folder::~collection_impl_folder ()
{
  if (fs_dir_p_)
    {
      tsk_fs_dir_close (fs_dir_p_);
      fs_dir_p_ = nullptr;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folder entry
// @param e Entry reference
// @return true/false if entry was found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
collection_impl_folder::get (entry_impl& e)
{
  while (pos_ < count_)
    {
      // get dir entry
      TSK_FS_FILE *fp = tsk_fs_dir_get (fs_dir_p_, pos_++);
      if (!fp)
        throw std::runtime_error (TSK_EXCEPTION_MSG);

      // if entry is not '.' and '..', create and return true
      fs_file f (fp);

      if (f.get_inode () != fs_file_.get_inode () &&
          f.get_name () != "."
          && f.get_name () != "..")
        {
          if (f.get_type () == f.fs_file_type::folder)
            e.folder_p = std::make_shared <folder_impl> (f);

          else
            e.file_p = std::make_shared <file_impl> (f);

          return true;
        }
    }

  // no more entries...
  return false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset collection
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
collection_impl_folder::reset ()
{
  pos_ = 0;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize object
// @param fp Pointer to file structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::folder_impl (const fs_file& fp)
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
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

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
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

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
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("folder does not exist"));

  return false; //!name_.empty () && name_[0] == '.';
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
folder_impl::rename (const std::string&)
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new file object
// @param name File name
// @return Pointer to file object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::file_type
folder_impl::new_file (const std::string& name) const
{
  file_type f = std::make_shared <mobius::io::file_impl_null> ();
  auto children = get_children ();
  entry_impl e;

  while (children->get (e))
    {
      auto file_p = e.file_p;

      if (file_p && file_p->get_name () == name)
        {
          if (file_p->is_deleted ())
            f = file_p;

          else
            return file_p;
        }
    }

  return f;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new folder object
// @param name Folder name
// @return Pointer to folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::folder_type
folder_impl::new_folder (const std::string& name) const
{
  folder_type f = std::make_shared <mobius::io::folder_impl_null> ();
  auto children = get_children ();
  entry_impl e;

  while (children->get (e))
    {
      auto folder_p = e.folder_p;

      if (folder_p && folder_p->get_name () == name)
        {
          if (folder_p->is_deleted ())
            f = folder_p;

          else
            return folder_p;
        }
    }

  return f;
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
    return std::make_shared <folder_impl> (parent);

  return std::make_shared <mobius::io::folder_impl_null> ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get children
// @return Collection
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
folder_impl::children_type
folder_impl::get_children () const
{
  return std::make_shared <collection_impl_folder> (fs_file_);
}

} // namespace mobius::core::vfs::tsk


