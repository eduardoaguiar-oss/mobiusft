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
#include "evidence_loader_impl.hpp"
#include <mobius/core/datasource/datasource_vfs.hpp>
#include <mobius/core/decoder/inifile.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/io/walker.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/framework/evidence_flag.hpp>
#include <mobius/framework/model/evidence.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string APP_ID = "chromium";
static const std::string APP_NAME = "Chromium";
static const std::string ANT_ID = "evidence.app-chromium";
static const std::string ANT_NAME = APP_NAME;
static const std::string ANT_VERSION = "1.0";

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Join paths
// @param root Root path
// @param rpath Relative path
// @return Joined path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
_join_paths (const std::string &root, const std::string &rpath)
{
    auto path = root;

    if (!rpath.empty ())
    {
        if (!path.empty ())
            path += '/';

        path += rpath;
    }

    if (path.find_first_of ("\\") != std::string::npos)
        path = mobius::core::string::replace (path, "/", "\\");

    return path;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get filename from path
// @param path Path
// @return Filename
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
_get_filename (const std::string &path)
{
    auto filename = path;

    auto pos = filename.find_last_of ("\\");
    if (pos != std::string::npos)
        filename = filename.substr (pos + 1);

    else
    {
        pos = filename.find_last_of ("/");
        if (pos != std::string::npos)
            filename = filename.substr (pos + 1);
    }

    return filename;
}

} // namespace

namespace mobius::extension::app::chromium
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param item Item object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
evidence_loader_impl::evidence_loader_impl (
    const mobius::framework::model::item &item, scan_type type
)
    : item_ (item),
      scan_type_ (type)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan item files for evidences
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::run ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);
    log.info (__LINE__, "Evidence loader <app-" + APP_ID + "> started");
    log.info (__LINE__, "Item UID: " + std::to_string (item_.get_uid ()));
    log.info (
        __LINE__,
        "Scan mode: " + std::to_string (static_cast<int> (scan_type_))
    );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Check if loader has already run for item
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (item_.has_ant (ANT_ID))
    {
        log.info (
            __LINE__,
            "Evidence loader <app-" + APP_ID + "> has already run"
        );
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Check datasource
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto datasource = item_.get_datasource ();

    if (!datasource)
        throw std::runtime_error ("item has no datasource");

    if (datasource.get_type () != "vfs")
        throw std::runtime_error ("datasource type is not VFS");

    if (!datasource.is_available ())
        throw std::runtime_error ("datasource is not available");

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Log starting event
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto transaction = item_.new_transaction ();
    item_.add_event ("app." + APP_ID + " started");
    transaction.commit ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Scan item files, according to scan_type
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    switch (scan_type_)
    {
    case scan_type::canonical_folders:
        _scan_canonical_folders ();
        break;

    case scan_type::all_folders:
        //_scan_all_folders ();
        break;

    default:
        log.warning (
            __LINE__,
            "invalid scan type: " +
                std::to_string (static_cast<int> (scan_type_))
        );
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Save evidences
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    _save_evidences ();

    transaction = item_.new_transaction ();
    item_.add_event ("app." + APP_ID + " ended");
    transaction.commit ();

    log.info (__LINE__, "Evidence loader <app-" + APP_ID + "> ended");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan canonical folders
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_folders ()
{
    auto vfs_datasource =
        mobius::core::datasource::datasource_vfs (item_.get_datasource ());
    auto vfs = vfs_datasource.get_vfs ();

    for (const auto &entry : vfs.get_root_entries ())
    {
        if (entry.is_folder ())
            _scan_canonical_root_folder (entry.get_folder ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan root folder for evidences
// @param folder Root folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_root_folder (
    const mobius::core::io::folder &folder
)
{
    username_ = {};
    auto w = mobius::core::io::walker (folder);

    // Users folders
    for (const auto &f : w.get_folders_by_pattern ("users/*"))
        _scan_canonical_user_folder (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan user folder for evidences
// @param folder User folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_user_folder (
    const mobius::core::io::folder &folder
)
{
    username_ = folder.get_name ();
    auto w = mobius::core::io::walker (folder);

    for (const auto &f : w.get_folders_by_pattern ("appdata/*"))
        _scan_all_folders (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan all subfolders of a folder
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_all_folders (const mobius::core::io::folder &folder)
{
    scan_folder (folder);

    // Scan subfolders
    auto w = mobius::core::io::walker (folder);

    for (const auto &f : w.get_folders ())
        _scan_all_folders (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder searching for Chromium evidences
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::scan_folder (const mobius::core::io::folder &folder)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    profile_ = {};
    profile_.set_username (username_);

    auto w = mobius::core::io::walker (folder);

    for (const auto &[name, f] : w.get_files_with_names ())
    {
        try
        {
            if (name == "cookies")
                ; // profile_.add_settings_dat_file (f);

            else if (name == "extension cookies")
                ; // profile_.add_settings_dat_file (f);

            else if (name == "login data")
                ; // profile_.add_settings_dat_file (f);

            else if (name == "preferences")
                ; // profile_.add_settings_dat_file (f);

            else if (name == "web data")
                profile_.add_web_data_file (f);

            else if (name == "bookmarks")
                ; // profile_.add_settings_dat_file (f);

            else if (name == "history provider cache")
                ; // profile_.add_settings_dat_file (f);

            else if (name == "history")
                ; // profile_.add_settings_dat_file (f);
        }
        catch (const std::exception &e)
        {
            log.warning (
                __LINE__,
                std::string (e.what ()) + " (file: " + f.get_path () + ")"
            );
        }
    }

    if (profile_)
        profiles_.push_back (profile_);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save evidences
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_evidences ()
{
    auto transaction = item_.new_transaction ();

    _save_autofills ();
    _save_credit_cards ();

    item_.set_ant (ANT_ID, ANT_NAME, ANT_VERSION);
    transaction.commit ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save autofill entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_autofills ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &a : p.get_autofill_entries ())
        {
            auto e = item_.new_evidence ("autofill-entry");

            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("username", p.get_username ());
            e.set_attribute ("name", a.name);
            e.set_attribute ("value", a.value);
            e.set_attribute ("metadata", a.metadata);

            e.set_tag ("app.browser");
            e.add_source (a.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save credit cards
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_credit_cards ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &cc : p.get_credit_cards ())
        {
            auto e = item_.new_evidence ("credit-card");
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("username", p.get_username ());
            e.set_attribute ("name", cc.name_on_card);
            e.set_attribute ("number", cc.card_number);
            e.set_attribute ("company", cc.network);

            if (cc.expiration_month && cc.expiration_year)
                e.set_attribute (
                    "expiration_date",
                    std::to_string (cc.expiration_year) + '-' +
                        std::to_string (cc.expiration_month)
                );

            e.set_attribute ("cvv", cc.cvv);

            auto metadata = cc.metadata.clone ();
            metadata.set ("type", cc.type);
            metadata.set ("network", cc.network);
            metadata.set ("bank_name", cc.bank_name);
            metadata.set ("card_issuer", cc.card_issuer);
            metadata.set ("use_count", cc.use_count);
            metadata.set ("use_date", to_string (cc.use_date));
            metadata.set ("nickname", cc.nickname);
            metadata.set ("card_number", cc.card_number);
            metadata.set ("unmasked_date", cc.unmask_date);

            e.set_attribute ("metadata", metadata);

            e.set_tag ("app.browser");
            e.add_source (cc.f);
        }
    }
}

/*
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save accounts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_accounts ()
{
    for (const auto &p : profiles_)
    {
        auto settings = p.get_main_settings ();

        mobius::core::pod::map metadata;
        metadata.set ("app_id", APP_ID);
        metadata.set ("app_name", APP_NAME);
        metadata.set ("network", "BitTorrent");
        metadata.set ("username", p.get_username ());
        metadata.set (
            "total_downloaded_bytes",
            settings.total_bytes_downloaded
        );
        metadata.set ("total_uploaded_bytes", settings.total_bytes_uploaded);
        metadata.set ("execution_count", settings.execution_count);
        metadata.set ("installation_time", settings.installation_time);
        metadata.set ("last_used_time", settings.last_used_time);
        metadata.set ("last_bin_change_time", settings.last_bin_change_time);
        metadata.set ("version", settings.version);
        metadata.set ("installation_version", settings.installation_version);
        metadata.set ("language", settings.language);
        metadata.set ("computer_id", settings.computer_id);
        metadata.set ("auto_start", settings.auto_start ? "yes" : "no");

        for (const auto &account : p.get_accounts ())
        {
            auto e_metadata = metadata.clone ();
            e_metadata.set ("first_dht_timestamp", account.first_dht_timestamp);
            e_metadata.set ("last_dht_timestamp", account.last_dht_timestamp);

            auto e = item_.new_evidence ("user-account");

            e.set_attribute ("account_type", "p2p.bittorrent");
            e.set_attribute ("id", account.client_id);
            e.set_attribute ("password", {});
            e.set_attribute ("password_found", "no");
            e.set_attribute ("is_deleted", account.f.is_deleted ());
            e.set_attribute ("metadata", e_metadata);
            e.set_tag ("p2p");

            for (const auto &f : account.files)
                e.add_source (f);

            e.add_source (settings.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save local files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_local_files ()
{
    for (const auto &profile : profiles_)
    {
        for (const auto &lf : profile.get_local_files ())
        {
            if (!lf.path.empty ())
            {
                auto lf_metadata = _get_metadata (lf);
                lf_metadata.set ("username", profile.get_username ());

                for (const auto &tf : lf.content_files)
                {
                    auto path = _join_paths (lf.path, tf.path);
                    auto filename = _get_filename (path);

                    auto e = item_.new_evidence ("local-file");

                    e.set_attribute ("username", profile.get_username ());
                    e.set_attribute ("filename", filename);
                    e.set_attribute ("path", path);
                    e.set_attribute ("app_id", APP_ID);
                    e.set_attribute ("app_name", APP_NAME);
                    // e.set_attribute ("hashes", get_file_hashes (tf));

                    auto tf_metadata = lf_metadata.clone ();
                    tf_metadata.set ("torrent_path", tf.path);
                    tf_metadata.set ("torrent_offset", tf.offset);
                    tf_metadata.set ("torrent_length", tf.length);
                    tf_metadata.set ("torrent_piece_length", tf.piece_length);
                    tf_metadata.set ("torrent_piece_offset", tf.piece_offset);

                    e.set_attribute ("metadata", tf_metadata);

                    e.set_tag ("p2p");
                    for (const auto &f : lf.sources)
                        e.add_source (f);
                }
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save received files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_received_files ()
{
    for (const auto &profile : profiles_)
    {
        for (const auto &lf : profile.get_local_files ())
        {
            if (lf.bytes_downloaded > 0 || lf.downloaded_seconds > 0)
            {
                auto lf_metadata = _get_metadata (lf);
                lf_metadata.set ("username", profile.get_username ());

                for (const auto &tf : lf.content_files)
                {
                    auto path = _join_paths (lf.path, tf.path);
                    auto filename = _get_filename (path);

                    auto e = item_.new_evidence ("received-file");

                    e.set_attribute ("timestamp", lf.added_time);
                    e.set_attribute ("username", profile.get_username ());
                    e.set_attribute ("filename", filename);
                    e.set_attribute ("path", path);
                    e.set_attribute ("app_id", APP_ID);
                    e.set_attribute ("app_name", APP_NAME);
                    // e.set_attribute ("hashes", get_file_hashes (tf));

                    auto tf_metadata = lf_metadata.clone ();
                    tf_metadata.set ("torrent_path", tf.path);
                    tf_metadata.set ("torrent_offset", tf.offset);
                    tf_metadata.set ("torrent_length", tf.length);
                    tf_metadata.set ("torrent_piece_length", tf.piece_length);
                    tf_metadata.set ("torrent_piece_offset", tf.piece_offset);

                    e.set_attribute ("metadata", tf_metadata);

                    e.set_tag ("p2p");
                    for (const auto &f : lf.sources)
                        e.add_source (f);
                }
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save sent files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_sent_files ()
{
    for (const auto &profile : profiles_)
    {
        for (const auto &lf : profile.get_local_files ())
        {
            if (lf.bytes_uploaded > 0)
            {
                auto lf_metadata = _get_metadata (lf);
                lf_metadata.set ("username", profile.get_username ());

                for (const auto &tf : lf.content_files)
                {
                    auto path = _join_paths (lf.path, tf.path);
                    auto filename = _get_filename (path);

                    auto e = item_.new_evidence ("sent-file");

                    e.set_attribute ("timestamp", lf.added_time);
                    e.set_attribute ("username", profile.get_username ());
                    e.set_attribute ("filename", filename);
                    e.set_attribute ("path", path);
                    e.set_attribute ("app_id", APP_ID);
                    e.set_attribute ("app_name", APP_NAME);
                    // e.set_attribute ("hashes", get_file_hashes (tf));

                    auto tf_metadata = lf_metadata.clone ();
                    tf_metadata.set ("torrent_path", tf.path);
                    tf_metadata.set ("torrent_offset", tf.offset);
                    tf_metadata.set ("torrent_length", tf.length);
                    tf_metadata.set ("torrent_piece_length", tf.piece_length);
                    tf_metadata.set ("torrent_piece_offset", tf.piece_offset);

                    e.set_attribute ("metadata", tf_metadata);

                    e.set_tag ("p2p");
                    for (const auto &f : lf.sources)
                        e.add_source (f);
                }
            }
        }
    }
}

*/

} // namespace mobius::extension::app::chromium
