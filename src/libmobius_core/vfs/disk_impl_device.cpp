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
#include <mobius/core/vfs/disk_impl_device.hpp>
#include <mobius/core/vfs/util.hpp>
#include <mobius/io/file.h>
#include <mobius/core/system/device_list.hpp>
#include <mobius/exception.inc>
#include <algorithm>
#include <stdexcept>

namespace mobius::core::vfs
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param uid Device UID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
disk_impl_device::disk_impl_device (const std::string& uid)
  : uid_ (uid)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param state Object state
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
disk_impl_device::disk_impl_device (const mobius::core::pod::map& state)
{
  uid_ = std::string (state.get ("uid"));
  name_ = std::string (state.get ("name"));
  size_ = std::int64_t (state.get ("size"));
  attributes_ = state.get ("attrs");

  metadata_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if disk is available
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
disk_impl_device::is_available () const
{
  bool rc = false;

  _load_device ();

  if (device_)
    {
      auto f = mobius::io::new_file_by_path (device_.get_node ());
      rc = f.exists ();
    }

  return rc;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if attribute exists
// @param id Attribute ID
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
disk_impl_device::has_attribute (const std::string& id) const
{
  return attributes_.contains (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set attribute
// @param id Attribute ID
// @param value Attribute value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
disk_impl_device::set_attribute (const std::string& id, const mobius::core::pod::data& value)
{
  attributes_.set (id, value);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get attribute
// @param id Attribute ID
// @return Data object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::data
disk_impl_device::get_attribute (const std::string& id) const
{
  return attributes_.get (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get attributes
// @return Attributes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::map
disk_impl_device::get_attributes () const
{
  _load_metadata ();
  return attributes_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get object state
// @return Object state
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::map
disk_impl_device::get_state () const
{
  _load_metadata ();

  mobius::core::pod::map state;

  state.set ("type", get_type ());
  state.set ("uid", uid_);
  state.set ("name", name_);
  state.set ("size", size_);
  state.set ("attrs", attributes_);

  return state;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get path to underlying file, when available
// @return File path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
disk_impl_device::get_path () const
{
  _load_device ();

  if (!device_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("device not found"));

  return device_.get_node ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create new reader
// @return New reader
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::io::reader
disk_impl_device::new_reader () const
{
  _load_device ();

  if (!device_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("device not found"));

  return device_.new_reader ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load device
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
disk_impl_device::_load_device () const
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Check if device is already loaded
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (device_loaded_)
    return;

  std::lock_guard lock (device_loaded_);

  if (device_loaded_)           // check again
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Load device
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::core::system::device_list dev_list;
  dev_list.scan ();

  auto iter = std::find_if (
                  dev_list.begin (),
                  dev_list.end (),
                  [this](const auto& dev){ return dev.get_property ("ID_SERIAL") == this->uid_; }
              );

  if (iter != dev_list.end ())
    device_ = *iter;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Set flag loaded
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  device_loaded_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load metadata
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
disk_impl_device::_load_metadata () const
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Check if metadata is already loaded
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  if (metadata_loaded_)
    return;

  std::lock_guard lock (metadata_loaded_);

  if (metadata_loaded_)           // check again
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Load device
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  _load_device ();

  if (!device_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("device not found"));

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Retrieve metadata
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string vendor = device_.get_property ("ID_VENDOR");
  std::string model = device_.get_property ("ID_MODEL");
  std::string serial = device_.get_property ("ID_SERIAL_SHORT");

  normalize_drive_info (vendor, model, serial);

  const std::string sector_size_str = device_.get_sysattr ("queue/logical_block_size");
  std::uint64_t sector_size = std::strtoull (sector_size_str.c_str (), nullptr, 0);

  const std::string sectors_str = device_.get_sysattr ("size");
  std::uint64_t sectors = std::strtoull (sectors_str.c_str (), nullptr, 0);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Fill attributes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  size_ = sectors * sector_size;
  name_ = vendor + " S/N: " + serial;

  attributes_.set ("uid", uid_);
  attributes_.set ("vendor", vendor);
  attributes_.set ("model", model);
  attributes_.set ("serial", serial);
  attributes_.set ("firmware", device_.get_property ("ID_REVISION"));
  attributes_.set ("wwn", device_.get_property ("ID_WWN"));
  attributes_.set ("size", size_);
  attributes_.set ("sectors", sectors);
  attributes_.set ("sector_size", sector_size);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Set flag loaded
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  metadata_loaded_ = true;
}

} // namespace mobius::core::vfs


