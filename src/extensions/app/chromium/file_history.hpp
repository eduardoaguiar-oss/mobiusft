#ifndef MOBIUS_EXTENSION_APP_CHROMIUM_FILE_HISTORY_WEB_DATA_HPP
#define MOBIUS_EXTENSION_APP_CHROMIUM_FILE_HISTORY_WEB_DATA_HPP

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
#include <mobius/core/database/database.hpp>
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/core/io/reader.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief History file decoder
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_history
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief History entry structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct history_entry
    {
        // @brief Record Index
        std::uint64_t idx = 0;

        // @brief URL
        std::string url;

        // @brief Title
        std::string title;

        // @brief Visit ID
        std::uint64_t visit_id = 0;

        // @brief Visit time
        mobius::core::datetime::datetime visit_time;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Download structure
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct download
    {
        // @brief Record Index
        std::uint64_t idx = 0;

        // @brief The id of the extension that created this download
        std::string by_ext_id;

        // @brief The name of the extension that created this download
        std::string by_ext_name;

        // @brief The id of the web app that created this download
        std::string by_web_app_id;

        // @brief Current path
        std::string current_path;

        // @brief Danger type
        std::string danger_type;

        // @brief Embedder download data
        std::string embedder_download_data;

        // @brief End time
        mobius::core::datetime::datetime end_time;

        // @brief ETag
        std::string etag;

        // @brief Full path
        std::string full_path;

        // @brief GUID
        std::string guid;

        // @brief Hash
        std::string hash;

        // @brief HTTP method
        std::string http_method = "GET";

        // @brief ID
        std::uint64_t id = 0;

        // @brief Interrupt reason
        std::uint32_t interrupt_reason = 0;

        // @brief Last access time
        mobius::core::datetime::datetime last_access_time;

        // @brief Last-Modified header value
        std::string last_modified;

        // @brief MIME type
        std::string mime_type;

        // @brief Opened
        bool opened = false;

        // @brief Original MIME type
        std::string original_mime_type;

        // @brief Received bytes
        std::uint64_t received_bytes = 0;

        // @brief Referrer
        std::string referrer;

        // @brief Site URL
        std::string site_url;

        // @brief Start time
        mobius::core::datetime::datetime start_time;

        // @brief State
        std::string state;

        // @brief Tab referrer URL
        std::string tab_referrer_url;

        // @brief Tab URL
        std::string tab_url;

        // @brief Target path
        std::string target_path;

        // @brief Total bytes
        std::uint64_t total_bytes = 0;

        // @brief Transient
        bool transient = false;

        // @brief URL
        std::string url;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_history (const mobius::core::io::reader &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if stream is an instance of History file
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const noexcept { return is_instance_; }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get schema version
    // @return Schema version
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::uint32_t
    get_schema_version () const
    {
        return schema_version_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get history entries
    // @return Vector of history entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<history_entry>
    get_history_entries () const
    {
        return history_entries_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get downloads
    // @return Vector of download entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<download>
    get_downloads () const
    {
        return downloads_;
    }

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

    // @brief Schema version
    std::uint32_t schema_version_ = 0;

    // @brief History entries
    std::vector<history_entry> history_entries_;

    // @brief Download entries
    std::vector<download> downloads_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _load_history (mobius::core::database::database &);
    void _load_downloads (mobius::core::database::database &);
    void _load_search_terms (mobius::core::database::database &);
};

} // namespace mobius::extension::app::chromium

#endif