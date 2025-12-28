#ifndef MOBIUS_EXTENSION_APP_SKYPE_VFS_PROCESSOR_IMPL_HPP
#define MOBIUS_EXTENSION_APP_SKYPE_VFS_PROCESSOR_IMPL_HPP

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
#include <vector>
#include "profile.hpp"

namespace mobius::extension::app::skype
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Skype <i>vfs_processor</i> implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class vfs_processor_impl
    : public mobius::framework::ant::vfs_processor_impl_base
{
  public:
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

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _scan_profile_folder (const mobius::core::io::folder &);
    void _scan_s4l_files (const mobius::core::io::folder &);
    void _decode_s4l_file (const mobius::core::io::file &);

    void _save_app_profiles ();
    void _save_calls ();
    void _save_contacts ();
    void _save_messages ();
    void _save_received_files ();
    void _save_remote_party_ip_addresses ();
    void _save_sent_files ();
    void _save_sms_messages ();
    void _save_user_accounts ();
    void _save_voicemails ();
};

} // namespace mobius::extension::app::skype

#endif
