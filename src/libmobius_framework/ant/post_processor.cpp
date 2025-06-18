// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <mobius/framework/ant/post_processor.hpp>
#include <mutex>
#include <string>
#include <unordered_map>

namespace
{
// @brief Map to hold post-processor implementation data
static std::unordered_map<
    std::string, mobius::framework::ant::post_processor_implementation_data>
    data;

// @brief Mutex to protect access to the factories map
static std::mutex mutex;

} // namespace

namespace mobius::framework::ant
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Register a post-processor
// @param id Unique identifier for the post-processor
// @param factory Function to create an instance of the post-processor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
register_post_processor_implementation (
    const std::string &id, const std::string &name,
    mobius::framework::ant::post_processor_implementation_builder factory
)
{
    post_processor_implementation_data data_entry;
    data_entry.id = id;
    data_entry.name = name;
    data_entry.factory = factory;

    std::lock_guard<std::mutex> lock (mutex);
    data[id] = data_entry;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unregister a post-processor
// @param id Unique identifier for the post-processor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
unregister_post_processor_implementation (const std::string &id)
{
    std::lock_guard<std::mutex> lock (mutex);
    data.erase (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get all registered post-processor implementations
// @return Vector of pairs containing the ID and name of each post-processor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<mobius::framework::ant::post_processor_implementation_data>
list_post_processor_implementations ()
{
    std::lock_guard<std::mutex> lock (mutex);
    std::vector<mobius::framework::ant::post_processor_implementation_data>
        implementations (data.size ());

    std::transform (
        data.begin (), data.end (), implementations.begin (),
        [] (const auto &pair) { return pair.second; }
    );

    return implementations;
}

} // namespace mobius::framework::ant
