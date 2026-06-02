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
#include <mobius/framework/processor/processor.hpp>
#include <atomic>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <string>

namespace mobius::framework::processor
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief <i>processor</i> implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class processor::impl
{
  public:
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
    std::vector<std::shared_ptr<processor_impl_base>> implementations_;

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
processor::impl::impl (
    const mobius::framework::model::item &item, const std::string &profile_id
)
    : item_ (item),
      profile_ (mobius::framework::processor::profile (profile_id))
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Build processor implementations
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &processor_id : profile_.get_processors ())
    {
        auto data = get_processor_implementation (processor_id);

        if (data)
        {
            try
            {
                implementations_.emplace_back (
                    data->factory (item_, profile_, mediator_)
                );
            }
            catch (const std::exception &e)
            {
                log.warning (
                    __LINE__, "Failed to create processor implementation for "
                              "processor '" +
                                  processor_id + "': " + e.what ()
                );
            }
        }

        else
        {
            log.warning (
                __LINE__, "No processor implementation found for processor '" +
                              processor_id + "'"
            );
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Run processor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
processor::impl::run ()
{
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
    _on_load_evidences ();
    _on_run_vfs ();
    _on_complete ();
    _on_stop ();

    finished_time_ = mobius::core::datetime::now ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update processing
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
processor::impl::update ()
{
    started_time_ = mobius::core::datetime::now ();

    _on_start ();
    _on_load_evidences ();
    _on_complete ();
    _on_stop ();

    finished_time_ = mobius::core::datetime::now ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get case item
// @return Case item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::framework::model::item
processor::impl::get_item () const
{
    return item_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get case profile
// @return Case profile
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
profile
processor::impl::get_profile () const
{
    return profile_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get current status
// @return Current status
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::map
processor::impl::get_status () const
{
    // Start with basic status information
    mobius::core::pod::map status = {
        {"profile", profile_.get_id ()},
        {"subprocessors", implementations_.size ()},
        {"started_time", started_time_},
    };

    if (finished_time_)
        status.set ("finished_time", finished_time_);

    else
        status.set ("current_time", mobius::core::datetime::now ());

    // Add current folder path if available
    {
        std::lock_guard<std::mutex> lock (status_mutex_);

        if (!current_folder_path_.empty ())
            status.set ("current_folder", current_folder_path_);
    }

    // Add total processed folders and files
    status.set ("processed_folders", processed_folders_.load ());
    status.set ("processed_files", processed_files_.load ());

    // Aggregate status from all implementations
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
processor::impl::_on_start ()
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
processor::impl::_on_stop ()
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
processor::impl::_on_complete ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    auto transaction = item_.new_transaction ();

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
processor::impl::_on_load_evidences ()
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
processor::impl::_on_run_vfs ()
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
processor::impl::_on_process_folder (const mobius::core::io::folder &folder)
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
processor::processor (
    const mobius::framework::model::item &item, const std::string &profile_id
)
    : impl_ (std::make_shared<impl> (item, profile_id))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Run processor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
processor::run ()
{
    impl_->run ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update processing
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
processor::update ()
{
    impl_->update ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get case item
// @return Case item
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::framework::model::item
processor::get_item () const
{
    return impl_->get_item ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get profile
// @return Profile
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
profile
processor::get_profile () const
{
    return impl_->get_profile ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get current status
// @return Current status
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::map
processor::get_status () const
{
    return impl_->get_status ();
}

namespace
{
// @brief Map to hold processor implementation data
static std::unordered_map<std::string, processor_implementation_data> data;

// @brief Mutex to protect access to the factories map
static std::mutex mutex;

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Register a processor
// @param id Unique identifier for the processor
// @param factory Function to create an instance of the processor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
register_processor_implementation (
    const std::string &id,
    const std::string &name,
    mobius::framework::processor::processor_implementation_builder factory
)
{
    processor_implementation_data data_entry;
    data_entry.id = id;
    data_entry.name = name;
    data_entry.factory = factory;

    std::lock_guard<std::mutex> lock (mutex);
    data[id] = data_entry;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unregister a processor
// @param id Unique identifier for the processor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
unregister_processor_implementation (const std::string &id)
{
    std::lock_guard<std::mutex> lock (mutex);
    data.erase (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get processor implementation data by ID
// @param id Unique identifier for the processor
// @return Optional containing the processor implementation data if found, or
//         std::nullopt if not found
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::optional<processor_implementation_data>
get_processor_implementation (const std::string &id)
{
    std::lock_guard<std::mutex> lock (mutex);

    auto iter = data.find (id);

    if (iter != data.end ())
        return iter->second;

    return std::nullopt;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get all registered processor implementations
// @return Vector of pairs containing the ID and name of each processor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<mobius::framework::processor::processor_implementation_data>
list_processor_implementations ()
{
    std::lock_guard<std::mutex> lock (mutex);
    std::vector<mobius::framework::processor::processor_implementation_data>
        implementations (data.size ());

    std::transform (
        data.begin (), data.end (), implementations.begin (),
        [] (const auto &pair) { return pair.second; }
    );

    return implementations;
}

} // namespace mobius::framework::processor
