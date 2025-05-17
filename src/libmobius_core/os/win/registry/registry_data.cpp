// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025
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
#include <mobius/core/charset.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/os/win/registry/registry_data.hpp>
#include <mobius/core/os/win/registry/registry_data_impl_null.hpp>
#include <stdexcept>

namespace mobius::core::os::win::registry
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief default constructor
// @param hdata hive_data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_data::registry_data ()
    : impl_ (std::make_shared<registry_data_impl_null> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief constructor from implementation pointer
// @param impl implementation pointer
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
registry_data::registry_data (std::shared_ptr<registry_data_impl_base> impl)
    : impl_ (impl)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get data as DWORD
// @return dword value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint32_t
registry_data::get_data_as_dword () const
{
    std::uint32_t value = 0;
    auto data = get_data ();

    if (get_type () == data_type::reg_dword_big_endian)
    {
        value = std::uint32_t (data[3]) | (std::uint32_t (data[2]) << 8) |
                (std::uint32_t (data[1]) << 16) |
                (std::uint32_t (data[0]) << 24);
    }

    else if (get_type () == data_type::reg_dword || data.size () == 4)
    {
        value = std::uint32_t (data[0]) | (std::uint32_t (data[1]) << 8) |
                (std::uint32_t (data[2]) << 16) |
                (std::uint32_t (data[3]) << 24);
    }

    else
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid data type"));

    return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get data as QWORD
// @return qword value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
registry_data::get_data_as_qword () const
{
    std::uint64_t value = 0;
    auto data = get_data ();

    if (get_type () == data_type::reg_qword || data.size () == 8)
    {
        value =
            std::uint64_t (data[0]) | (std::uint64_t (data[1]) << 8) |
            (std::uint64_t (data[2]) << 16) | (std::uint64_t (data[3]) << 24) |
            (std::uint64_t (data[4]) << 32) | (std::uint64_t (data[5]) << 40) |
            (std::uint64_t (data[6]) << 48) | (std::uint64_t (data[7]) << 56);
    }

    else
        throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("invalid data type"));

    return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief get data as UTF-8 string
// @param encoding data encoding
// @return string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
registry_data::get_data_as_string (const std::string &encoding) const
{
    return mobius::core::conv_charset_to_utf8 (get_data (), encoding);
}

} // namespace mobius::core::os::win::registry
