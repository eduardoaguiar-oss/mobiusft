#ifndef MOBIUS_CORE_FILE_DECODER_TORRENT_HPP
#define MOBIUS_CORE_FILE_DECODER_TORRENT_HPP

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
#include <mobius/core/io/reader.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <cstdint>
#include <memory>
#include <string>

namespace mobius::core::file_decoder
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Torrent file decoder class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class torrent
{
public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief File structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct file
    {
        std::string name;
        std::string path;
        std::uint64_t length = 0;
        std::uint64_t offset = 0;
        std::uint64_t piece_length = 0;
        std::uint64_t piece_offset = 0;
        mobius::core::datetime::datetime creation_time;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    torrent();
    explicit torrent(const mobius::core::io::reader&);
    torrent(torrent&&) noexcept = default;
    torrent(const torrent&) noexcept = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    torrent& operator= (const torrent&) noexcept = default;
    torrent& operator= (torrent&&) noexcept = default;
    operator bool () const;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Function prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string get_name() const;
    std::uint64_t get_piece_length() const;
    std::uint64_t get_length() const;
    mobius::core::datetime::datetime get_creation_time() const;
    std::string get_created_by() const;
    std::string get_encoding() const;
    std::string get_comment() const;
    std::string get_announce() const;
    std::string get_info_hash() const;
    std::vector<std::string> get_announce_list() const;
    std::vector<file> get_files() const;
    std::vector<std::string> get_pieces() const;

private:
    // @brief Implementation class forward declaration
    class impl;

    // @brief Implementation pointer
    std::shared_ptr <impl> impl_;
};

} // namespace mobius::core::file_decoder

#endif


