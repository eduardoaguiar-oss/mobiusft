// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// along with this program. If not, see <https://www.gnu.org/licenses/>.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <mobius/core/datasource/datasource_vfs.hpp>
#include <mobius/core/io/walker.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/resource.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/framework/ant/ant_impl_vfs_processor.hpp>
#include <mobius/framework/ant/vfs_processor.hpp>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string VERSION = "1.0.0";
static constexpr bool DEBUG = true;

} // namespace

namespace mobius::framework::ant
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ant_impl_vfs_processor::ant_impl_vfs_processor (
    const mobius::framework::model::item &item,
    const mobius::framework::case_profile &case_profile
)
    : item_ (item),
      case_profile_ (case_profile),
      progress_ (0.0),
      processed_folders_ (0),
      processed_files_ (0)
{
    flag_all_folders_ = case_profile_.get_processor_scope () == "all";

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Build vfs-processor implementations
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &processor_id : case_profile_.get_processors ())
    {
        auto data = mobius::framework::ant::get_vfs_processor_implementation (
            processor_id
        );

        if (data)
            implementations_.emplace_back (
                data->factory (item_, case_profile_)
            );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get version
// @return Version
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
ant_impl_vfs_processor::get_version () const
{
    return VERSION;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get current progress
// @return Current progress
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
double
ant_impl_vfs_processor::get_progress () const
{
    std::lock_guard<std::mutex> lock (status_mutex_);

    return progress_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get current status
// @return Current status
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::map
ant_impl_vfs_processor::get_status () const
{
    std::lock_guard<std::mutex> lock (status_mutex_);

    mobius::core::pod::map status = {
        {"processed_folders", processed_folders_.load ()},
        {"processed_files", processed_files_.load ()},
        {"current_folder", current_folder_path_},
        {"vfs_processors_count", implementations_.size ()},
    };
    return status;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Start execution
// @note This method is thread-safe and can be called concurrently.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
ant_impl_vfs_processor::run ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);
    processed_folders_.store (0);
    processed_files_.store (0);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get VFS object from item
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto datasource = item_.get_datasource ();

    if (!datasource)
        throw std::runtime_error ("No datasource available");

    if (datasource.get_type () != "vfs")
        throw std::runtime_error ("Datasource is not VFS");

    mobius::core::datasource::datasource_vfs d_vfs (datasource);
    auto vfs = d_vfs.get_vfs ();

    if (!vfs.is_available ())
        throw std::runtime_error ("VFS is not available");

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get root entries. If "all folders" flag is set, process all folders.
    // Otherwise, process only "home", "users" and "documents and settings"
    // folders.
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    log.info (__LINE__, "Starting VFS processing");

    for (const auto &entry : vfs.get_root_entries ())
    {
        if (entry.is_folder ())
        {
            auto folder = entry.get_folder ();

            if (flag_all_folders_)
                _process_folder (folder);

            else
                _scan_user_folders (folder);
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Notify implementations that processing is done
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    log.info (__LINE__, "Saving evidences");

    auto transaction = item_.new_transaction ();

    for (const auto &impl : implementations_)
    {
        try
        {
            impl->on_complete ();
        }
        catch (const std::exception &e)
        {
            mobius::core::log log (__FILE__, __FUNCTION__);
            log.warning (__LINE__, e.what ());
        }
    }

    transaction.commit ();

    log.info (__LINE__, "VFS processing completed");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan user folders
// @param folder Root folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
ant_impl_vfs_processor::_scan_user_folders (
    const mobius::core::io::folder &folder
)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        mobius::core::io::walker w (folder);

        for (const auto &[name, child] : w.get_folders_with_names ())
        {
            if (name == "home" || name == "users" ||
                name == "documents and settings")
                _process_folder (child);

            else if (name == "windows.old")
            {
                mobius::core::io::walker wo_walker (child);

                for (const auto &user_folder :
                     wo_walker.get_folders_by_name ("users"))
                    _process_folder (user_folder);
            }
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Process folder
// @param folder Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
ant_impl_vfs_processor::_process_folder (const mobius::core::io::folder &folder)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    {
        processed_folders_.fetch_add (1);

        std::lock_guard<std::mutex> lock (status_mutex_);
        current_folder_path_ = folder.get_path ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Notify implementations
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &impl : implementations_)
    {
        try
        {
            impl->on_folder (folder);
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, e.what ());
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Process children
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    try
    {
        for (const auto &entry : folder.get_children ())
        {
            if (entry.is_folder ())
                _process_folder (entry.get_folder ());

            else
                processed_files_.fetch_add (1);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::framework::ant
