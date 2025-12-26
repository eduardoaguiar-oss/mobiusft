#ifndef MOBIUS_CORE_UI_UI_HPP
#define MOBIUS_CORE_UI_UI_HPP

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
#include <mobius/core/ui/ui_impl_base.hpp>
#include <functional>
#include <memory>
#include <string>

namespace mobius::core::ui
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Data structure to hold UI implementation data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
using implementation_builder = std::function<std::shared_ptr<ui_impl_base> ()>;

struct implementation_data
{
    // @brief Unique identifier for the UI implementation
    std::string id;

    // @brief Name of the UI implementation
    std::string name;

    // @brief Function to create an instance of the UI implementation
    implementation_builder builder;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// UI resource
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
using resource_type = std::function<std::shared_ptr<ui_impl_base> ()>;

template <typename T>
constexpr resource_type
make_resource ()
{
    return [] () { return std::make_shared<T> (); };
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Register an UI implementation
// @param id Unique identifier for the UI implementation
// @param name Name of the UI implementation
// @tparam T Type of the UI implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T>
void
register_implementation (const std::string &id, const std::string &name)
{
    register_implementation (
        id, name, [] () { return std::make_shared<T> (); }
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Function prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void register_implementation (
    const std::string &, const std::string &, implementation_builder
);
void unregister_implementation (const std::string &);
void set_implementation (const std::string &);
std::shared_ptr<ui_impl_base> get_implementation ();
std::vector<implementation_data> list_implementations ();

void init ();
void start ();
void stop ();
void flush ();

} // namespace mobius::core::ui

#endif
