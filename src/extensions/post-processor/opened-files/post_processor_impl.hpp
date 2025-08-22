#ifndef MOBIUS_EXTENSION_POST_PROCESSOR_OPENED_FILES_POST_PROCESSOR_IMPL_HPP
#define MOBIUS_EXTENSION_POST_PROCESSOR_OPENED_FILES_POST_PROCESSOR_IMPL_HPP

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
#include <mobius/framework/ant/post_processor_impl_base.hpp>
#include <mobius/framework/model/item.hpp>

namespace mobius::extension::post_processor_opened_files
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Opened Files post-processor implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class post_processor_impl
    : public mobius::framework::ant::post_processor_impl_base
{
  public:
    explicit post_processor_impl (
        mobius::framework::ant::post_processor_coordinator &coordinator,
        const mobius::framework::model::item &item
    );
    void process_evidence (mobius::framework::model::evidence) final;

  private:
    // @brief Post-processor coordinator
    mobius::framework::ant::post_processor_coordinator &coordinator_;

    // @brief Case item
    mobius::framework::model::item item_;
};

} // namespace mobius::extension::post_processor_opened_files

#endif