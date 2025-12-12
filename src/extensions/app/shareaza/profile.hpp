#ifndef MOBIUS_EXTENSION_APP_SHAREAZA_PROFILE_HPP
#define MOBIUS_EXTENSION_APP_SHAREAZA_PROFILE_HPP

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
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/folder.hpp>
#include <mobius/core/pod/map.hpp>
#include <mobius/framework/evidence_flag.hpp>
#include <string>
#include <vector>

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Shareaza profile class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class profile
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Searched text
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct searched_text
    {
        // attributes
        mobius::core::datetime::datetime timestamp;
        std::string text;

        // metadata
        mobius::core::pod::map metadata;

        // source file
        mobius::core::io::file f;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    profile () = default;
    profile (const profile &) noexcept = default;
    profile (profile &&) noexcept = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    profile &operator= (const profile &) noexcept = default;
    profile &operator= (profile &&) noexcept = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if profile is valid
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const noexcept
    {
        return bool (folder_);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get username
    // @return username
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_username () const
    {
        return username_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get folder
    // @return Folder object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::io::folder
    get_folder () const
    {
        return folder_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get path to profile
    // @return Path to profile
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_path () const
    {
        return (folder_) ? folder_.get_path () : "";
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get creation time
    // @return Creation time
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_creation_time () const
    {
        return creation_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get last modified time
    // @return Last modified time
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_last_modified_time () const
    {
        return last_modified_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get Gnutella GUID
    // @return Gnutella GUID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_gnutella_guid () const
    {
        return gnutella_guid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get BitTorrent GUID
    // @return BitTorrent GUID
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_bittorrent_guid () const
    {
        return bittorrent_guid_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get User Identity
    // @return User Identity
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_identity () const
    {
        return identity_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get source files
    // @return Source files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<mobius::core::io::file>
    get_source_files () const
    {
        return source_files;;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get searched texts
    // @return Searched texts
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<searched_text>
    get_searched_texts () const
    {
        return searched_texts_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void add_profile_xml_file (const mobius::core::io::file &);
    void add_searches_dat_file (const mobius::core::io::file &);

  private:
    // @brief Folder object
    mobius::core::io::folder folder_;

    // @brief Username
    std::string username_;

    // @brief Creation time
    mobius::core::datetime::datetime creation_time_;

    // @brief Last modified time
    mobius::core::datetime::datetime last_modified_time_;

    // @brief Gnutella GUID
    std::string gnutella_guid_;

    // @brief BitTorrent GUID
    std::string bittorrent_guid_;

    // @brief User identity
    std::string identity_;

    // @brief Profile.xml source file
    mobius::core::io::file profile_xml_f;

    // @brief Source files
    std::vector<mobius::core::io::file> source_files;

    // @brief Searched texts
    std::vector<searched_text> searched_texts_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _set_folder (const mobius::core::io::folder &);
    void _update_mtime (const mobius::core::io::file &);
};

} // namespace mobius::extension::app::shareaza

#endif
