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
#include <mobius/core/decoder/lnk.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/io/walker.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/framework/utils.hpp>
#include <format>
#include <unordered_map>
#include <set>
#include <string>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
const std::unordered_map<std::string, std::string> SEARCH_ASSIST_FIELDS = {
    {"5001", "internet"},
    {"5603", "filename"},
    {"5604", "file-content"},
    {"5647", "computers"}
};

} // namespace

namespace mobius::extension::os::win
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Profile implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class profile::impl
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl () = default;
    impl (const impl &) noexcept = delete;
    impl (impl &&) noexcept = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl &operator= (const impl &) noexcept = delete;
    impl &operator= (impl &&) noexcept = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if profile is valid
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const noexcept
    {
        return is_valid_;
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
    // @brief Check if profile is deleted
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_deleted () const
    {
        return is_deleted_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if profile is active
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_active () const
    {
        return is_active_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get profile metadata
    // @return Profile metadata
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::pod::map
    get_metadata () const
    {
        return metadata_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get installed programs
    // @return Vector of installed programs
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<installed_program>
    get_installed_programs () const
    {
        return installed_programs_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of installed programs
    // @return Number of installed programs
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::size_t
    get_installed_programs_count () const
    {
        return installed_programs_.size ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get autofill entries
    // @return Vector of autofill entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<autofill>
    get_autofill_entries () const
    {
        return autofill_entries_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of autofill entries
    // @return Number of autofill entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::size_t
    get_autofill_entries_count () const
    {
        return autofill_entries_.size ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get opened files
    // @return Vector of opened files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<opened_file>
    get_opened_files () const
    {
        return opened_files_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of opened files
    // @return Number of opened files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::size_t
    get_opened_files_count () const
    {
        return opened_files_.size ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void add_ntuser_dat_file (const mobius::core::io::file &);
    void add_recent_lnk_file (const mobius::core::io::file &);

  private:
    // @brief Folder object
    mobius::core::io::folder folder_;

    // @brief Username
    std::string username_;

    // @brief Is valid flag
    bool is_valid_ = false;

    // @brief Is deleted flag
    bool is_deleted_ = false;

    // @brief Is active flag
    bool is_active_ = false;

    // @brief Creation time
    mobius::core::datetime::datetime creation_time_;

    // @brief Last modified time
    mobius::core::datetime::datetime last_modified_time_;

    // @brief NTUSER.DAT file
    mobius::core::io::file ntuser_dat_file_;

    // @brief Profile metadata
    mobius::core::pod::map metadata_;

    // @brief Autofill entries
    std::vector<autofill> autofill_entries_;

    // @brief Installed programs
    std::vector<installed_program> installed_programs_;

    // @brief Opened files
    std::vector<opened_file> opened_files_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _set_folder (const mobius::core::io::folder &);
    void _update_mtime (const mobius::core::io::file &);

    void _load_installed_programs (
        const mobius::core::os::win::registry::hive_file &
    );
    void _load_metadata (const mobius::core::os::win::registry::hive_file &);
    void _load_search_assist_entries (
        const mobius::core::os::win::registry::hive_file &
    );
    void _load_wordwheel_queries (
        const mobius::core::os::win::registry::hive_file &
    );
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

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Set data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    folder_ = f;
    last_modified_time_ = f.get_modification_time ();
    creation_time_ = f.get_creation_time ();
    username_ = mobius::framework::get_username_from_path (f.get_path ());

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Emit sampling_folder event
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::emit ("sampling_folder", std::string ("os.win.profiles"), f);
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
// @brief Add NTUSER.DAT file
// @param file NTUSER.DAT file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_ntuser_dat_file (const mobius::core::io::file &f)
{
    // Check if it is a valid NTUSER.DAT file
    if (!f || f.get_size () == 0)
        return;

    if (ntuser_dat_file_ && !ntuser_dat_file_.is_deleted ())
        return;

    // Decode hive file
    mobius::core::os::win::registry::hive_file hive_file (f.new_reader ());

    if (!hive_file.is_instance ())
        return;

    // Clear old artifacts
    installed_programs_.clear ();
    autofill_entries_.clear ();

    // Load artifacts
    _load_metadata (hive_file);
    _load_installed_programs (hive_file);
    _load_search_assist_entries (hive_file);
    _load_wordwheel_queries (hive_file);

    // Set folder based on parent of NTUSER.DAT file
    _set_folder (f.get_parent ());
    _update_mtime (f);

    // Set active flag. Active path = /FSxx/Users/USERNAME/NTUSER.DAT
    if (f && !f.is_deleted ())
    {
        auto path_segments = mobius::core::string::split (f.get_path (), "/");
        is_active_ =
            (path_segments.size () == 5 && path_segments[2] == "Users");
    }

    // Set profile flags
    is_deleted_ = f.is_deleted ();
    is_valid_ = true;

    // Write info to log
    mobius::core::log log (__FILE__, __func__);
    log.info (__LINE__, "File decoded [NTUSER.DAT]: " + f.get_path ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load metadata from NTUSER.DAT hive file
// @param hive_file NTUSER.DAT hive file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_metadata (
    const mobius::core::os::win::registry::hive_file &hive_file
)
{
    mobius::core::log log (__FILE__, __func__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get root key
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto root_key = hive_file.get_root_key ();
        if (!root_key)
            return;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Extract user SID from UserSid value, if it exists
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto user_sid_value = root_key.get_value_by_path (
            "\\Software\\Microsoft\\Windows\\CurrentVersion\\FileAssociations\\"
            "UserSid"
        );

        metadata_.set (
            "user_sid", user_sid_value.get_data_as_string ("utf-16le")
        );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get current locale
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto locale_value = root_key.get_value_by_path (
            "Control Panel\\International\\LocaleName"
        );

        metadata_.set ("locale", locale_value.get_data_as_string ("utf-16le"));

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get current country
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto country_value = root_key.get_value_by_path (
            "Control Panel\\International\\Geo\\Name"
        );

        metadata_.set (
            "country", country_value.get_data_as_string ("utf-16le")
        );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // If LogonStats key exists, extract logon-related metadata
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto logon_stats_key = root_key.get_key_by_path (
            "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\LogonStats"
        );

        if (logon_stats_key)
        {
            auto first_logon_time_value =
                logon_stats_key.get_value_by_name ("FirstLogonTime");
            metadata_.set (
                "first_logon_time",
                first_logon_time_value.get_data_as_datetime ()
            );

            auto first_logon_time_on_current_installation =
                logon_stats_key.get_value_by_name (
                    "FirstLogonTimeOnCurrentInstallation"
                );
            metadata_.set (
                "first_logon_time_on_current_installation",
                first_logon_time_on_current_installation.get_data_as_datetime ()
            );

            auto last_logon_build_number =
                logon_stats_key.get_value_by_name ("LastLogonBuildNumber");
            metadata_.set (
                "last_logon_build_number",
                last_logon_build_number.get_data_as_dword ()
            );

            auto last_logon_build_revision =
                logon_stats_key.get_value_by_name ("LastLogonBuildRevision");
            metadata_.set (
                "last_logon_build_revision",
                last_logon_build_revision.get_data_as_dword ()
            );
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load installed programs from NTUSER.DAT hive file
// @param hive_file NTUSER.DAT hive file
// @see Forensic Analysis of the Windows Registry, by Lih Wern Wong, p.8
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_installed_programs (
    const mobius::core::os::win::registry::hive_file &hive_file
)
{
    mobius::core::log log (__FILE__, __func__);

    try
    {
        auto root_key = hive_file.get_root_key ();
        if (!root_key)
            return;

        auto programs = mobius::extension::os::win::get_installed_programs (
            root_key, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
        );

        std::copy (
            programs.begin (), programs.end (),
            std::back_inserter (installed_programs_)
        );

        programs = mobius::extension::os::win::get_installed_programs (
            root_key, "Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersio"
                      "n\\Uninstall"
        );

        std::copy (
            programs.begin (), programs.end (),
            std::back_inserter (installed_programs_)
        );
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load search assist entries from NTUSER.DAT hive file
// @param hive_file NTUSER.DAT hive file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_search_assist_entries (
    const mobius::core::os::win::registry::hive_file &hive_file
)
{
    mobius::core::log log (__FILE__, __func__);

    try
    {
        auto root_key = hive_file.get_root_key ();
        if (!root_key)
            return;

        for (const auto &key : root_key.get_keys_by_mask (
                 "Software\\Microsoft\\Windows\\CurrentVersion\\Search\\SearchA"
                 "ssistant\\ACMru\\*"
             ))
        {
            auto iter = SEARCH_ASSIST_FIELDS.find (key.get_name ());

            if (iter == SEARCH_ASSIST_FIELDS.end ())
            {
                log.development (
                    __LINE__,
                    std::format (
                        "unhandled search assist key: {}", key.get_name ()
                    )
                );
            }

            else
            {
                for (const auto &value : key.get_values ())
                {
                    auto data = value.get_data ();

                    if (data)
                    {
                        autofill entry;
                        entry.field_name = iter->second;
                        entry.value = data.get_data_as_string ("utf-16le");
                        entry.f = ntuser_dat_file_;

                        entry.metadata.set (
                            "registry_key_path",
                            std::format (
                                "HKCU\\Software\\Microsoft\\Windows\\CurrentVer"
                                "sion\\Search\\SearchAssistant\\ACMru\\{}",
                                key.get_name ()
                            )
                        );
                        entry.metadata.set (
                            "registry_value_name", value.get_name ()
                        );
                        autofill_entries_.push_back (entry);
                    }
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Load WordWheel autocomplete entries from NTUSER.DAT hive file
// @param hive_file NTUSER.DAT hive file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_load_wordwheel_queries (
    const mobius::core::os::win::registry::hive_file &hive_file
)
{
    mobius::core::log log (__FILE__, __func__);

    try
    {
        // Get root key
        auto root_key = hive_file.get_root_key ();
        if (!root_key)
            return;

        // Get WordWheelQuery key, if any
        auto ww_key = root_key.get_key_by_path (
            "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\WordWheelQ"
            "uery"
        );
        if (!ww_key)
            return;

        // Iterate through MRUListEx values (each value represents a search query)
        for (const auto &[idx, data] : get_mrulistex (ww_key))
        {
            autofill entry;
            entry.field_name = "search_query";
            entry.value = data.get_data_as_string ("utf-16le");
            entry.f = ntuser_dat_file_;

            entry.metadata.set (
                "registry_key_path", "Software\\Microsoft\\Windows\\Current"
                                     "Version\\Explorer\\WordWheelQuery"
            );
            entry.metadata.set ("mru_index", idx);

            autofill_entries_.push_back (entry);
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add recent file
// @param file Recent file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_recent_lnk_file (const mobius::core::io::file &f)
{
    // Get reader
    if (!f || f.is_deleted () || f.get_size () < 0x4c)
        return;

    auto reader = f.new_reader ();
    if (!reader)
        return;

    // Decode file as LNK file
    auto lnkfile = mobius::core::decoder::lnk (f.new_reader ());
    auto local_base_path = lnkfile.get_local_base_path ();

    if (local_base_path.empty ())
        return;

    // If file's creation time is different, we have two separated events
    std::set<mobius::core::datetime::datetime> timestamps;

    auto creation_time = f.get_creation_time ();

    if (creation_time)
        timestamps.insert (creation_time);

    auto modification_time = f.get_modification_time ();

    if (modification_time)
        timestamps.insert (modification_time);

    // Create an opened_file entry for each different timestamp
    for (const auto &timestamp : timestamps)
    {
        opened_file entry;
        entry.timestamp = timestamp;
        entry.path = local_base_path;
        entry.f = f;

        // If the LNK file has a common path suffix, append it to the path.
        // This is because some LNK files (e.g. from Quick Access) only store the filename in the local
        auto common_path_suffix = lnkfile.get_common_path_suffix ();

        if (!common_path_suffix.empty ())
        {
            if (!entry.path.ends_with ('\\'))
                entry.path += '\\';
            entry.path += common_path_suffix;
        }

        // Set metadata
        entry.metadata.set (
            "lnk-path", mobius::core::io::to_win_path (f.get_path ())
        );
        entry.metadata.set (
            "target-creation-time", lnkfile.get_creation_time ()
        );
        entry.metadata.set ("target-access-time", lnkfile.get_access_time ());
        entry.metadata.set ("target-write-time", lnkfile.get_write_time ());
        entry.metadata.set ("target-size", lnkfile.get_file_size ());
        entry.metadata.set ("netbios-name", lnkfile.get_netbios_name ());
        entry.metadata.set (
            "drive-serial-number",
            std::format ("0x{:08x}", lnkfile.get_drive_serial_number ())
        );
        entry.metadata.set ("relative-path", lnkfile.get_relative_path ());

        // Add entry to opened files
        opened_files_.push_back (entry);
    }

    mobius::core::log log (__FILE__, __func__);
    log.info (__LINE__, "File decoded [Recent .lnk]: " + f.get_path ());
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
operator bool () const noexcept
{
    return impl_->operator bool ();
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
// @brief Check if profile is deleted
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
profile::is_deleted () const
{
    return impl_->is_deleted ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if profile is active
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
profile::is_active () const
{
    return impl_->is_active ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get profile metadata
// @return Profile metadata
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::pod::map
profile::get_metadata () const
{
    return impl_->get_metadata ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get installed programs
// @return Vector of installed programs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<installed_program>
profile::get_installed_programs () const
{
    return impl_->get_installed_programs ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of installed programs
// @return Number of installed programs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::size_t
profile::get_installed_programs_count () const
{
    return impl_->get_installed_programs_count ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get autofill entries
// @return Vector of autofill entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<profile::autofill>
profile::get_autofill_entries () const
{
    return impl_->get_autofill_entries ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of autofill entries
// @return Number of autofill entries
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::size_t
profile::get_autofill_entries_count () const
{
    return impl_->get_autofill_entries_count ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get opened files
// @return Vector of opened files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<profile::opened_file>
profile::get_opened_files () const
{
    return impl_->get_opened_files ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of opened files
// @return Number of opened files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::size_t
profile::get_opened_files_count () const
{
    return impl_->get_opened_files_count ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add NTUSER.DAT file
// @param file NTUSER.DAT file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_ntuser_dat_file (const mobius::core::io::file &f)
{
    impl_->add_ntuser_dat_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add recent file
// @param file Recent file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_recent_lnk_file (const mobius::core::io::file &f)
{
    impl_->add_recent_lnk_file (f);
}

} // namespace mobius::extension::os::win
