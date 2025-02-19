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
#include "block_impl_disk.h"
#include <mobius/exception.inc>
#include <stdexcept>
#include <vector>

namespace mobius::vfs
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Constructor
//! \param disk Datasource object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
block_impl_disk::block_impl_disk (const disk& disk)
 : disk_ (disk),
   size_ (disk.get_size ()),
   attributes_ (disk.get_attributes ().clone ())
{
  attributes_.set ("description", disk.get_name ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Constructor
//! \param state Object state
//! \deprecated since=2.5 datasource type blocks
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
block_impl_disk::block_impl_disk (const mobius::pod::map& state)
{
  auto classname = state.get <std::string> ("classname");

  if (classname != "disk" && classname != "datasource")
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid state"));

  if (state.contains ("disk"))
    disk_ = disk (mobius::pod::map (state.get ("disk")));
  else
    disk_ = disk (mobius::pod::map (state.get ("datasource")));

  size_ = static_cast <std::int64_t> (state.get ("size"));
  uid_ = static_cast <std::int64_t> (state.get ("uid"));
  is_handled_ = static_cast <bool> (state.get ("is_handled"));
  attributes_ = mobius::pod::map (state.get ("attributes"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get object state
//! \return Object state
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::pod::map
block_impl_disk::get_state () const
{
  mobius::pod::map state;

  // metadata
  state.set ("classname", "disk");
  state.set ("disk", disk_.get_state ());
  state.set ("size", size_);
  state.set ("uid", uid_);
  state.set ("is_handled", is_handled_);
  state.set ("attributes", attributes_);

  // children
  std::vector <mobius::pod::data> l;
  for (const auto& child : get_children ())
    l.push_back (child.get_uid ());
  state.set ("children", l);

  return state;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Set complete flag
//! \param flag Flag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
block_impl_disk::set_complete (bool)
{
  throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("disk blocks are always complete"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Set available flag
//! \param flag Flag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
block_impl_disk::set_available (bool)
{
  throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("disk blocks are always available"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Add parent
//! \param parent Block object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
block_impl_disk::add_parent (const block&)
{
  throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("block does not accept parent block"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Create new reader
//! \return New reader
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::io::reader
block_impl_disk::new_reader () const
{
  return disk_.new_reader ();
}

} // namespace mobius::vfs
