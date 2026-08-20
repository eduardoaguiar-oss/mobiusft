#ifndef MOBIUS_EXTENSION_APP_SAFARI_FILE_HISTORY_DB_HPP
#define MOBIUS_EXTENSION_APP_SAFARI_FILE_HISTORY_DB_HPP

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

namespace mobius::extension::app::safari
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief History.db file decoder
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_history_db
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Visited URLs (from history_items and history_visits tables)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct visited_url
    {
        // @brief Record index number
        std::uint64_t idx = 0;

        // @brief Autocomplete triggers
        mobius::core::bytearray autocomplete_triggers;

        // @brief Daily visit counts
        mobius::core::bytearray daily_visit_counts;

        // @brief Domain expansion
        std::string domain_expansion;

        // @brief Id
        std::int64_t item_id;

        // @brief Should recompute derived visit counts
        std::int64_t should_recompute_derived_visit_counts;

        // @brief Status code
        std::int64_t status_code;

        // @brief Url
        std::string url;

        // @brief Visit count
        std::int64_t visit_count;

        // @brief Visit count score
        std::int64_t visit_count_score;

        // @brief Weekly visit counts
        mobius::core::bytearray weekly_visit_counts;

        // @brief Attributes
        std::int64_t attributes;

        // @brief Generation
        std::int64_t generation;

        // @brief History item
        std::int64_t history_item;

        // @brief Http non get
        std::string http_non_get;

        // @brief Id
        std::int64_t visit_id;

        // @brief Load successful
        bool load_successful = false;

        // @brief Origin
        std::int64_t origin;

        // @brief Redirect destination
        std::int64_t redirect_destination;

        // @brief Redirect source
        std::int64_t redirect_source;

        // @brief Score
        std::int64_t score;

        // @brief Synthesized
        std::string synthesized;

        // @brief Title
        std::string title;

        // @brief Visit time
        mobius::core::datetime::datetime visit_time;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_history_db (const mobius::core::io::reader &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if stream is an instance of History.db file
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const noexcept
    {
        return is_instance_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get visited URLs
    // @return Visited URLs
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<visited_url>
    get_visited_urls () const
    {
        return visited_urls_;
    }

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

   // @brief Visited URLs
    std::vector<visited_url> visited_urls_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_visited_urls (mobius::core::database::database &);
};

} // namespace mobius::extension::app::safari

#endif
