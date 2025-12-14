#ifndef MOBIUS_EXTENSION_APP_EMULETORRENT_VFS_PROCESSOR_IMPL_HPP
#define MOBIUS_EXTENSION_APP_EMULETORRENT_VFS_PROCESSOR_IMPL_HPP

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
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
#include <mobius/core/datetime/datetime.hpp>
#include <mobius/framework/ant/vfs_processor_impl_base.hpp>
#include <mobius/framework/case_profile.hpp>
#include <mobius/framework/evidence_flag.hpp>
#include <mobius/framework/model/item.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include "profile.hpp"

namespace mobius::extension::app::emuletorrent
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief EmuleTorrent <i>vfs_processor</i> implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class vfs_processor_impl
    : public mobius::framework::ant::vfs_processor_impl_base
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Remote source (another users sharing file)
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct remote_source
    {
        mobius::core::datetime::datetime timestamp;
        std::string ip;
        std::uint16_t port;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief File
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct file
    {
        // basic attributes
        std::string hash_ed2k;
        std::string hash_sha1;
        std::string hash_sha2_256;
        std::string account_guid;
        std::string username;
        std::string filename;
        std::string path;
        std::uint64_t size = 0;

        // flags
        mobius::framework::evidence_flag flag_downloaded;
        mobius::framework::evidence_flag flag_uploaded;
        mobius::framework::evidence_flag flag_shared;
        mobius::framework::evidence_flag flag_completed;
        mobius::framework::evidence_flag flag_corrupted;

        // transfer info
        mobius::core::datetime::datetime download_started_time;
        mobius::core::datetime::datetime download_completed_time;

        // metadata
        mobius::core::pod::map metadata;

        // remote sources
        std::vector<remote_source> remote_sources;

        // evidence source
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

    // @brief File catalog
    std::vector<file> files_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _scan_roaming_folder (const mobius::core::io::folder &);
    void _scan_local_folder (const mobius::core::io::folder &);
    void _decode_bt_fastresume_file (const mobius::core::io::file &);
    void _decode_ed2k_fastresume_file (const mobius::core::io::file &);

    void _save_app_profiles ();
    void _save_local_files ();
    void _save_received_files ();
    void _save_remote_party_shared_files ();
    void _save_sent_files ();
    void _save_shared_files ();
    void _save_user_accounts ();
};

} // namespace mobius::extension::app::emuletorrent

#endif
