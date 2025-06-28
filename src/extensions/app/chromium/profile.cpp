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
#include "profile.hpp"
#include <algorithm>
#include "file_web_data.hpp"
// #include <mobius/core/file_decoder/torrent.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/value_selector.hpp>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @see
// https://github.com/obsidianforensics/hindsight/blob/main/documentation/Evolution%20of%20Chrome%20Databases%20(v35).pdf
// @see
// https://medium.com/@jsaxena017/web-browser-forensics-part-1-chromium-browser-family-99b807083c25
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
namespace mobius::extension::app::chromium
{ /*
 // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 // @brief Get accounts
 // @return vector of accounts
 // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 std::vector<profile::account>
 profile::get_accounts () const
 {
     std::vector<account> accounts;

     std::transform (accounts_.begin (), accounts_.end (),
                     std::back_inserter (accounts),
                     [] (const auto &pair) { return pair.second; });

     std::sort (accounts.begin (), accounts.end (),
                [] (const auto &a, const auto &b)
                { return a.client_id < b.client_id; });

     return accounts;
 }

 // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 // @brief Get local files
 // @return vector of local files
 // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 std::vector<profile::local_file>
 profile::get_local_files () const
 {
     std::vector<local_file> local_files;

     std::transform (local_files_.begin (), local_files_.end (),
                     std::back_inserter (local_files),
                     [] (const auto &pair) { return pair.second; });

     std::sort (local_files.begin (), local_files.end (),
                [] (const auto &a, const auto &b)
                { return a.torrent_name < b.torrent_name; });

     return local_files;
 }*/

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add Preferences file
// @param f Preferences file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_preferences_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto reader = f.new_reader ();
    if (!reader)
        return;
    /*
        file_dht_dat dht_dat (reader);
        if (!dht_dat)
        {
            log.warning (__LINE__, "File is not a valid dht.dat file");
            return;
        }

        log.info (__LINE__, "File " + f.get_path () + " is a valid dht.dat
       file");

        //
       =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add account
        //
       =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        auto client_id = dht_dat.get_client_id ();
        if (client_id.empty ())
        {
            log.warning (__LINE__, "Client ID is empty");
            return;
        }

        auto [iter, _] = accounts_.try_emplace (client_id);
        auto &acc = iter->second;
        std::ignore = _;

        acc.client_id = client_id;

        if (!acc.first_dht_timestamp ||
            dht_dat.get_timestamp () < acc.first_dht_timestamp)
            acc.first_dht_timestamp = dht_dat.get_timestamp ();

        if (!acc.last_dht_timestamp ||
            dht_dat.get_timestamp () > acc.last_dht_timestamp)
            acc.last_dht_timestamp = dht_dat.get_timestamp ();

        acc.files.push_back (f);
        acc.ip_addresses.emplace (dht_dat.get_ip_address (),
                                  dht_dat.get_timestamp ());

        //
       =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Get data from file
        // @note Always accept the first file found;
        // Prefer non-deleted files over deleted ones;
        // When deletion status is the same, prefer files named exactly
       "dht.dat"
        //
       =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        if (!acc.f || (acc.f.is_deleted () && !f.is_deleted ()) ||
            (acc.f.is_deleted () == f.is_deleted () &&
             acc.f.get_name () != "dht.dat" && f.get_name () == "dht.dat"))
        {
            acc.f = f;
        }
    */
    is_valid_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add Web Data file
// @param f Web Data file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_web_data_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    file_web_data web_data (f.new_reader());

    if (!web_data)
    {
        log.info (__LINE__, "File is not a valid 'Web Data' file");
        return;
    }

    log.info (
        __LINE__,
        "File " + f.get_path () + " is a valid 'Web Data' file"
    );

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add autofill entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &entry : web_data.get_autofill_entries ())
    {
        autofill a;

        a.idx = entry.idx;
        a.name = entry.name;
        a.value = entry.value;
        a.count = entry.count;
        a.date_created = entry.date_created;
        a.date_last_used = entry.date_last_used;
        a.username = username_;
        a.is_encrypted = entry.is_encrypted;
        a.f = f;

        autofill_.push_back (a);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add credit cards
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &card : web_data.get_credit_cards ())
    {
        credit_card c;
        c.idx = card.idx;
        c.card_number = card.card_number;
        c.card_number_encrypted = card.card_number_encrypted;
        c.name_on_card = card.name_on_card;
        c.expiration_month = card.expiration_month;
        c.expiration_year = card.expiration_year;
        c.cvv = card.cvv;
        c.origin = card.origin;
        c.use_count = card.use_count;
        c.use_date = card.use_date;
        c.nickname = card.nickname;
        c.type = card.type;
        c.network = card.network;
        c.bank_name = card.bank_name;
        c.card_issuer = card.card_issuer;
        c.metadata = card.metadata;
        c.date_modified = card.date_modified;
        c.unmask_date = card.unmask_date;
        c.f = f;

        credit_cards_.push_back (c);
    }

    is_valid_ = true;
}

/*
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add settings.dat file
// @param f Settings.dat file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_settings_dat_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto reader = f.new_reader ();
    if (!reader)
        return;

    file_settings_dat settings_dat (reader);
    if (!settings_dat)
    {
        log.warning (__LINE__, "File is not a valid settings.dat file");
        return;
    }

    log.info (__LINE__,
              "File " + f.get_path () + " is a valid settings.dat file");

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Create settings object
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    settings s;

    s.computer_id = settings_dat.get_computer_id ();
    s.auto_start = settings_dat.get_autostart ();
    s.total_bytes_downloaded = settings_dat.get_total_bytes_downloaded ();
    s.total_bytes_uploaded = settings_dat.get_total_bytes_uploaded ();
    s.installation_time = settings_dat.get_installation_time ();
    s.last_used_time = settings_dat.get_last_used_time ();
    s.last_bin_change_time = settings_dat.get_last_bin_change_time ();
    s.execution_count = settings_dat.get_execution_count ();
    s.version = settings_dat.get_version ();
    s.installation_version = settings_dat.get_installation_version ();
    s.language = settings_dat.get_language ();
    s.f = f;

    settings_.push_back (s);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Get data from file
    // @note Always accept the first file found;
    // Prefer non-deleted files over deleted ones;
    // When deletion status is the same, prefer files named exactly
    // "settings.dat"
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (!main_settings_.f ||
        (main_settings_.f.is_deleted () && !f.is_deleted ()) ||
        (main_settings_.f.is_deleted () == f.is_deleted () &&
         main_settings_.f.get_name () != "settings.dat" &&
         f.get_name () == "settings.dat"))
    {
        main_settings_ = s;
        is_valid_ = true;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add torrent file
// @param f Torrent file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_torrent_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto reader = f.new_reader ();
    if (!reader)
        return;

    mobius::core::file_decoder::torrent torrent (reader);
    if (!torrent)
    {
        log.warning (__LINE__, "File is not a valid torrent file");
        return;
    }

    log.info (__LINE__, "File " + f.get_path () + " is a valid torrent file");

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Add torrent file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    local_file &lf = local_files_[f.get_name ()];

    bool overwrite =
        !lf.torrent_file || (lf.torrent_file.is_deleted () && !f.is_deleted ());
    mobius::core::value_selector vs (overwrite);

    lf.creation_time = vs (lf.creation_time, torrent.get_creation_time ());
    lf.torrent_file = vs (lf.torrent_file, f);
    lf.blocksize = vs (lf.blocksize, torrent.get_piece_length ());
    lf.torrent_name = vs (lf.torrent_name, torrent.get_name ());
    lf.size = vs (lf.size, torrent.get_length ());
    lf.created_by = vs (lf.created_by, torrent.get_created_by ());
    lf.encoding = vs (lf.encoding, torrent.get_encoding ());
    lf.comment = vs (lf.comment, torrent.get_comment ());
    lf.info_hash = vs (lf.info_hash, torrent.get_info_hash ());

    std::vector<torrent_content_file> content_files;
    auto torrent_files = torrent.get_files ();

    std::transform (torrent_files.begin (), torrent_files.end (),
                    std::back_inserter (content_files),
                    [] (const auto &file)
                    {
                        return torrent_content_file {
                            file.name,         file.path,
                            file.length,       file.offset,
                            file.piece_length, file.piece_offset,
                            file.creation_time};
                    });
    lf.content_files = vs (lf.content_files, content_files);
}
*/
} // namespace mobius::extension::app::chromium
