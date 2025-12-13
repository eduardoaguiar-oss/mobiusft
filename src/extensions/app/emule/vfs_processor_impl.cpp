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
static const std::string SAMPLING_ID = "sampling";
static const std::string APP_ID = "emule";
static const std::string APP_NAME = "Emule";

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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update metadata map, preferring non null values
// @param metadata Metadata map
// @param other Other metadata map
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
update_metadata (
    mobius::core::pod::map &metadata, const mobius::core::pod::map &other
)
{
    for (const auto &[k, v] : other)
    {
        auto old_v = metadata.get (k);

        if (!metadata.contains (k) || (old_v.is_null () && !v.is_null ()))
            metadata.set (k, v);
    }
}

} // namespace

namespace mobius::extension::app::emule
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
    //_scan_profile_folder (folder);
    //_scan_arestra_folder (folder);
    //_scan_ntuser_dat_folder (folder);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Called when processing is complete
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::on_complete ()
{
    auto transaction = item_.new_transaction ();

    //_save_app_profiles ();
    //_save_autofills ();

    transaction.commit ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for ___ARESTRA___ files
// @param folder Folder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*void
vfs_processor_impl::_scan_arestra_folder (const mobius::core::io::folder &folder)
{
    mobius::core::log log (__FILE__, __FUNCTION__);
    mobius::core::io::walker w (folder);

    for (const auto &[name, f] : w.get_files_with_names ())
    {
        try
        {
            //if (mobius::core::string::startswith (name, "___arestra___"))
            //    _decode_arestra_file (f);
        }
        catch (const std::exception &e)
        {
            log.warning (
                __LINE__,
                std::string (e.what ()) + " (file: " + f.get_path () + ")"
            );
        }
    }
}*/

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode ARESTRA file
// @param f ARESTRA file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*void
vfs_processor_impl::_decode_arestra_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_arestra arestra (f.new_reader ());

    if (!arestra)
    {
        log.info (
            __LINE__,
            "File " + f.get_path () + " is not a valid PBTHash.dat file"
        );
        return;
    }

    log.info (__LINE__, "File decoded [___ARESTRA___]. Path: " + f.get_path ());

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Create file object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    profile::file fobj;

    // set attributes
    fobj.hash_sha1 = arestra.get_hash_sha1 ();
    fobj.username = get_username_from_path (f.get_path ());
    fobj.download_started_time = arestra.get_download_started_time ();
    fobj.size = arestra.get_file_size ();
    fobj.arestra_f = f;

    // set filename
    fobj.filename = mobius::core::io::path (f.get_path ()).get_filename ();
    fobj.filename.erase (0, 13); // remove "___ARESTRA___"

    // set flags
    fobj.flag_downloaded = true;
    fobj.flag_corrupted = arestra.is_corrupted ();
    fobj.flag_shared = false; // @see thread_share.pas (line 1065)
    fobj.flag_completed = arestra.is_completed ();

    // add remote_sources
    for (const auto &[ip, port] : arestra.get_alt_sources ())
    {
        profile::remote_source r_source;
        r_source.timestamp = f.get_modification_time ();
        r_source.ip = ip;
        r_source.port = port;

        fobj.remote_sources.push_back (r_source);
    }

    // set metadata
    fobj.metadata.set ("arestra_signature", arestra.get_signature ());
    fobj.metadata.set ("arestra_file_version", arestra.get_version ());
    fobj.metadata.set (
        "download_started_time", arestra.get_download_started_time ()
    );
    fobj.metadata.set ("downloaded_bytes", arestra.get_progress ());
    fobj.metadata.set ("verified_bytes", arestra.get_phash_verified ());
    fobj.metadata.set ("is_paused", arestra.is_paused ());
    fobj.metadata.set ("media_type", arestra.get_media_type ());
    fobj.metadata.set ("param1", arestra.get_param1 ());
    fobj.metadata.set ("param2", arestra.get_param2 ());
    fobj.metadata.set ("param3", arestra.get_param3 ());
    fobj.metadata.set ("kwgenre", arestra.get_kw_genre ());
    fobj.metadata.set ("title", arestra.get_title ());
    fobj.metadata.set ("artist", arestra.get_artist ());
    fobj.metadata.set ("album", arestra.get_album ());
    fobj.metadata.set ("category", arestra.get_category ());
    fobj.metadata.set ("year", arestra.get_year ());
    fobj.metadata.set ("language", arestra.get_language ());
    fobj.metadata.set ("url", arestra.get_url ());
    fobj.metadata.set ("comment", arestra.get_comment ());
    fobj.metadata.set ("subfolder", arestra.get_subfolder ());

    files_.push_back (fobj);
}
*/

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan folder for µTorrent/BitTorrent profiles
// @param folder Folder to scan
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
vfs_processor_impl::_scan_profile_folder (const mobius::core::io::folder &folder)
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
        {/*
            if (name == "shareh.dat")
                p.add_shareh_file (f);

            else if (name == "sharel.dat")
                p.add_sharel_file (f);

            else if (name == "torrenth.dat")
                p.add_torrenth_file (f);

            else if (name == "phashidx.dat" || name == "phashidxtemp.dat" ||
                     name == "tempphash.dat")
                p.add_phashidx_file (f);*/
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

} // namespace mobius::extension::app::emule
