#ifndef MOBIUS_FRAMEWORK_ANT_POST_PROCESSOR_HPP
#define MOBIUS_FRAMEWORK_ANT_POST_PROCESSOR_HPP

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
#include <functional>
#include <memory>
#include <mobius/framework/ant/ant.hpp>
#include <mobius/framework/ant/ant_impl_post_processor.hpp>
#include <mobius/framework/ant/post_processor_impl_base.hpp>
#include <mobius/framework/model/item.hpp>
#include <string>
#include <utility>
#include <vector>

namespace mobius::framework::ant
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>post_processor</i> ANT handle class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class post_processor : public ant
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Constructor
    // @param item Case item object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit post_processor (const mobius::framework::model::item &item)
        : ant (std::make_shared<ant_impl_post_processor> (item))
    {
    }
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Post processor implementation functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
using post_processor_implementation_builder = std::function<std::shared_ptr<
    post_processor_impl_base> (const mobius::framework::model::item &)>;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Data structure to hold post-processor implementation data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct post_processor_implementation_data
{
    // @brief Unique identifier for the post-processor
    std::string id;

    // @brief Name of the post-processor
    std::string name;

    // @brief Function to create an instance of the post-processor
    mobius::framework::ant::post_processor_implementation_builder factory;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Post processor implementation prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void register_post_processor_implementation (
    const std::string &, const std::string &,
    post_processor_implementation_builder
);

void unregister_post_processor_implementation (const std::string &);

std::vector<post_processor_implementation_data>
list_post_processor_implementations ();

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Register a post-processor implementation
// @param id Unique identifier for the post-processor
// @param name Name of the post-processor
// @tparam T Type of the post-processor implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T>
void
register_post_processor_implementation (
    const std::string &id, const std::string &name
)
{
    register_post_processor_implementation (
        id, name, [] (const mobius::framework::model::item &item)
        { return std::make_shared<T> (item); }
    );
}

} // namespace mobius::framework::ant

#endif
