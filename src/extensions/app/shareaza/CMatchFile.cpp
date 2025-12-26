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
#include "CMatchFile.hpp"

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode CMatchFile structure
// @param version Structure version
// @see MatchObjects.cpp - CMatchFile::Serialize
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
CMatchFile::decode (mobius::core::decoder::mfc &decoder, int version)
{
    if (version >= 10)
        size_ = decoder.get_qword ();
    else
        size_ = decoder.get_dword ();

    str_size_ = decoder.get_string ();
    hash_sha1_ = decoder.get_hex_string (20);
    hash_tiger_ = decoder.get_hex_string (24);
    hash_ed2k_ = decoder.get_hex_string (16);

    if (version >= 13)
    {
        hash_bth_ = decoder.get_hex_string (20);
        hash_md5_ = decoder.get_hex_string (16);
    }

    tri_busy_ = decoder.get_dword ();
    tri_push_ = decoder.get_dword ();
    tri_stable_ = decoder.get_dword ();
    speed_ = decoder.get_dword ();
    str_speed_ = decoder.get_string ();
    flag_expanded_ = decoder.get_bool ();
    flag_existing_ = decoder.get_bool ();
    flag_download_ = decoder.get_bool ();
    flag_one_valid_ = decoder.get_bool ();

    auto n_preview = decoder.get_count ();

    if (n_preview > 0)
        preview_ = decoder.get_data (n_preview);

    auto total = decoder.get_count ();

    for (std::uint32_t i = 0; i < total; i++)
    {
        CQueryHit query_hit;
        query_hit.decode (decoder, version);

        query_hits_.push_back (query_hit);
    }

    if (version >= 14)
        found_time_ = decoder.get_ctime ();
}

} // namespace mobius::extension::app::shareaza
