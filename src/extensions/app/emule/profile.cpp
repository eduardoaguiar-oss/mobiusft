// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
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
#include "profile.hpp"
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/decoder/inifile.hpp>
#include <mobius/core/io/line_reader.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/core/value_selector.hpp>
#include <string>
#include "common.hpp"
#include "file_key_index_dat.hpp"
#include "file_known_met.hpp"
#include "file_stored_searches_met.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get username from path
// @param path Path to profile
// @return Username extracted from path
//
// @note Paths are in the following format: /FSxx/Users/username/... or
// /FSxx/home/username/... where FSxx is the filesystem identifier.
// Example: /FS01/Users/johndoe/AppData/Local/Google/Chrome/User Data/
// In this case, the username is "johndoe".
// If the path does not match the expected format, an empty string is returned.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_username_from_path (const std::string &path)
{
    auto dirnames = mobius::core::string::split (path, "/");

    if (dirnames.size () > 3 &&
        (dirnames[2] == "Users" || dirnames[2] == "home"))
        return dirnames[3]; // Username is the fourth directory

    return {}; // No username found
}

} // namespace

namespace mobius::extension::app::emule
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set folder
// @param f Folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::_set_folder (const mobius::core::io::folder &f)
{
    if (folder_ || !f)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Set data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    folder_ = f;
    last_modified_time_ = f.get_modification_time ();
    creation_time_ = f.get_creation_time ();
    username_ = get_username_from_path (f.get_path ());
    std::tie (app_id_, app_name_) = get_app_from_path (f.get_path ());

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Emit sampling_folder event
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::emit (
        "sampling_folder", std::string ("app.emule.profiles"), f
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update last modified time based on file
// @param f File
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::_update_mtime (const mobius::core::io::file &f)
{
    if (!f)
        return;

    if (!last_modified_time_ ||
        f.get_modification_time () > last_modified_time_)
        last_modified_time_ = f.get_modification_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add AC_SearchStrings.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_ac_searchstrings_dat_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // Get reader
        auto reader = f.new_reader ();
        if (!reader)
            return;

        // Decode file
        mobius::core::io::line_reader lr (reader, "utf-16", "\r\n");
        std::string line;
        std::size_t rec_number = 0;

        while (lr.read (line))
        {
            ++rec_number;

            if (!line.empty ())
            {
                autofill af;

                af.is_deleted = f.is_deleted ();
                af.value = line;
                af.id = "search";
                af.f = f;

                af.metadata.set ("record_number", rec_number);

                autofills_.push_back (af);
            }
        }

        // Set folder and update mtime
        log.info (
            __LINE__, "File decoded [AC_SearchStrings.dat]: " + f.get_path ()
        );

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // Emit sampling_file event
        mobius::core::emit (
            "sampling_file", std::string ("app.emule.ac_searchstrings_dat"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add key_index.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_key_index_dat_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_key_index_dat key_index (f.new_reader ());

        if (!key_index)
        {
            log.info (
                __LINE__, "File is not an instance of KeyIndex.dat. Path: " +
                              f.get_path ()
            );
            return;
        }

        log.info (__LINE__, "File decoded [key_index.dat]: " + f.get_path ());

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add remote files
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &k : key_index.get_keys ())
        {
            for (const auto &source : k.sources)
            {
                auto hash_ed2k = mobius::core::string::toupper (source.id);

                for (const auto &name : source.names)
                {
                    auto metadata = get_metadata_from_tags (name.tags);
                    metadata.set ("network", "Kamdelia");
                    metadata.set ("key_id", k.id);
                    metadata.set ("lifetime", name.lifetime);

                    for (const auto &ip : name.ips)
                    {
                        remote_file rf;
                        rf.timestamp = ip.last_published;
                        rf.ip = ip.value;
                        rf.username = username_;
                        rf.source_files.push_back (f);
                        rf.metadata = metadata.clone ();
                        rf.filename = metadata.get<std::string> ("name");

                        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
                        // Content hashes
                        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
                        std::vector<mobius::core::pod::data> hashes = {
                            {"ed2k", hash_ed2k}
                        };

                        auto aich_hash =
                            metadata.get<std::string> ("hash_aich");

                        if (!aich_hash.empty ())
                            hashes.push_back ({"aich", aich_hash});

                        rf.hashes = hashes;

                        remote_files_.push_back (rf);
                    }
                }
            }
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.emule.key_index_dat"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add known.met file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_known_met_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_known_met known_met (f.new_reader ());

        if (!known_met)
        {
            log.info (
                __LINE__,
                "File is not an instance of Known.met. Path: " + f.get_path ()
            );
            return;
        }

        log.info (__LINE__, "File decoded [known.met]: " + f.get_path ());

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add local files
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &kf : known_met.get_known_files ())
        {
            auto metadata = get_metadata_from_tags (kf.tags);

            local_file lf;

            lf.username = username_;
            lf.filename = metadata.get<std::string> ("name");
            lf.flag_downloaded = true;
            lf.flag_uploaded =
                metadata.get<std::int64_t> ("uploaded_bytes") > 0;
            lf.flag_shared = mobius::framework::evidence_flag::always;
            lf.flag_corrupted = metadata.get<bool> ("is_corrupted");
            lf.flag_completed = true; // @see CPartFile::PerformFileCompleteEnd
            lf.app_id = app_id_;
            lf.app_name = app_name_;

            metadata.set ("flag_downloaded", to_string (lf.flag_downloaded));
            metadata.set ("flag_uploaded", to_string (lf.flag_uploaded));
            metadata.set ("flag_shared", to_string (lf.flag_shared));
            metadata.set ("flag_corrupted", to_string (lf.flag_corrupted));
            metadata.set ("flag_completed", to_string (lf.flag_completed));
            metadata.set ("last_modification_time", kf.last_modification_time);
            metadata.set ("network", "eDonkey");

            lf.metadata = metadata;
            lf.hashes = get_file_hashes (kf);
            lf.f = f;

            local_files_.push_back (lf);
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.emule.known_met"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add preferences.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_preferences_dat_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto reader = f.new_reader ();
        if (!reader)
            return;

        mobius::core::decoder::data_decoder decoder (reader);

        auto version = decoder.get_uint8 ();
        auto emule_guid = decoder.get_hex_string_by_size (16);

        log.info (__LINE__, "File decoded [Preferences.dat]: " + f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Set attributes
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        bool overwrite = !preferences_dat_f_ ||
                         (preferences_dat_f_.is_deleted () && !f.is_deleted ());
        mobius::core::value_selector vs (overwrite);

        preferences_dat_version_ = vs (preferences_dat_version_, version);
        emule_guid_ = vs (emule_guid_, emule_guid);
        preferences_dat_f_ = vs (preferences_dat_f_, f);
        source_files_.push_back (f);

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file",
            "app.emule.preferences_dat." +
                mobius::core::string::to_string (version, 5),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add preferences.ini file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_preferences_ini_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto reader = f.new_reader ();
        if (!reader)
            return;

        mobius::core::decoder::inifile ini (reader);

        auto incoming_dir = ini.get_value ("emule", "incomingdir");
        auto temp_dir = ini.get_value ("emule", "tempdir");
        auto nick = ini.get_value ("emule", "nick");
        auto app_version = ini.get_value ("emule", "appversion");
        auto auto_start = ini.get_value ("emule", "autostart") == "1";

        log.info (__LINE__, "File decoded [Preferences.ini]: " + f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Set attributes
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        bool overwrite = !preferences_dat_f_ ||
                         (preferences_dat_f_.is_deleted () && !f.is_deleted ());
        mobius::core::value_selector vs (overwrite);

        incoming_dir_ = vs (incoming_dir_, incoming_dir);
        temp_dir_ = vs (temp_dir_, temp_dir);
        nick_ = vs (nick_, nick);
        app_version_ = vs (app_version_, app_version);
        auto_start_ = vs (auto_start_, auto_start);

        preferences_ini_f_ = vs (preferences_ini_f_, f);
        source_files_.push_back (f);

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.emule.preferences_ini"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add preferenceskad.dat file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_preferenceskad_dat_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto reader = f.new_reader ();
        if (!reader)
            return;

        mobius::core::decoder::data_decoder decoder (reader);

        auto kamdelia_ip = decoder.get_ipv4_le ();
        decoder.skip (2);

        auto c1 = decoder.get_uint32_le ();
        auto c2 = decoder.get_uint32_le ();
        auto c3 = decoder.get_uint32_le ();
        auto c4 = decoder.get_uint32_le ();

        auto kamdelia_guid = mobius::core::string::to_hex (c1, 8) +
                             mobius::core::string::to_hex (c2, 8) +
                             mobius::core::string::to_hex (c3, 8) +
                             mobius::core::string::to_hex (c4, 8);

        log.info (
            __LINE__, "File decoded [Preferenceskad.dat]: " + f.get_path ()
        );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Set attributes
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        bool overwrite =
            !preferenceskad_dat_f_ ||
            (preferenceskad_dat_f_.is_deleted () && !f.is_deleted ());
        mobius::core::value_selector vs (overwrite);

        kamdelia_guid_ = vs (kamdelia_guid_, kamdelia_guid);
        kamdelia_ip_ = vs (kamdelia_ip_, kamdelia_ip);
        preferenceskad_dat_f_ = vs (preferenceskad_dat_f_, f);
        source_files_.push_back (f);

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.emule.preferenceskad_dat"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add statistics.ini file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_statistics_ini_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto reader = f.new_reader ();
        if (!reader)
            return;

        mobius::core::decoder::inifile ini (reader);
        std::uint64_t total_downloaded_bytes = 0;
        std::uint64_t total_uploaded_bytes = 0;
        std::uint64_t download_completed_files = 0;

        if (ini.has_value ("statistics", "TotalDownloadedBytes"))
            total_downloaded_bytes = std::stol (
                ini.get_value ("statistics", "TotalDownloadedBytes")
            );

        if (ini.has_value ("statistics", "TotalUploadedBytes"))
            total_uploaded_bytes =
                std::stol (ini.get_value ("statistics", "TotalUploadedBytes"));

        if (ini.has_value ("statistics", "DownCompletedFiles"))
            download_completed_files =
                std::stol (ini.get_value ("statistics", "DownCompletedFiles"));

        log.info (__LINE__, "File decoded [Statistics.ini]: " + f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Set attributes
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        bool overwrite = !statistics_ini_f_ ||
                         (statistics_ini_f_.is_deleted () && !f.is_deleted ());
        mobius::core::value_selector vs (overwrite);

        total_downloaded_bytes_ =
            vs (total_downloaded_bytes_, total_downloaded_bytes);
        total_uploaded_bytes_ =
            vs (total_uploaded_bytes_, total_uploaded_bytes);
        download_completed_files_ =
            vs (download_completed_files_, download_completed_files);
        statistics_ini_f_ = vs (statistics_ini_f_, f);
        source_files_.push_back (f);

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.emule.statistics_ini"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add StoredSearches.met file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_storedsearches_met_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_stored_searches_met stored_searches (f.new_reader ());

        if (!stored_searches)
        {
            log.info (
                __LINE__,
                "File is not an instance of StoredSearches.met. Path: " +
                    f.get_path ()
            );
            return;
        }

        auto version = stored_searches.get_version ();
        log.info (
            __LINE__, "File decoded [StoredSearches.met]: " + f.get_path ()
        );

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add searches
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &s : stored_searches.get_searches ())
        {
            autofill af;

            af.is_deleted = f.is_deleted ();
            af.value = s.expression;
            af.id = "search";
            af.f = f;

            af.metadata = mobius::core::pod::map ();
            af.metadata.set ("stored_searches_version", version);
            af.metadata.set ("search_id", s.id);
            af.metadata.set ("e_type", s.e_type);
            af.metadata.set ("special_title", s.special_title);
            af.metadata.set ("filetype", s.filetype);
            af.metadata.set ("file_count", s.files.size ());

            autofills_.push_back (af);
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file",
            "app.emule.storedsearches_met." +
                mobius::core::string::to_string (version, 5),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::emule
