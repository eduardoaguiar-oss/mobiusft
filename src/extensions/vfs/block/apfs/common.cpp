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
#include "common.hpp"
#include <mobius/core/decoder/data_decoder.hpp>

namespace mobius::extension::vfs::block::apfs
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test block checksum
// @param data Block data
// @return True if checksum is valid, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
test_checksum (const mobius::core::bytearray &data)
{
    mobius::core::decoder::data_decoder decoder (data);

    std::uint64_t checksum = decoder.get_uint64_le ();
    std::size_t siz = data.size () - 8;
    std::uint64_t sum1 = 0;
    std::uint64_t sum2 = 0;

    while (siz)
    {
        sum1 += decoder.get_uint32_le ();
        sum2 += sum1;
        sum1 %= 0xffffffff;
        sum2 %= 0xffffffff;
        siz -= 4;
    }

    uint32_t c0 = static_cast<uint32_t> (checksum);
    uint32_t c1 = static_cast<uint32_t> (checksum >> 32);

    sum1 = (sum1 + c0) % 0xFFFFFFFFULL;
    sum2 = (sum2 + sum1) % 0xFFFFFFFFULL;

    sum1 = (sum1 + c1) % 0xFFFFFFFFULL;
    sum2 = (sum2 + sum1) % 0xFFFFFFFFULL;

    return (sum1 == 0 && sum2 == 0);
}

} // namespace mobius::extension::vfs::block::apfs
