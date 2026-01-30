#ifndef MOBIUS_EXTENSION_APP_GECKO_FILE_PLACES_SQLITE_HPP
#define MOBIUS_EXTENSION_APP_GECKO_FILE_PLACES_SQLITE_HPP

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
#include <mobius/core/database/database.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/reader.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace mobius::extension::app::gecko
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Places.sqlite file decoder
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_places_sqlite
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Bookmark structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct bookmark
    {
        // @brief Record index number
        std::uint64_t idx = 0;

        // @brief Dateadded
        mobius::core::datetime::datetime date_added;

        // @brief Fk
        std::int64_t fk;

        // @brief Folder Type
        std::string folder_type;

        // @brief Guid
        std::string guid;

        // @brief Id
        std::int64_t id;

        // @brief Keyword Id
        std::int64_t keyword_id;

        // @brief Lastmodified
        mobius::core::datetime::datetime last_modified;

        // @brief Parent
        std::int64_t parent;

        // @brief Parent name
        std::string parent_name;

        // @brief Position
        std::int64_t position;

        // @brief Syncchangecounter
        std::int64_t sync_change_counter;

        // @brief Syncstatus
        std::int64_t sync_status;

        // @brief Title
        std::string title;

        // @brief Type
        std::int64_t type;

        // @brief URL
        std::string url;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief visited_url structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct visited_url
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // From moz_historyvisits table
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // @brief From Visit
        std::int64_t from_visit;

        // @brief Id
        std::int64_t visit_id;

        // @brief Place Id
        std::int64_t place_id;

        // @brief Session
        std::int64_t session;

        // @brief Source
        std::int64_t source;

        // @brief Triggeringplaceid
        std::int64_t triggering_place_id;

        // @brief Visit Date
        mobius::core::datetime::datetime visit_date;

        // @brief Visit Type
        std::int64_t visit_type;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // From moz_places table
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // @brief Alt Frecency
        std::int64_t alt_frecency;

        // @brief Description
        std::string description;

        // @brief Favicon Id
        std::int64_t favicon_id;

        // @brief Foreign Count
        std::int64_t foreign_count;

        // @brief Frecency
        std::int64_t frecency;

        // @brief Guid
        std::string guid;

        // @brief Hidden
        std::int64_t hidden;

        // @brief Id
        std::int64_t places_id;

        // @brief Last Visit Date
        mobius::core::datetime::datetime last_visit_date;

        // @brief Origin Id
        std::int64_t origin_id;

        // @brief Preview Image Url
        std::string preview_image_url;

        // @brief Recalc Alt Frecency
        std::int64_t recalc_alt_frecency;

        // @brief Recalc Frecency
        std::int64_t recalc_frecency;

        // @brief Rev Host
        std::string rev_host;

        // @brief Site Name
        std::string site_name;

        // @brief Title
        std::string title;

        // @brief Typed
        std::int64_t typed;

        // @brief Url
        std::string url;

        // @brief Url Hash
        std::int64_t url_hash;

        // @brief Visit Count
        std::int64_t visit_count;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_places_sqlite (const mobius::core::io::reader &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if stream is an instance of places.sqlite file
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const noexcept
    {
        return is_instance_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get bookmarks
    // @return Vector of bookmarks
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<bookmark>
    get_bookmarks () const
    {
        return bookmarks_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get visited URLs
    // @return Vector of visited URLs
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<visited_url>
    get_visited_urls () const
    {
        return visited_urls_;
    }

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

    // @brief Bookmarks vector
    std::vector<bookmark> bookmarks_;

    // @brief Visited URLs vector
    std::vector<visited_url> visited_urls_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_bookmarks (mobius::core::database::database &);
    void _load_visited_urls (mobius::core::database::database &);
};

} // namespace mobius::extension::app::gecko

#endif
