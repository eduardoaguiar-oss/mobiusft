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
#include <mobius/framework/ant/ant_impl_vfs_processor.hpp>
#include <mobius/framework/ant/vfs_processor.hpp>
#include <mutex>
#include <unordered_map>
#include <string>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Map to hold vfs-processor implementation data
static std::unordered_map<
    std::string,
    mobius::framework::ant::vfs_processor_implementation_data>
    data;

// @brief Mutex to protect access to the factories map
static std::mutex mutex;

} // namespace

namespace mobius::framework::ant
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param item Case item object
// @param case_profile Case profile object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
vfs_processor::vfs_processor (
    const mobius::framework::model::item &item,
    const mobius::framework::case_profile &case_profile
)
    : ant (std::make_shared<ant_impl_vfs_processor> (item, case_profile))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Register a vfs-processor
// @param id Unique identifier for the vfs-processor
// @param factory Function to create an instance of the vfs-processor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
register_vfs_processor_implementation (
    const std::string &id,
    const std::string &name,
    mobius::framework::ant::vfs_processor_implementation_builder factory
)
{
    vfs_processor_implementation_data data_entry;
    data_entry.id = id;
    data_entry.name = name;
    data_entry.factory = factory;

    std::lock_guard<std::mutex> lock (mutex);
    data[id] = data_entry;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unregister a vfs-processor
// @param id Unique identifier for the vfs-processor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
unregister_vfs_processor_implementation (const std::string &id)
{
    std::lock_guard<std::mutex> lock (mutex);
    data.erase (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get a registered vfs-processor implementation
// @param id Unique identifier for the vfs-processor
// @return Optional containing the vfs-processor implementation data if found,
// empty optional otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::optional<mobius::framework::ant::vfs_processor_implementation_data>
get_vfs_processor_implementation (const std::string &id)
{
    std::lock_guard<std::mutex> lock (mutex);

    auto iter = data.find (id);
    if (iter != data.end ())
        return iter->second;

    return std::nullopt;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get all registered vfs-processor implementations
// @return Vector of pairs containing the ID and name of each vfs-processor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<mobius::framework::ant::vfs_processor_implementation_data>
list_vfs_processor_implementations ()
{
    std::lock_guard<std::mutex> lock (mutex);
    std::vector<mobius::framework::ant::vfs_processor_implementation_data>
        implementations (data.size ());

    std::transform (
        data.begin (), data.end (), implementations.begin (),
        [] (const auto &pair) { return pair.second; }
    );

    return implementations;
}

} // namespace mobius::framework::ant
