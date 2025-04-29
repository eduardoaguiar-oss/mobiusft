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
#include <mobius/core/vfs/block.hpp>
#include <mobius/core/vfs/block_impl_null.hpp>
#include <mobius/core/vfs/block_impl_disk.hpp>
#include <mobius/core/vfs/block_impl_slice.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/resource.hpp>
#include <mobius/exception.inc>
#include <stdexcept>

namespace mobius::core::vfs
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
block::block ()
 : impl_ (std::make_shared <block_impl_null> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor from implementation pointer
// @param impl Implementation pointer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
block::block (const std::shared_ptr <block_impl_base>& impl)
  : impl_ (impl)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param state Object state
//! \deprecated since=2.5 datasource type blocks
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
block::block (const mobius::core::pod::map& state)
{
  if (!state.contains ("classname"))
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid state"));

  const std::string classname = static_cast <std::string> (state.get ("classname"));

  if (classname == "slice")
    impl_ = std::make_shared <block_impl_slice> (state);

  else if (classname == "disk" || classname == "datasource")
    impl_ = std::make_shared <block_impl_disk> (state);

  else
    {
      auto builder = mobius::core::get_resource_value <block_builder_resource_type> ("vfs.block.builder." + classname);
      *this = builder (state);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create child slice block from block
// @param type Block type
// @param start Start position at parent block
// @param end End position at parent block
// @return Block object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
block
block::new_slice_block (const std::string& type, offset_type start, offset_type end)
{
  auto b = mobius::core::vfs::new_slice_block (*this, type, start, end);
  add_child (b);

  return b;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add freespaces between non-contiguous blocks
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
block::add_freespaces ()
{
  using address_type = std::uint64_t;

  // sort blocks
  auto sorted_blocks = get_children ();

  std::sort (sorted_blocks.begin (),
             sorted_blocks.end (),
             [](const block& b1, const block& b2)
               {
                 return static_cast <std::int64_t> (b1.get_attribute ("start_address")) <
                        static_cast <std::int64_t> (b2.get_attribute ("start_address"));
               }
            );

  // fill in freespace where it is necessary
  address_type pos = 0;
  std::vector <block> blocks;

  for (const auto& b : sorted_blocks)
    {
      address_type b_start_address = static_cast <std::int64_t> (b.get_attribute ("start_address"));
      address_type b_end_address = static_cast <std::int64_t> (b.get_attribute ("end_address"));

      if (pos < b_start_address)
        {
          address_type start_address = pos;
          address_type end_address = b_start_address - 1;

          auto freespace_block = new_slice_block ("freespace", start_address, end_address);
          freespace_block.set_attribute ("start_address", start_address);
          freespace_block.set_attribute ("end_address", end_address);
          freespace_block.set_attribute ("size", end_address - start_address + 1);
          freespace_block.set_attribute ("description", "Freespace");

          blocks.push_back (freespace_block);
        }

      blocks.push_back (b);
      pos = b_end_address + 1;
    }

  // add last block, if necessary
  address_type ending_address = get_size () - 1;

  if (pos < ending_address)
    {
      address_type start_address = pos;
      address_type end_address = ending_address;

      auto freespace_block = new_slice_block ("freespace", start_address, end_address);
      freespace_block.set_attribute ("start_address", start_address);
      freespace_block.set_attribute ("end_address", end_address);
      freespace_block.set_attribute ("size", end_address - start_address + 1);
      freespace_block.set_attribute ("description", "Freespace");

      blocks.push_back (freespace_block);
    }

  set_children (blocks);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create slice block from block
// @param parent_block Parent block
// @param type Block type
// @param start Start position at parent block
// @param end End position at parent block
// @return Block object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
block
new_slice_block (
  const block& parent_block,
  const std::string& type,
  mobius::core::vfs::block::offset_type start,
  mobius::core::vfs::block::offset_type end
)
{
  // get parent block offset
  mobius::core::vfs::block::offset_type offset = 0;

  if (parent_block.has_attribute ("offset"))
    offset = parent_block.get_attribute <std::int64_t> ("offset");

  // create slice block
  auto b = block (std::make_shared <block_impl_slice> (parent_block, type, start, end));
  b.set_attribute ("offset", offset + start);

  // return newly created block
  return b;
}

} // namespace mobius::core::vfs


