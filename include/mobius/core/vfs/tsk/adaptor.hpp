#ifndef MOBIUS_CORE_VFS_TSK_ADAPTOR_IMPL_HPP
#define MOBIUS_CORE_VFS_TSK_ADAPTOR_IMPL_HPP

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008-2026 Eduardo Aguiar
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
#include <mobius/core/io/reader.hpp>
#include <mobius/core/io/folder.hpp>
#include <cstdint>

struct TSK_IMG_INFO;
struct TSK_FS_INFO;

namespace mobius::core::vfs::tsk
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief libtsk adaptor
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class adaptor
{
public:
  adaptor (const mobius::core::io::reader, std::uint64_t);
  ~adaptor ();
  mobius::core::io::folder get_root_folder () const;

private:
  mobius::core::io::reader reader_;
  std::uint64_t offset_;

  mutable TSK_IMG_INFO *img_info_ = nullptr;
  mutable TSK_FS_INFO *fs_info_ = nullptr;

  void _create_tsk () const;
};

} // namespace mobius::core::vfs::tsk

#endif


