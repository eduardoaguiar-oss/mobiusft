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
#include <mobius/core/io/local/reader_impl.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/exception_posix.inc>
#include <stdexcept>

namespace mobius::core::io::local
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// @param path path to local file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
reader_impl::reader_impl (const std::string& path)
{
  FILE *fp = fopen (path.c_str (), "rb");
  if (!fp)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

  fp_ = std::shared_ptr <FILE> (fp, fclose);

  // evaluate file size
  if (fseek (fp_.get (), 0, SEEK_END) == -1)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

  size_ = ftell (fp_.get ());

  // set position to the start of the file
  ::rewind (fp_.get ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief set read position
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

  if (fseek (fp_.get (), offset, whence) == -1)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

  clearerr (fp_.get ()); // clear eof status
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get read position
// @return read position in bytes from the beginning of data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
reader_impl::offset_type
reader_impl::tell () const
{
  long rc = ftell (fp_.get ());
  if (rc == -1)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

  return rc;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief read bytes from reader
// @param size size in bytes
// @return bytearray containing data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
reader_impl::read (size_type size)
{
  mobius::core::bytearray buffer (size);
  size_t count = fread (buffer.data (), 1, size, fp_.get ());

  if (count == 0 && ferror (fp_.get ()))
    {
      clearerr (fp_.get ());
      throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
    }

  buffer.resize (count);
  return buffer;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief check if end-of-file indicator is set
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
reader_impl::eof () const
{
  return static_cast<size_type> (tell ()) >= size_;
}

} // namespace mobius::core::io::local


