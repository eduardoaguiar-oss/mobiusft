#ifndef MOBIUS_EXTENSION_APP_INTERNET_EXPLORER_FILE_MSIECF_HPP
#define MOBIUS_EXTENSION_APP_INTERNET_EXPLORER_FILE_MSIECF_HPP

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
#include <mobius/core/io/reader.hpp>
#include <mobius/core/pod/data.hpp>
#include <unordered_map>
#include <string>
#include <vector>

namespace mobius::extension::app::internet_explorer
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief MSIECF format file decoder
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class file_msiecf
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Cache directory entry
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct cache_directory_entry
    {
        std::uint32_t file_count = 0;
        std::string dirname;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Hash table record
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct hash_table_record
    {
        std::uint8_t type = 0;
        std::uint32_t hash = 0;
        std::uint32_t offset = 0;
        std::uint32_t hash_table_idx = 0;
        std::uint32_t hash_entry_idx = 0;
        std::uint32_t original_hash = 0;
        bool is_deleted = false;
        bool is_active = false;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief URL data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct url
    {
        std::uint32_t record_offset = 0;
        std::uint32_t record_hash = 0;
        std::uint32_t record_hash_table_idx = 0;
        std::uint32_t record_hash_entry_idx = 0;
        std::uint32_t record_original_hash = 0;
        std::uint32_t record_calculated_hash = 0;
        std::uint8_t record_type = 0;

        mobius::core::datetime::datetime modification_time;
        mobius::core::datetime::datetime access_time;
        mobius::core::datetime::datetime expiration_time;
        mobius::core::datetime::datetime last_sync_time;
        mobius::core::datetime::datetime cached_file_creation_time;

        std::uint32_t calculated_hash = 0;
        std::uint64_t cached_file_size = 0;
        std::uint32_t group_offset = 0;
        std::uint32_t release_seconds = 0;
        std::uint32_t location_offset = 0;
        std::uint32_t cache_dir_index = 0;
        std::uint32_t filename_offset = 0;
        std::uint32_t flags = 0;
        std::uint32_t data_offset = 0;
        std::uint32_t data_size = 0;
        std::uint32_t extension_offset = 0;
        std::uint32_t hits = 0;

        std::string location;
        std::string location_type;
        std::string location_value;
        std::string location_username;
        std::string filename;
        std::string dirname;
        std::string page_title;
        std::string query_string;
        std::string http_response;

        mobius::core::datetime::date start_period;
        mobius::core::datetime::date end_period;

        mobius::core::datetime::datetime local_time;

        bool is_deleted = false;
        bool is_reallocated = false;

        std::unordered_map<std::uint8_t, mobius::core::pod::data> tags;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_msiecf (const mobius::core::io::reader &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if stream is an instance of MSIECF file
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const noexcept
    {
        return is_instance_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get file signature
    // @return File signature
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_signature () const
    {
        return signature_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get file version
    // @return File version
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_version () const
    {
        return version_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get file size
    // @return File size
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    uint32_t
    get_size () const
    {
        return size_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get hash table offset
    // @return Hash table offset
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    uint32_t
    get_hash_table_offset () const
    {
        return hash_table_offset_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get total blocks
    // @return Total blocks
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    uint32_t
    get_total_blocks () const
    {
        return total_blocks_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get allocated blocks
    // @return Allocated blocks
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    uint32_t
    get_allocated_blocks () const
    {
        return allocated_blocks_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get cache size limit
    // @return Cache size limit
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    uint64_t
    get_cache_size_limit () const
    {
        return cache_size_limit_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get cache size
    // @return Cache size
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    uint64_t
    get_cache_size () const
    {
        return cache_size_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get cache size non releasable
    // @return Cache size non releasable
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    uint64_t
    get_cache_size_non_releasable () const
    {
        return cache_size_non_releasable_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get cache directories
    // @return Cache directories
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<cache_directory_entry>
    get_directories () const
    {
        return directories_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<url> get_urls () const;

  private:
    // @brief Flag is instance
    bool is_instance_ = false;

    // @brief File signature
    std::string signature_;

    // @brief File version
    std::string version_;

    // @brief File size
    uint32_t size_ = 0;

    // @brief Hash table offset
    uint32_t hash_table_offset_ = 0;

    // @brief Total blocks
    uint32_t total_blocks_ = 0;

    // @brief Allocated blocks
    uint32_t allocated_blocks_ = 0;

    // @brief Cache size limit
    uint64_t cache_size_limit_ = 0;

    // @brief Cache size
    uint64_t cache_size_ = 0;

    // @brief Cache size non releasable
    uint64_t cache_size_non_releasable_ = 0;

    // @brief Cache directories
    std::vector<cache_directory_entry> directories_;

    // @brief Offset -> URL mapping
    std::unordered_map<std::uint32_t, url> urls_;

    // Helper functions
    std::uint32_t _decode_hash_table (mobius::core::decoder::data_decoder &);
    void _decode_record (mobius::core::decoder::data_decoder &, const hash_table_record &);
    void _decode_url_record (mobius::core::decoder::data_decoder &, const hash_table_record &);
};

} // namespace mobius::extension::app::internet_explorer

#endif
