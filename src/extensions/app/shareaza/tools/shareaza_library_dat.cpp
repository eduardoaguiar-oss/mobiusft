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
#include "CLibrary.hpp"
#include <mobius/core/application.h>
#include <mobius/io/file.h>
#include <iostream>
#include <unistd.h>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
  std::cerr << std::endl;
  std::cerr << "use: shareaza_library_dat [OPTIONS] <path>" << std::endl;
  std::cerr << "e.g: shareaza_library_dat Library1.dat" << std::endl;
  std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show CXMLElement data
// @param indent Number of indentation tabs
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
// @brief Show CSharedSource data
// @param indent Number of indentation tabs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_csharedsource (const mobius::extension::app::shareaza::CSharedSource& s, int indent)
{
  const std::string indent_text (indent, '\t');

  std::cout << std::endl;
  std::cout << indent_text << "Timestamp: " << s.get_timestamp () << std::endl;
  std::cout << indent_text << "URL: " << s.get_url () << std::endl;
  std::cout << indent_text << "IP: " << s.get_ip () << std::endl;
  std::cout << indent_text << "Port: " << s.get_port () << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show CLibraryFile data
// @param indent Number of indentation tabs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_clibraryfile (const mobius::extension::app::shareaza::CLibraryFile& f, int indent)
{
  const std::string indent_text (indent, '\t');

  std::cout << std::endl;
  std::cout << indent_text << "File name: " << f.get_name () << std::endl;
  std::cout << indent_text << "Path: " << f.get_path () << std::endl;
  std::cout << indent_text << "Index: " << f.get_index () << std::endl;
  std::cout << indent_text << "Size: " << f.get_size () << std::endl;
  std::cout << indent_text << "Last modification time: " << f.get_last_modification_time () << std::endl;
  std::cout << indent_text << "Size: " << f.get_size () << std::endl;
  std::cout << indent_text << "Virtual size: " << f.get_virtual_size () << std::endl;
  std::cout << indent_text << "Virtual base: " << f.get_virtual_base () << std::endl;
  std::cout << indent_text << "Is shared: " << f.is_shared () << std::endl;
  std::cout << indent_text << "Hash SHA-1: " << f.get_hash_sha1 () << std::endl;
  std::cout << indent_text << "Hash Tiger: " << f.get_hash_tiger () << std::endl;
  std::cout << indent_text << "Hash ED2K: " << f.get_hash_ed2k () << std::endl;
  std::cout << indent_text << "Hash MD5: " << f.get_hash_md5 () << std::endl;
  std::cout << indent_text << "Hash BTH: " << f.get_hash_bth () << std::endl;
  std::cout << indent_text << "URI: " << f.get_uri () << std::endl;
  std::cout << indent_text << "B_Verify: " << f.get_b_verify () << std::endl;
  std::cout << indent_text << "B_Cached preview: " << f.get_b_cached_preview () << std::endl;
  std::cout << indent_text << "B_Bogus: " << f.get_b_bogus () << std::endl;
  std::cout << indent_text << "B_metadata_auto: " << f.get_b_metadata_auto () << std::endl;
  std::cout << indent_text << "Metadata date/time: " << f.get_metadata_time () << std::endl;
  std::cout << indent_text << "Rating: " << f.get_rating () << std::endl;
  std::cout << indent_text << "Comments: " << f.get_comments () << std::endl;
  std::cout << indent_text << "Share tags: " << f.get_share_tags () << std::endl;
  std::cout << indent_text << "Hits total: " << f.get_hits_total () << std::endl;
  std::cout << indent_text << "Uploads started: " << f.get_uploads_started () << std::endl;

  std::cout << indent_text << "PXML: " << std::endl;
  show_cxmlelement (f.get_pxml (), indent + 1);

  std::cout << indent_text << "Sources: " << std::endl;

  for (const auto& s : f.get_sources ())
    show_csharedsource (s, indent + 1);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show CLibraryFolder data
// @param indent Number of indentation tabs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_clibraryfolder (const mobius::extension::app::shareaza::CLibraryFolder& folder, int indent = 1)
{
  const std::string indent_text (indent, '\t');

  std::cout << std::endl;
  std::cout << indent_text << "+ " << folder.get_path () << std::endl;
  std::cout << indent_text << "  Is shared: " << folder.is_shared () << std::endl;
  std::cout << indent_text << "  Is expanded: " << folder.is_expanded () << std::endl;
  std::cout << indent_text << "  Files:" << std::endl;

  for (const auto& f : folder.get_files ())
    show_clibraryfile (f, indent + 1);

  for (const auto& child: folder.get_children ())
    show_clibraryfolder (child, indent + 1);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show Shareaza Library.dat info
// @param path Library.dat path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_clibrary (const std::string& path)
{
  std::cout << std::endl;
  std::cout << ">> " << path << std::endl;

  auto f = mobius::io::new_file_by_path (path);
  auto decoder = mobius::decoder::mfc (f.new_reader ());
  auto clib = mobius::extension::app::shareaza::CLibrary (decoder);

  std::cout << std::endl;
  std::cout << ">> " << path << std::endl;
  std::cout << "\tLast modification time: " << clib.get_last_modification_time () << std::endl;
  std::cout << "\tVersion: " << clib.get_version () << std::endl;
  std::cout << "\tNext index: " << clib.get_next_index () << std::endl;
  std::cout << "\tIndex map count: " << clib.get_index_map_count () << std::endl;
  std::cout << "\tName map count: " << clib.get_name_map_count () << std::endl;
  std::cout << "\tPath map count: " << clib.get_path_map_count () << std::endl;

  for (const auto& folder: clib.get_folders ())
    show_clibraryfolder (folder);
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Main function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
main (int argc, char **argv)
{
  mobius::core::application app;
  std::cerr << app.get_name () << " v" << app.get_version () << std::endl;
  std::cerr << app.get_copyright () << std::endl;
  std::cerr << "Shareaza Library.dat viewer v1.0" << std::endl;
  std::cerr << "by Eduardo Aguiar" << std::endl;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Parse command line
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
      std::cerr << std::endl;
      std::cerr << "Error: you must enter at least one path to Library.dat file" << std::endl;
      usage ();
      exit (EXIT_FAILURE);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Show files info
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  while (optind < argc)
    {
      try
        {
          show_clibrary (argv[optind]);
        }
      catch (const std::exception& e)
        {
          std::cerr <<  "Error: " << e.what () << std::endl;
          exit (EXIT_FAILURE);
        }

      optind++;
    }

  return EXIT_SUCCESS;
}


