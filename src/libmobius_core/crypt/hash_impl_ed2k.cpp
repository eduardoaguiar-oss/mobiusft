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
#include <mobius/core/crypt/hash_impl_ed2k.hpp>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr std::uint64_t BLOCK_SIZE = 9728000ul; // 9500 * 1024

} // namespace

namespace mobius::core::crypt
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hash_impl_ed2k::hash_impl_ed2k ()
    : md4_accum_ ("md4"),
      md4_ ("md4")
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Reset hash value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hash_impl_ed2k::reset ()
{
    md4_accum_.reset ();
    md4_.reset ();
    size_ = 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update hash value
// @param data Data block
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
hash_impl_ed2k::update (const mobius::core::bytearray &data)
{
    if (size_ + data.size () <= BLOCK_SIZE)
        md4_.update (data);

    else
    {
        // bytes needed to fill current block
        const std::uint64_t bytes_to_go = BLOCK_SIZE - (size_ % BLOCK_SIZE);

        // get data size
        const std::uint64_t data_size = data.size ();

        std::uint64_t i = 0;
        std::uint64_t chunk_size = std::min (bytes_to_go, data_size);

        while (i < data_size)
        {
            // we start to accumulate MD4 hashes right after the first block is
            // full
            if (size_ + i >= BLOCK_SIZE && ((size_ + i) % BLOCK_SIZE) == 0)
            {
                md4_accum_.update (md4_.get_digest ());
                md4_.reset ();
            }

            md4_.update (data.slice (i, i + chunk_size - 1));

            i += chunk_size;
            chunk_size = std::min (BLOCK_SIZE, data_size - i);
        }
    }

    size_ += data.size ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get digest
// @param data remaining data
// @return digest
//
// If data size is <= BLOCK_SIZE, hash ED2K is equal to hash MD4
//
// If data size > BLOCK_SIZE, hash ED2K is equal to MD4 hash of each block,
// including the last block, which may be incomplete
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::bytearray
hash_impl_ed2k::get_digest ()
{
    if (size_ <= BLOCK_SIZE)
        return md4_.get_digest ();

    else
    {
        md4_accum_.update (md4_.get_digest ());
        return md4_accum_.get_digest ();
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clone object
// @return Pointer to newly created object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr<hash_impl_base>
hash_impl_ed2k::clone () const
{
    auto h = std::make_shared<hash_impl_ed2k> ();

    h->md4_accum_ = md4_accum_.clone ();
    h->md4_ = md4_.clone ();
    h->size_ = size_;

    return h;
}

} // namespace mobius::core::crypt
