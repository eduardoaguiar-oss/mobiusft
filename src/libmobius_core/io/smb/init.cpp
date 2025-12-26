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
#include <cstdint>
#include <cstring>
#include <libsmbclient.h>
#include <mobius/core/exception.inc>
#include <mobius/core/io/smb/init.hpp>
#include <mutex>
#include <stdexcept>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Local variables
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr int DEBUG_LEVEL = 0; //< Libsmbclient debug level
static std::once_flag is_initialized_;
static std::string smb_user_;
static std::string smb_passwd_;
static std::string smb_workgroup_;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Authentication function passed to smbc_init
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_get_auth_data_fn (const char *, // server
                   const char *, // share
                   char *workgroup, int workgroup_size, char *username,
                   int username_size, char *password, int password_size)
{
    if (smb_user_.size () >= static_cast<std::uint64_t> (username_size))
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("SMB user name too large"));

    if (smb_passwd_.size () >= static_cast<std::uint64_t> (password_size))
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("SMB password too large"));

    if (smb_workgroup_.size () >= static_cast<std::uint64_t> (workgroup_size))
        throw std::invalid_argument (
            MOBIUS_EXCEPTION_MSG ("SMB workgroup name too large"));

    strcpy (username, smb_user_.c_str ());
    strcpy (password, smb_passwd_.c_str ());
    strcpy (workgroup, smb_workgroup_.c_str ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize SMB library
// @param user User name
// @param passwd User password
// @param workgroup Workgroup name
// @see
// https://gitlab.com/samba-team/samba/-/blob/master/examples/libsmbclient/testbrowse.c
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_init (const std::string &user, const std::string &passwd,
       const std::string &workgroup)
{
    if (!user.empty () || !passwd.empty () || !workgroup.empty ())
    {
        smb_user_ = user;
        smb_passwd_ = passwd;
        smb_workgroup_ = workgroup;

        SMBCCTX *context = smbc_new_context ();
        if (!context)
            throw std::runtime_error (
                MOBIUS_EXCEPTION_MSG ("could not allocate smbc context"));

        smbc_setDebug (context, DEBUG_LEVEL);
        smbc_setFunctionAuthData (context, _get_auth_data_fn);

        if (!smbc_init_context (context))
        {
            smbc_free_context (context, 0);
            throw std::runtime_error (
                MOBIUS_EXCEPTION_MSG ("could not initialize smbc context"));
        }

        smbc_set_context (context);
    }
}

} // namespace

namespace mobius::core::io::smb
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize SMB library
// @param user User name
// @param passwd User password
// @param workgroup Workgroup name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
init (const std::string &user, const std::string &passwd,
      const std::string &workgroup)
{
    std::call_once (is_initialized_, _init, user, passwd, workgroup);
}

} // namespace mobius::core::io::smb
