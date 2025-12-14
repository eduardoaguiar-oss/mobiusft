// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
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
#include <mobius/core/exception.inc>
#include <mobius/core/resource.hpp>
#include <mobius/core/ui/ui.hpp>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>
#include <string>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// @brief Map to hold UI implementation data
static std::unordered_map<std::string, mobius::core::ui::implementation_data>
    data;

// @brief Mutex to protect access to the implementation map
static std::mutex data_mutex;

// @brief Current UI implementation instance
std::shared_ptr<mobius::core::ui::ui_impl_base> current_impl;

// @brief Current UI implementation ID
static std::string current_impl_id;

} // namespace

namespace mobius::core::ui
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Register an UI implementation
// @param id Unique identifier for the UI implementation
// @param name Name of the UI implementation
// @param builder Function to create an instance of the UI implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
register_implementation (
    const std::string &id,
    const std::string &name,
    mobius::core::ui::implementation_builder builder
)
{
    mobius::core::ui::implementation_data data_entry;
    data_entry.id = id;
    data_entry.name = name;
    data_entry.builder = builder;

    std::lock_guard<std::mutex> lock (data_mutex);
    data[id] = data_entry;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unregister an UI implementation
// @param id Unique identifier for the UI implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
unregister_implementation (const std::string &id)
{
    if (id == current_impl_id)
    {
        current_impl.reset ();
        current_impl_id.clear ();
    }

    std::lock_guard<std::mutex> lock (data_mutex);
    data.erase (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get all registered UI implementations
// @return Vector of pairs containing the ID and name of each UI implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<implementation_data>
list_vfs_processor_implementations ()
{
    std::lock_guard<std::mutex> lock (data_mutex);

    std::vector<implementation_data> implementations (data.size ());

    std::transform (
        data.begin (), data.end (), implementations.begin (),
        [] (const auto &pair) { return pair.second; }
    );

    return implementations;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set current UI implementation class
// @param id UI implementation ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
set_implementation (const std::string &id)
{
    // Check if current implementation is already set
    if (current_impl)
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("UI implementation already set")
        );

    // search implementation
    std::lock_guard<std::mutex> lock (data_mutex);

    auto iter = data.find (id);
    if (iter == data.end ())
        throw std::runtime_error (
            MOBIUS_EXCEPTION_MSG ("UI implementation '" + id + "' not found")
        );

    // build implementation
    current_impl = iter->second.builder ();
    current_impl_id = id;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get current UI implementation class
// @return Smart pointer to UI implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::shared_ptr<ui_impl_base>
get_implementation ()
{
    // if current_impl is not set, create using first implementation available
    if (!current_impl)
    {
        std::lock_guard<std::mutex> lock (data_mutex);

        auto iter = data.begin ();

        if (iter == data.end ())
            throw std::runtime_error (
                MOBIUS_EXCEPTION_MSG ("no UI implementation found")
            );

        current_impl = iter->second.builder ();
        current_impl_id = iter->second.id;
    }

    // return implementation instance
    return current_impl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Init user interface
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
init ()
{
    get_implementation ()
        ->flush (); // use flush to force get_implementation call
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Start user interface
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
start ()
{
    get_implementation ()->start ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Stop user interface
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
stop ()
{
    get_implementation ()->stop ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Flush UI events
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
flush ()
{
    get_implementation ()->flush ();
}

} // namespace mobius::core::ui
