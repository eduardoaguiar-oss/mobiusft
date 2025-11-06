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
#include "vfs_processor_impl.hpp"
#include <mobius/core/datasource/datasource_vfs.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/io/uri.hpp>
#include <mobius/core/io/walker.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/framework/model/evidence.hpp>
#include <string>
#include <vector>
#include "profile.hpp"

#include <iostream>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
//    - https://bebinary4n6.blogspot.com/2019/07/
//    - https://arxiv.org/pdf/1603.05369.pdf
//    - https://answers.microsoft.com/en-us/skype/forum/all/where-is-the-maindb-file-for-new-skype/b4d3f263-a97e-496e-aa28-e1dbb63e7687
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string APP_ID = "skype";
static const std::string APP_NAME = "Skype";

} // namespace

namespace mobius::extension::app::skype
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Skype <i>vfs_processor</i> implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class vfs_processor_impl::impl
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl (
        const mobius::framework::model::item &,
        const mobius::framework::case_profile &
    );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Function prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void on_folder (const mobius::core::io::folder &);
    void on_complete ();

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
    void _save_contacts ();
    void _save_received_files ();
    void _save_sent_files ();
    void _save_user_accounts ();
    void _save_voicemails ();
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param item Item object
// @param case_profile Case profile object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
vfs_processor_impl::impl::impl (
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
vfs_processor_impl::impl::on_folder (const mobius::core::io::folder &folder)
{
    _scan_profile_folder (folder);
    _scan_s4l_files (folder);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Called when processing is complete
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::impl::on_complete ()
{
    auto transaction = item_.new_transaction ();

    _save_app_profiles ();
    _save_contacts ();
    _save_received_files ();
    _save_sent_files ();
    _save_user_accounts ();
    _save_voicemails ();

    transaction.commit ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for Skype profiles
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::impl::_scan_profile_folder (
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
            if (name == "main.db")
                p.add_main_db_file (f);

            else if (name == "skype.db")
                p.add_skype_db_file (f);
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
    {
        profiles_.push_back (p);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for s4l-xxx.db files
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::impl::_scan_s4l_files (
    const mobius::core::io::folder &folder
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Scan folder
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    try
    {
        auto w = mobius::core::io::walker (folder);

        for (const auto &f : w.get_files_by_pattern ("s4l-*.db"))
            _decode_s4l_file (f);
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__,
            std::string (e.what ()) + " (folder: " + folder.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode s4l-xxx.db file
// @param f s4l-xxx.db file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::impl::_decode_s4l_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        profile p;
        p.add_s4l_db_file (f);

        if (p)
            profiles_.push_back (p);
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save app profiles
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::impl::_save_app_profiles ()
{
    for (const auto &p : profiles_)
    {
        auto e = item_.new_evidence ("app-profile");

        // Attributes
        e.set_attribute ("app_id", APP_ID);
        e.set_attribute ("app_name", APP_NAME);
        e.set_attribute ("username", p.get_username ());
        e.set_attribute ("creation_time", p.get_creation_time ());
        e.set_attribute ("last_modified_time", p.get_last_modified_time ());
        e.set_attribute ("path", p.get_path ());

        // Metadata
        auto metadata = mobius::core::pod::map ();
        metadata.set ("skype_id", p.get_account_id ());
        metadata.set ("skype_name", p.get_account_name ());
        metadata.set ("num_accounts", p.size_accounts ());
        metadata.set ("num_contacts", p.size_contacts ());
        metadata.set ("num_file_transfers", p.size_file_transfers ());
        metadata.set ("num_voicemails", p.size_voicemails ());

        e.set_attribute ("metadata", metadata);

        // Tags and sources
        e.set_tag ("app.chat");
        // e.add_source (p.get_folder ()); // @todo add folder or file as source
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save contacts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::impl::_save_contacts ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &c : p.get_contacts ())
        {
            auto e = item_.new_evidence ("contact");
            e.set_attribute ("id", c.id);
            e.set_attribute ("name", c.name);
            e.set_attribute ("accounts", c.accounts);
            e.set_attribute ("addresses", c.addresses);
            e.set_attribute ("birthday", c.birthday);
            e.set_attribute ("emails", c.emails);
            e.set_attribute ("names", c.names);
            e.set_attribute ("notes", c.notes);
            e.set_attribute ("organizations", c.organizations);
            e.set_attribute ("phone_numbers", c.phone_numbers);
            e.set_attribute ("web_addresses", c.web_addresses);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("username", p.get_username ());

            // Set metadata
            auto metadata = mobius::core::pod::map ();
            metadata.set ("skype_id", p.get_account_id ());
            metadata.set ("skype_name", p.get_account_name ());
            metadata.update (c.metadata);

            e.set_attribute ("metadata", metadata);

            // Tags and sources
            e.set_tag ("app.chat");
            e.add_source (c.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save received files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::impl::_save_received_files ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &ft : p.get_file_transfers ())
        {
            if (ft.type == 1)
            {
                auto e = item_.new_evidence ("received-file");

                // Set attributes
                e.set_attribute ("timestamp", ft.timestamp);
                e.set_attribute ("filename", ft.filename);
                e.set_attribute ("path", ft.path);
                e.set_attribute ("app_id", APP_ID);
                e.set_attribute ("app_name", APP_NAME);
                e.set_attribute ("username", p.get_username ());

                // Set metadata
                auto metadata = mobius::core::pod::map ();
                metadata.set ("skype_id", p.get_account_id ());
                metadata.set ("skype_name", p.get_account_name ());
                metadata.update (ft.metadata);
                e.set_attribute ("metadata", metadata);

                // Tags and sources
                e.set_tag ("app.chat");
                e.add_source (ft.f);
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save sent files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::impl::_save_sent_files ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &ft : p.get_file_transfers ())
        {
            if (ft.type == 2)
            {
                auto e = item_.new_evidence ("sent-file");

                // Set attributes
                e.set_attribute ("timestamp", ft.timestamp);
                e.set_attribute ("filename", ft.filename);
                e.set_attribute ("path", ft.path);
                e.set_attribute ("app_id", APP_ID);
                e.set_attribute ("app_name", APP_NAME);
                e.set_attribute ("username", p.get_username ());

                // Set metadata
                auto metadata = mobius::core::pod::map ();
                metadata.set ("skype_id", p.get_account_id ());
                metadata.set ("skype_name", p.get_account_name ());
                metadata.update (ft.metadata);
                e.set_attribute ("metadata", metadata);

                // Tags and sources
                e.set_tag ("app.chat");
                e.add_source (ft.f);
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save accounts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::impl::_save_user_accounts ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &acc : p.get_accounts ())
        {
            auto e = item_.new_evidence ("user-account");

            // Set attributes
            e.set_attribute ("account_type", "app.skype");
            e.set_attribute ("id", acc.id);
            e.set_attribute ("password", mobius::core::bytearray {});
            e.set_attribute ("password_found", false);
            e.set_attribute ("is_deleted", acc.f.is_deleted ());
            e.set_attribute ("phones", acc.phone_numbers);
            e.set_attribute ("emails", acc.emails);
            e.set_attribute ("organizations", acc.organizations);
            e.set_attribute ("addresses", acc.addresses);
            e.set_attribute ("names", acc.names);

            // Set metadata
            auto metadata = acc.metadata.clone ();
            metadata.set ("username", p.get_username ());
            metadata.set ("skype_id", p.get_account_id ());
            metadata.set ("skype_name", p.get_account_name ());
            metadata.set ("app_id", APP_ID);
            metadata.set ("app_name", APP_NAME);
            e.set_attribute ("metadata", metadata);

            // Tags and sources
            e.set_tag ("app.chat");
            e.add_source (acc.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save voicemails
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::impl::_save_voicemails ()
{
    for (const auto &p : profiles_)
    {
        for (const auto &vm : p.get_voicemails ())
        {
            auto e = item_.new_evidence ("voicemail");

            // Set attributes
            e.set_attribute ("timestamp", vm.timestamp);
            e.set_attribute ("duration", vm.duration);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("username", p.get_username ());

            // Set metadata
            auto metadata = mobius::core::pod::map ();
            metadata.set ("skype_id", p.get_account_id ());
            metadata.set ("skype_name", p.get_account_name ());
            metadata.update (vm.metadata);
            e.set_attribute ("metadata", metadata);

            // Tags and sources
            e.set_tag ("app.chat");
            e.add_source (vm.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param item Item object
// @param case_profile Case profile object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
vfs_processor_impl::vfs_processor_impl (
    const mobius::framework::model::item &item,
    const mobius::framework::case_profile &case_profile
)
    : impl_ (std::make_shared<impl> (item, case_profile))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan all subfolders of a folder
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::on_folder (const mobius::core::io::folder &folder)
{
    impl_->on_folder (folder);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Called when processing is complete
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::on_complete ()
{
    impl_->on_complete ();
}

} // namespace mobius::extension::app::skype
