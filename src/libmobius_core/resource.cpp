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
#include <algorithm>
#include <mobius/core/mediator.hpp>
#include <mobius/core/resource.hpp>
#include <mobius/core/string_functions.hpp>
#include <mutex>
#include <string>
#include <unordered_map>

namespace mobius::core
{
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Resources
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Resources
std::unordered_map<std::string, resource> resources_;

// @brief Resource mutex
static std::mutex mutex_;
} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add resource
// @param id Resource ID
// @param r Resource
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
add_resource (const std::string &id, const resource &r)
{
    {
        std::lock_guard<std::mutex> lock (mutex_);
        resources_[id] = r;
    }

    mobius::core::emit ("resource-added", id, r);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remove resource
// @param id Resource ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
remove_resource (const std::string &id)
{
    {
        std::lock_guard<std::mutex> lock (mutex_);
        resources_.erase (id);
    }

    mobius::core::emit ("resource-removed", id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if resource exists
// @param id Resource ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
has_resource (const std::string &id)
{
    std::lock_guard<std::mutex> lock (mutex_);
    return resources_.find (id) != resources_.end ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get resource
// @param id Resource ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
resource
get_resource (const std::string &id)
{
    std::lock_guard<std::mutex> lock (mutex_);

    auto p = resources_.find (id);

    if (p != resources_.end ())
        return p->second;

    return resource ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get resources
// @param group_id Group ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<resource>
get_resources (const std::string &group_id)
{
    std::vector<resource> resources;
    std::string prefix = group_id + '.';

    // get resources
    {
        std::lock_guard<std::mutex> lock (mutex_);

        for (const auto &p : resources_)
            if (mobius::core::string::startswith (p.first, prefix))
                resources.push_back (p.second);
    }

    // sort resources by ID
    std::sort (resources.begin (), resources.end (),
               [] (const auto &a, const auto &b)
               { return a.get_id () < b.get_id (); });

    return resources;
}

} // namespace mobius::core
