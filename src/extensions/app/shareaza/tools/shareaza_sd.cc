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
#include "CDownload.h"
#include <mobius/core/application.h>
#include <mobius/io/file.h>
#include <iostream>
#include <unistd.h>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
  std::cout << std::endl;
  std::cout << "use: shareaza_sd [OPTIONS] <path>" << std::endl;
  std::cout << "e.g: shareaza_sd 819F11.sd" << std::endl;
  std::cout << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Show CXMLElement data
//! \param indent Number of indentation tabs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
show_cxmlelement (const mobius::extension::app::shareaza::CXMLElement& e, int indent)
{
  const std::string name = e.get_name ();

  if (name.empty ())
    return;

  std::cout << std::string (indent, '\t') << '<' << name;

  for (const auto& [k, v] : e.get_attributes ())
    std::cout << ' ' << k << "=\"" << v << '"';

  std::cout << '>' << e.get_value () << std::endl;

  for (const auto& child : e)
    show_cxmlelement (child, indent + 1);

  std::cout << std::string (indent, '\t') << "</" << name << '>' << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Show CDownloadSource data
//! \param indent Number of indentation tabs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
show_cdownloadsource (const mobius::extension::app::shareaza::CDownloadSource& s, int indent)
{
  const std::string indent_text = std::string (indent, '\t');

  std::cout << std::endl;
  std::cout << indent_text << "URL: " << s.get_url () << std::endl;
  std::cout << indent_text << "Name: " << s.get_name () << std::endl;
  std::cout << indent_text << "IP: " << s.get_ip () << std::endl;
  std::cout << indent_text << "Port: " << s.get_port () << std::endl;
  std::cout << indent_text << "Last seen time: " << s.get_last_seen_time () << std::endl;
  std::cout << indent_text << "Nick: " << s.get_nick () << std::endl;
  std::cout << indent_text << "Speed: " << s.get_speed ()  << std::endl;
  std::cout << indent_text << "GUID: " << s.get_guid () << std::endl;
  std::cout << indent_text << "Protocol ID: " << s.get_protocol_id () << std::endl;
  std::cout << indent_text << "Protocol name: " << s.get_protocol_name () << std::endl;
  std::cout << indent_text << "Country code: " << s.get_country_code () << std::endl;
  std::cout << indent_text << "Country name: " << s.get_country_name () << std::endl;
  std::cout << indent_text << "Server: " << s.get_server () << std::endl;
  std::cout << indent_text << "Server IP: " << s.get_server_ip () << std::endl;
  std::cout << indent_text << "Server port: " << s.get_server_port () << std::endl;
  std::cout << indent_text << "Index: " << s.get_index () << std::endl;
  std::cout << indent_text << "Flag push only: " << s.get_b_push_only () << std::endl;
  std::cout << indent_text << "Flag close conn: " << s.get_b_close_conn () << std::endl;
  std::cout << indent_text << "Flag read content: " << s.get_b_read_content () << std::endl;
  std::cout << indent_text << "Flag hash auth: " << s.get_b_hash_auth () << std::endl;
  std::cout << indent_text << "Flag hash SHA-1: " << s.get_b_hash_sha1 () << std::endl;
  std::cout << indent_text << "Flag hash Tiger: " << s.get_b_hash_tiger () << std::endl;
  std::cout << indent_text << "Flag hash ED2K: " << s.get_b_hash_ed2k () << std::endl;
  std::cout << indent_text << "Flag hash BTH: " << s.get_b_hash_bth () << std::endl;
  std::cout << indent_text << "Flag hash MD5: " << s.get_b_hash_md5 () << std::endl;
  std::cout << indent_text << "Flag client extended: " << s.get_b_client_extended () << std::endl;
  std::cout << indent_text << "Flag meta ignore: " << s.get_b_meta_ignore () << std::endl;

  std::cout << indent_text << "Fragments:" << std::endl;
  for (const auto& f : s.get_fragments ())
    std::cout << indent_text << '\t' << f.offset << '\t' << f.size << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Show CBTInfo data
//! \param indent Number of indentation tabs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
show_cbtinfo (const mobius::extension::app::shareaza::CBTInfo& bt, int indent)
{
  const std::string indent_text = std::string (indent, '\t');

  std::cout << std::endl;
  std::cout << indent_text << "Version: " << bt.get_version () << std::endl;
  std::cout << indent_text << "Hash BTH: " << bt.get_hash_bth () << std::endl;
  std::cout << indent_text << "Size: " << bt.get_size () << std::endl;
  std::cout << indent_text << "Block size: " << bt.get_block_size () << std::endl;
  std::cout << indent_text << "Block count: " << bt.get_block_count () << std::endl;
  std::cout << indent_text << "Total uploaded: " << bt.get_total_uploaded () << std::endl;
  std::cout << indent_text << "Total downloaded: " << bt.get_total_downloaded () << std::endl;
  std::cout << indent_text << "Name: " << bt.get_name () << std::endl;
  std::cout << indent_text << "Encoding: " << bt.get_encoding () << std::endl;
  std::cout << indent_text << "Comments: " << bt.get_comments () << std::endl;
  std::cout << indent_text << "Creation time: " << bt.get_creation_time () << std::endl;
  std::cout << indent_text << "Created by: " << bt.get_created_by () << std::endl;
  std::cout << indent_text << "Flag private: " << bt.get_b_private () << std::endl;
  std::cout << indent_text << "Main tracker: " << bt.get_main_tracker () << std::endl;
  std::cout << indent_text << "Tracker index: " << bt.get_tracker_index () << std::endl;
  std::cout << indent_text << "Tracker mode: " << bt.get_tracker_mode () << std::endl;

  std::cout << indent_text << "Files:" << std::endl;
  for (const auto& f : bt.get_files ())
    {
      std::cout << std::endl;
      std::cout << indent_text << "\tPath: " << f.path << std::endl;
      std::cout << indent_text << "\tName: " << f.name << std::endl;
      std::cout << indent_text << "\tSize: " << f.size << std::endl;
      std::cout << indent_text << "\tPriority: " << f.priority << std::endl;
      std::cout << indent_text << "\tHash SHA-1: " << f.hash_sha1 << std::endl;
      std::cout << indent_text << "\tHash ED2K: " << f.hash_ed2k << std::endl;
      std::cout << indent_text << "\tHash Tiger: " << f.hash_tiger << std::endl;
      std::cout << indent_text << "\tHash MD5: " << f.hash_md5 << std::endl;
    }

  std::cout << indent_text << "Trackers:" << std::endl;
  for (const auto& t : bt.get_trackers ())
    {
      std::cout << std::endl;
      std::cout << indent_text << "\tType: " << t.type << std::endl;
      std::cout << indent_text << "\tTier: " << t.tier << std::endl;
      std::cout << indent_text << "\tAddress: " << t.address << std::endl;
      std::cout << indent_text << "\tLast access time: " << t.last_access_time << std::endl;
      std::cout << indent_text << "\tLast success time: " << t.last_success_time << std::endl;
      std::cout << indent_text << "\tNext try time: " << t.next_try_time << std::endl;
      std::cout << indent_text << "\tFailures: " << t.failures << std::endl;
    }

  std::cout << indent_text << "Block hashes: " << std::endl;
  for (const auto& h : bt.get_block_hashes ())
    std::cout << indent_text << '\t' << h << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Show Shareaza .sd file info
//! \param path .sd path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_sd (const std::string& path)
{
  std::cout << std::endl;
  std::cout << ">> " << path << std::endl;

  auto f = mobius::io::new_file_by_path (path);
  mobius::extension::app::shareaza::CDownload cdownload (f.new_reader ());

  std::cout << "\tIs instance: " << bool (cdownload) << std::endl;

  if (!cdownload)
    return;

  std::cout << "\tSignature: " << cdownload.get_signature () << std::endl;
  std::cout << "\tVersion: " << cdownload.get_version () << std::endl;
  std::cout << "\tSize: " << cdownload.get_size () << std::endl;
  std::cout << "\tEstimated size: " << cdownload.get_estimated_size () << std::endl;
  std::cout << "\tIs shared: " << cdownload.is_shared () << std::endl;
  std::cout << "\tIs expanded: " << cdownload.is_expanded () << std::endl;
  std::cout << "\tIs paused: " << cdownload.is_paused () << std::endl;
  std::cout << "\tIs boosted: " << cdownload.is_boosted () << std::endl;
  std::cout << "\tIs seeding: " << cdownload.is_seeding () << std::endl;
  std::cout << "\tSer ID: " << cdownload.get_ser_id () << std::endl;
  std::cout << "\tSearch keyword: " << cdownload.get_search_keyword () << std::endl;
  std::cout << "\tHash SHA-1: " << cdownload.get_hash_sha1 () << std::endl;
  std::cout << "\tHash Tiger: " << cdownload.get_hash_tiger () << std::endl;
  std::cout << "\tHash ED2K: " << cdownload.get_hash_ed2k () << std::endl;
  std::cout << "\tHash BTH: " << cdownload.get_hash_bth () << std::endl;
  std::cout << "\tHash MD5: " << cdownload.get_hash_md5 () << std::endl;
  std::cout << "\tLocal name: " << cdownload.get_local_name () << std::endl;
  std::cout << "\tName: " << cdownload.get_name () << std::endl;
  std::cout << "\tTorrent success: " << cdownload.get_torrent_success () << std::endl;
  std::cout << "\tServing file name: " << cdownload.get_serving_file_name () << std::endl;

  std::cout << "\tPXML:" << std::endl;
  show_cxmlelement (cdownload.get_pxml (), 2);

  std::cout << "\tBTInfo:" << std::endl;
  show_cbtinfo (cdownload.get_btinfo (), 2);

  std::cout << "\tSources:" << std::endl;
  for (const auto& source : cdownload.get_sources ())
    show_cdownloadsource (source, 2);

  std::cout << "\tParts:" << std::endl;
  for (const auto& p : cdownload.get_parts ())
    {
      std::cout << std::endl;
      std::cout << "\t\tPath: " << p.path << std::endl;
      std::cout << "\t\tName: " << p.name << std::endl;
      std::cout << "\t\tOffset: " << p.offset << std::endl;
      std::cout << "\t\tSize: " << p.size << std::endl;
      std::cout << "\t\tPriority: " << p.priority << std::endl;
      std::cout << "\t\tFlag write: " << p.b_write << std::endl;
    }

  std::cout << "\tFragments:" << std::endl;
  for (const auto& f : cdownload.get_fragments ())
    std::cout << "\t\t" << f.offset << '\t' << f.size << std::endl;
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief main function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
main (int argc, char **argv)
{
  mobius::core::application app;
  std::cout << app.get_name () << " v" << app.get_version () << std::endl;
  std::cout << app.get_copyright () << std::endl;
  std::cout << "Shareaza SD file viewer v1.0" << std::endl;
  std::cout << "by Eduardo Aguiar" << std::endl;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // parse command line
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  int opt;

  while ((opt = getopt (argc, argv, "h")) != EOF)
    {
      switch (opt)
        {
        case 'h':
          usage ();
          exit (EXIT_SUCCESS);
          break;

        default:
          usage ();
          exit (EXIT_FAILURE);
        }
    }

  if (optind >= argc)
    {
      std::cout << std::endl;
      std::cout << "Error: you must enter at least one path to .sd file" << std::endl;
      usage ();
      exit (EXIT_FAILURE);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // show hive info
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  while (optind < argc)
    {
      try
        {
          show_sd (argv[optind]);
        }
      catch (const std::exception& e)
        {
          std::cout <<  "Error: " << e.what () << std::endl;
          exit (EXIT_FAILURE);
        }

      optind++;
    }

  return EXIT_SUCCESS;
}
