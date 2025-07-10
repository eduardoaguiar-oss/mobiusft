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
static const std::string ANT_ID = "evidence.app-chromium";
static const std::string ANT_NAME = "App Chromium";
static const std::string ANT_VERSION = "1.0";

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
    log.info (__LINE__, "Evidence loader <app-chromium> started");
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
        log.info (__LINE__, "Evidence loader <app-chromium> has already run");
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
    item_.add_event ("app.chromium started");
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
    item_.add_event ("app.chromium has ended");
    transaction.commit ();

    log.info (__LINE__, "Evidence loader <app-chromium> has ended");
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
                profile_.add_history_file (f);
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
    {
        profile_.set_folder (folder);
        profiles_.push_back (profile_);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save evidences
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_evidences ()
{
    auto transaction = item_.new_transaction ();

    _save_accounts ();
    _save_app_profiles ();
    _save_autofills ();
    _save_credit_cards ();
    _save_received_files ();
    _save_visited_urls ();

    item_.set_ant (ANT_ID, ANT_NAME, ANT_VERSION);
    transaction.commit ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save accounts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_accounts ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &acc : p.get_accounts ())
        {
            auto e = item_.new_evidence ("user-account");

            // Set attributes
            e.set_attribute ("account_type", "app.chromium");
            e.set_attribute ("id", acc.id);
            e.set_attribute ("password", {});
            e.set_attribute ("password_found", "no");
            e.set_attribute ("is_deleted", acc.f.is_deleted ());

            // Set phones
            e.set_attribute (
                "phones",
                mobius::core::string::join (acc.phone_numbers, "\n")
            );

            // Set emails
            e.set_attribute (
                "emails",
                mobius::core::string::join (acc.emails, "\n")
            );

            // Set organizations
            e.set_attribute (
                "organizations",
                mobius::core::string::join (acc.organizations, "\n")
            );

            // Set addressess
            e.set_attribute (
                "addresses",
                mobius::core::string::join (acc.addresses, "\n")
            );

            // Set names
            e.set_attribute (
                "names",
                mobius::core::string::join (acc.names, "\n")
            );

            // Set metadata
            auto metadata = acc.metadata.clone ();
            metadata.set ("username", p.get_username ());
            metadata.set ("app_name", p.get_app_name ());
            metadata.set ("app_id", p.get_app_id ());
            e.set_attribute ("metadata", metadata);

            // Tags and sources
            e.set_tag ("app.browser");
            e.add_source (acc.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save app profiles
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_app_profiles ()
{
    for (const auto &p : profiles_)
    {
        auto e = item_.new_evidence ("app-profile");

        // Attributes
        e.set_attribute ("app_id", p.get_app_id ());
        e.set_attribute ("app_name", p.get_app_name ());
        e.set_attribute ("username", p.get_username ());
        e.set_attribute ("creation_time", p.get_creation_time ());
        e.set_attribute ("last_modified_time", p.get_last_modified_time ());
        e.set_attribute ("path", p.get_path ());

        // Metadata
        auto metadata = mobius::core::pod::map ();

        metadata.set ("profile_name", p.get_profile_name ());
        metadata.set ("num_accounts", p.size_accounts ());
        metadata.set ("num_autofill_entries", p.size_autofill_entries ());
        metadata.set ("num_credit_cards", p.size_credit_cards ());
        metadata.set ("num_downloads", p.size_downloads ());
        metadata.set ("num_history_entries", p.size_history_entries ());

        e.set_attribute ("metadata", metadata);

        // Tags and sources
        e.set_tag ("app.browser");
        e.add_source (p.get_folder ());
    }
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
            auto e = item_.new_evidence ("autofill");

            e.set_attribute ("field_name", a.name);
            e.set_attribute ("app_name", p.get_app_name ());
            e.set_attribute ("app_id", p.get_app_id ());
            e.set_attribute ("username", p.get_username ());
            e.set_attribute ("is_encrypted", a.is_encrypted);

            if (a.is_encrypted)
            {
                e.set_attribute ("encrypted_value", a.value);
                e.set_attribute ("value", "<ENCRYPTED>");
            }
            else
                e.set_attribute ("value", a.value.to_string ());

            auto metadata = a.metadata.clone ();
            metadata.set ("count", a.count);
            metadata.set ("date_created", a.date_created);
            metadata.set ("date_last_used", a.date_last_used);
            metadata.set ("record_number", a.idx);
            e.set_attribute ("metadata", metadata);

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
            e.set_attribute ("app_id", p.get_app_id ());
            e.set_attribute ("app_name", p.get_app_name ());
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
            metadata.set ("record_number", cc.idx);

            e.set_attribute ("metadata", metadata);

            e.set_tag ("app.browser");
            e.add_source (cc.f);
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
        for (const auto &entry : profile.get_downloads ())
        {
            if (entry.start_time)
            {
                auto e = item_.new_evidence ("received-file");
                auto path = mobius::core::string::first_of (
                    entry.target_path,
                    entry.full_path
                );

                e.set_attribute ("timestamp", entry.start_time);
                e.set_attribute ("username", profile.get_username ());
                e.set_attribute ("path", path);
                e.set_attribute ("filename", _get_filename (path));
                e.set_attribute ("app_id", profile.get_app_id ());
                e.set_attribute ("app_name", profile.get_app_name ());

                auto metadata = mobius::core::pod::map ();
                metadata.set ("start_time", entry.start_time);
                metadata.set ("end_time", entry.end_time);
                metadata.set ("current_path", entry.current_path);
                metadata.set ("full_path", entry.full_path);
                metadata.set ("target_path", entry.target_path);
                metadata.set ("site_url", entry.site_url);
                metadata.set ("tab_url", entry.tab_url);
                metadata.set ("tab_referrer_url", entry.tab_referrer_url);
                metadata.set ("url", entry.url);
                metadata.set ("referrer", entry.referrer);
                metadata.set ("received_bytes", entry.received_bytes);
                metadata.set ("total_bytes", entry.total_bytes);
                metadata.set ("state", entry.state);
                metadata.set ("mime_type", entry.mime_type);
                metadata.set ("original_mime_type", entry.original_mime_type);
                metadata.set ("record_number", entry.idx);
                metadata.set ("id", entry.id);
                metadata.set ("guid", entry.guid);
                metadata.set ("extension_id", entry.by_ext_id);
                metadata.set ("extenstion_name", entry.by_ext_name);
                metadata.set ("web_app_id", entry.by_web_app_id);
                metadata.set ("danger_type", entry.danger_type);
                metadata.set (
                    "embedder_download_data",
                    entry.embedder_download_data
                );
                metadata.set ("etag", entry.etag);
                metadata.set ("hash", entry.hash);
                metadata.set ("http_method", entry.http_method);
                metadata.set ("interrupt_reason", entry.interrupt_reason);
                metadata.set ("last_access_time", entry.last_access_time);
                metadata.set ("last_modified", entry.last_modified);
                metadata.set ("opened", entry.opened);
                metadata.set ("transient", entry.transient);

                e.set_attribute ("metadata", metadata);

                e.set_tag ("p2p");
                e.add_source (entry.f);
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save visited URLs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_visited_urls ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &entry : p.get_history_entries ())
        {
            auto e = item_.new_evidence ("visited-url");
            e.set_attribute ("username", p.get_username ());
            e.set_attribute ("url", entry.url);
            e.set_attribute ("title", entry.title);
            e.set_attribute ("timestamp", entry.visit_time);

            auto metadata = mobius::core::pod::map ();
            metadata.set ("record_number", entry.idx);
            metadata.set ("visit_id", entry.visit_id);
            metadata.set ("app_id", p.get_app_id ());
            metadata.set ("app_name", p.get_app_name ());
            e.set_attribute ("metadata", metadata);

            e.set_tag ("app.browser");
            e.add_source (entry.f);
        }
    }
}

} // namespace mobius::extension::app::chromium
