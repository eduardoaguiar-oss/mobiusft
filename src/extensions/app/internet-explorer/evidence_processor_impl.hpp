#ifndef MOBIUS_EXTENSION_APP_INTERNET_EXPLORER_EVIDENCE_PROCESSOR_IMPL_HPP
#define MOBIUS_EXTENSION_APP_INTERNET_EXPLORER_EVIDENCE_PROCESSOR_IMPL_HPP

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
#include <mobius/framework/evidence_processor/evidence_processor_impl_base.hpp>
#include <mobius/framework/evidence_processor/mediator.hpp>
#include <mobius/framework/evidence_processor/profile.hpp>
#include <mobius/framework/model/item.hpp>

namespace mobius::extension::app::internet_explorer
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Internet Explorer <i>evidence_processor</i> implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class evidence_processor_impl : public mobius::framework::evidence_processor::evidence_processor_impl_base
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Visited URL struct
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct visited_url
    {
        // @brief Timestamp
        mobius::core::datetime::datetime timestamp;

        // @brief Title
        std::string title;

        // @brief Url
        std::string url;

        // @brief Username
        std::string username;
        
        // @brief Metadata
        mobius::core::pod::map metadata;

        // @brief Source file
        mobius::core::io::file f;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit evidence_processor_impl (
        const mobius::framework::model::item &,
        const mobius::framework::evidence_processor::profile &,
        const mobius::framework::evidence_processor::mediator &
    );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Function prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void on_folder_entered (const mobius::core::io::folder &) final;
    void on_complete () final;

  private:
    // @brief Case item
    mobius::framework::model::item item_;

    // @brief Mediator
    mobius::framework::evidence_processor::mediator mediator_;

    // @brief Visited URLs
    std::vector<visited_url> visited_urls_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _scan_folder (const mobius::core::io::folder &);
    void _decode_index_dat_file (const mobius::core::io::file &);
    void _decode_webcachev01_dat_file (const mobius::core::io::file &);
    void _save_visited_urls ();
};

} // namespace mobius::extension::app::internet_explorer

#endif
