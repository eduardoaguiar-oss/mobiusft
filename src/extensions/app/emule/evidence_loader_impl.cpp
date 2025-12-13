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
#include "file_key_index_dat.hpp"
#include "file_known_met.hpp"
#include "file_part_met.hpp"
#include "file_part_met_txtsrc.hpp"
#include <mobius/core/datasource/datasource_vfs.hpp>
#include <mobius/core/decoder/data_decoder.hpp>
#include <mobius/core/decoder/inifile.hpp>
#include <mobius/core/io/line_reader.hpp>
#include <mobius/core/io/walker.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/framework/model/evidence.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Versions examined: Emule 0.50a and DreaMule 3.2
//
// Emule main forensic files:
// @see
// http://www.emule-project.net/home/perl/help.cgi?l=1&rm=show_topic&topic_id=106
//
// . AC_SearchStrings.dat: Stores search strings
//
// . Cancelled.met: Files cancelled before completing downloading
//
// . Clients.met: Credit control file. Control credits of each peer (dl and ul)
//
// . Downloads.txt/bak: Summary of .part.met files (part name and url)
//
// . KeyIndex.dat: Kamdelia search result file, with sources, IPs and filenames
//
// . Known.met: Shared files, downloading files, downloaded files
//
// . Preferences.dat: Stores the user GUID for eDonkey network
//
// . PreferencesKad.dat: Stores Kademlia network client ID
//
// . Sharedir.dat: Stores the paths to all shared directories
//
// . Statistics.ini: Stores statistics about program usage
//
// . StoredSearches.met: Stores open searches (ongoing searches)
//
// . *.part.met: information about a file being downloaded (not in known.met)
//
// DreaMule forensic files:
// . *.part.met.txtsrc: list of sources, with IP and expiration date/time
//
// Kademlia forensic files:
// . key_index.dat: stores a chunk of Kademlia's Distributed Hash Table,
//   including search hash value, hits (files) and peers sharing those files
//
// All Date/Times are stored in Coordinated Universal Time (UTC).
// @see
// https://msdn.microsoft.com/pt-br/library/windows/desktop/ms724397(v=vs.85).aspx
//
// According to eMule Homepage: "Your Incoming and Temporary directory are
// always shared"
// @see
// https://www.emule-project.net/home/perl/help.cgi?l=1&topic_id=112&rm=show_topic
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constants
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::string APP_ID = "emule";
static const std::string APP_NAME = "Emule";
static const std::string ANT_ID = "evidence.app-emule";
static const std::string ANT_NAME = APP_NAME;
static const std::string ANT_VERSION = "1.0";

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get vector of hashes for a given file
// @param f File structure
// @return Vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename T>
std::vector<mobius::core::pod::data>
get_file_hashes (const T &f)
{
    std::vector<mobius::core::pod::data> hashes = {
        {"ed2k", mobius::core::string::toupper (f.hash_ed2k)}};

    auto iter = std::find_if (f.tags.begin (), f.tags.end (),
                              [] (const auto &t)
                              { return t.get_id () == 0x27; } // FT_AICH_HASH
    );

    if (iter != f.tags.end ())
    {
        auto hash_value = iter->get_value ();
        hashes.push_back ({"aich", static_cast<std::string> (hash_value)});
    }

    return hashes;
}

} // namespace

namespace mobius::extension::app::emule
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param item Item object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
evidence_loader_impl::evidence_loader_impl (
    const mobius::framework::model::item &item, scan_type type)
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
    log.info (__LINE__, "Evidence loader <app-" + APP_ID + "> started");
    log.info (__LINE__, "Item UID: " + std::to_string (item_.get_uid ()));
    log.info (__LINE__,
              "Scan mode: " + std::to_string (static_cast<int> (scan_type_)));

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Check if loader has already run for item
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (item_.has_ant (ANT_ID))
    {
        log.info (__LINE__,
                  "Evidence loader <app-" + APP_ID + "> has already run");
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
    item_.add_event ("app." + APP_ID + " started");
    transaction.commit ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Scan item files, according to scan_type
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    switch (scan_type_)
    {
    case scan_type::canonical_folders:
        _scan_canonical_folders ();
        break;

    case scan_type::all_folders:
        //_scan_all_folders ();
        break;

    default:
        log.warning (__LINE__,
                     "invalid scan type: " +
                         std::to_string (static_cast<int> (scan_type_)));
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Save evidences
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    _save_evidences ();

    transaction = item_.new_transaction ();
    item_.add_event ("app." + APP_ID + " ended");
    transaction.commit ();

    log.info (__LINE__, "Evidence loader <app-" + APP_ID + "> ended");
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
    const mobius::core::io::folder &folder)
{
    username_ = {};
    auto w = mobius::core::io::walker (folder);

    // Users folders
    for (const auto &f : w.get_folders_by_pattern ("users/*"))
        _scan_canonical_user_folder (f);

    // Win XP folders
    for (const auto &f : w.get_folders_by_path ("program files/emule"))
        _scan_canonical_emule_xp_folder (f);

    for (const auto &f : w.get_folders_by_path ("program files/dreamule"))
        _scan_canonical_emule_xp_folder (f);

    for (const auto &f : w.get_folders_by_path ("arquivos de programas/emule"))
        _scan_canonical_emule_xp_folder (f);

    for (const auto &f :
         w.get_folders_by_path ("arquivos de programas/dreamule"))
        _scan_canonical_emule_xp_folder (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan user folder for evidences
// @param folder User folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_user_folder (
    const mobius::core::io::folder &folder)
{
    username_ = folder.get_name ();
    auto w = mobius::core::io::walker (folder);

    for (const auto &f : w.get_folders_by_path ("downloads/emule/incoming"))
        _scan_canonical_emule_download_folder (f);

    for (const auto &f : w.get_folders_by_path ("downloads/emule/temp"))
        _scan_canonical_emule_download_folder (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan emule/dreamule folder for evidences
// @param folder emule/dreamule folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_emule_xp_folder (
    const mobius::core::io::folder &folder)
{
    auto w = mobius::core::io::walker (folder);

    for (const auto &f : w.get_folders_by_name ("incoming"))
        _scan_canonical_emule_download_folder (f);

    for (const auto &f : w.get_folders_by_name ("temp"))
        _scan_canonical_emule_download_folder (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Scan Download/emule folder
// @param folder Download/emule folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_scan_canonical_emule_download_folder (
    const mobius::core::io::folder &folder)
{
    mobius::core::io::walker w (folder);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decoder .part.met files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &f : w.get_files_by_pattern ("*.part.met"))
        _decode_part_met_file (f);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decoder .part.met.txtsrc files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &f : w.get_files_by_pattern ("*.part.met.txtsrc"))
        _decode_part_met_txtsrc_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode .part.met file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_part_met_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_part_met part_met (f.new_reader ());

        if (!part_met)
        {
            log.info (__LINE__, "File is not an instance of .part.met. Path: " +
                                    f.get_path ());
            return;
        }

        log.info (__LINE__, ".part.met file decoded. Path: " + f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Create local file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto metadata = get_metadata_from_tags (part_met.get_tags ());

        local_file lf;

        lf.username = username_;
        lf.path = f.get_path ();
        lf.path.erase (lf.path.size () - 4);
        lf.filename = metadata.get<std::string> ("name");
        lf.is_deleted = f.is_deleted ();
        lf.f = f;

        lf.flag_downloaded = true;
        lf.flag_uploaded = metadata.get<std::int64_t> ("uploaded_bytes") > 0;
        lf.flag_shared = mobius::framework::evidence_flag::always;
        lf.flag_corrupted = metadata.get<bool> ("is_corrupted");
        lf.flag_completed = part_met.get_total_gap_size () == 0;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Metadata
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        metadata.set ("file_version", part_met.get_version ());
        metadata.set ("flag_downloaded", to_string (lf.flag_downloaded));
        metadata.set ("flag_uploaded", to_string (lf.flag_uploaded));
        metadata.set ("flag_shared", to_string (lf.flag_shared));
        metadata.set ("flag_corrupted", to_string (lf.flag_corrupted));
        metadata.set ("flag_completed", to_string (lf.flag_completed));
        metadata.set ("timestamp", part_met.get_timestamp ());
        metadata.set ("total_gap_size", part_met.get_total_gap_size ());
        metadata.set ("network", "eDonkey");

        lf.metadata = metadata;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Content hashes
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        std::vector<mobius::core::pod::data> hashes = {
            {"ed2k",
             mobius::core::string::toupper (part_met.get_hash_ed2k ())}};

        auto aich_hash = metadata.get<std::string> ("hash_aich");

        if (!aich_hash.empty ())
            hashes.push_back ({"aich", aich_hash});

        lf.hashes = hashes;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add local file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        local_files_.push_back (lf);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add part.met file to the list of part.met files
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto iter = part_met_files_.find (f.get_name ());

        if (iter == part_met_files_.end () ||
            (iter->second.is_deleted && !f.is_deleted ()))
            part_met_files_[f.get_name ()] = lf;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode .part.met.txtsrc file
// @param f File object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_decode_part_met_txtsrc_file (
    const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_part_met_txtsrc txtsrc (f.new_reader ());

        if (!txtsrc)
        {
            log.info (__LINE__,
                      "File is not an instance of .part.met.txtsrc. Path: " +
                          f.get_path ());
            return;
        }

        log.info (__LINE__,
                  ".part.met.txtsrc file decoded. Path: " + f.get_path ());

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get corresponding part.met file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto part_met_txtsrc_name = f.get_name ();
        auto part_met_name =
            part_met_txtsrc_name.substr (0, part_met_txtsrc_name.size () - 7);
        auto iter = part_met_files_.find (part_met_name);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Create remote file if part.met file is found
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        if (iter != part_met_files_.end ())
        {
            auto lf = iter->second;

            for (const auto &source : txtsrc.get_sources ())
            {
                auto rf = remote_file ();

                rf.username = username_;
                rf.timestamp = f.get_modification_time ();
                rf.ip = source.ip;
                rf.port = source.port;
                rf.filename = lf.filename;
                rf.part_met_f = lf.f;
                rf.part_met_txtsrc_f = f;
                rf.hashes = lf.hashes.clone ();
                rf.metadata = lf.metadata.clone ();

                remote_files_.push_back (rf);
            }
        }
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save evidences
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_evidences ()
{
    auto transaction = item_.new_transaction ();

    _save_local_files ();
    _save_p2p_remote_files ();
    _save_received_files ();
    _save_sent_files ();
    _save_shared_files ();

    item_.set_ant (ANT_ID, ANT_NAME, ANT_VERSION);
    transaction.commit ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save local files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_local_files ()
{
    for (const auto &lf : local_files_)
    {
        auto e = item_.new_evidence ("local-file");

        e.set_attribute ("username", lf.username);
        e.set_attribute ("filename", lf.filename);
        e.set_attribute ("path", lf.path);
        e.set_attribute ("app_id", APP_ID);
        e.set_attribute ("app_name", APP_NAME);
        e.set_attribute ("hashes", lf.hashes);
        e.set_attribute ("metadata", lf.metadata);

        e.set_tag ("app.p2p");
        e.add_source (lf.f);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save received files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_received_files ()
{
    for (const auto &lf : local_files_)
    {
        if (lf.flag_downloaded.is_yes ())
        {
            auto e = item_.new_evidence ("received-file");

            e.set_attribute ("username", lf.username);
            e.set_attribute ("filename", lf.filename);
            e.set_attribute ("path", lf.path);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("hashes", lf.hashes);
            e.set_attribute ("metadata", lf.metadata);

            e.set_tag ("app.p2p");
            e.add_source (lf.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save remote files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_p2p_remote_files ()
{
    for (const auto &rf : remote_files_)
    {
        auto e = item_.new_evidence ("remote-party-shared-file");

        e.set_attribute ("timestamp", rf.timestamp);
        e.set_attribute ("ip", rf.ip);
        e.set_attribute ("port", rf.port);
        e.set_attribute ("filename", rf.filename);
        e.set_attribute ("username", rf.username);
        e.set_attribute ("app_id", APP_ID);
        e.set_attribute ("app_name", APP_NAME);
        e.set_attribute ("hashes", rf.hashes);
        e.set_attribute ("metadata", rf.metadata);

        e.set_tag ("app.p2p");

        e.add_source (rf.key_index_dat_f);
        e.add_source (rf.part_met_f);
        e.add_source (rf.part_met_txtsrc_f);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save sent files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_sent_files ()
{
    for (const auto &lf : local_files_)
    {
        if (lf.flag_uploaded.is_yes ())
        {
            auto e = item_.new_evidence ("sent-file");

            e.set_attribute ("username", lf.username);
            e.set_attribute ("filename", lf.filename);
            e.set_attribute ("path", lf.path);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("hashes", lf.hashes);
            e.set_attribute ("metadata", lf.metadata);

            e.set_tag ("app.p2p");
            e.add_source (lf.f);
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Save shared files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
evidence_loader_impl::_save_shared_files ()
{
    for (const auto &lf : local_files_)
    {
        if (lf.flag_shared.is_yes () || lf.flag_shared.is_always ())
        {
            auto e = item_.new_evidence ("shared-file");

            e.set_attribute ("username", lf.username);
            e.set_attribute ("filename", lf.filename);
            e.set_attribute ("path", lf.path);
            e.set_attribute ("app_id", APP_ID);
            e.set_attribute ("app_name", APP_NAME);
            e.set_attribute ("hashes", lf.hashes);
            e.set_attribute ("metadata", lf.metadata);

            e.set_tag ("app.p2p");
            e.add_source (lf.f);
        }
    }
}

} // namespace mobius::extension::app::emule
