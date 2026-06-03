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
#include <mobius/framework/processor/processor_registry.hpp>
#include <mutex>
#include <unordered_map>
#include <string>

namespace
{
// @brief Map to hold processor implementation data
static std::unordered_map<
    std::string,
    mobius::framework::processor::processor_implementation_data>
    data;

// @brief Mutex to protect access to the factories map
static std::mutex mutex;

} // namespace

namespace mobius::framework::processor
{

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Register a processor
// @param id Unique identifier for the processor
// @param factory Function to create an instance of the processor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
register_processor_implementation (
    const std::string &id,
    const std::string &name,
    mobius::framework::processor::processor_implementation_builder factory
)
{
    processor_implementation_data data_entry;
    data_entry.id = id;
    data_entry.name = name;
    data_entry.factory = factory;

    std::lock_guard<std::mutex> lock (mutex);
    data[id] = data_entry;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unregister a processor
// @param id Unique identifier for the processor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
unregister_processor_implementation (const std::string &id)
{
    std::lock_guard<std::mutex> lock (mutex);
    data.erase (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get processor implementation data by ID
// @param id Unique identifier for the processor
// @return Optional containing the processor implementation data if found, or
//         std::nullopt if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::optional<processor_implementation_data>
get_processor_implementation (const std::string &id)
{
    std::lock_guard<std::mutex> lock (mutex);

    auto iter = data.find (id);

    if (iter != data.end ())
        return iter->second;

    return std::nullopt;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get all registered processor implementations
// @return Vector of pairs containing the ID and name of each processor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<mobius::framework::processor::processor_implementation_data>
list_processor_implementations ()
{
    std::lock_guard<std::mutex> lock (mutex);
    std::vector<mobius::framework::processor::processor_implementation_data>
        implementations (data.size ());

    std::transform (
        data.begin (), data.end (), implementations.begin (),
        [] (const auto &pair) { return pair.second; }
    );

    return implementations;
}

} // namespace mobius::framework::processor
