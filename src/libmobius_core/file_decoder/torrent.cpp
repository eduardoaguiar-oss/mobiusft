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
#include <mobius/core/file_decoder/torrent.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/decoder/btencode.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/exception.inc>
#include <mobius/core/pod/map.hpp>
#include <mobius/core/string_functions.hpp>
#include <stdexcept>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get path from torrent path data
// @param root Root path
// @param data Path data
// @return Path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
_get_path (const std::string& root, const mobius::core::pod::data& data)
{
    std::string path = root;

    if (data.is_list())
    {
        std::vector<mobius::core::pod::data> path_list(data);

        for(const auto& path_item : path_list)
        {
            if(!path.empty())
                path += "/";
            path += static_cast<mobius::core::bytearray>(path_item).to_string();
        }
    }

    return path;
}

} // namespace

namespace mobius::core::file_decoder
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Torrent implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class torrent::impl
{
public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl() = default;
    impl(const impl&) = delete;
    impl(impl&&) = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl& operator= (const impl&) = delete;
    impl& operator= (impl&&) = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Constructor
    // @param reader Reader object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit impl (const mobius::core::io::reader& reader)
        : reader_ (reader)
    {
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if object is valid
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_instance() const
    {
        _load_data();
        return is_instance_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get file name
    // @return File name
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_name() const
    {
        _load_data();
        return name_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get piece length
    // @return Piece length
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_piece_length() const
    {
        _load_data();
        return piece_length_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get length
    // @return Length
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint64_t
    get_length() const
    {
        _load_data();
        return length_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get creation time
    // @return Creation time
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_creation_time() const
    {
        _load_data();
        return creation_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get created by
    // @return Created by
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_created_by() const
    {
        _load_data();
        return created_by_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get encoding
    // @return Encoding
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_encoding() const
    {
        _load_data();
        return encoding_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get comment
    // @return Comment
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_comment() const
    {
        _load_data();
        return comment_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get announce
    // @return Announce
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_announce() const
    {
        _load_data();
        return announce_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get announce list
    // @return Announce list
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<std::string>
    get_announce_list() const
    {
        _load_data();
        return announce_list_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get info hash
    // @return Info hash
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_info_hash() const
    {
        _load_data();
        return info_hash_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get files
    // @return Files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector <file>
    get_files() const
    {
        _load_data();
        return files_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get pieces
    // @return Pieces
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector <std::string>
    get_pieces() const
    {
        _load_data();
        return pieces_;
    }

private:
    // @brief Reader object
    mobius::core::io::reader reader_;

    // @brief Data loaded flag
    mutable bool data_loaded_ = false;

    // @brief Is instance flag
    mutable bool is_instance_ = false;

    // @brief File name
    mutable std::string name_;

    // @brief Piece length
    mutable std::uint64_t piece_length_ = 0;

    // @brief Length
    mutable std::uint64_t length_ = 0;

    // @brief Creation time
    mutable mobius::core::datetime::datetime creation_time_;

    // @brief Created by
    mutable std::string created_by_;

    // @brief Encoding
    mutable std::string encoding_;

    // @brief Comment
    mutable std::string comment_;

    // @brief Announce
    mutable std::string announce_;

    // @brief Info hash
    mutable std::string info_hash_;

    // @brief Announce list
    mutable std::vector<std::string> announce_list_;

    // @brief Files
    mutable std::vector <file> files_;

    // @brief Pieces
    mutable std::vector <std::string> pieces_;

    // Helper functions
    void _load_data () const;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load data on demand
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
torrent::impl::_load_data () const
{
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Check if data is already loaded
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (data_loaded_)
        return;

    data_loaded_ = true;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (!reader_)
        return;

    auto data = mobius::core::decoder::btencode (reader_);
    if (!data.is_map())
        return;

    auto metadata = static_cast<mobius::core::pod::map>(data);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get file data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    announce_ = metadata.pop<std::string>("announce");
    created_by_ = metadata.pop<std::string>("created by");
    encoding_ = metadata.pop<std::string>("encoding");
    comment_ = metadata.pop<std::string>("comment");
    creation_time_ = mobius::core::datetime::new_datetime_from_unix_timestamp(metadata.pop<std::int64_t>("creation date", 0));

    auto announce_list_data = metadata.pop("announce-list");
    if (announce_list_data.is_list())
    {
        std::vector<mobius::core::pod::data> announce_list(announce_list_data);
        for (const auto& announce : announce_list)
        {
            if (announce.is_list())
            {
                std::vector<mobius::core::pod::data> announce_items(announce);
                for (const auto& item : announce_items)
                    announce_list_.emplace_back(static_cast<mobius::core::bytearray>(item).to_string());
            }
        }
    }

    info_hash_ = metadata.get<std::string>("info hash");
    if (info_hash_.empty())
        info_hash_ = metadata.get<std::string>("infohash");
    if (info_hash_.empty())
        info_hash_ = metadata.get<std::string>("info_hash");

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get 'info' data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto info_data = metadata.get("info");
    if (!info_data.is_map())
        return;

    auto info = static_cast<mobius::core::pod::map>(info_data);

    // Get data from info dict
    name_ = info.get<std::string>("name.utf-8");
    if (name_.empty())
        name_ = info.get<std::string>("name");

    piece_length_ = info.get<std::int64_t>("piece length", 0);
    length_ = info.get<std::int64_t>("length", 0);

    // Get files
    auto files_data = info.get("files");
    if(files_data.is_list())
    {
        std::vector<mobius::core::pod::data> files(files_data);
        std::uint64_t total_size = 0;

        for (const auto& file_data : files)
        {
            auto file_map = static_cast<mobius::core::pod::map>(file_data);

            file f;
            f.length = file_map.get<std::int64_t>("length", 0);
            f.offset = file_map.get<std::int64_t>("offset", 0);
            f.piece_length = file_map.get<std::int64_t>("piece length", 0);
            f.piece_offset = file_map.get<std::int64_t>("piece offset", 0);
            f.creation_time = file_map.get<mobius::core::datetime::datetime>("creation time");

            auto path_data = file_map.get("path.utf-8");
            if (path_data.is_null())
                path_data = file_map.get("path");
            f.path = _get_path(name_, path_data);

            total_size += f.length;
            files_.push_back(f);
        }

        if(length_ == 0)
            length_ = total_size;
    }
    else
    {
        file f;
        f.length = length_;
        f.offset = info.get<std::int64_t>("offset", 0);
        f.piece_length = info.get<std::int64_t>("piece length", 0);
        f.piece_offset = info.get<std::int64_t>("piece offset", 0);
        f.creation_time = info.get<mobius::core::datetime::datetime>("creation time");
        f.name = name_;

        auto path_data = info.get("path.utf-8");
        if (path_data.is_null())
            path_data = info.get("path");
        f.path = _get_path(name_, path_data);

        files_.push_back(f);
    }

    // Get pieces
    auto pieces_data = info.get("pieces");
    if (pieces_data.is_bytearray())
    {
        auto pieces = static_cast<mobius::core::bytearray>(pieces_data);

        for(std::size_t i = 0; i < pieces.size(); i += 20)
        {
            auto piece = pieces.slice(i, i + 19);
            pieces_.push_back(piece.to_hexstring());
        }
    }

    // File is a torrent file
    is_instance_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Default constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
torrent::torrent ()
  : impl_ (std::make_shared <impl> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param reader Reader object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
torrent::torrent (const mobius::core::io::reader& reader)
  : impl_ (std::make_shared <impl> (reader))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if object is valid
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
torrent::operator bool () const
{
    return impl_->is_instance();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get file name
// @return File name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
torrent::get_name() const
{
    return impl_->get_name();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get piece length
// @return Piece length
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
torrent::get_piece_length () const
{
    return impl_->get_piece_length();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get length
// @return Length
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
torrent::get_length () const
{
    return impl_->get_length();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get creation time
// @return Creation time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
torrent::get_creation_time () const
{
    return impl_->get_creation_time();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get created by
// @return Created by
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
torrent::get_created_by () const
{
    return impl_->get_created_by();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get encoding
// @return Encoding
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
torrent::get_encoding () const
{
    return impl_->get_encoding();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get comment
// @return Comment
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
torrent::get_comment () const
{
    return impl_->get_comment();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get announce
// @return Announce
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
torrent::get_announce () const
{
    return impl_->get_announce();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get announce list
// @return Announce list
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<std::string>
torrent::get_announce_list () const
{
    return impl_->get_announce_list();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get info hash
// @return Info hash
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
torrent::get_info_hash () const
{
    return impl_->get_info_hash();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get files
// @return Files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <torrent::file>
torrent::get_files () const
{
    return impl_->get_files();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get pieces
// @return Pieces
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <std::string>
torrent::get_pieces () const
{
    return impl_->get_pieces();
}

} // namespace mobius::core::file_decoder


