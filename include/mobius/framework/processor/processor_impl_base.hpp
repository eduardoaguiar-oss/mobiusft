#ifndef MOBIUS_FRAMEWORK_PROCESSOR_PROCESSOR_IMPL_BASE_HPP
#define MOBIUS_FRAMEWORK_PROCESSOR_PROCESSOR_IMPL_BASE_HPP

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
#include <mobius/core/pod/map.hpp>
#include <mobius/framework/model/evidence.hpp>

namespace mobius::framework::processor
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>processor_impl_base</i> class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class processor_impl_base
{
  public:
    virtual ~processor_impl_base () = default;

    virtual void on_start () {};
    virtual void on_run () {};
    virtual void on_complete () {};
    virtual void on_evidence_created (mobius::framework::model::evidence &) {};
    virtual void on_evidence_attribute_modified (mobius::framework::model::evidence &, const std::string &) {};
    virtual void on_evidence_tag_modified (mobius::framework::model::evidence &, const std::string &) {};
    virtual void on_stop () {};
    
    virtual mobius::core::pod::map
    get_status () const
    {
        return {};
    }
};

} // namespace mobius::framework::processor

#endif
