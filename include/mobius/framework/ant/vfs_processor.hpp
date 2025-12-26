#ifndef MOBIUS_FRAMEWORK_ANT_VFS_PROCESSOR_HPP
#define MOBIUS_FRAMEWORK_ANT_VFS_PROCESSOR_HPP

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <mobius/framework/ant/ant.hpp>
#include <mobius/framework/ant/vfs_processor_impl_base.hpp>
#include <mobius/framework/case_profile.hpp>
#include <mobius/framework/model/item.hpp>
#include <functional>
#include <optional>
#include <memory>
#include <utility>
#include <string>
#include <vector>

namespace mobius::framework::ant
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>vfs_processor</i> ANT class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class vfs_processor : public ant
{
  public:
    explicit vfs_processor (
        const mobius::framework::model::item &,
        const mobius::framework::case_profile &
    );
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// VFS processor implementation functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
using vfs_processor_implementation_builder =
    std::function<std::shared_ptr<vfs_processor_impl_base> (
        const mobius::framework::model::item &,
        const mobius::framework::case_profile &
    )>;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Data structure to hold vfs-processor implementation data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct vfs_processor_implementation_data
{
    // @brief Unique identifier for the vfs-processor
    std::string id;

    // @brief Name of the vfs-processor
    std::string name;

    // @brief Function to create an instance of the vfs-processor
    mobius::framework::ant::vfs_processor_implementation_builder factory;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// VFS processor implementation prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void register_vfs_processor_implementation (
    const std::string &,
    const std::string &,
    vfs_processor_implementation_builder
);

void unregister_vfs_processor_implementation (const std::string &);

std::optional<vfs_processor_implementation_data>
get_vfs_processor_implementation (const std::string &);

std::vector<vfs_processor_implementation_data>
list_vfs_processor_implementations ();

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Register a vfs-processor implementation
// @param id Unique identifier for the vfs-processor
// @param name Name of the vfs-processor
// @tparam T Type of the vfs-processor implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T>
void
register_vfs_processor_implementation (
    const std::string &id, const std::string &name
)
{
    register_vfs_processor_implementation (
        id, name,
        [] (const mobius::framework::model::item &item,
            const mobius::framework::case_profile &profile)
        { return std::make_shared<T> (item, profile); }
    );
}

} // namespace mobius::framework::ant

#endif
