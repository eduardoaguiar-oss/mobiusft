#ifndef MOBIUS_EXTENSION_APP_CHROMIUM_VFS_PROCESSOR_IMPL_HPP
#define MOBIUS_EXTENSION_APP_CHROMIUM_VFS_PROCESSOR_IMPL_HPP

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <mobius/framework/ant/vfs_processor_impl_base.hpp>
#include <mobius/framework/case_profile.hpp>
#include <mobius/framework/model/item.hpp>
#include <string>
#include <vector>
#include "file_local_state.hpp"
#include "profile.hpp"

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Chromium <i>vfs_processor</i> implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class vfs_processor_impl : public mobius::framework::ant::vfs_processor_impl_base
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Datatypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    using encryption_key = file_local_state::encryption_key;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit vfs_processor_impl (
        const mobius::framework::model::item &,
        const mobius::framework::case_profile &
    );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Function prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void on_folder (const mobius::core::io::folder &) final;
    void on_complete () final;

  private:
    // @brief Case item
    mobius::framework::model::item item_;

    // @brief User name
    std::string username_;

    // @brief Current profile
    profile profile_;

    // @brief Profiles found
    std::vector<profile> profiles_;

    // @brief Encryption keys found
    std::vector<encryption_key> encryption_keys_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _scan_local_state (const mobius::core::io::folder &);
    void _scan_profile (const mobius::core::io::folder &);
    void _decode_local_state_file (const mobius::core::io::file &);

    void _save_app_profiles ();
    void _save_autofills ();
    void _save_bookmarked_urls ();
    void _save_cookies ();
    void _save_credit_cards ();
    void _save_encryption_keys ();
    void _save_passwords ();
    void _save_pdis ();
    void _save_received_files ();
    void _save_user_accounts ();
    void _save_visited_urls ();
};

} // namespace mobius::extension::app::chromium

#endif
