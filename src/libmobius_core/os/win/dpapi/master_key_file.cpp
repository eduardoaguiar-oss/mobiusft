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
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/io/bytearray_io.hpp>
#include <mobius/core/os/win/dpapi/master_key_file.hpp>
#include <mobius/core/string_functions.hpp>

namespace mobius::core::os::win::dpapi
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
master_key_file::master_key_file (mobius::core::io::reader reader)
{
    mobius::core::decoder::data_decoder decoder (reader);

    // general data
    revision_ = decoder.get_uint32_le ();
    decoder.skip (8); // dummy1 and dummy2
    guid_ = mobius::core::string::toupper (
        decoder.get_string_by_size (72, "UTF-16LE"));
    decoder.skip (8); // dummy3 and dummy4
    flags_ = decoder.get_uint32_le ();

    // data block size
    auto master_key_size = decoder.get_uint64_le ();
    auto backup_key_size = decoder.get_uint64_le ();
    auto credhist_size = decoder.get_uint64_le ();
    auto domain_key_size = decoder.get_uint64_le ();

    // keys
    if (master_key_size)
    {
        auto data = decoder.get_bytearray_by_size (master_key_size);
        auto reader = mobius::core::io::new_bytearray_reader (data);
        master_key_ = master_key (reader, flags_);
    }

    if (backup_key_size)
    {
        auto data = decoder.get_bytearray_by_size (backup_key_size);
        auto reader = mobius::core::io::new_bytearray_reader (data);
        backup_key_ = master_key (reader, flags_);
    }

    if (credhist_size)
    {
        decoder.skip (4); // CREDHIST revision
        credhist_guid_ = decoder.get_guid ();
    }

    //! \todo implement domain key
    if (domain_key_size)
        decoder.skip (domain_key_size);
}

} // namespace mobius::core::os::win::dpapi
