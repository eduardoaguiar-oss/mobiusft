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
#include "vfs.h"
#include "filesystem.h"
#include "block_impl_disk.h"
#include <mobius/core/log.h>
#include <mobius/core/resource.h>
#include <mobius/exception.inc>
#include <mobius/string_functions.h>
#include <mobius/thread_safe_flag.h>
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline constexpr int VERSION = 1;

} // namespace

namespace mobius::vfs
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief vfs implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class vfs::impl
{
public:

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl () = default;
  impl (const impl&) = delete;
  impl (impl&&) = delete;
  explicit impl (const mobius::pod::map&);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl& operator= (const impl&) = delete;
  impl& operator= (impl&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::pod::map get_state () const;
  void clear ();
  void rescan ();
  bool is_available () const;
  std::size_t add_disk (const mobius::vfs::disk&);
  void remove_disk (std::size_t);
  std::vector <disk> get_disks () const;
  std::vector <block> get_blocks () const;
  std::vector<mobius::io::entry> get_root_entries () const;

private:
  // @brief Datasources
  std::vector <disk> disks_;

  // @brief Data blocks
  mutable std::vector <block> blocks_;

  // @brief Filesystems
  mutable std::vector <filesystem> filesystems_;

  // @brief Root entries
  mutable std::vector <mobius::io::entry> root_entries_;

  // @brief Data blocks loaded flag
  mutable thread_safe_flag blocks_loaded_;

  // @brief Root entries loaded flag
  mutable thread_safe_flag root_entries_loaded_;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Helper functions
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void _load_blocks () const;
  void _load_root_entries () const;
  void _add_blocks (std::vector <block>&) const;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param state Object state
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
vfs::impl::impl (const mobius::pod::map& state)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create disks
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (state.contains ("disks"))
    {
      auto disks = state.get <std::vector <mobius::pod::data>> ("disks");

      for (const auto& d_state : disks)
        disks_.emplace_back (mobius::pod::map (d_state));
    }

  //! \deprecated since=2.5 old datasources
  else if (state.contains ("datasources"))
    {
      auto disks = state.get <std::vector <mobius::pod::data>> ("datasources");

      for (const auto& d_state : disks)
        disks_.emplace_back (mobius::pod::map (d_state));
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create blocks
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (state.contains ("blocks"))
    {
      auto blocks = state.get <std::vector <mobius::pod::data>> ("blocks");

      // Build blocks
      for (const auto& b_data : blocks)
        {
          auto b_state = mobius::pod::map (b_data);
          blocks_.push_back (mobius::vfs::block (b_state));
        }

      // Add parents and children to blocks
      for (const auto& b_data : blocks)
        {
          auto b_state = mobius::pod::map (b_data);
          auto uid = b_state.get <std::int64_t> ("uid");
          auto block = blocks_.at (uid - 1);
          auto parents = b_state.get <std::vector <std::int64_t>> ("parents");
          auto children = b_state.get <std::vector <std::int64_t>> ("children");

          for (std::uint64_t parent_uid : parents)
            {
              if (parent_uid > 0 && parent_uid <= blocks_.size ())
                block.add_parent (blocks_[parent_uid - 1]);

              else
                throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid parent UID: " + std::to_string (parent_uid)));
            }

          for (std::uint64_t child_uid : children)
            {
              if (child_uid > 0 && child_uid <= blocks_.size ())
                block.add_child (blocks_[child_uid - 1]);

              else
                throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid child UID: " + std::to_string (child_uid)));
            }
        }

      blocks_loaded_ = true;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get object state
// @return Object state
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::pod::map
vfs::impl::get_state () const
{
  mobius::pod::map state;
  state.set ("version", VERSION);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Datasources
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::vector <mobius::pod::data> disks;

  for (const auto& d : get_disks ())
    disks.push_back (d.get_state ());

  state.set ("disks", disks);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Blocks
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::vector <mobius::pod::data> blocks;

  for (const auto& b : get_blocks ())
    blocks.push_back (b.get_state ());

  state.set ("blocks", blocks);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Return state
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  return state;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Rescan blocks and root entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs::impl::rescan ()
{
  std::lock_guard lock (blocks_loaded_);
  blocks_.clear ();
  blocks_loaded_ = false;

  std::lock_guard lock2 (root_entries_loaded_);
  filesystems_.clear ();
  root_entries_.clear ();
  root_entries_loaded_ = false;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clear VFS
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs::impl::clear ()
{
  disks_.clear ();
  rescan ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if VFS is available
// @return True/False
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
vfs::impl::is_available () const
{
  return !disks_.empty () &&
          std::all_of (
            disks_.cbegin (),
            disks_.cend (),
            [](const disk& d){ return d.is_available (); }
          );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add disk
// @param d Datasource object
// @return Datasource index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::size_t
vfs::impl::add_disk (const mobius::vfs::disk& d)
{
  disks_.push_back (d);
  rescan ();

  return disks_.size () - 1;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove disk
// @param idx Datasource index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs::impl::remove_disk (std::size_t idx)
{
  if (idx < disks_.size ())
    {
      disks_.erase (disks_.begin () + idx);
      rescan ();
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get disks
// @return Datasources
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <disk>
vfs::impl::get_disks () const
{
  return disks_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get blocks
// @return blocks
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <block>
vfs::impl::get_blocks () const
{
  _load_blocks ();
  return blocks_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get root entries
// @return Root entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <mobius::io::entry>
vfs::impl::get_root_entries () const
{
  _load_root_entries ();
  return root_entries_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add new blocks to VFS, recursively
// @param blocks New blocks
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs::impl::_add_blocks (std::vector <block>& blocks) const
{
  for (auto& b : blocks)
    {
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Add block if UID == 0
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      if (!b.get_uid ())
        {
          b.set_uid (blocks_.size () + 1);
          blocks_.push_back (b);
        }

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Add children, recursively
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      auto children = b.get_children ();
      _add_blocks (children);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load blocks on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs::impl::_load_blocks () const
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Check if blocks are already loaded
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (blocks_loaded_)
    return;

  std::lock_guard lock (blocks_loaded_);

  if (blocks_loaded_)           // check again, after lock
    return;

  mobius::core::log log (__FILE__, __FUNCTION__);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create disk blocks
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  blocks_.clear ();

  for (const auto& d : get_disks ())
    {
      block b (std::make_shared <block_impl_disk> (d));
      b.set_uid (blocks_.size () + 1);
      blocks_.push_back (b);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create decoders vector
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::vector <block_decoder_resource_type> decoders;

  log.debug (__LINE__, "decoders:");

  for (const auto& resource : mobius::core::get_resources ("vfs.block.decoder"))
    {
      log.debug (__LINE__, resource.get_id ());
      auto decoder = resource.get_value <block_decoder_resource_type> ();
      decoders.push_back (decoder);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Main loop: While new blocks are handled, continue
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bool flag_run = true;

  while (flag_run)
    {
      flag_run = false;

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Create incomplete blocks vector
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      std::vector <block> incomplete_blocks;

      std::copy_if (blocks_.begin (),
                    blocks_.end (),
                    std::back_inserter (incomplete_blocks),
                    [](const block& b){
                        return !b.is_complete ();
                    }
                   );

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Create unknown blocks vector
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      std::vector <block> unknown_blocks;

      std::copy_if (blocks_.begin (),
                    blocks_.end (),
                    std::back_inserter (unknown_blocks),
                    [](const block& b){
                        return !b.is_handled () && b.is_available ();
                    }
                   );

      log.debug (__LINE__, "Unknown blocks: ");

      for (const auto& b : unknown_blocks)
        log.debug (__LINE__, std::to_string (b.get_uid ()));

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // Try to decode blocks
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      for (auto& b : unknown_blocks)
        {
         log.debug (__LINE__, "Detecting block: " + std::to_string (b.get_uid ()));

          for (auto& decoder : decoders)
            {
              log.debug (__LINE__, "Decoder...");
              log.debug (__LINE__, "Children: " + std::to_string (b.get_children ().size ()));

              std::string text;
              for (const auto& vb : blocks_)
                text += ' ' + std::to_string (vb.get_uid ());
              log.debug (__LINE__, "VFS blocks (1):" + text);

              std::vector <block> new_blocks;

              try
                {
                  if (decoder (b, new_blocks, incomplete_blocks))
                    {
                      log.debug (__LINE__, "Detected!");

                      // Add blocks and subblocks to block list
                      _add_blocks (new_blocks);

                      // Add blocks to current block children list
                      std::for_each (
                         new_blocks.begin (),
                         new_blocks.end (),
                         [&b](auto& c) { b.add_child (c); }
                      );

                      // Set current block handled
                      b.set_handled (true);

                      flag_run = true;
                    }
                }
              catch (const std::exception& e)
                {
                  log.warning (__LINE__, e.what ());
                }

              log.debug (__LINE__, "Children: " + std::to_string (b.get_children ().size ()));

              text.clear ();
              for (const auto& vb : blocks_)
                text += ' ' + std::to_string (vb.get_uid ());
              log.debug (__LINE__, "VFS blocks (2):" + text);
            }
        }
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Set blocks loaded
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  blocks_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load root entries on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs::impl::_load_root_entries () const
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Check if root entries are already loaded
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (root_entries_loaded_)
    return;

  std::lock_guard lock (root_entries_loaded_);

  if (root_entries_loaded_)           // check again, after lock
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Load filesystems
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::core::log log (__FILE__, __FUNCTION__);
  filesystems_.clear ();

  for (const auto& block : get_blocks ())
    {
      if (block.get_type () == "filesystem")
        {
          try
            {
              filesystems_.emplace_back (
                 block.new_reader (),
                 0,
                 block.get_attribute <std::string> ("impl_type")
              );
            }
          catch (const std::exception& e)
            {
              log.warning (__LINE__, e.what ());
            }
        }
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Load root entries
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  root_entries_.clear ();
  std::uint64_t idx = 1;

  for (const auto& fs : filesystems_)
    {
      try
        {
          const std::string name = "FS" + mobius::string::to_string (idx, 2);

          auto folder = fs.get_root_folder ();
          folder.set_name (name);
          folder.set_path ("/" + name);

          root_entries_.emplace_back (folder);
          idx++;
        }
      catch (const std::exception& e)
        {
          log.warning (__LINE__, e.what ());
        }
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Set root entries loaded
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  root_entries_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
vfs::vfs ()
  : impl_ (std::make_shared <impl> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param state Object state
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
vfs::vfs (const mobius::pod::map& state)
  : impl_ (std::make_shared <impl> (state))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get object state
// @return Object state
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::pod::map
vfs::get_state () const
{
  return impl_->get_state ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clear VFS
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs::clear ()
{
  impl_->clear ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Rescan VFS blocks and root entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs::rescan ()
{
  impl_->rescan ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if VFS is available
// @return True/False
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
vfs::is_available () const
{
  return impl_->is_available ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add disk
// @param d Datasource object
// @return Datasource index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::size_t
vfs::add_disk (const mobius::vfs::disk& d)
{
  return impl_->add_disk (d);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove disk
// @param idx Datasource index
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs::remove_disk (std::size_t idx)
{
  impl_->remove_disk (idx);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get disks
// @return Datasources
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<disk>
vfs::get_disks () const
{
  return impl_->get_disks ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get blocks
// @return blocks
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <block>
vfs::get_blocks () const
{
  return impl_->get_blocks ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get root entries
// @return Root entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<mobius::io::entry>
vfs::get_root_entries () const
{
  return impl_->get_root_entries ();
}

} // namespace mobius::vfs


