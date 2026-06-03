#ifndef MOBIUS_FRAMEWORK_PROCESSOR_PROCESSOR_HPP
#define MOBIUS_FRAMEWORK_PROCESSOR_PROCESSOR_HPP

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
#include <mobius/framework/model/item.hpp>
#include <mobius/framework/processor/processor_impl_base.hpp>
#include <mobius/framework/processor/profile.hpp>
#include <memory>
#include <string>

namespace mobius::framework::processor
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>processor</i> class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class processor
{
  public:
    processor (const mobius::framework::model::item &, const std::string &);
    processor (processor &&) noexcept = default;
    processor (const processor &) noexcept = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    processor &operator= (const processor &) noexcept = default;
    processor &operator= (processor &&) noexcept = default;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Function prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void run ();
    void update ();
    mobius::framework::model::item get_item () const;
    profile get_profile () const;
    mobius::core::pod::map get_status () const;

  private:
    // @brief Implementation class forward declaration
    class impl;

    // @brief Implementation pointer
    std::shared_ptr<impl> impl_;
};

} // namespace mobius::framework::processor

#endif
