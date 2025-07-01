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
#include "common.hpp"
#include <limits>
#include <iostream>
#include <vector>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Debug flag
// This flag is used to enable or disable debug output in the code.
// When set to true, additional debug information will be printed to the
// console. When set to false, debug output will be suppressed. This can be
// useful for development and troubleshooting purposes.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static constexpr bool DEBUG = true;

} // namespace

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Generate SQL statement with version-aware column replacements
// @param sql_template The SQL template string with
// ${column,start_version,end_version} placeholders
// @param schema_version The current schema version to check against
// @return Processed SQL statement with appropriate columns based on version
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
generate_sql (const std::string &sql_template, int64_t schema_version)
{
    std::string result = sql_template;
    std::string::size_type pos = 0;

    while ((pos = result.find ("${", pos)) != std::string::npos)
    {
        // Find the closing bracket
        auto end_pos = result.find ("}", pos);
        if (end_pos == std::string::npos)
        {
            pos += 2; // Skip the "${" and continue
            continue;
        }

        // Extract the placeholder content
        std::string placeholder = result.substr (pos + 2, end_pos - pos - 2);

        // Split by comma
        std::vector<std::string> parts;
        std::string::size_type prev = 0;
        std::string::size_type comma_pos = 0;

        while ((comma_pos = placeholder.find (',', prev)) != std::string::npos)
        {
            parts.push_back (placeholder.substr (prev, comma_pos - prev));
            prev = comma_pos + 1;
        }
        parts.push_back (placeholder.substr (prev));

        // Default to empty string if column should be excluded
        std::string replacement = "NULL";

        // Get column name and version ranges
        std::string column_name = parts[0];
        int64_t start_version = -1;
        int64_t end_version = std::numeric_limits<int64_t>::max ();

        // Parse start_version if provided
        if (parts.size () > 1 && !parts[1].empty ())
        {
            try
            {
                start_version = std::stoll (parts[1]);
            }
            catch (...)
            {
                // Keep default if parsing fails
            }
        }

        // Parse end_version if provided
        if (parts.size () > 2 && !parts[2].empty ())
        {
            try
            {
                end_version = std::stoll (parts[2]);
            }
            catch (...)
            {
                // Keep default if parsing fails
            }
        }

        // Check if current schema version is within range
        if (schema_version >= start_version && schema_version <= end_version)
            replacement = column_name;

        // Replace the placeholder with the column name or empty string
        result.replace (pos, end_pos - pos + 1, replacement);

        // Continue searching from current position
        // No need to advance pos since the replacement might be shorter than
        // the placeholder
    }

    if (DEBUG)
        std::cout << "Generated SQL (schema version=" << schema_version
                  << "): " << result << std::endl;

    return result;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert chromium timestamp to date/time
// @param timestamp Numerical value representing the timestamp
// @return Date/time object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
get_datetime (std::uint64_t timestamp)
{
    if (timestamp == 0 || timestamp == 86400000000)
        return {};

    else if (timestamp <= std::numeric_limits<std::uint32_t>::max ())
        return mobius::core::datetime::new_datetime_from_unix_timestamp (
            timestamp
        );

    else if (timestamp < 10000000000000000)
        return mobius::core::datetime::new_datetime_from_unix_timestamp (
            timestamp / 1000000
        );

    else
        return mobius::core::datetime::new_datetime_from_nt_timestamp (
            timestamp * 10
        );
}

} // namespace mobius::extension::app::chromium