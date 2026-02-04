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
#include "profile.hpp"
#include <mobius/core/file_decoder/torrent.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/core/value_selector.hpp>
#include <mobius/framework/utils.hpp>
#include <algorithm>
#include <map>
#include "file_dht_dat.hpp"
#include "file_resume_dat.hpp"
#include "file_settings_dat.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @see https://www.forensicfocus.com/articles/forensic-analysis-of-the-%CE%BCtorrent-peer-to-peer-client-in-windows/
// @see https://robertpearsonblog.wordpress.com/2016/11/10/utorrent-forensic-artifacts/
// @see https://robertpearsonblog.wordpress.com/2016/11/11/utorrent-and-windows-10-forensic-nuggets-of-info/
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace mobius::extension::app::utorrent
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class profile::impl
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if profile is valid
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_valid () const
    {
        return bool (folder_);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get username
    // @return username
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_username () const
    {
        return username_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get folder
    // @return Folder object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::io::folder
    get_folder () const
    {
        return folder_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get path to profile
    // @return Path to profile
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_path () const
    {
        return (folder_) ? folder_.get_path () : "";
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get creation time
    // @return Creation time
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_creation_time () const
    {
        return creation_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get last modified time
    // @return Last modified time
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::datetime::datetime
    get_last_modified_time () const
    {
        return last_modified_time_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get main settings
    // @return settings
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    settings
    get_main_settings () const
    {
        return main_settings_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get all settings found
    // @return settings
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<settings>
    get_settings () const
    {
        return settings_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of local files
    // @return number of local files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::size_t
    size_local_files () const
    {
        return local_files_.size ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<account> get_accounts () const;
    std::vector<local_file> get_local_files () const;
    void add_dht_dat_file (const mobius::core::io::file &);
    void add_resume_dat_file (const mobius::core::io::file &);
    void add_settings_dat_file (const mobius::core::io::file &);
    void add_torrent_file (const mobius::core::io::file &);

  private:
    // @brief Folder object
    mobius::core::io::folder folder_;

    // @brief Username
    std::string username_;

    // @brief Creation time
    mobius::core::datetime::datetime creation_time_;

    // @brief Last modified time
    mobius::core::datetime::datetime last_modified_time_;

    // @brief Accounts
    std::map<std::string, account> accounts_;

    // @brief Local files
    std::map<std::string, local_file> local_files_;

    // @brief Main settings
    settings main_settings_;

    // @brief Settings found
    std::vector<settings> settings_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _set_folder (const mobius::core::io::folder &);
    void _update_mtime (const mobius::core::io::file &f);
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set folder
// @param f Folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_set_folder (const mobius::core::io::folder &f)
{
    if (folder_ || !f)
        return;

    folder_ = f;

    last_modified_time_ = f.get_modification_time ();
    creation_time_ = f.get_creation_time ();
    username_ = mobius::framework::get_username_from_path (f.get_path ());

    mobius::core::emit (
        "sampling_folder", std::string ("app.utorrent.profiles"), f
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update last modified time based on file
// @param f File
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_update_mtime (const mobius::core::io::file &f)
{
    if (!f)
        return;

    if (!last_modified_time_ ||
        f.get_modification_time () > last_modified_time_)
        last_modified_time_ = f.get_modification_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get accounts
// @return vector of accounts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<profile::account>
profile::impl::get_accounts () const
{
    std::vector<account> accounts;

    std::transform (
        accounts_.begin (), accounts_.end (), std::back_inserter (accounts),
        [] (const auto &pair) { return pair.second; }
    );

    std::sort (
        accounts.begin (), accounts.end (),
        [] (const auto &a, const auto &b) { return a.client_id < b.client_id; }
    );

    return accounts;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get local files
// @return vector of local files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<profile::local_file>
profile::impl::get_local_files () const
{
    std::vector<local_file> local_files;

    std::transform (
        local_files_.begin (), local_files_.end (),
        std::back_inserter (local_files),
        [] (const auto &pair) { return pair.second; }
    );

    std::sort (
        local_files.begin (), local_files.end (),
        [] (const auto &a, const auto &b)
        { return a.torrent_name < b.torrent_name; }
    );

    return local_files;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add dht.dat file
// @param f dht.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_dht_dat_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto reader = f.new_reader ();
    if (!reader)
        return;

    file_dht_dat dht_dat (reader);
    if (!dht_dat)
        return;

    log.info (__LINE__, "File decoded [dht.dat]: " + f.get_path ());

    _set_folder (f.get_parent ());
    _update_mtime (f);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add account
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto client_id = dht_dat.get_client_id ();
    if (client_id.empty ())
    {
        log.warning (__LINE__, "Client ID is empty");
        return;
    }

    auto [iter, _] = accounts_.try_emplace (client_id);
    auto &acc = iter->second;
    std::ignore = _;

    acc.client_id = client_id;

    if (!acc.first_dht_timestamp ||
        dht_dat.get_timestamp () < acc.first_dht_timestamp)
        acc.first_dht_timestamp = dht_dat.get_timestamp ();

    if (!acc.last_dht_timestamp ||
        dht_dat.get_timestamp () > acc.last_dht_timestamp)
        acc.last_dht_timestamp = dht_dat.get_timestamp ();

    acc.files.push_back (f);
    acc.ip_addresses.emplace (
        dht_dat.get_ip_address (), dht_dat.get_timestamp ()
    );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get data from file
    // @note Always accept the first file found;
    // Prefer non-deleted files over deleted ones;
    // When deletion status is the same, prefer files named exactly "dht.dat"
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (!acc.f || (acc.f.is_deleted () && !f.is_deleted ()) ||
        (acc.f.is_deleted () == f.is_deleted () &&
         acc.f.get_name () != "dht.dat" && f.get_name () == "dht.dat"))
    {
        acc.f = f;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Emit sampling_file event
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::emit (
        "sampling_file", std::string ("app.utorrent.dht_dat"), reader
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add resume.dat file
// @param f Resume.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_resume_dat_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto reader = f.new_reader ();
    if (!reader)
        return;

    file_resume_dat resume_dat (reader);
    if (!resume_dat)
        return;

    log.info (__LINE__, "File decoded [resume.dat]: " + f.get_path ());

    _set_folder (f.get_parent ());
    _update_mtime (f);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &entry : resume_dat.get_entries ())
    {
        local_file &lf = local_files_[entry.torrent_name];

        bool overwrite = !lf.resume_file ||
                         (lf.resume_file.is_deleted () && !f.is_deleted ()) ||
                         (lf.resume_file.is_deleted () == f.is_deleted () &&
                          lf.resume_file.get_name () != "resume.dat" &&
                          f.get_name () == "resume.dat");

        mobius::core::value_selector vs (overwrite);

        lf.name = vs (lf.name, entry.name);
        lf.metadata = vs (lf.metadata, entry.metadata);
        lf.added_timestamp = vs (lf.added_timestamp, entry.added_timestamp);
        lf.blocksize = vs (lf.blocksize, entry.blocksize);
        lf.bytes_downloaded = vs (lf.bytes_downloaded, entry.bytes_downloaded);
        lf.bytes_uploaded = vs (lf.bytes_uploaded, entry.bytes_uploaded);
        lf.caption = vs (lf.caption, entry.caption);
        lf.completed_timestamp =
            vs (lf.completed_timestamp, entry.completed_timestamp);
        lf.downloaded_time = vs (lf.downloaded_time, entry.downloaded_time);
        lf.download_url = vs (lf.download_url, entry.download_url);
        lf.is_auto_managed = vs (lf.is_auto_managed, entry.is_auto_managed);
        lf.is_corrupted = vs (lf.is_corrupted, entry.is_corrupted);
        lf.is_paused = vs (lf.is_paused, entry.is_paused);
        lf.is_seeding = vs (lf.is_seeding, entry.is_seeding);
        lf.is_sharing = vs (lf.is_sharing, entry.is_sharing);
        lf.is_super_seeding = vs (lf.is_super_seeding, entry.is_super_seeding);
        lf.is_sequential_downloading =
            vs (lf.is_sequential_downloading, entry.is_sequential_downloading);
        lf.is_uploading = vs (lf.is_uploading, entry.is_uploading);
        lf.is_visible = vs (lf.is_visible, entry.is_visible);
        lf.last_download_timestamp =
            vs (lf.last_download_timestamp, entry.last_download_timestamp);
        lf.last_seen_complete_timestamp =
            vs (lf.last_seen_complete_timestamp,
                entry.last_seen_complete_timestamp);
        lf.last_upload_timestamp =
            vs (lf.last_upload_timestamp, entry.last_upload_timestamp);
        lf.metadata_time = vs (lf.metadata_time, entry.metadata_timestamp);
        lf.path = vs (lf.path, entry.path);
        lf.seeded_time = vs (lf.seeded_time, entry.seeded_time);
        lf.resume_file = vs (lf.resume_file, f);
        lf.sources.push_back (f);
        lf.torrent_name = vs (lf.torrent_name, entry.torrent_name);

        // Peers
        std::set<std::pair<std::string, std::uint16_t>> peers;
        std::transform (
            entry.peers.begin (), entry.peers.end (),
            std::inserter (peers, peers.end ()),
            [] (const auto &p) { return std::make_pair (p.ip, p.port); }
        );
        lf.peers = vs (lf.peers, peers);

        // Content files
        std::vector<torrent_content_file> content_files;
        std::transform (
            entry.content_files.begin (), entry.content_files.end (),
            std::back_inserter (content_files),
            [] (const auto &file)
            {
                return torrent_content_file {
                    file.name,         file.path,         file.length,
                    file.offset,       file.piece_length, file.piece_offset,
                    file.creation_time
                };
            }
        );
        lf.content_files = vs (lf.content_files, content_files);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Emit sampling_file event
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::emit (
        "sampling_file", std::string ("app.utorrent.resume_dat"), reader
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add settings.dat file
// @param f Settings.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_settings_dat_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto reader = f.new_reader ();
    if (!reader)
        return;

    file_settings_dat settings_dat (reader);
    if (!settings_dat)
        return;

    log.info (__LINE__, "File decoded [settings.dat]: " + f.get_path ());

    _set_folder (f.get_parent ());
    _update_mtime (f);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Create settings object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    settings s;

    s.external_ip = settings_dat.get_external_ip ();
    s.settings_saved_time = settings_dat.get_settings_saved_time ();

    s.metadata.set ("auto_start", settings_dat.get_autostart ());
    s.metadata.set ("bind_port", settings_dat.get_bind_port ());
    s.metadata.set ("cached_host", settings_dat.get_cached_host ());
    s.metadata.set ("computer_id", settings_dat.get_computer_id ());
    s.metadata.set (
        "dir_active_downloads", settings_dat.get_dir_active_downloads ()
    );
    s.metadata.set (
        "dir_completed_downloads", settings_dat.get_dir_completed_downloads ()
    );
    s.metadata.set ("dir_torrent_files", settings_dat.get_dir_torrent_files ());
    s.metadata.set ("execution_count", settings_dat.get_execution_count ());
    s.metadata.set ("exe_path", settings_dat.get_exe_path ());
    s.metadata.set ("external_ip", settings_dat.get_external_ip ());
    s.metadata.set ("fileguard", settings_dat.get_fileguard ());
    s.metadata.set (
        "installation_version", settings_dat.get_installation_version ()
    );
    s.metadata.set ("installation_time", settings_dat.get_installation_time ());
    s.metadata.set ("language", settings_dat.get_language ());
    s.metadata.set (
        "last_bin_change_time", settings_dat.get_last_bin_change_time ()
    );
    s.metadata.set ("last_used_time", settings_dat.get_last_used_time ());
    s.metadata.set ("runtime", settings_dat.get_runtime ());
    s.metadata.set ("save_path", settings_dat.get_save_path ());
    s.metadata.set ("statistics_time", settings_dat.get_statistics_time ());
    s.metadata.set ("ssdp_uuid", settings_dat.get_ssdp_uuid ());
    s.metadata.set (
        "total_bytes_downloaded", settings_dat.get_total_bytes_downloaded ()
    );
    s.metadata.set (
        "total_bytes_uploaded", settings_dat.get_total_bytes_uploaded ()
    );
    s.metadata.set ("version", settings_dat.get_version ());
    s.f = f;

    for (const auto &[k, v] : settings_dat.get_metadata ())
        if (!v.is_map () && !v.is_list ())
            s.metadata.set (k, v);

    settings_.push_back (s);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get data from file
    // @note Always accept the first file found;
    // Prefer non-deleted files over deleted ones;
    // When deletion status is the same, prefer files named exactly
    // "settings.dat"
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (!main_settings_.f ||
        (main_settings_.f.is_deleted () && !f.is_deleted ()) ||
        (main_settings_.f.is_deleted () == f.is_deleted () &&
         main_settings_.f.get_name () != "settings.dat" &&
         f.get_name () == "settings.dat"))
    {
        main_settings_ = s;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Emit sampling_file event
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::emit (
        "sampling_file", std::string ("app.utorrent.settings_dat"), reader
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add torrent file
// @param f Torrent file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_torrent_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto reader = f.new_reader ();
    if (!reader)
        return;

    mobius::core::file_decoder::torrent torrent (reader);
    if (!torrent)
        return;

    log.info (__LINE__, "File decoded [torrent]: " + f.get_path ());

    _update_mtime (f);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add torrent file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    local_file &lf = local_files_[f.get_name ()];

    bool overwrite =
        !lf.torrent_file || (lf.torrent_file.is_deleted () && !f.is_deleted ());
    mobius::core::value_selector vs (overwrite);

    lf.blocksize = vs (lf.blocksize, torrent.get_piece_length ());
    lf.comment = vs (lf.comment, torrent.get_comment ());
    lf.created_by = vs (lf.created_by, torrent.get_created_by ());
    lf.creation_time = vs (lf.creation_time, torrent.get_creation_time ());
    lf.encoding = vs (lf.encoding, torrent.get_encoding ());
    lf.info_hash = vs (lf.info_hash, torrent.get_info_hash ());
    lf.size = vs (lf.size, torrent.get_length ());
    lf.torrent_file = vs (lf.torrent_file, f);
    lf.torrent_name = vs (lf.torrent_name, torrent.get_name ());

    std::vector<torrent_content_file> content_files;
    auto torrent_files = torrent.get_files ();

    std::transform (
        torrent_files.begin (), torrent_files.end (),
        std::back_inserter (content_files),
        [] (const auto &file)
        {
            return torrent_content_file {file.name,         file.path,
                                         file.length,       file.offset,
                                         file.piece_length, file.piece_offset,
                                         file.creation_time};
        }
    );

    lf.content_files = vs (lf.content_files, content_files);
    lf.sources.push_back (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
profile::profile ()
    : impl_ (std::make_shared<impl> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if profile is valid
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
profile::
operator bool () const
{
    return impl_->is_valid ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get username
// @return username
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
profile::get_username () const
{
    return impl_->get_username ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get folder
// @return Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::io::folder
profile::get_folder () const
{
    return impl_->get_folder ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get path to profile
// @return Path to profile
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
profile::get_path () const
{
    return impl_->get_path ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get creation time
// @return Creation time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
profile::get_creation_time () const
{
    return impl_->get_creation_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get last modified time
// @return Last modified time
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::datetime::datetime
profile::get_last_modified_time () const
{
    return impl_->get_last_modified_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get accounts
// @return vector of accounts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<profile::account>
profile::get_accounts () const
{
    return impl_->get_accounts ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get local files
// @return vector of local files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<profile::local_file>
profile::get_local_files () const
{
    return impl_->get_local_files ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of local files
// @return number of local files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::size_t
profile::size_local_files () const
{
    return impl_->size_local_files ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get main settings
// @return settings
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
profile::settings
profile::get_main_settings () const
{
    return impl_->get_main_settings ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get all settings found
// @return settings
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<profile::settings>
profile::get_settings () const
{
    return impl_->get_settings ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add dht.dat file
// @param f dht.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_dht_dat_file (const mobius::core::io::file &f)
{
    impl_->add_dht_dat_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add resume.dat file
// @param f Resume.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_resume_dat_file (const mobius::core::io::file &f)
{
    impl_->add_resume_dat_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add settings.dat file
// @param f Settings.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_settings_dat_file (const mobius::core::io::file &f)
{
    impl_->add_settings_dat_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add torrent file
// @param f Torrent file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_torrent_file (const mobius::core::io::file &f)
{
    impl_->add_torrent_file (f);
}

} // namespace mobius::extension::app::utorrent
