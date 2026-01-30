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
#include "vfs_processor_impl.hpp"
#include <mobius/core/datasource/datasource_vfs.hpp>
#include <mobius/core/io/walker.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/framework/model/evidence.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string APP_ID = "gecko";
static const std::string APP_NAME = "Gecko";
static const std::string APP_FAMILY = "Gecko";

} // namespace

namespace mobius::extension::app::gecko
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param item Item object
// @param case_profile Case profile object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
vfs_processor_impl::vfs_processor_impl (
    const mobius::framework::model::item &item,
    const mobius::framework::case_profile &
)
    : item_ (item)
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan all subfolders of a folder
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::on_folder (const mobius::core::io::folder &folder)
{
    _scan_profile_folder (folder);
    //_scan_arestra_folder (folder);
    //_scan_ntuser_dat_folder (folder);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Called when processing is complete
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::on_complete ()
{
    _save_app_profiles ();
    _save_autofills ();
    _save_bookmarked_urls ();
    _save_cookies ();
    _save_received_files ();
    _save_searched_texts ();
    _save_visited_urls ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for Gecko based browser profiles
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_scan_profile_folder (
    const mobius::core::io::folder &folder
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Scan folder
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto w = mobius::core::io::walker (folder);
    profile p;

    for (const auto &[name, f] : w.get_files_with_names ())
    {
        try
        {
            if (name == "cookies.sqlite")
                p.add_cookies_sqlite (f);

            else if (name == "downloads.sqlite")
                p.add_downloads_sqlite (f);

            else if (name == "formhistory.sqlite")
                p.add_formhistory_sqlite (f);

            else if (name == "places.sqlite")
                p.add_places_sqlite (f);

            else if (name == "signons.sqlite")
                p.add_signons_sqlite (f);
        }
        catch (const std::exception &e)
        {
            log.warning (
                __LINE__,
                std::string (e.what ()) + " (file: " + f.get_path () + ")"
            );
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // If we have a new profile, add it to the profiles list
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (p)
        profiles_.push_back (p);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save app profiles
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_app_profiles ()
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

        metadata.set ("app_family", APP_FAMILY);
        metadata.set ("profile_id", p.get_profile_id ());
        metadata.set ("bookmarks_count", p.get_bookmarks_count ());
        metadata.set ("cookies_count", p.get_cookies_count ());
        metadata.set ("downloads_count", p.get_downloads_count ());
        metadata.set ("form_history_count", p.get_form_history_count ());
        metadata.set ("visited_urls_count", p.get_visited_urls_count ());

        e.set_attribute ("metadata", metadata);

        // Sources
        e.add_source (p.get_folder ());

        // Tags
        e.set_tag ("app.browser");
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save autofill entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_autofills ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &fh : p.get_form_history ())
        {
            if (fh.fieldname != "searchbar-history")
            {
                // Attributes
                auto e = item_.new_evidence ("autofill");

                e.set_attribute ("field_name", fh.fieldname);
                e.set_attribute ("app_name", p.get_app_name ());
                e.set_attribute ("app_id", p.get_app_id ());
                e.set_attribute ("username", p.get_username ());
                e.set_attribute ("value", fh.value);
                e.set_attribute ("app_family", APP_FAMILY);

                // Metadata
                auto metadata = mobius::core::pod::map ();
                metadata.set ("app_family", APP_FAMILY);
                metadata.set ("profile_id", p.get_profile_id ());
                metadata.update (fh.metadata);
                e.set_attribute ("metadata", metadata);

                // Tags and sources
                e.set_tag ("app.browser");
                e.add_source (fh.f);
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save bookmarked URLs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_bookmarked_urls ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &b : p.get_bookmarks ())
        {
            // Attributes
            auto e = item_.new_evidence ("bookmarked-url");
            e.set_attribute ("url", b.url);
            e.set_attribute ("app_name", p.get_app_name ());
            e.set_attribute ("app_family", APP_FAMILY);
            e.set_attribute ("username", p.get_username ());
            e.set_attribute ("name", b.name);
            e.set_attribute ("creation_time", b.creation_time);
            e.set_attribute ("folder", b.folder);

            // Metadata
            auto metadata = mobius::core::pod::map ();
            metadata.set ("app_family", APP_FAMILY);
            metadata.set ("profile_id", p.get_profile_id ());
            metadata.update (b.metadata);
            e.set_attribute ("metadata", metadata);

            // Tags and sources
            e.set_tag ("app.browser");
            e.add_source (b.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save cookies
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_cookies ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &c : p.get_cookies ())
        {
            // Attributes
            auto e = item_.new_evidence ("cookie");
            e.set_attribute ("app_id", p.get_app_id ());
            e.set_attribute ("app_name", p.get_app_name ());
            e.set_attribute ("app_family", APP_FAMILY);
            e.set_attribute ("username", p.get_username ());
            e.set_attribute ("name", c.name);
            e.set_attribute ("value", c.value);
            e.set_attribute ("domain", c.domain);
            e.set_attribute ("creation_time", c.creation_time);
            e.set_attribute ("last_access_time", c.last_access_time);
            e.set_attribute ("last_update_time", c.last_update_time);
            e.set_attribute ("expiration_time", c.expiration_time);
            e.set_attribute ("is_deleted", c.f.is_deleted ());

            // Metadata
            auto metadata = mobius::core::pod::map ();
            metadata.set ("app_family", APP_FAMILY);
            metadata.set ("profile_id", p.get_profile_id ());
            metadata.update (c.metadata);
            e.set_attribute ("metadata", metadata);

            // Tags and sources
            e.set_tag ("app.browser");
            e.add_source (c.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save received files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_received_files ()
{
    for (const auto &profile : profiles_)
    {
        for (const auto &entry : profile.get_downloads ())
        {
            if (entry.timestamp)
            {
                // Attributes
                auto e = item_.new_evidence ("received-file");
                e.set_attribute ("timestamp", entry.timestamp);
                e.set_attribute ("username", profile.get_username ());
                e.set_attribute ("path", entry.path);
                e.set_attribute ("filename", entry.filename);
                e.set_attribute ("app_id", profile.get_app_id ());
                e.set_attribute ("app_name", profile.get_app_name ());
                e.set_attribute ("app_family", APP_FAMILY);

                // Metadata
                auto metadata = mobius::core::pod::map ();
                metadata.set ("app_family", APP_FAMILY);
                metadata.set ("profile_id", profile.get_profile_id ());
                metadata.update (entry.metadata);
                e.set_attribute ("metadata", metadata);

                // Tags and sources
                e.set_tag ("app.browser");
                e.add_source (entry.f);
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save searched texts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_searched_texts ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &fh : p.get_form_history ())
        {
            if (fh.fieldname == "searchbar-history")
            {
                if (fh.first_used)
                {
                    auto e = item_.new_evidence ("searched-text");

                    e.set_attribute ("timestamp", fh.first_used);
                    e.set_attribute ("search_type", fh.fieldname);
                    e.set_attribute ("text", fh.value);
                    e.set_attribute ("username", p.get_username ());

                    auto metadata = mobius::core::pod::map ();
                    metadata.set ("app_family", APP_FAMILY);
                    metadata.set ("profile_id", p.get_profile_id ());
                    metadata.update (fh.metadata);
                    e.set_attribute ("metadata", metadata);

                    e.set_tag ("app.browser");
                    e.add_source (fh.f);
                }

                if (fh.last_used && fh.last_used != fh.first_used)
                {
                    auto e = item_.new_evidence ("searched-text");

                    e.set_attribute ("timestamp", fh.last_used);
                    e.set_attribute ("search_type", fh.fieldname);
                    e.set_attribute ("text", fh.value);
                    e.set_attribute ("username", p.get_username ());

                    auto metadata = mobius::core::pod::map ();
                    metadata.set ("app_family", APP_FAMILY);
                    metadata.set ("profile_id", p.get_profile_id ());
                    metadata.update (fh.metadata);
                    e.set_attribute ("metadata", metadata);

                    e.set_tag ("app.browser");
                    e.add_source (fh.f);
                }
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save visited URLs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_visited_urls ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &entry : p.get_visited_urls ())
        {
            // Attributes
            auto e = item_.new_evidence ("visited-url");
            e.set_attribute ("username", p.get_username ());
            e.set_attribute ("timestamp", entry.timestamp);
            e.set_attribute ("title", entry.title);
            e.set_attribute ("url", entry.url);
            e.set_attribute ("app_family", APP_FAMILY);

            // Metadata
            auto metadata = mobius::core::pod::map ();
            metadata.set ("app_id", p.get_app_id ());
            metadata.set ("app_name", p.get_app_name ());
            metadata.set ("app_family", APP_FAMILY);
            metadata.set ("profile_id", p.get_profile_id ());
            metadata.update (entry.metadata);

            e.set_attribute ("metadata", metadata);

            // Tags and sources
            e.set_tag ("app.browser");
            e.add_source (entry.f);
        }
    }
}

} // namespace mobius::extension::app::gecko
