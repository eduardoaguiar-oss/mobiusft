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
#include <mobius/core/datasource/datasource.hpp>
#include <mobius/core/datasource/datasource_vfs.hpp>
#include <mobius/core/io/walker.hpp>
#include <mobius/core/log.hpp>
#include <mobius/framework/ant/vfs_processor.hpp> // @deprecated, remove when vfs_processor is removed
#include <mobius/framework/evidence_processor/engine.hpp>
#include <mobius/framework/evidence_processor/evidence_processor_registry.hpp>
#include <mobius/framework/evidence_processor/mediator.hpp>
#include <atomic>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <string>

namespace mobius::framework::evidence_processor
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>engine</i> implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class engine::impl
{
  public:
    enum class mode
    {
        none,
        run,
        update
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl (const impl &) = delete;
    impl (impl &&) = delete;
    impl (const mobius::framework::model::item &, const std::string &);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl &operator= (const impl &) = delete;
    impl &operator= (impl &&) = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void run ();
    void update ();
    mobius::framework::model::item get_item () const;
    profile get_profile () const;
    mobius::core::pod::map get_status () const;

  private:
    // @brief Case item
    mobius::framework::model::item item_;

    // @brief Case profile
    profile profile_;

    // @brief Mediator
    mediator mediator_;

    // @brief Processor implementations
    std::vector<std::shared_ptr<evidence_processor_impl_base>> implementations_;

    // @deprecated VFS processor implementations
    std::vector<
        std::shared_ptr<mobius::framework::ant::vfs_processor_impl_base>>
        vfs_implementations_;

    // @brief Current mode
    mode current_mode_ = mode::none;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Processing status
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Started processing time
    mobius::core::datetime::datetime started_time_;

    // @brief Finished processing time
    mobius::core::datetime::datetime finished_time_;

    // @brief Total number of folders processed
    std::atomic<size_t> processed_folders_ = 0;

    // @brief Total number of files processed
    std::atomic<size_t> processed_files_ = 0;

    // @brief Current folder path
    std::string current_folder_path_;

    // @brief Mutex for status updates
    mutable std::mutex status_mutex_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _on_start ();
    void _on_load_evidences ();
    void _on_run_vfs ();
    void _on_process_folder (const mobius::core::io::folder &);
    void _on_complete ();
    void _on_stop ();
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param item Case item
// @param profile_id Profile ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
engine::impl::impl (
    const mobius::framework::model::item &item, const std::string &profile_id
)
    : item_ (item),
      profile_ (mobius::framework::evidence_processor::profile (profile_id))
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Build processor implementations
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &processor_id : profile_.get_processors ())
    {
        auto data = get_evidence_processor_implementation (processor_id);

        if (data)
        {
            try
            {
                implementations_.emplace_back (
                    data->factory (item_, profile_, mediator_)
                );

                log.info (
                    __LINE__,
                    "evidence_processor implementation: " + processor_id
                );
            }
            catch (const std::exception &e)
            {
                log.warning (
                    __LINE__,
                    "Failed to create evidence_processor implementation for "
                    "processor '" +
                        processor_id + "': " + e.what ()
                );
            }
        }

        else
        {
            // @begin-deprecated vfs_processor support, remove when all vfs_processor_impl
            // are migrated to evidence_processor_impl
            auto vfs_data =
                mobius::framework::ant::get_vfs_processor_implementation (
                    processor_id
                );

            if (vfs_data)
            {
                try
                {
                    mobius::framework::case_profile case_profile (profile_id);

                    vfs_implementations_.emplace_back (
                        vfs_data->factory (item_, case_profile)
                    );

                    log.info (
                        __LINE__,
                        "evidence_processor implementation (deprecated): " + processor_id
                    );
                }
                catch (const std::exception &e)
                {
                    log.warning (
                        __LINE__,
                        "Failed to create VFS processor implementation for "
                        "processor '" +
                            processor_id + "': " + e.what ()
                    );
                }
            }

            // @end-deprecated
            else
            {
                log.warning (
                    __LINE__, "No evidence_processor implementation found for "
                              "processor '" +
                                  processor_id + "'"
                );
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Run processor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
engine::impl::run ()
{
    current_mode_ = mode::run;
    started_time_ = mobius::core::datetime::now ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get datasource from item
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto datasource = item_.get_datasource ();

    if (!datasource)
        throw std::runtime_error ("Item has no datasource");

    if (!datasource.is_available ())
        throw std::runtime_error ("Datasource is not available");

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Run
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    _on_start ();
    _on_run_vfs ();
    _on_complete ();
    _on_stop ();

    finished_time_ = mobius::core::datetime::now ();
    current_mode_ = mode::none;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update processing
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
engine::impl::update ()
{
    current_mode_ = mode::update;
    started_time_ = mobius::core::datetime::now ();

    _on_start ();
    _on_load_evidences ();
    _on_complete ();
    _on_stop ();

    finished_time_ = mobius::core::datetime::now ();
    current_mode_ = mode::none;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get case item
// @return Case item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::framework::model::item
engine::impl::get_item () const
{
    return item_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get case profile
// @return Case profile
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
profile
engine::impl::get_profile () const
{
    return profile_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get current status
// @return Current status
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::map
engine::impl::get_status () const
{
    // Start with basic status information
    mobius::core::pod::map status = {
        {"profile", profile_.get_id ()},
        {"evidence_processors",
         implementations_.size () + vfs_implementations_.size ()},
        {"started_time", started_time_},
    };

    // Add finished time if processing is complete, otherwise add current time
    if (finished_time_)
        status.set ("finished_time", finished_time_);

    else
        status.set ("current_time", mobius::core::datetime::now ());

    // Mode run status
    if (current_mode_ == mode::run)
    {
        status.set ("processed_folders", processed_folders_.load ());
        status.set ("processed_files", processed_files_.load ());

        std::lock_guard<std::mutex> lock (status_mutex_);

        if (!current_folder_path_.empty ())
            status.set ("current_folder", current_folder_path_);
    }

    // Aggregate status from implementations
    for (const auto &impl : implementations_)
    {
        try
        {
            status.update (impl->get_status ());
        }
        catch (const std::exception &e)
        {
            mobius::core::log log (__FILE__, __FUNCTION__);
            log.warning (__LINE__, e.what ());
        }
    }

    return status;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Call on_start for all implementations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
engine::impl::_on_start ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    for (const auto &impl : implementations_)
    {
        try
        {
            impl->on_start ();
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, e.what ());
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Call on_stop for all implementations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
engine::impl::_on_stop ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    for (const auto &impl : implementations_)
    {
        try
        {
            impl->on_stop ();
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, e.what ());
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Call on_complete for all implementations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
engine::impl::_on_complete ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    auto transaction = item_.new_transaction ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @deprecated Notify VFS implementations that processing is complete
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &impl : vfs_implementations_)
    {
        try
        {
            impl->on_complete ();
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, e.what ());
        }
    }

    _on_load_evidences ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Notify implementations that processing is complete
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &impl : implementations_)
    {
        try
        {
            impl->on_complete ();
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, e.what ());
        }
    }

    transaction.commit ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load evidences already processed and feed them back into the
// processor, to feed events to implementations.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
engine::impl::_on_load_evidences ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    for (auto &e : item_.get_evidences ())
    {
        for (const auto &impl : implementations_)
        {
            try
            {
                impl->on_evidence_created (e);
            }
            catch (const std::exception &e)
            {
                log.warning (__LINE__, e.what ());
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Run VFS processor implementation
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
engine::impl::_on_run_vfs ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get datasource from item
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto datasource = item_.get_datasource ();

    if (!datasource)
        throw std::runtime_error ("Item has no datasource");

    if (!datasource.is_available ())
        throw std::runtime_error ("Datasource is not available");

    if (datasource.get_type () != "vfs")
    {
        log.info (__LINE__, "Datasource is not VFS, skipping VFS processing");
        return;
    }

    mobius::core::datasource::datasource_vfs d_vfs (datasource);
    auto vfs = d_vfs.get_vfs ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get root entries. If "all folders" flag is set, process all folders.
    // Otherwise, process only "home", "users" and "documents and settings"
    // folders.
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    log.info (__LINE__, "Starting VFS processing");
    processed_folders_.store (0);
    processed_files_.store (0);

    auto flag_all_folders = profile_.get_processor_scope () == "all";

    for (const auto &entry : vfs.get_root_entries ())
    {
        if (entry.is_folder ())
        {
            auto folder = entry.get_folder ();

            if (flag_all_folders)
                _on_process_folder (folder);

            else
            {
                mobius::core::io::walker w (folder);

                for (const auto &[name, child] : w.get_folders_with_names ())
                {
                    if (name == "home" || name == "users" ||
                        name == "documents and settings")
                        _on_process_folder (child);

                    else if (name == "windows.old")
                    {
                        mobius::core::io::walker wo_walker (child);

                        for (const auto &user_folder :
                             wo_walker.get_folders_by_name ("users"))
                            _on_process_folder (user_folder);
                    }
                }
            }
        }
    }

    log.info (__LINE__, "Finished VFS processing");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Process folder
// @param folder Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
engine::impl::_on_process_folder (const mobius::core::io::folder &folder)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    {
        processed_folders_.fetch_add (1);

        std::lock_guard<std::mutex> lock (status_mutex_);
        current_folder_path_ = folder.get_path ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Notify implementations that we're entering a folder
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &impl : implementations_)
    {
        try
        {
            impl->on_folder_entered (folder);
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, e.what ());
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @deprecated Notify VFS implementations that we're entering a folder
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &impl : vfs_implementations_)
    {
        try
        {
            impl->on_folder_enter (folder);
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
                _on_process_folder (entry.get_folder ());

            else
                processed_files_.fetch_add (1);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Notify implementations that we're exiting a folder
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &impl : implementations_)
    {
        try
        {
            impl->on_folder_exited (folder);
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, e.what ());
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @deprecated Notify VFS implementations that we're exiting a folder
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &impl : vfs_implementations_)
    {
        try
        {
            impl->on_folder_exit (folder);
        }
        catch (const std::exception &e)
        {
            log.warning (__LINE__, e.what ());
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Clear folder children cache to free memory
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto folder_proxy = folder;
    folder_proxy.reload ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param item Case item
// @param profile_id Profile ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
engine::engine (
    const mobius::framework::model::item &item, const std::string &profile_id
)
    : impl_ (std::make_shared<impl> (item, profile_id))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Run processor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
engine::run ()
{
    impl_->run ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update processing
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
engine::update ()
{
    impl_->update ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get case item
// @return Case item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::framework::model::item
engine::get_item () const
{
    return impl_->get_item ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get profile
// @return Profile
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
profile
engine::get_profile () const
{
    return impl_->get_profile ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get current status
// @return Current status
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::map
engine::get_status () const
{
    return impl_->get_status ();
}

} // namespace mobius::framework::evidence_processor
