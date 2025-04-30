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
#include <mobius/core/io/local/writer_impl.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/exception_posix.inc>
#include <stdexcept>

namespace mobius::core::io::local
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor
// @param path path to local file
// @param overwrite overwrite flag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
writer_impl::writer_impl (const std::string& path, bool overwrite)
{
  FILE *fp = nullptr;

  // if overwrite is not set, try to open an existing file
  if (!overwrite)
    {
      fp = fopen (path.c_str (), "r+b");

      if (!fp && errno != ENOENT)
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

      if (fseek (fp, 0, SEEK_END) == -1)
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
    }

  // create a new file either if overwrite is set or if file does not exist
  if (!fp)
    {
      fp = fopen (path.c_str (), "wb");
      if (!fp)
        throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
    }

  fp_ = std::shared_ptr <FILE> (fp, fclose);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set write position
// @param offset offset in bytes
// @param w either beginning, current or end
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
writer_impl::seek (offset_type offset, whence_type w)
{
  int iw = 0;

  if (w == whence_type::beginning)
    iw = SEEK_SET;

  else if (w == whence_type::current)
    iw = SEEK_CUR;

  else if (w == whence_type::end)
    iw = SEEK_END;

  if (fseek (fp_.get (), offset, iw) == -1)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

  clearerr (fp_.get ()); // clear eof status
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get write position
// @return Write position in bytes from the beginning of data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
writer_impl::offset_type
writer_impl::tell () const
{
  long rc = ftell (fp_.get ());

  if (rc == -1)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

  return rc;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief write bytes to writer
// @param data a bytearray
// @return number of bytes written
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
writer_impl::size_type
writer_impl::write (const mobius::core::bytearray& data)
{
  size_t count = fwrite (data.data (), 1, data.size (), fp_.get ());

  if (count != data.size ())
    {
      if (ferror (fp_.get ()))
        {
          clearerr (fp_.get ());
          throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
        }
      else
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("could not write bytearray"));
    }

  return count;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief flush data to file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
writer_impl::flush ()
{
  if (fflush (fp_.get ()) == EOF)
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);
}

} // namespace mobius::core::io::local


