// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "writer_impl.h"
#include <mobius/exception.inc>
#include <mobius/exception_posix.inc>
#include <stdexcept>
#include <libsmbclient.h>
#include <fcntl.h>

namespace mobius
{
namespace io
{
namespace smb
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Constructor
//! \param url URL to SMB file
//! \param overwrite Overwrite flag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
writer_impl::writer_impl (const std::string& url, bool overwrite)
{
  int flags = O_WRONLY | O_CREAT;

  if (overwrite)
    flags |= O_TRUNC;

  fd_ = smbc_open (url.c_str (), flags, 0644);

  if (fd_ < 0)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
writer_impl::~writer_impl ()
{
  if (fd_ != -1)
    smbc_close (fd_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Set writing position
//! \param offset Offset in bytes
//! \param w Either beginning, current or end
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
writer_impl::seek (offset_type offset, whence_type w)
{
  int whence = 0;

  if (w == whence_type::beginning)
    whence = SEEK_SET;

  else if (w == whence_type::current)
    whence = SEEK_CUR;

  else if (w == whence_type::end)
    whence = SEEK_END;

  if (smbc_lseek (fd_, offset, whence) < 0)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Get writing position
//! \return Writing position in bytes from the beginning of data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
writer_impl::offset_type
writer_impl::tell () const
{
  offset_type off = smbc_lseek (fd_, 0, SEEK_CUR);

  if (off < 0)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
  
  return off;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Write data to writer
//! \param data Data
//! \return Number of bytes written
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
writer_impl::size_type
writer_impl::write (const mobius::bytearray& data)
{
  auto count = smbc_write (fd_, data.data (), data.size ());

  if (count < 0)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

  else if (static_cast <size_type> (count) != data.size ())
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("could not write bytearray"));

  return count;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Flush data to file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
writer_impl::flush ()
{
}

} // namespace smb
} // namespace io
} // namespace mobius
