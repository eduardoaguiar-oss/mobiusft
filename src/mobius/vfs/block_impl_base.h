#ifndef MOBIUS_VFS_BLOCK_IMPL_BASE_H
#define MOBIUS_VFS_BLOCK_IMPL_BASE_H

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
#include <mobius/io/reader.h>
#include <mobius/pod/map.h>
#include <cstdint>
#include <string>
#include <vector>

namespace mobius::vfs
{
class block;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Data block implementation base class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class block_impl_base
{
public:
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Datatypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  using uid_type = std::uint32_t;
  using size_type = std::uint64_t;
  using offset_type = std::int64_t;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors and destructor
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  block_impl_base () = default;
  block_impl_base (const block_impl_base&) = delete;
  block_impl_base (block_impl_base&&) = delete;
  virtual ~block_impl_base () = default;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Assignment operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  block_impl_base& operator= (const block_impl_base&) = delete;
  block_impl_base& operator= (block_impl_base&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  virtual explicit operator bool () const noexcept = 0;
  virtual mobius::pod::map get_state () const = 0;
  virtual uid_type get_uid () const = 0;
  virtual void set_uid (uid_type) = 0;
  virtual std::string get_type () const = 0;
  virtual size_type get_size () const = 0;

  virtual void add_parent (const block&) = 0;
  virtual std::vector <block> get_parents () const = 0;
  virtual void add_child (const mobius::vfs::block&) = 0;
  virtual void set_children (const std::vector<block>&) = 0;
  virtual std::vector<mobius::vfs::block> get_children () const = 0;

  virtual bool has_attribute (const std::string&) const = 0;
  virtual mobius::pod::data get_attribute (const std::string&) const = 0;
  virtual void set_attribute (const std::string&, const mobius::pod::data&) = 0;
  virtual mobius::pod::map get_attributes () const = 0;

  virtual bool is_handled () const = 0;
  virtual void set_handled (bool) = 0;
  virtual bool is_complete () const = 0;
  virtual void set_complete (bool) = 0;
  virtual bool is_available () const = 0;
  virtual void set_available (bool) = 0;

  virtual mobius::io::reader new_reader () const = 0;
};

} // namespace mobius::vfs

#endif


