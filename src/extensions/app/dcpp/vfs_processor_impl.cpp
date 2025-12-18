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
#include <mobius/core/io/path.hpp>
#include <mobius/core/io/uri.hpp>
#include <mobius/core/io/walker.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/framework/evidence_flag.hpp>
#include <mobius/framework/model/evidence.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References: DCPlusPlus-0.883 source code
//
// DC++ main files (* decoded by MobiusFT):
//
// . ADLSearch.xml: Auto-download search configuration file
// * DCPlusPlus.xml: profile configuration file
// . Queue.xml: File transfer queue
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string APP_ID = "dcpp";
static const std::string APP_NAME = "DC++";

} // namespace

namespace mobius::extension::app::dcpp
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
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Called when processing is complete
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::on_complete ()
{
    auto transaction = item_.new_transaction ();

    _save_app_profiles ();
    _save_ip_addresses ();
    _save_user_accounts ();

    transaction.commit ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for DC++ profiles
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
            if (name == "dcplusplus.xml")
                p.add_dcplusplus_xml_file (f);

            else if (name == "queue.xml")
                p.add_queue_xml_file (f);

            else if (name == "adlsearch.xml")
                p.add_adlsearch_xml_file (f);
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
        e.set_attribute ("app_id", APP_ID);
        e.set_attribute ("app_name", APP_NAME);
        e.set_attribute ("username", p.get_username ());
        e.set_attribute ("creation_time", p.get_creation_time ());
        e.set_attribute ("last_modified_time", p.get_last_modified_time ());
        e.set_attribute ("path", p.get_path ());

        // Metadata
        auto metadata = mobius::core::pod::map ();
        metadata.set ("cid", p.get_cid ());
        metadata.set ("config_version", p.get_config_version ());
        metadata.set ("external_ip", p.get_external_ip ());
        metadata.set ("nickname", p.get_nickname ());
        metadata.set ("network", "Direct Connect");
        metadata.set (
            "total_downloaded",
            static_cast<std::int64_t> (p.get_total_downloaded ())
        );
        metadata.set (
            "total_uploaded",
            static_cast<std::int64_t> (p.get_total_uploaded ())
        );
        e.set_attribute ("metadata", metadata);

        // Tags and sources
        e.set_tag ("app.p2p");
        e.add_source (p.get_folder ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save IP addresses
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_ip_addresses ()
{
    for (const auto &p : profiles_)
    {
        auto external_ip = p.get_external_ip ();
        auto external_ip_time = p.get_external_ip_time ();

        if (!external_ip.empty () && external_ip_time)
        {
            // Create evidence
            auto e = item_.new_evidence ("ip-address");

            e.set_attribute ("timestamp", external_ip_time);
            e.set_attribute ("address", external_ip);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("username", p.get_username ());

            // Metadata
            mobius::core::pod::map metadata;
            metadata.set ("cid", p.get_cid ());
            metadata.set ("config_version", p.get_config_version ());
            metadata.set ("nickname", p.get_nickname ());
            metadata.set ("network", "Direct Connect");
            metadata.set (
                "total_downloaded",
                static_cast<std::int64_t> (p.get_total_downloaded ())
            );
            metadata.set (
                "total_uploaded",
                static_cast<std::int64_t> (p.get_total_uploaded ())
            );
            e.set_attribute ("metadata", metadata);

            // Set tags and source files
            e.set_tag ("app.p2p");

            for (const auto &sf : p.get_source_files ())
                e.add_source (sf);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save user accounts
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_save_user_accounts ()
{
    for (const auto &p : profiles_)
    {
        auto cid = p.get_cid ();

        if (!cid.empty ())
        {
            // Create evidence
            auto e = item_.new_evidence ("user-account");
            e.set_attribute ("account_type", "p2p.direct-connect");
            e.set_attribute ("id", cid);
            e.set_attribute ("name", p.get_nickname ());
            e.set_attribute ("password", {});
            e.set_attribute ("password_found", "no");

            // Metadata
            mobius::core::pod::map metadata;
            metadata.set ("app_id", APP_ID);
            metadata.set ("app_name", APP_NAME);
            metadata.set ("username", p.get_username ());
            metadata.set ("network", "Direct Connect");
            metadata.set ("cid", cid);
            metadata.set ("config_version", p.get_config_version ());
            metadata.set ("external_ip", p.get_external_ip ());
            metadata.set ("nickname", p.get_nickname ());
            metadata.set (
                "total_downloaded",
                static_cast<std::int64_t> (p.get_total_downloaded ())
            );
            metadata.set (
                "total_uploaded",
                static_cast<std::int64_t> (p.get_total_uploaded ())
            );
            e.set_attribute ("metadata", metadata);

            // Set tags and sources
            e.set_tag ("app.p2p");

            for (const auto &sf : p.get_source_files ())
                e.add_source (sf);
        }
    }
}

} // namespace mobius::extension::app::dcpp
