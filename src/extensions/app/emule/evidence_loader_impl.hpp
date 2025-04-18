#ifndef MOBIUS_EXTENSION_APP_EMULE_EVIDENCE_LOADER_IMPL_HPP
#define MOBIUS_EXTENSION_APP_EMULE_EVIDENCE_LOADER_IMPL_HPP

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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
#include <mobius/datetime/datetime.h>
#include <mobius/framework/evidence_flag.h>
#include <mobius/framework/evidence_loader_impl_base.h>
#include <mobius/io/file.h>
#include <map>
#include <vector>

namespace mobius::extension::app::emule
{

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Emule account
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct account
{
    bool is_deleted = false;

    // from preferences.dat
    std::string username;
    std::string emule_guid;
    std::uint8_t preferences_dat_version = 0;

    // from preferenceskad.dat
    std::string kamdelia_guid;
    std::string kamdelia_ip;

    // from preferences.ini
    std::string incoming_dir;
    std::string temp_dir;
    std::string nick;
    std::string app_version;
    mobius::framework::evidence_flag auto_start;

    // from statistics.ini
    std::uint64_t total_downloaded_bytes = 0;
    std::uint64_t total_uploaded_bytes = 0;
    std::uint64_t download_completed_files = 0;

    // source files
    mobius::io::file preferences_dat_f;
    mobius::io::file preferences_ini_f;
    mobius::io::file preferenceskad_dat_f;
    mobius::io::file statistics_ini_f;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Autofill
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct autofill
{
    bool is_deleted = false;
    std::string username;
    std::string value;
    std::string id;
    mobius::pod::map metadata;
    mobius::io::file f;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Local file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct local_file
{
    // attributes
    std::string path;
    std::string filename;
    std::string username;

    // metadata
    mobius::pod::map metadata;

    // hashes
    mobius::pod::data hashes;

    // flags
    mobius::framework::evidence_flag flag_downloaded;
    mobius::framework::evidence_flag flag_uploaded;
    mobius::framework::evidence_flag flag_shared;
    mobius::framework::evidence_flag flag_completed;
    mobius::framework::evidence_flag flag_corrupted;

    // files
    mobius::io::file f;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Remote file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct remote_file
{
    // attributes
    mobius::datetime::datetime timestamp;
    std::string ip;
    std::uint16_t port = 0;
    std::string filename;
    std::string username;

    // metadata
    mobius::pod::map metadata;

    // hashes
    mobius::pod::data hashes;

    // files
    mobius::io::file f;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>Emule evidence_loader</i> implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class evidence_loader_impl : public mobius::framework::evidence_loader_impl_base
{
public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit evidence_loader_impl (const mobius::model::item&, scan_type);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Function prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void run () final;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if object is valid
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    explicit operator bool () const noexcept final
    {
        return true;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get evidence_loader type
    // @return Type as string
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_type () const final
    {
        return "app-emule";
    }

private:
    // @brief Case item
    mobius::model::item item_;

    // @brief Scan type
    scan_type scan_type_;

    // @brief User name
    std::string username_;

    // @brief Account data
    account account_;

    // @brief Accounts found
    std::vector <account> accounts_;

    // @brief Autofills found
    std::vector <autofill> autofills_;

    // @brief Local files
    std::vector <local_file> local_files_;

    // @brief Remote files
    std::vector <remote_file> remote_files_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _scan_canonical_folders ();
    void _scan_canonical_root_folder (const mobius::io::folder&);
    void _scan_canonical_user_folder (const mobius::io::folder&);
    void _scan_canonical_emule_xp_folder (const mobius::io::folder& folder);
    void _scan_canonical_emule_config_folder (const mobius::io::folder&);
    void _scan_canonical_emule_download_folder (const mobius::io::folder&);

    void _decode_ac_searchstrings_dat_file (const mobius::io::file&);
    void _decode_cancelled_met_file (const mobius::io::file&);
    void _decode_key_index_dat_file (const mobius::io::file&);
    void _decode_known_met_file (const mobius::io::file&);
    void _decode_part_met_file (const mobius::io::file&);
    void _decode_preferences_dat_file (const mobius::io::file&);
    void _decode_preferences_ini_file (const mobius::io::file&);
    void _decode_preferenceskad_dat_file (const mobius::io::file&);
    void _decode_statistics_ini_file (const mobius::io::file&);
    void _decode_storedsearches_met_file (const mobius::io::file&);

    void _save_evidences ();
    void _save_accounts ();
    void _save_autofills ();
    void _save_local_files ();
    void _save_p2p_remote_files ();
    void _save_received_files ();
    void _save_sent_files ();
    void _save_shared_files ();
};

} // namespace mobius::extension::app::emule

#endif
