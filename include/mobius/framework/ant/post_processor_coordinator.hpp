#ifndef MOBIUS_FRAMEWORK_ANT_POST_PROCESSOR_COORDINATOR_HPP
#define MOBIUS_FRAMEWORK_ANT_POST_PROCESSOR_COORDINATOR_HPP

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
#include <memory>
#include <mobius/framework/ant/post_processor_impl_base.hpp>
#include <mobius/framework/model/evidence.hpp>
#include <mobius/framework/model/item.hpp>
#include <string>
#include <vector>

namespace mobius::framework::ant
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>post_processor_coordinator</i> class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class post_processor_coordinator
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit post_processor_coordinator (
        const mobius::framework::model::item &item
    );
    void on_evidence (const mobius::framework::model::evidence &);
    void on_tag_modified (
        const mobius::framework::model::evidence &, const std::string &
    );
    void on_attribute_modified (
        const mobius::framework::model::evidence &, const std::string &
    );

  private:
    // @brief Case item object
    mobius::framework::model::item item_;

    // @brief Post-processors
    std::vector<std::shared_ptr<post_processor_impl_base>> post_processors_;
};

} // namespace mobius::framework::ant

#endif
