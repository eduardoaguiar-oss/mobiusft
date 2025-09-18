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
#include <mobius/core/decoder/json/parser.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/io/uri.hpp>
#include <mobius/core/io/walker.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/framework/evidence_flag.hpp>
#include <mobius/framework/model/evidence.hpp>
#include <iomanip>
#include <sstream>
#include "common.hpp"
#include "file_local_state.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// - Chromium folder structure:
//
// @see
// https://chromium.googlesource.com/chromium/src/+/HEAD/docs/user_data_dir.md
//
//  - Local State: File containing global settings and state, including v10 and
//  v20 encrypted keys
//  - Profiles: Each user profile has its own folder, typically named "Profile
//  X" or "Default"
//      - Bookmarks: File containing the user's bookmarks
//      - Cookies: File containing cookies for the profile
//      - History: File containing the browsing history
//      - Login Data: File containing saved passwords and login information
//      - Preferences: File containing user preferences and settings
//      - Web Data: File containing autofill data and other web-related
//      information
//      - Network: Folder containing network-related data, such as DNS cache and
//      protocol handlers
//           - Cookies: File containing cookies for the profile
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string ANT_ID = "evidence.app-chromium";
static const std::string ANT_NAME = "App Chromium";
static const std::string ANT_VERSION = "1.2";
static const std::string SAMPLING_ID = "sampling";
static const std::string APP_FAMILY = "chromium";
static const std::string APP_NAME = "Chromium";
static const std::string APP_ID = "chromium";

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get filename from path
// @param path Path
// @return Filename
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
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
    // Scan for evidences
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    _scan_canonical_folders ();
    _save_evidences ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Log ending event
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
    _scan_local_state (folder);
    _scan_profile (folder);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for Local State files
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_local_state (const mobius::core::io::folder &folder)
{
    auto w = mobius::core::io::walker (folder);

    for (const auto &f : w.get_files_by_name ("local state"))
        _decode_local_state_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode Local State file
// @param f Local State file to decode
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_local_state_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_local_state fls (f.new_reader ());

        if (!fls)
        {
            log.info (__LINE__, "File is not a valid 'Local State' file");
            return;
        }

        log.info (
            __LINE__, "File " + f.get_path () + " is a valid 'Local State' file"
        );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add encryption keys
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &ek : fls.get_encryption_keys ())
        {
            encryption_key ekey (ek);
            ekey.f = f;

            encryption_keys_.push_back (ekey);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for Chromium profiles
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_profile (const mobius::core::io::folder &folder)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // ==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Reset profile if we are starting a new folder scan
    // ==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (profile_ && !mobius::core::string::startswith (
                        folder.get_path (), profile_.get_path ()
                    ))
    {
        profile_ = {};
    }

    bool is_new = !bool (profile_);

    // ==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Scan folder
    // ==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto w = mobius::core::io::walker (folder);

    for (const auto &[name, f] : w.get_files_with_names ())
    {
        try
        {
            if (name == "bookmarks")
                profile_.add_bookmarks_file (f);

            else if (name == "cookies")
                profile_.add_cookies_file (f);

            else if (name == "extension cookies")
                profile_.add_cookies_file (f);

            else if (name == "login data")
                profile_.add_login_data_file (f);

            else if (name == "login data for account")
                profile_.add_login_data_file (f);

            else if (name == "history")
                profile_.add_history_file (f);

            else if (name == "preferences")
                profile_.add_preferences_file (f);

            else if (name == "safe browsing cookies")
                profile_.add_cookies_file (f);

            else if (name == "web data")
                profile_.add_web_data_file (f);
        }
        catch (const std::exception &e)
        {
            log.warning (
                __LINE__,
                std::string (e.what ()) + " (file: " + f.get_path () + ")"
            );
        }
    }

    // ==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // If we have a new profile, add it to the profiles list
    // ==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (profile_ && is_new)
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

    _save_app_profiles ();
    _save_autofills ();
    _save_bookmarked_urls ();
    _save_cookies ();
    _save_credit_cards ();
    _save_encryption_keys ();
    _save_passwords ();
    _save_pdis ();
    _save_received_files ();
    _save_user_accounts ();
    _save_visited_urls ();

    item_.set_ant (ANT_ID, ANT_NAME, ANT_VERSION);
    transaction.commit ();
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
        e.set_attribute ("app_family", APP_FAMILY);

        // Metadata
        auto metadata = mobius::core::pod::map ();

        metadata.set ("profile_name", p.get_profile_name ());
        metadata.set ("last_engagement_time", p.get_last_engagement_time ());
        metadata.set ("created_by_version", p.get_created_by_version ());
        metadata.set ("num_accounts", p.size_accounts ());
        metadata.set ("num_autofill_entries", p.size_autofill_entries ());
        metadata.set ("num_autofill_profiles", p.size_autofill_profiles ());
        metadata.set ("num_bookmarks", p.size_bookmarks ());
        metadata.set ("num_cookies", p.size_cookies ());
        metadata.set ("num_credit_cards", p.size_credit_cards ());
        metadata.set ("num_downloads", p.size_downloads ());
        metadata.set ("num_history_entries", p.size_history_entries ());
        metadata.set ("num_logins", p.size_logins ());

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
            e.set_attribute ("encrypted_value", a.encrypted_value);
            e.set_attribute ("value", a.value);
            e.set_attribute ("app_family", APP_FAMILY);

            auto metadata = a.metadata.clone ();

            metadata.set ("count", a.count);
            metadata.set ("date_created", a.date_created);
            metadata.set ("date_last_used", a.date_last_used);
            metadata.set ("record_number", a.idx);
            metadata.set ("schema_version", a.schema_version);
            metadata.set ("profile_name", p.get_profile_name ());
            e.set_attribute ("metadata", metadata);

            e.set_tag ("app.browser");
            e.add_source (a.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save bookmarked URLs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void
evidence_loader_impl::_save_bookmarked_urls ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &b : p.get_bookmarks ())
        {
            auto e = item_.new_evidence ("bookmarked-url");
            e.set_attribute ("url", b.url);
            e.set_attribute ("app_name", p.get_app_name ());
            e.set_attribute ("app_family", APP_FAMILY);
            e.set_attribute ("username", p.get_username ());
            e.set_attribute ("name", b.name);
            e.set_attribute ("creation_time", b.creation_time);
            e.set_attribute ("last_modified_time", b.last_modified_time);
            e.set_attribute ("last_used_time", b.last_used_time);
            e.set_attribute ("folder", b.folder_name);

            auto metadata = mobius::core::pod::map ();
            metadata.set ("id", b.id);
            metadata.set ("guid", b.guid);
            metadata.set ("app_id", p.get_app_id ());
            e.set_attribute ("metadata", metadata);

            e.set_tag ("app.browser");
            e.add_source (b.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save cookies
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_cookies ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &c : p.get_cookies ())
        {
            auto e = item_.new_evidence ("cookie");
            e.set_attribute ("app_id", p.get_app_id ());
            e.set_attribute ("app_name", p.get_app_name ());
            e.set_attribute ("app_family", APP_FAMILY);
            e.set_attribute ("username", p.get_username ());
            e.set_attribute ("name", c.name);
            e.set_attribute ("value", c.value);
            e.set_attribute ("value_encrypted", c.encrypted_value);
            e.set_attribute ("value_is_encrypted", !c.value && c.encrypted_value);
            e.set_attribute ("domain", c.host_key);
            e.set_attribute ("creation_time", c.creation_utc);
            e.set_attribute ("last_access_time", c.last_access_utc);
            e.set_attribute ("last_update_time", c.last_update_utc);
            e.set_attribute ("expiration_time", c.expires_utc);
            e.set_attribute ("is_deleted", c.f.is_deleted ());
            e.set_attribute ("is_encrypted", !c.value && c.encrypted_value);

            auto metadata = mobius::core::pod::map ();
            metadata.set ("record_idx", c.idx);
            metadata.set ("schema_version", c.schema_version);
            metadata.set ("has_cross_site_ancestor", c.has_cross_site_ancestor);
            metadata.set ("has_expires", c.has_expires);
            metadata.set ("is_httponly", c.is_httponly);
            metadata.set ("is_persistent", c.is_persistent);
            metadata.set ("is_same_party", c.is_same_party);
            metadata.set ("is_secure", c.is_secure);
            metadata.set ("path", c.path);
            metadata.set ("persistent", c.persistent);
            metadata.set ("priority", c.priority);
            metadata.set ("same_site", c.samesite);
            metadata.set ("secure", c.secure);
            metadata.set ("source_port", c.source_port);
            metadata.set ("source_scheme", c.source_scheme);
            metadata.set ("source_type", c.source_type);
            metadata.set ("top_frame_site_key", c.top_frame_site_key);
            e.set_attribute ("metadata", metadata);

            e.set_tag ("app.browser");
            e.add_source (c.f);
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
            e.set_attribute ("encrypted_name", cc.name_on_card_encrypted);
            e.set_attribute ("number", cc.card_number);
            e.set_attribute ("encrypted_number", cc.card_number_encrypted);
            e.set_attribute ("company", cc.network);
            e.set_attribute ("app_family", APP_FAMILY);

            if (cc.expiration_month && cc.expiration_year)
                e.set_attribute (
                    "expiration_date", std::to_string (cc.expiration_year) +
                                           '-' +
                                           std::to_string (cc.expiration_month)
                );

            auto metadata = mobius::core::pod::map ();
            metadata.set ("id", cc.id);
            metadata.set ("guid", cc.guid);
            metadata.set ("expiration_month", cc.expiration_month);
            metadata.set ("expiration_year", cc.expiration_year);
            metadata.set ("origin", cc.origin);
            metadata.set ("iban", cc.iban);
            metadata.set ("cvc", cc.cvc);
            metadata.set ("last_four", cc.last_four);
            metadata.set ("card_art_url", cc.card_art_url);
            metadata.set (
                "card_info_retrieval_enrollment_state",
                cc.card_info_retrieval_enrollment_state
            );
            metadata.set ("type", cc.type);
            metadata.set ("network", cc.network);
            metadata.set ("bank_name", cc.bank_name);
            metadata.set ("card_issuer", cc.card_issuer);
            metadata.set ("use_count", cc.use_count);
            metadata.set ("use_date", to_string (cc.use_date));
            metadata.set ("unmasked_date", cc.unmask_date);
            metadata.set ("date_modified", to_string (cc.date_modified));
            metadata.set ("nickname", cc.nickname);
            metadata.set ("record_number", cc.idx);

            e.set_attribute ("metadata", metadata);

            e.set_tag ("app.browser");
            e.add_source (cc.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save encryption keys
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_encryption_keys ()
{
    for (const auto &ek : encryption_keys_)
    {
        const auto [app_id, app_name] = get_app_from_path (ek.f.get_path ());
        const auto username = get_username_from_path (ek.f.get_path ());

        // create evidence
        auto e = item_.new_evidence ("encryption-key");

        e.set_attribute ("key_type", "chromium." + ek.type);
        e.set_attribute ("id", ek.id);
        e.set_attribute ("app_family", APP_FAMILY);
        e.set_attribute ("encrypted_value", ek.value);

        // value is empty, as key is not decrypted yet
        e.set_attribute ("value", mobius::core::bytearray {});

        // metadata
        auto metadata = ek.metadata.clone ();
        metadata.set ("app_id", app_id);
        metadata.set ("app_name", app_name);
        metadata.set ("app_family", APP_FAMILY);
        metadata.set ("username", username);
        e.set_attribute ("metadata", metadata);

        // tag and source
        e.set_tag ("app.browser");
        e.add_source (ek.f);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save passwords
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_passwords ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &login : p.get_logins ())
        {
            auto uri = mobius::core::io::uri (login.origin_url);
            auto domain = uri.get_host ();

            // Set attributes
            auto e = item_.new_evidence ("password");
            e.set_attribute ("password_type", "net.http/" + domain);
            e.set_attribute ("value", mobius::core::bytearray {});
            e.set_attribute ("value_encrypted", login.password_value);
            e.set_attribute ("value_is_encrypted", true);
            e.set_attribute (
                "description", "Web password. URL: " + login.origin_url
            );
            e.set_attribute ("app_family", APP_FAMILY);

            // Set metadata
            auto metadata = mobius::core::pod::map ();
            metadata.set ("username", p.get_username ());
            metadata.set ("app_name", p.get_app_name ());
            metadata.set ("app_id", p.get_app_id ());
            metadata.set ("record_idx", login.idx);
            metadata.set ("action_url", login.action_url);
            metadata.set ("avatar_url", login.avatar_url);
            metadata.set ("blacklisted_by_user", login.blacklisted_by_user);
            metadata.set ("date_created", login.date_created);
            metadata.set ("date_last_used", login.date_last_used);
            metadata.set (
                "date_password_modified", login.date_password_modified
            );
            metadata.set ("date_received", login.date_received);
            metadata.set ("date_synced", login.date_synced);
            metadata.set ("display_name", login.display_name);
            metadata.set ("federation_url", login.federation_url);
            metadata.set (
                "generation_upload_status", login.generation_upload_status
            );
            metadata.set ("icon_url", login.icon_url);
            metadata.set ("is_zero_click", login.is_zero_click);
            metadata.set ("keychain_identifier", login.keychain_identifier);
            metadata.set ("origin_url", login.origin_url);
            metadata.set ("password_element", login.password_element);
            metadata.set ("password_type", login.password_type);
            metadata.set ("preferred", login.preferred);
            metadata.set ("scheme", login.scheme);
            metadata.set ("sender_email", login.sender_email);
            metadata.set ("sender_name", login.sender_name);
            metadata.set (
                "sender_profile_image_url", login.sender_profile_image_url
            );
            metadata.set (
                "sharing_notification_displayed",
                login.sharing_notification_displayed
            );
            metadata.set ("signon_realm", login.signon_realm);
            metadata.set ("skip_zero_click", login.skip_zero_click);
            metadata.set ("ssl_valid", login.ssl_valid);
            metadata.set ("submit_element", login.submit_element);
            metadata.set ("times_used", login.times_used);
            metadata.set ("use_additional_auth", login.use_additional_auth);
            metadata.set ("username_element", login.username_element);
            metadata.set ("username_value", login.username_value);
            e.set_attribute ("metadata", metadata);

            e.set_tag ("app.browser");
            e.add_source (login.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save PDI entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_pdis ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &ap : p.get_autofill_profiles ())
        {
            auto ap_metadata = mobius::core::pod::map ();
            ap_metadata.set ("app_id", p.get_app_id ());
            ap_metadata.set ("app_name", p.get_app_name ());
            ap_metadata.set ("username", p.get_username ());
            ap_metadata.set ("autofill_profile_guid", ap.guid);
            ap_metadata.set ("autofill_profile_in_trash", ap.is_in_trash);

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Add e-mails
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            for (const auto &email : ap.emails)
            {
                if (!email.empty ())
                {
                    auto e = item_.new_evidence ("pdi");
                    e.set_attribute ("pdi_type", "email");
                    e.set_attribute ("value", email);
                    e.set_attribute ("app_family", APP_FAMILY);
                    e.set_attribute ("metadata", ap_metadata);

                    e.set_tag ("app.browser");
                    e.add_source (ap.f);
                }
            }

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Add phones
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            for (const auto &phone : ap.phones)
            {
                auto value = phone.number;
                if (!phone.type.empty ())
                    value += " (" + phone.type + ")";

                if (!value.empty ())
                {
                    auto e = item_.new_evidence ("pdi");
                    e.set_attribute ("pdi_type", "phone");
                    e.set_attribute ("value", value);
                    e.set_attribute ("app_family", APP_FAMILY);

                    auto metadata = ap_metadata.clone ();
                    metadata.set ("phone_number", phone.number);
                    metadata.set ("phone_type", phone.type);
                    e.set_attribute ("metadata", metadata);

                    e.set_tag ("app.browser");
                    e.add_source (ap.f);
                }
            }

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Add addresses
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            for (const auto &address : ap.addresses)
            {
                std::string value;

                if (!address.street_address.empty ())
                    value += address.street_address;
                else if (!address.address_line_1.empty ())
                {
                    value += address.address_line_1;
                    if (!address.address_line_2.empty ())
                        value += " " + address.address_line_2;
                }
                if (!address.city.empty ())
                    value += ", " + address.city;
                if (!address.state.empty ())
                    value += "/" + address.state;
                if (!address.zip_code.empty ())
                    value += ", " + address.zip_code;
                if (!address.country.empty ())
                    value += ", " + address.country;

                if (!value.empty ())
                {
                    auto e = item_.new_evidence ("pdi");
                    e.set_attribute ("pdi_type", "address");
                    e.set_attribute ("value", value);
                    e.set_attribute ("app_family", APP_FAMILY);
                    e.set_attribute ("metadata", ap_metadata);

                    e.set_tag ("app.browser");
                    e.add_source (ap.f);
                }
            }

            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            // Add names
            // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            for (const auto &name : ap.names)
            {
                std::string value;

                if (!name.full_name.empty ())
                    value = name.full_name;

                else if (!name.first_name.empty ())
                {
                    value = name.first_name;
                    if (!name.last_name.empty ())
                        value += " " + name.last_name;
                }

                if (!value.empty ())
                {
                    auto e = item_.new_evidence ("pdi");
                    e.set_attribute ("pdi_type", "fullname");
                    e.set_attribute ("value", value);
                    e.set_attribute ("app_family", APP_FAMILY);
                    e.set_attribute ("metadata", ap_metadata);

                    e.set_tag ("app.browser");
                    e.add_source (ap.f);
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
        for (const auto &entry : profile.get_downloads ())
        {
            if (entry.start_time)
            {
                auto e = item_.new_evidence ("received-file");
                auto path = mobius::core::string::first_of (
                    entry.target_path, entry.full_path
                );

                e.set_attribute ("timestamp", entry.start_time);
                e.set_attribute ("username", profile.get_username ());
                e.set_attribute ("path", path);
                e.set_attribute ("filename", _get_filename (path));
                e.set_attribute ("app_id", profile.get_app_id ());
                e.set_attribute ("app_name", profile.get_app_name ());
                e.set_attribute ("app_family", APP_FAMILY);

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
                    "embedder_download_data", entry.embedder_download_data
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
// @brief Save accounts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_user_accounts ()
{
    for (const auto &p : profiles_)
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Save accounts from Preferences
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &acc : p.get_accounts ())
        {
            auto e = item_.new_evidence ("user-account");

            // Set attributes
            e.set_attribute ("account_type", "app.chromium");
            e.set_attribute ("id", acc.id);
            e.set_attribute ("password", mobius::core::bytearray {});
            e.set_attribute ("password_found", false);
            e.set_attribute ("is_deleted", acc.f.is_deleted ());
            e.set_attribute ("app_family", APP_FAMILY);
            e.set_attribute ("phones", acc.phone_numbers);
            e.set_attribute ("emails", acc.emails);
            e.set_attribute ("organizations", acc.organizations);
            e.set_attribute ("addresses", acc.addresses);
            e.set_attribute ("names", acc.names);

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

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Save accounts from logins
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &login : p.get_logins ())
        {
            auto uri = mobius::core::io::uri (login.origin_url);
            auto domain = uri.get_host ();

            auto e = item_.new_evidence ("user-account");

            // Set attributes
            e.set_attribute ("account_type", "net.http/" + domain);
            e.set_attribute ("id", login.username_value);
            e.set_attribute ("password", mobius::core::bytearray {});
            e.set_attribute ("password_encrypted", login.password_value);
            e.set_attribute ("password_is_encrypted", true);
            e.set_attribute ("password_found", true);
            e.set_attribute ("is_deleted", login.f.is_deleted ());
            e.set_attribute ("app_family", APP_FAMILY);

            // Set metadata
            auto metadata = mobius::core::pod::map ();
            metadata.set ("username", p.get_username ());
            metadata.set ("app_name", p.get_app_name ());
            metadata.set ("app_id", p.get_app_id ());
            metadata.set ("record_idx", login.idx);
            metadata.set ("schema_version", login.schema_version);
            metadata.set ("action_url", login.action_url);
            metadata.set ("avatar_url", login.avatar_url);
            metadata.set ("blacklisted_by_user", login.blacklisted_by_user);
            metadata.set ("date_created", login.date_created);
            metadata.set ("date_last_used", login.date_last_used);
            metadata.set (
                "date_password_modified", login.date_password_modified
            );
            metadata.set ("date_received", login.date_received);
            metadata.set ("date_synced", login.date_synced);
            metadata.set ("display_name", login.display_name);
            metadata.set ("federation_url", login.federation_url);
            metadata.set (
                "generation_upload_status", login.generation_upload_status
            );
            metadata.set ("icon_url", login.icon_url);
            metadata.set ("is_zero_click", login.is_zero_click);
            metadata.set ("keychain_identifier", login.keychain_identifier);
            metadata.set ("origin_url", login.origin_url);
            metadata.set ("password_element", login.password_element);
            metadata.set ("password_type", login.password_type);
            metadata.set ("preferred", login.preferred);
            metadata.set ("scheme", login.scheme);
            metadata.set ("sender_email", login.sender_email);
            metadata.set ("sender_name", login.sender_name);
            metadata.set (
                "sender_profile_image_url", login.sender_profile_image_url
            );
            metadata.set (
                "sharing_notification_displayed",
                login.sharing_notification_displayed
            );
            metadata.set ("signon_realm", login.signon_realm);
            metadata.set ("skip_zero_click", login.skip_zero_click);
            metadata.set ("ssl_valid", login.ssl_valid);
            metadata.set ("submit_element", login.submit_element);
            metadata.set ("times_used", login.times_used);
            metadata.set ("use_additional_auth", login.use_additional_auth);
            metadata.set ("username_element", login.username_element);
            metadata.set ("username_value", login.username_value);
            e.set_attribute ("metadata", metadata);

            // Tags and sources
            e.set_tag ("app.browser");
            e.add_source (login.f);
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
            e.set_attribute ("app_family", APP_FAMILY);

            auto metadata = mobius::core::pod::map ();

            metadata.set ("app_id", p.get_app_id ());
            metadata.set ("app_name", p.get_app_name ());
            metadata.set ("row_number", entry.idx);
            metadata.set ("schema_version", entry.schema_version);
            metadata.set ("id", entry.id);
            metadata.set ("visit_id", entry.visit_id);
            metadata.set ("from_visit", entry.from_visit);
            metadata.set ("favicon_id", entry.favicon_id);
            metadata.set ("hidden", entry.hidden);
            metadata.set ("last_visit_time", entry.last_visit_time);
            metadata.set ("typed_count", entry.typed_count);
            metadata.set ("visit_count", entry.visit_count);
            metadata.set ("visit_time", entry.visit_time);
            metadata.set (
                "consider_for_ntp_most_visited",
                entry.consider_for_ntp_most_visited
            );
            metadata.set ("external_referrer_url", entry.external_referrer_url);
            metadata.set (
                "incremented_omnibox_typed_score",
                entry.incremented_omnibox_typed_score
            );
            metadata.set ("is_indexed", entry.is_indexed);
            metadata.set ("is_known_to_sync", entry.is_known_to_sync);
            metadata.set ("opener_visit", entry.opener_visit);
            metadata.set ("originator_cache_guid", entry.originator_cache_guid);
            metadata.set ("originator_from_visit", entry.originator_from_visit);
            metadata.set (
                "originator_opener_visit", entry.originator_opener_visit
            );
            metadata.set ("originator_visit_id", entry.originator_visit_id);
            metadata.set ("publicly_routable", entry.publicly_routable);
            metadata.set ("segment_id", entry.segment_id);
            metadata.set (
                "visit_duration", duration_to_string (entry.visit_duration)
            );
            metadata.set ("visit_url", entry.visit_url);
            metadata.set ("visited_link_id", entry.visited_link_id);
            e.set_attribute ("metadata", metadata);

            e.set_tag ("app.browser");
            e.add_source (entry.f);
        }
    }
}

} // namespace mobius::extension::app::chromium
