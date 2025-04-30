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
#include <mobius/core/io/path.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/core/exception.inc>
#include <stdexcept>
#include <fnmatch.h>

namespace mobius::core::io
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr char SEPARATOR = '/';

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove dot segments
// @param path path as string
// @return path without "." and ".." dot segments
// @see RFC 3986 - section 5.2.4
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
_remove_dot_segments (const std::string& path)
{
  std::string input (path);
  std::string out;

  while (!input.empty ())
    {
      // Remove duplicated separators
      if (input.size () >= 2 && input[0] == SEPARATOR && input[1] == SEPARATOR)
        input.erase (0, 1);

      // 5.2.4.A (remove "../" or "./")
      else if (input.size () >= 3 && input[0] == '.' && input[1] == '.' && input[2] == SEPARATOR)
        input = input.substr (3);

      else if (input.size () >= 2 && input[0] == '.' && input[1] == SEPARATOR)
        input = input.substr (2);

      // 5.2.4.B (replace "/./" or "/." for "/")
      else if (input.size () >= 3 && input[0] == SEPARATOR && input[1] == '.' && input[2] == SEPARATOR)
        input = SEPARATOR + input.substr (3);

      else if (input.size () == 2 && input[0] == SEPARATOR && input[1] == '.')
        input = SEPARATOR;

      // 5.2.4.C (replace "/../" or "/.." for "/" and remove last segment)
      else if (input.size () >= 4 && input[0] == SEPARATOR && input[1] == '.' && input[2] == '.' && input[3] == SEPARATOR)
        {
          input = SEPARATOR + input.substr (4);

          auto pos = out.rfind (SEPARATOR);
          if (pos != std::string::npos)
            out.erase (pos);
        }

      else if (input.size () == 3 && input[0] == SEPARATOR && input[1] == '.' && input[2] == '.')
        {
          input = SEPARATOR;

          auto pos = out.rfind (SEPARATOR);
          if (pos != std::string::npos)
            out.erase (pos);
        }

      // 5.2.4.D (clear input if it is either "." or "..")
      else if (input == "." || input == "..")
        input.clear ();

      // 5.2.4.E
      else
        {
          std::string::size_type begin = 0;

          if (!input.empty () && input[0] == SEPARATOR)
            begin++;

          std::string::size_type pos = input.find (SEPARATOR, begin);

          if (pos == std::string::npos)
            pos = input.length ();

          if (out.empty () || out[out.size () - 1] != SEPARATOR)
            out += input.substr (0, pos);
          else
            out += input.substr (begin, pos);

          input = input.substr (pos);
        }
    }

  return out;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param value Path value as char array
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
path::path (const char *value)
  : value_ (_remove_dot_segments (value))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param value Path value as string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
path::path (const std::string& value)
  : value_ (_remove_dot_segments (value))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get dirname from path
// @return Dirname (all but last segment of path)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
path::get_dirname () const
{
  std::string dirname;
  auto pos = value_.rfind (SEPARATOR);

  if (pos != std::string::npos)
    dirname = value_.substr (0, pos);

  return dirname;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get filename from path
// @return Filename
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
path::get_filename () const
{
  std::string filename;
  auto pos = value_.rfind (SEPARATOR);

  if (pos == std::string::npos)
    filename = value_;
  else
    filename = value_.substr (pos+1);

  return filename;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get prefix from path
// @return File prefix (path without extension)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
path::get_prefix () const
{
  const std::string extension = get_extension ();
  std::string prefix;

  if (!extension.empty ())
    prefix = value_.substr (0, value_.size () - extension.size () - 1);

  return prefix;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get prefix from filename
// @return File prefix (filename without extension)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
path::get_filename_prefix () const
{
  const std::string extension = get_extension ();
  const std::string filename = get_filename ();

  return filename.substr (0, filename.size () - extension.size () - 1);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get extension from path
// @return File extension
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
path::get_extension () const
{
  std::string extension;
  const std::string filename = get_filename ();
  auto pos = filename.rfind ('.');

  if (pos != std::string::npos)
    extension = filename.substr (pos+1);

  return extension;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get parent path
// @return Parent path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
path
path::get_parent () const
{
  return path (get_dirname ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get sibling path by name
// @param filename Sibling file name
// @return Sibling path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
path
path::get_sibling_by_name (const std::string& filename) const
{
  if (filename.find ('/') != std::string::npos || (filename.size () >= 2 && filename.compare (0, 2, "..") == 0))
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid filename"));

  std::string p;
  auto pos = value_.rfind (SEPARATOR);

  if (pos == std::string::npos)
    p = filename;
  else
    p = value_.substr (0, pos + 1) + filename;

  return path (p);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get sibling path by extension
// @param ext Sibling extension
// @return Sibling path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
path
path::get_sibling_by_extension (const std::string& ext) const
{
  if (ext.find ('/') != std::string::npos || (ext.size () >= 2 && ext.compare (0, 2, "..") == 0))
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid extension"));

  return path (get_prefix () + '.' + ext);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get child path by name
// @param name Child name
// @return Child path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
path
path::get_child_by_name (const std::string& name) const
{
  if (name.find ('/') != std::string::npos || (name.size () >= 2 && name.compare (0, 2, "..") == 0))
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid child name"));

  std::string p;

  if (value_.empty ())
    p = name;
  else
    p = value_ + SEPARATOR + name;

  return path (p);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get child path by path
// @param path Child path
// @return Child path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
path
path::get_child_by_path (const std::string& path) const
{
  std::string p;

  if (value_.empty ())
    p = path;
  else
    p = value_ + SEPARATOR + path;

  mobius::core::io::path child_path (p);

  if (!mobius::core::string::startswith (child_path.get_value (), value_))
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid child path"));

  return child_path;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if path is absolute
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
path::is_absolute () const
{
  return !value_.empty () && value_[0] == SEPARATOR;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if filename matches pattern
// @param pattern fnmatch function pattern
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
path::filename_match (const std::string& pattern) const
{
  const std::string filename = get_filename ();

  return fnmatch (pattern.c_str (), filename.c_str (), FNM_NOESCAPE) == 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if two path objects are equal
// @param lhs path object
// @param rhs path object
// @return true if lhs == rhs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator== (const path& lhs, const path& rhs)
{
  return lhs.get_value () == rhs.get_value ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if one path object is less than another one
// @param lhs Path object
// @param rhs Path object
// @return true if lhs < rhs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator< (const path& lhs, const path& rhs)
{
  return lhs.get_value () < rhs.get_value ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert path to string
// @param p Path object
// @return String representation of path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
to_string (const path& p)
{
  return p.get_value ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Write path representation to std::ostream
// @param stream ostream reference
// @param p Path object
// @return reference to ostream
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::ostream&
operator<< (std::ostream& stream, const path& p)
{
  stream << p.get_value ();

  return stream;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Join two paths
// @param p1 Path object
// @param p2 Path object
// @return New path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
path
join (const path& p1, const path& p2)
{
  if (p2.is_absolute ())
    return p2;

  return path (p1.get_value () + SEPARATOR + p2.get_value ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Create path from Win path
// @param value Path
// @return New path object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
path
new_path_from_win (const std::string& value)
{
  return path (mobius::core::string::replace (value, "\\", "/"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get path formatted for Win
// @return Path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
to_win_path (const path& path)
{
  return mobius::core::string::replace (path.get_value (), "/", "\\");
}

} // namespace mobius::core::io


