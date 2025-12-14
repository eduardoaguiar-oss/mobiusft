// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
// Eduardo Aguiar
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
#include <mobius/core/crypt/gcrypt/util.hpp>
#include <mobius/core/exception.inc>
#include <mutex>
#include <stdexcept>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Internal variables and constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Mininum libgcrypt version needed
static constexpr const char *LIBGCRYPT_MIN_VERSION = "1.9.0";

// @brief Is libgcrypt initialized?
static std::once_flag is_libgcrypt_initialized_;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize libgcrypt
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
_init_libgcrypt ()
{
    if (!gcry_check_version (LIBGCRYPT_MIN_VERSION))
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("libgcrypt too old"));

    gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
}

} // namespace

namespace mobius::core::crypt::gcrypt
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Initialize libgcrypt
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
init ()
{
    std::call_once (is_libgcrypt_initialized_, _init_libgcrypt);

    if (!gcry_control (GCRYCTL_INITIALIZATION_FINISHED_P))
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("libgcrypt has not been initialized"));
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get libgcrypt error message
// @param err Error number
// @return Error message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_error_message (gcry_error_t err)
{
    const char *text = gcry_strerror (err);

    if (text)
        return text;

    return "libgcrypt error " + std::to_string (err);
}

} // namespace mobius::core::crypt::gcrypt
