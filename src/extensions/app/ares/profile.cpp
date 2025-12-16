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
#include <mobius/core/io/path.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/core/value_selector.hpp>
#include <mobius/framework/utils.hpp>
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include "file_pbthash.hpp"
#include "file_phash.hpp"
#include "file_shareh.hpp"
#include "file_sharel.hpp"
#include "file_torrenth.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
//    . Ares Galaxy 246 source code
//    . Forensic Analysis of Ares Galaxy Peer-to-Peer Network (Kolenbrander)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace mobius::extension::app::ares
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class profile::impl
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if profile is valid
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_valid () const
    {
        return bool (folder_);
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
    // @brief Get number of files in profile
    // @return Number of files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::size_t
    size_files () const
    {
        _consolidate_files ();
        return consolidated_files_.size ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get files in profile
    // @return Vector of files
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<file>
    get_files () const
    {
        _consolidate_files ();
        return consolidated_files_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void add_phashidx_file (const mobius::core::io::file &);
    void add_shareh_file (const mobius::core::io::file &);
    void add_sharel_file (const mobius::core::io::file &);
    void add_torrenth_file (const mobius::core::io::file &);
    void add_tempdl_phash_file (const mobius::core::io::file &);
    void add_tempdl_pbthash_file (const mobius::core::io::file &);
    void add_tempul_udpphash_file (const mobius::core::io::file &);

  private:
    // @brief Folder object
    mobius::core::io::folder folder_;

    // @brief Username
    std::string username_;

    // @brief Creation time
    mobius::core::datetime::datetime creation_time_;

    // @brief Last modified time
    mobius::core::datetime::datetime last_modified_time_;

    // @brief Account files
    std::map<std::string, file> files_;

    // @brief Consolidated files
    mutable std::vector<file> consolidated_files_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _set_folder (const mobius::core::io::folder &);
    void _consolidate_files () const;
    void _update_mtime (const mobius::core::io::file &);
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

    folder_ = f;

    last_modified_time_ = f.get_modification_time ();
    creation_time_ = f.get_creation_time ();
    username_ = mobius::framework::get_username_from_path (f.get_path ());

    mobius::core::emit (
        "sampling_folder", std::string ("app.ares.profiles"), f
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Consolidate files from map to vector
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::_consolidate_files () const
{
    if (!consolidated_files_.empty ())
        return;

    for (const auto &[hash_sha1, af] : files_)
    {
        std::ignore = hash_sha1;

        if (af.torrent_files.size () == 0)
            consolidated_files_.push_back (af);

        else
        {
            for (const auto &tf : af.torrent_files)
            {
                file f = af;

                f.size = tf.size;
                f.filename = tf.name;
                f.path = tf.path;
                f.hash_sha1.clear ();

                f.metadata = af.metadata.clone ();
                f.metadata.set ("torrent_file_idx", tf.idx);
                f.metadata.set (
                    "torrent_last_modification_time", tf.last_modification_time
                );

                consolidated_files_.push_back (f);
            }
        }
    }
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
// @brief Add PHashIdx.dat file
// @param f PHashIdx.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_phashidx_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_phash ph (f.new_reader ());

    if (!ph)
    {
        log.info (
            __LINE__,
            "File " + f.get_path () + " is not a valid PHashIdx.dat file."
        );
        return;
    }
    log.info (__LINE__, "File decoded [PHashIdx]: " + f.get_path ());

    _set_folder (f.get_parent ());
    _update_mtime (f);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add PHashIdx entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_deleted = f.is_deleted ();

    for (const auto &entry : ph)
    {
        auto [iter, success] = files_.try_emplace (entry.hash_sha1, file {});
        std::ignore = success;
        auto &fobj = iter->second;

        if (!fobj.phashidx_f || (fobj.phashidx_f.is_deleted () && !is_deleted))
        {
            fobj.hash_sha1 = entry.hash_sha1;
            fobj.username = username_;
            fobj.phashidx_idx = entry.idx;
            fobj.flag_completed =
                true; // PHashIdx.dat entries are always completed
            fobj.flag_downloaded = true;
            fobj.phashidx_f = f;

            if (fobj.size)
                fobj.metadata.set ("downloaded_bytes", fobj.size);
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Emit sampling_file event
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::emit (
        "sampling_file", std::string ("app.ares.phashidx_dat"), f.new_reader ()
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add ShareH.dat file
// @param f Share history file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_shareh_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_shareh fh (f.new_reader ());

    if (!fh)
    {
        log.info (
            __LINE__,
            "File " + f.get_path () + " is not a valid ShareH.dat file"
        );
        return;
    }

    log.info (__LINE__, "File decoded [ShareH.dat]: " + f.get_path ());

    _set_folder (f.get_parent ());
    _update_mtime (f);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add share history entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_deleted = f.is_deleted ();

    for (const auto &entry : fh)
    {
        auto [iter, success] = files_.try_emplace (entry.hash_sha1, file {});
        std::ignore = success;
        auto &fobj = iter->second;

        if (!fobj.shareh_f || (fobj.shareh_f.is_deleted () && !is_deleted))
        {
            fobj.hash_sha1 = entry.hash_sha1;
            fobj.username = username_;
            fobj.download_completed_time = entry.download_completed_time;
            fobj.shareh_idx = entry.idx;
            fobj.shareh_f = f;

            fobj.flag_shared = entry.is_shared;
            fobj.flag_completed = true; // ShareH entries are always completed
            fobj.flag_corrupted = entry.is_corrupted;

            if (fobj.download_completed_time)
                fobj.flag_downloaded = true;

            fobj.metadata.set ("title", entry.title);
            fobj.metadata.set ("artist", entry.artist);
            fobj.metadata.set ("album", entry.album);
            fobj.metadata.set ("category", entry.category);
            fobj.metadata.set ("year", entry.year);
            fobj.metadata.set ("language", entry.language);
            fobj.metadata.set ("url", entry.url);
            fobj.metadata.set ("comment", entry.comment);
            fobj.metadata.set (
                "download_completed_time", entry.download_completed_time
            );
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Emit sampling_file event
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::emit (
        "sampling_file", std::string ("app.ares.shareh_dat"), f.new_reader ()
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add ShareL.dat file
// @param f Share library file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_sharel_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_sharel fl (f.new_reader ());

    if (!fl)
    {
        log.info (
            __LINE__,
            "File " + f.get_path () + " is not a valid ShareL.dat file"
        );
        return;
    }

    log.info (__LINE__, "File decoded [ShareL.dat]: " + f.get_path ());

    _set_folder (f.get_parent ());
    _update_mtime (f);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add share library entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_deleted = f.is_deleted ();

    for (const auto &entry : fl)
    {
        auto [iter, success] = files_.try_emplace (entry.hash_sha1, file {});
        std::ignore = success;
        auto &fobj = iter->second;

        if (!fobj.sharel_f || (fobj.sharel_f.is_deleted () && !is_deleted))
        {
            // attributes
            fobj.hash_sha1 = entry.hash_sha1;
            fobj.username = username_;
            fobj.path = entry.path;
            fobj.size = entry.size;
            fobj.sharel_idx = entry.idx;
            fobj.sharel_f = f;

            if (!fobj.path.empty ())
            {
                auto cpath =
                    mobius::core::string::replace (fobj.path, "\\", "/");
                fobj.filename = mobius::core::io::path (cpath).get_filename ();
            }

            // flags
            fobj.flag_corrupted.set_if_unknown (entry.is_corrupted);
            fobj.flag_shared.set_if_unknown (
                true
            ); // ShareL is shared by default, unless it is flagged
               // "no" in the corresponding ShareH entry.

            fobj.flag_completed = true; // ShareL entries are always completed

            // metadata
            fobj.metadata.set ("media_type", entry.media_type);
            fobj.metadata.set ("param1", entry.param1);
            fobj.metadata.set ("param2", entry.param2);
            fobj.metadata.set ("param3", entry.param3);
            fobj.metadata.set ("path", entry.path);
            fobj.metadata.set ("title", entry.title);
            fobj.metadata.set ("artist", entry.artist);
            fobj.metadata.set ("album", entry.album);
            fobj.metadata.set ("category", entry.category);
            fobj.metadata.set ("year", entry.year);
            fobj.metadata.set ("vidinfo", entry.vidinfo);
            fobj.metadata.set ("language", entry.language);
            fobj.metadata.set ("url", entry.url);
            fobj.metadata.set ("comment", entry.comment);
            fobj.metadata.set ("hash_of_phash", entry.hash_of_phash);
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Emit sampling_file event
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::emit (
        "sampling_file", std::string ("app.ares.sharel_dat"), f.new_reader ()
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add TorrentH.dat file
// @param f TorrentH.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_torrenth_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_torrenth th (f.new_reader ());

    if (!th)
    {
        log.info (
            __LINE__,
            "File " + f.get_path () + " is not a valid TorrentH.dat file"
        );
        return;
    }

    log.info (__LINE__, "File decoded [TorrentH.dat]: " + f.get_path ());

    _set_folder (f.get_parent ());
    _update_mtime (f);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add TorrentH entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_deleted = f.is_deleted ();

    for (const auto &entry : th)
    {
        auto [iter, success] = files_.try_emplace (entry.hash_sha1, file {});
        std::ignore = success;
        auto &fobj = iter->second;

        if (!fobj.shareh_f || (fobj.shareh_f.is_deleted () && !is_deleted))
        {
            fobj.hash_sha1 = entry.hash_sha1;
            fobj.username = username_;
            fobj.torrenth_idx = entry.idx;
            fobj.torrenth_f = f;
            fobj.filename = entry.name;
            fobj.size = entry.size;
            fobj.download_started_time = entry.timestamp;

            fobj.flag_shared = true;     // @see DHT/thread_dht.pas (line 412)
            fobj.flag_downloaded = true; // @see DHT/dhtkeywords.pas (line 355)
            fobj.flag_completed = true;  // @see DHT/dhtkeywords.pas (line 355)
            fobj.flag_corrupted = false; // @see DHT/dhtkeywords.pas (line 355)

            fobj.metadata.set ("seeds", entry.seeds);
            fobj.metadata.set ("media_type", entry.media_type);
            fobj.metadata.set (
                "evaluated_hash_sha1", entry.evaluated_hash_sha1
            );
            fobj.metadata.set ("torrent_name", entry.name);
            fobj.metadata.set ("torrent_url", entry.url);
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Emit sampling_file event
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::emit (
        "sampling_file", std::string ("app.ares.torrenth_dat"), f.new_reader ()
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add TempDL/PHash_XXX.dat file
// @param f TempDL/PHash_XXX.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_tempdl_phash_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_phash phash (f.new_reader ());

    if (!phash)
    {
        log.info (
            __LINE__, "File " + f.get_path () + " is not a valid PHash.dat file"
        );
        return;
    }

    log.info (__LINE__, "File decoded [PHash.dat]: " + f.get_path ());

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Process entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_deleted = f.is_deleted ();

    for (const auto &entry : phash)
    {
        auto hash_sha1 = entry.hash_sha1;
        auto [iter, success] = files_.try_emplace (hash_sha1, file {});
        std::ignore = success;
        auto &fobj = iter->second;

        if (!fobj.tempdl_phash_f ||
            (fobj.tempdl_phash_f.is_deleted () && !is_deleted))
        {
            fobj.hash_sha1 = hash_sha1;
            fobj.username = username_;
            fobj.flag_downloaded = true;
            fobj.tempdl_phash_f = f;
            fobj.metadata.set ("pieces_count", entry.pieces_count);

            if (entry.is_completed != 2) // STATE_UNKNOWN
            {
                fobj.flag_completed = bool (entry.is_completed);
                fobj.metadata.set ("downloaded_bytes", entry.progress);
                fobj.metadata.set ("pieces_completed", entry.pieces_completed);
                fobj.metadata.set ("pieces_to_go", entry.pieces_to_go);
                fobj.metadata.set ("piece_size", entry.piece_size);
            }
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add TempDL/PBTHash_XXX.dat file
// @param f TempDL/PBTHash_XXX.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_tempdl_pbthash_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_pbthash pbthash (f.new_reader ());

    if (!pbthash)
    {
        log.info (
            __LINE__,
            "File " + f.get_path () + " is not a valid PBTHash.dat file"
        );
        return;
    }

    log.info (__LINE__, "File decoded [PBTHash.dat]: " + f.get_path ());

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Process file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_deleted = f.is_deleted ();

    auto hash_sha1 = pbthash.get_hash_sha1 ();
    auto [iter, success] = files_.try_emplace (hash_sha1, file {});
    std::ignore = success;
    auto &fobj = iter->second;

    if (!fobj.tempdl_pbthash_f ||
        (fobj.tempdl_pbthash_f.is_deleted () && !is_deleted))
    {
        fobj.hash_sha1 = hash_sha1;
        fobj.username = username_;
        fobj.size = pbthash.get_file_size ();
        fobj.tempdl_pbthash_f = f;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Set path and name
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto path = pbthash.get_torrent_path ();
        if (!path.empty ())
            fobj.path = path;

        auto name = pbthash.get_torrent_name ();
        if (!name.empty ())
            fobj.filename = name;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Test if every piece is checked
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto pieces = pbthash.get_pieces ();
        auto is_checked = std::all_of (
            pieces.begin (), pieces.end (),
            [] (const auto &p) { return p.is_checked; }
        );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Set flags
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        fobj.flag_downloaded = true;
        fobj.flag_completed.set_if_unknown (pbthash.is_completed ());
        fobj.flag_uploaded.set_if_unknown (pbthash.get_bytes_uploaded () > 0);
        fobj.flag_shared.set_if_unknown (pbthash.is_seeding ());
        fobj.flag_corrupted.set_if_unknown (!is_checked);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Set torrent files
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto files = pbthash.get_files ();

        std::transform (
            files.begin (), files.end (),
            std::back_inserter (fobj.torrent_files),
            [] (const auto &tf)
            {
                return torrent_file {
                    tf.idx, tf.size, tf.last_modification_time, tf.name, tf.path
                };
            }
        );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Set metadata
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        fobj.metadata.set ("torrent_url", "magnet:?xt=urn:btih:" + hash_sha1);
        fobj.metadata.set ("torrent_pieces_count", pbthash.get_pieces_count ());
        fobj.metadata.set ("torrent_piece_size", pbthash.get_piece_size ());
        fobj.metadata.set ("torrent_files_count", pbthash.get_files_count ());
        fobj.metadata.set (
            "torrent_bytes_downloaded", pbthash.get_bytes_downloaded ()
        );
        fobj.metadata.set (
            "torrent_bytes_uploaded", pbthash.get_bytes_uploaded ()
        );
        fobj.metadata.set ("torrent_path", pbthash.get_torrent_path ());
        fobj.metadata.set ("torrent_name", pbthash.get_torrent_name ());
        fobj.metadata.set (
            "torrent_download_started_time",
            pbthash.get_download_started_time ()
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add TempUL/UDPPHash_XXX.dat file
// @param f TempUL/UDPPHash_XXX.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::impl::add_tempul_udpphash_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_phash phash (f.new_reader ());

    if (!phash)
    {
        log.info (
            __LINE__, "File " + f.get_path () + " is not a valid PHash.dat file"
        );
        return;
    }

    log.info (__LINE__, "File decoded [UDPPHash.dat]: " + f.get_path ());

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Process entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool is_deleted = f.is_deleted ();

    for (const auto &entry : phash)
    {
        auto hash_sha1 = entry.hash_sha1;
        auto [iter, success] = files_.try_emplace (hash_sha1, file {});
        std::ignore = success;
        auto &fobj = iter->second;

        if (!fobj.tempul_udpphash_f ||
            (fobj.tempul_udpphash_f.is_deleted () && !is_deleted))
        {
            fobj.hash_sha1 = hash_sha1;
            fobj.username = username_;
            fobj.flag_uploaded = true;
            fobj.tempul_udpphash_f = f;
            fobj.metadata.set ("pieces_count", entry.pieces_count);
        }
    }
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
    return impl_->is_valid ();
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
// @return Folder
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
// @brief Get number of files in profile
// @return Number of files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::size_t
profile::size_files () const
{
    return impl_->size_files ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get files in profile
// @return Vector of files
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<profile::file>
profile::get_files () const
{
    return impl_->get_files ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add PHashIdx.dat file
// @param f PHashIdx.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_phashidx_file (const mobius::core::io::file &f)
{
    impl_->add_phashidx_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add ShareH.dat file
// @param f ShareH.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_shareh_file (const mobius::core::io::file &f)
{
    impl_->add_shareh_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add ShareL.dat file
// @param f ShareL.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_sharel_file (const mobius::core::io::file &f)
{
    impl_->add_sharel_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add TorrentH.dat file
// @param f TorrentH.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_torrenth_file (const mobius::core::io::file &f)
{
    impl_->add_torrenth_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add TempDL/PHash_XXX.dat file
// @param f TempDL/PHash_XXX.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_tempdl_phash_file (const mobius::core::io::file &f)
{
    impl_->add_tempdl_phash_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add TempDL/PBTHash_XXX.dat file
// @param f TempDL/PBTHash_XXX.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_tempdl_pbthash_file (const mobius::core::io::file &f)
{
    impl_->add_tempdl_pbthash_file (f);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add TempUL/UDPPHash_XXX.dat file
// @param f TempUL/UDPPHash_XXX.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_tempul_udpphash_file (const mobius::core::io::file &f)
{
    impl_->add_tempul_udpphash_file (f);
}

} // namespace mobius::extension::app::ares
