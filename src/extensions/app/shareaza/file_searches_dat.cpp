// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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
#include "file_searches_dat.hpp"
#include "CWndSearch.hpp"
#include <mobius/decoder/mfc.h>
#include <unordered_map>

namespace mobius::extension::app::shareaza
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
file_searches_dat::file_searches_dat (const mobius::io::reader& reader)
{
  if (!reader || reader.get_size () < 2)
    return;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create main section
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto decoder = mobius::decoder::mfc (reader);
  decoder.seek (0);

  section_ = mobius::core::file_decoder::section (reader, "File");

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Decode entries
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::uint32_t count = decoder.get_count ();
  std::uint64_t idx = 0;

  if (count > 1)
    return;

  while (count)
    {
      // decode CWndSearch structure
      auto entry_section = section_.new_child ("Entry #" + std::to_string (++idx));

      CWndSearch cwndsearch;
      cwndsearch.decode (decoder);
      process_cwndsearch_ (cwndsearch);

      entry_section.end ();

      count = decoder.get_count ();
    }

  section_.end ();
  is_instance_ = true;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Process CndSearch structure
// @param cwndsearch CWndSearch structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
file_searches_dat::process_cwndsearch_ (const CWndSearch& cwndsearch)
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Remote file stats
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  struct remote_file_stats
  {
    mobius::datetime::datetime first_hit_time;
    mobius::datetime::datetime last_hit_time;
    std::uint32_t count = 0;
  };

  std::unordered_map <std::string, remote_file_stats> rf_stats;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Process match files
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto cmatchlist = cwndsearch.get_match_list ();

  for (const auto& mf : cmatchlist.get_match_files ())
    {
      for (const auto& q : mf.get_query_hits ())
        {
          remote_file rf;
          rf.match_file = mf;
          rf.query_hit = q;

          remote_files_.push_back (rf);

          // Fill remote file stats
          auto search_id = q.get_search_id ();
          auto timestamp = mf.get_found_time ();
          auto p = rf_stats.try_emplace (search_id);
          auto& rfc_stat = p.first->second;

          rfc_stat.count++;

          if (!rfc_stat.first_hit_time || rfc_stat.first_hit_time > timestamp)
            rfc_stat.first_hit_time = timestamp;

          if (!rfc_stat.last_hit_time || rfc_stat.last_hit_time < timestamp)
            rfc_stat.last_hit_time = timestamp;
        }
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Process searches
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& search : cwndsearch.get_searches ())
    {
      auto qs = search.get_query_search ();

      file_searches_dat::search s;
      s.obj = search;
      s.id = qs.get_guid ();
      s.text = qs.get_text ();

      // Get data from remote files
      auto iter = rf_stats.find (s.id);

      if (iter != rf_stats.end ())
        {
          s.timestamp = iter->second.first_hit_time;
          s.first_hit_time = iter->second.first_hit_time;
          s.last_hit_time = iter->second.last_hit_time;
          s.file_count = iter->second.count;
        }

      searches_.push_back (s);
    }
}

} // namespace mobius::extension::app::shareaza




