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
#include "user.h"
#include <mobius/exception.inc>
#include <mobius/exception_posix.inc>
#include <pwd.h>
#include <stdexcept>

namespace mobius::system
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param uid user ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
user::user (uid_t uid) : uid_ (uid)
{
  struct passwd pw;
  struct passwd *result;
  char buffer[1024];

  if (getpwuid_r (uid, &pw, buffer, sizeof (buffer), &result))
    throw std::runtime_error (MOBIUS_EXCEPTION_POSIX);

  if (result)
    {
      name_ = pw.pw_name;
      passwd_ = pw.pw_passwd;
      gid_ = pw.pw_gid;
      info_ = pw.pw_gecos;
      home_folder_ = pw.pw_dir;
      shell_ = pw.pw_shell;
      is_valid_ = true;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get user ID
// @return user ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uid_t
user::get_id () const
{
  if (!is_valid_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid user"));

  return uid_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get user name
// @return user name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
user::get_name () const
{
  if (!is_valid_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid user"));

  return name_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get user passwd
// @return user password, if available
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
user::get_passwd () const
{
  if (!is_valid_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid user"));

  return passwd_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get user extra information
// @return user information, if available
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
user::get_info () const
{
  if (!is_valid_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid user"));

  return info_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get user home folder
// @return home folder as string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
user::get_home_folder () const
{
  if (!is_valid_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid user"));

  return home_folder_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get user shell
// @return shell command
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
user::get_shell () const
{
  if (!is_valid_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid user"));

  return shell_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get group
// @return group
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
group
user::get_group () const
{
  if (!is_valid_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid user"));

  return group (gid_);
}

} // namespace mobius::system


