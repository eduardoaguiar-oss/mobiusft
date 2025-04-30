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
#include <mobius/core/io/smb/reader_impl.hpp>
#include <mobius/core/exception_posix.inc>
#include <stdexcept>
#include <libsmbclient.h>
#include <fcntl.h>

namespace mobius::core::io::smb
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param url URL to SMB file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
reader_impl::reader_impl (const std::string& url)
{
  fd_ = smbc_open (url.c_str (), O_RDONLY, 0);

  if (fd_ < 0)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

  // evaluate file size
  struct stat st;

  if (smbc_fstat (fd_, &st) < 0)
    {
      smbc_close (fd_);
      fd_ = -1;
      throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
    }

  size_ = st.st_size;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Destructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
reader_impl::~reader_impl ()
{
  if (fd_ != -1)
    smbc_close (fd_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set read position
// @param offset offset in bytes
// @param w either beginning, current or end
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
reader_impl::seek (offset_type offset, whence_type w)
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
// @brief Get read position
// @return read position in bytes from the beginning of data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
reader_impl::offset_type
reader_impl::tell () const
{
  offset_type off = smbc_lseek (fd_, 0, SEEK_CUR);

  if (off < 0)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

  return off;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Read data from reader
// @param size Size in bytes
// @return bytearray containing data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
reader_impl::read (size_type size)
{
  mobius::core::bytearray buffer (size);
  ssize_t count = smbc_read (fd_, buffer.data (), size);

  if (count < 0)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

  buffer.resize (count);
  return buffer;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if end-of-file indicator is set
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
reader_impl::eof () const
{
  return static_cast <size_type> (tell ()) >= size_;
}

} // namespace mobius::core::io::smb


