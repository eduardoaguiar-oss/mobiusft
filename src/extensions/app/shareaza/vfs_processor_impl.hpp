#ifndef MOBIUS_EXTENSION_APP_SHAREAZA_VFS_PROCESSOR_IMPL_HPP
#define MOBIUS_EXTENSION_APP_SHAREAZA_VFS_PROCESSOR_IMPL_HPP

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
#include <mobius/framework/ant/vfs_processor_impl_base.hpp>
#include <mobius/framework/case_profile.hpp>
#include <mobius/framework/model/item.hpp>
#include "profile.hpp"

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Shareaza <i>vfs_processor</i> implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class vfs_processor_impl
    : public mobius::framework::ant::vfs_processor_impl_base
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Autofill
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct autofill
    {
        bool is_deleted = false;
        std::string username;
        std::string value;
        std::string id;
        mobius::core::io::file f;
    };

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

    // @brief Profiles found
    std::vector<profile> profiles_;

    // @brief Autofill data
    std::vector<autofill> autofills_;

    // @brief Local files from .sd files
    std::vector<profile::local_file> local_files_;

    // @brief Remote files from .sd files
    std::vector<profile::remote_file> remote_files_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _scan_ntuser_dat_files (const mobius::core::io::folder &);
    void _scan_profile_folder (const mobius::core::io::folder &);
    void _scan_sd_files (const mobius::core::io::folder &);
    void _decode_ntuser_dat_file (const mobius::core::io::file &);
    void _decode_sd_file (const mobius::core::io::file &);

    void _save_app_profiles ();
    void _save_autofills ();
    void _save_local_files ();
    void _save_received_files ();
    void _save_remote_party_shared_files ();
    void _save_searched_texts ();
    void _save_sent_files ();
    void _save_shared_files ();
    void _save_user_accounts ();
};

} // namespace mobius::extension::app::shareaza

#endif
