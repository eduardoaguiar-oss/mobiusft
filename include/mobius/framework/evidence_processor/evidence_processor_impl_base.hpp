#ifndef MOBIUS_FRAMEWORK_EVIDENCE_PROCESSOR_EVIDENCE_PROCESSOR_IMPL_BASE_HPP
#define MOBIUS_FRAMEWORK_EVIDENCE_PROCESSOR_EVIDENCE_PROCESSOR_IMPL_BASE_HPP

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
#include <mobius/core/io/folder.hpp>
#include <mobius/core/pod/map.hpp>
#include <mobius/framework/model/evidence.hpp>

namespace mobius::framework::evidence_processor
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>evidence_processor_impl_base</i> class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class evidence_processor_impl_base
{
  public:
    virtual ~evidence_processor_impl_base () = default;

    // @brief Called when processing starts
    virtual void on_start () {};

    // @brief Called when processing stops
    virtual void on_stop () {};

    // @brief Called when processing is complete
    virtual void on_complete () {};

    // @brief Called when a folder is entered
    virtual void on_folder_entered (const mobius::core::io::folder &) {};

    // @brief Called when a folder is exited
    virtual void on_folder_exited (const mobius::core::io::folder &) {};

    // @brief Called when a new evidence is created
    virtual void on_evidence_created (mobius::framework::model::evidence) {};

    // @brief Called when an evidence attribute is modified
    virtual void on_evidence_attribute_modified (
        mobius::framework::model::evidence, const std::string &
    ) {};

    // @brief Called when an evidence tag is modified
    virtual void on_evidence_tag_modified (
        mobius::framework::model::evidence, const std::string &
    ) {};

    // @brief Get current status
    virtual mobius::core::pod::map
    get_status () const
    {
        return {};
    }
};

} // namespace mobius::framework::evidence_processor

#endif
