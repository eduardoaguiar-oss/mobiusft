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
#include "block_impl_slice.h"
#include <mobius/exception.inc>
#include <stdexcept>
#include <vector>

namespace mobius::vfs
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Constructor
//! \param parent Parent block object
//! \param type Block type
//! \param start Start position at parent block
//! \param end End position at parent block
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
block_impl_slice::block_impl_slice (
  const block& parent,
  const std::string& type,
  offset_type start,
  offset_type end
)
 : type_ (type),
   start_ ((start >= 0) ? start : start + parent.get_size ()),
   end_ ((end >= 0) ? end : end + parent.get_size ()),
   size_ (end_ - start_ + 1),
   parent_ (parent)
{
  attributes_.set ("start", start_);
  attributes_.set ("end", end_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Constructor
//! \param state Object state
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
block_impl_slice::block_impl_slice (const mobius::pod::map& state)
{
  if (state.get ("classname") != "slice")
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid state"));

  type_ = static_cast <std::string> (state.get ("type"));
  start_ = static_cast <std::int64_t> (state.get ("start"));
  end_ = static_cast <std::int64_t> (state.get ("end"));
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
block_impl_slice::get_state () const
{
  mobius::pod::map state;

  // set metadata
  state.set ("classname", "slice");
  state.set ("type", type_);
  state.set ("start", start_);
  state.set ("end", end_);
  state.set ("size", size_);
  state.set ("uid", uid_);
  state.set ("is_handled", is_handled_);
  state.set ("attributes", attributes_);

  // set parents
  std::vector <mobius::pod::data> lp;
  lp.push_back (parent_.get_uid ());
  state.set ("parents", lp);

  // set children
  std::vector <mobius::pod::data> lc;
  for (const auto& child : get_children ())
    lc.push_back (child.get_uid ());
  state.set ("children", lc);

  return state;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Set complete flag
//! \param flag Flag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
block_impl_slice::set_complete (bool)
{
  throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("slice blocks are always complete"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Set available flag
//! \param flag Flag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
block_impl_slice::set_available (bool)
{
  throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("slice blocks are always available"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Add parent
//! \param parent Block object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
block_impl_slice::add_parent (const block& parent)
{
  if (parent_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("parent block already set"));

  parent_ = parent;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Create new reader
//! \return New reader
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::io::reader
block_impl_slice::new_reader () const
{
  if (!parent_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("parent block not set"));

  auto reader = parent_.new_reader ();

  if (start_ > 0 || end_ < (parent_.get_size () - 1))
    return mobius::io::new_slice_reader (reader, start_, end_);

  return reader;
}

} // namespace mobius::vfs
