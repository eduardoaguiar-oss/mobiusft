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
#include <mobius/core/decoder/xml/dom.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/core/value_selector.hpp>
#include <mobius/framework/utils.hpp>
#include <string>
#include "CLibrary.hpp"
#include "CThumbCache.hpp"
#include "common.hpp"
#include "file_searches_dat.hpp"

namespace mobius::extension::app::shareaza
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
    username_ = mobius::framework::get_username_from_path (f.get_path ());

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Emit sampling_folder event
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::emit (
        "sampling_folder", std::string ("app.shareaza.profiles"), f
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
// @brief Add library.dat file
// @param f library.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_library_dat_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto decoder = mobius::core::decoder::mfc (f.new_reader ());
        CLibrary clib (decoder);

        if (!clib)
        {
            log.info (
                __LINE__,
                "File is not an instance of Library.dat. Path: " + f.get_path ()
            );
            return;
        }

        log.info (__LINE__, "File decoded [library.dat]: " + f.get_path ());

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Check if library file is the most recent one
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        if (!library_dat_mtime_ ||
            library_dat_mtime_ < clib.get_last_modification_time ())
        {
            local_files_.clear ();
            remote_files_.clear ();
            library_dat_mtime_ = clib.get_last_modification_time ();

            for (const auto &cfile : clib.get_all_files ())
            {
                local_file lf;

                // Attributes
                lf.filename = cfile.get_name ();
                lf.path = cfile.get_path ();
                lf.username = username_;
                lf.hashes = get_file_hashes (cfile);
                lf.flag_uploaded = cfile.get_uploads_started () > 0;
                lf.flag_shared = cfile.is_shared ();
                lf.f = f;

                // Thumbnail data
                auto thumbnail = thumbcache_.get (cfile.get_path ());
                if (thumbnail)
                {
                    lf.thumbnail_data = thumbnail->image_data;
                    lf.shareaza_db3_f = shareaza_db3_f_;
                }

                // Metadata
                lf.metadata.set ("flag_downloaded", "unknown");
                lf.metadata.set (
                    "flag_uploaded", lf.flag_uploaded ? "true" : "false"
                );
                lf.metadata.set (
                    "flag_shared", lf.flag_shared ? "true" : "false"
                );
                lf.metadata.set ("flag_corrupted", "unknown");
                lf.metadata.set ("flag_completed", "true");

                lf.metadata.set ("size", cfile.get_size ());
                lf.metadata.set ("index", cfile.get_index ());
                lf.metadata.set ("virtual_size", cfile.get_virtual_size ());
                lf.metadata.set ("virtual_base", cfile.get_virtual_base ());
                lf.metadata.set ("uri", cfile.get_uri ());

                auto rating = cfile.get_rating ();
                if (rating != -1)
                    lf.metadata.set ("rating", rating);

                lf.metadata.set ("comments", cfile.get_comments ());
                lf.metadata.set ("share_tags", cfile.get_share_tags ());
                lf.metadata.set ("hits_total", cfile.get_hits_total ());
                lf.metadata.set (
                    "uploads_started", cfile.get_uploads_started ()
                );
                lf.metadata.set (
                    "last_modification_time",
                    cfile.get_last_modification_time ()
                );
                lf.metadata.set ("metadata_time", cfile.get_metadata_time ());

                if (thumbnail)
                    lf.metadata.set (
                        "thumbnail_last_write_time", thumbnail->last_write_time
                    );

                for (const auto &[k, v] : cfile.get_pxml ().get_metadata ())
                    lf.metadata.set (k, v);

                local_files_.push_back (lf);

                // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
                // Add remote files
                // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
                for (const auto &source : cfile.get_sources ())
                {
                    remote_file rf;

                    rf.timestamp = source.get_timestamp ();
                    rf.ip = source.get_ip ();
                    rf.port = source.get_port ();
                    rf.filename = cfile.get_name ();
                    rf.username = username_;
                    rf.hashes = get_file_hashes (cfile);
                    rf.f = f;

                    // metadata
                    rf.metadata.set ("size", cfile.get_size ());
                    rf.metadata.set ("index", cfile.get_index ());
                    rf.metadata.set ("virtual_size", cfile.get_virtual_size ());
                    rf.metadata.set ("virtual_base", cfile.get_virtual_base ());
                    rf.metadata.set ("url", source.get_url ());
                    rf.metadata.set ("schema_uri", cfile.get_uri ());

                    auto rating = cfile.get_rating ();
                    if (rating != -1)
                        rf.metadata.set ("rating", rating);

                    rf.metadata.set ("comments", cfile.get_comments ());
                    rf.metadata.set ("share_tags", cfile.get_share_tags ());
                    rf.metadata.set ("hits_total", cfile.get_hits_total ());
                    rf.metadata.set (
                        "uploads_started", cfile.get_uploads_started ()
                    );
                    rf.metadata.set (
                        "last_modification_time",
                        cfile.get_last_modification_time ()
                    );
                    rf.metadata.set (
                        "metadata_time", cfile.get_metadata_time ()
                    );

                    if (thumbnail)
                    {
                        rf.metadata.set (
                            "thumbnail_last_write_time",
                            thumbnail->last_write_time
                        );
                        rf.thumbnail_data = thumbnail->image_data;
                        rf.shareaza_db3_f = shareaza_db3_f_;
                    }

                    for (const auto &[k, v] : cfile.get_pxml ().get_metadata ())
                        rf.metadata.set (k, v);

                    remote_files_.push_back (rf);
                }
            }
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.shareaza.library_dat"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add profile.xml file
// @param f profile.xml file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_profile_xml_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        mobius::core::decoder::xml::dom dom (f.new_reader ());
        auto root = dom.get_root_element ();

        log.info (__LINE__, "File decoded [profile.xml]: " + f.get_path ());

        bool overwrite = !profile_xml_f_ ||
                         (profile_xml_f_.is_deleted () && !f.is_deleted ());
        mobius::core::value_selector vs (overwrite);

        gnutella_guid_ =
            vs (gnutella_guid_, mobius::core::string::toupper (
                                    root.get_property_by_path ("gnutella/guid")
                                ));
        bittorrent_guid_ =
            vs (bittorrent_guid_,
                mobius::core::string::toupper (
                    root.get_property_by_path ("bittorrent/guid")
                ));
        identity_ =
            vs (identity_,
                root.get_property_by_path ("identity/handle/primary"));

        profile_xml_f_ = f;
        source_files.push_back (f);

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.shareaza.profile_xml"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add Searches.dat file
// @param f Searches.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_searches_dat_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_searches_dat searches_dat (f.new_reader ());

        if (!searches_dat)
        {
            log.info (__LINE__, "File is not a valid Searches.dat file");
            return;
        }

        log.info (__LINE__, "File decoded [searches.dat]: " + f.get_path ());

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add searches
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &s : searches_dat.get_searches ())
        {
            auto cmanagedsearch = s.obj;
            auto qs = cmanagedsearch.get_query_search ();

            searched_text st;

            // Attributes
            st.timestamp = s.timestamp;
            st.text = s.text;
            st.f = f;

            // Metadata
            st.metadata.set ("search_id", qs.get_guid ());
            st.metadata.set ("first_hit_time", s.first_hit_time);
            st.metadata.set ("last_hit_time", s.last_hit_time);
            st.metadata.set ("file_count", s.file_count);
            st.metadata.set ("schema_uri", qs.get_uri ());
            st.metadata.set ("priority", cmanagedsearch.get_priority ());
            st.metadata.set ("flag_active", cmanagedsearch.get_flag_active ());
            st.metadata.set (
                "flag_receive", cmanagedsearch.get_flag_receive ()
            );
            st.metadata.set ("flag_allow_g1", cmanagedsearch.get_allow_g1 ());
            st.metadata.set ("flag_allow_g2", cmanagedsearch.get_allow_g2 ());
            st.metadata.set (
                "flag_allow_ed2k", cmanagedsearch.get_allow_ed2k ()
            );
            st.metadata.set ("flag_allow_dc", cmanagedsearch.get_allow_dc ());
            st.metadata.set ("flag_want_url", qs.get_flag_want_url ());
            st.metadata.set ("flag_want_dn", qs.get_flag_want_dn ());
            st.metadata.set ("flag_want_xml", qs.get_flag_want_xml ());
            st.metadata.set ("flag_want_com", qs.get_flag_want_com ());
            st.metadata.set ("flag_want_pfs", qs.get_flag_want_pfs ());
            st.metadata.set ("min_size", qs.get_min_size ());
            st.metadata.set ("max_size", qs.get_max_size ());
            st.metadata.set (
                "cmanagedsearch_version", cmanagedsearch.get_version ()
            );
            st.metadata.set ("cquerysearch_version", qs.get_version ());

            searched_texts_.push_back (st);
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add remote files
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &search_rf : searches_dat.get_remote_files ())
        {
            const auto &mf = search_rf.match_file;
            const auto &q = search_rf.query_hit;

            remote_file rf;

            // attributes
            rf.timestamp = mf.get_found_time ();
            rf.ip = q.get_ip ();
            rf.port = q.get_port ();
            rf.filename = q.get_name ();
            rf.username = username_;

            // metadata
            rf.metadata.set ("size", mf.get_size ());
            rf.metadata.set ("peer_nickname", q.get_nick ());
            rf.metadata.set ("url", q.get_url ());
            rf.metadata.set ("schema_uri", q.get_schema_uri ());
            rf.metadata.set ("rating", q.get_rating ());
            rf.metadata.set ("comments", q.get_comments ());
            rf.metadata.set ("protocol_id", q.get_protocol_id ());
            rf.metadata.set ("protocol_name", q.get_protocol_name ());
            rf.metadata.set ("search_id", q.get_search_id ());
            rf.metadata.set ("client_id", q.get_client_id ());
            rf.metadata.set ("speed", q.get_speed ());
            rf.metadata.set ("str_speed", q.get_str_speed ());
            rf.metadata.set ("vendor_code", q.get_vendor_code ());
            rf.metadata.set ("vendor_name", q.get_vendor_name ());
            rf.metadata.set ("tri_push", q.get_tri_push ());
            rf.metadata.set ("tri_busy", q.get_tri_busy ());
            rf.metadata.set ("tri_stable", q.get_tri_stable ());
            rf.metadata.set ("tri_measured", q.get_tri_measured ());
            rf.metadata.set ("up_slots", q.get_up_slots ());
            rf.metadata.set ("up_queue", q.get_up_queue ());
            rf.metadata.set ("b_chat", q.get_b_chat ());
            rf.metadata.set ("b_browse_host", q.get_b_browse_host ());
            rf.metadata.set ("b_matched", q.get_b_matched ());
            rf.metadata.set ("b_size", q.get_b_size ());
            rf.metadata.set ("b_preview", q.get_b_preview ());
            rf.metadata.set ("b_collection", q.get_b_collection ());
            rf.metadata.set ("b_bogus", q.get_b_bogus ());
            rf.metadata.set ("b_download", q.get_b_download ());
            rf.metadata.set ("b_exact_match", q.get_b_exact_match ());
            rf.metadata.set ("index", q.get_index ());
            rf.metadata.set ("hit_sources", q.get_hit_sources ());
            rf.metadata.set ("partial", q.get_partial ());
            rf.metadata.set ("schema_plural", q.get_schema_plural ());

            for (const auto &[k, v] : q.get_pxml ().get_metadata ())
                rf.metadata.set (k, v);

            // other data
            rf.thumbnail_data = mf.get_preview ();
            rf.hashes = get_file_hashes (mf);
            rf.f = f;

            // add remote file
            remote_files_.push_back (rf);
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.shareaza.searches_dat"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add Shareaza.db3 file
// @param f Shareaza.db3 file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_shareaza_db3_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto thumbcache = CThumbCache (f.new_reader ());

        if (!thumbcache)
        {
            log.info (
                __LINE__,
                "File is not a valid Shareaza.db3 file. Path: " + f.get_path ()
            );
            return;
        }

        log.info (__LINE__, "File decoded [shareaza.db3]: " + f.get_path ());

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Update account cache, if necessary
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        if (!shareaza_db3_f_ ||
            (shareaza_db3_f_.is_deleted () && !f.is_deleted ()))
        {
            shareaza_db3_f_ = f;
            thumbcache_ = thumbcache;
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.shareaza.shareaza_db3"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
    }
}

} // namespace mobius::extension::app::shareaza
