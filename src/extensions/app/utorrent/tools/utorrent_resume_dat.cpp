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
#include "../file_resume_dat.hpp"
#include <mobius/core/application.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/string_functions.hpp>
#include <iostream>
#include <unistd.h>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
  std::cerr << std::endl;
  std::cerr << "use: utorrent_file_resume_dat [OPTIONS] <path>" << std::endl;
  std::cerr << "e.g: utorrent_file_resume_dat resume.dat" << std::endl;
  std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show uTorrent resume.dat info
// @param path resume.dat path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_resume_dat (const std::string& path)
{
    std::cout << std::endl;
    std::cout << ">> " << path << std::endl;

    auto f = mobius::core::io::new_file_by_path (path);
    auto reader = f.new_reader ();

    mobius::extension::app::utorrent::file_resume_dat dat (reader);
    if (!dat)
    {
        std::cerr << "\tFile is not an instance of resume.dat" << std::endl;
        return;
    }

    for(const auto& entry : dat.get_entries())
    {
        std::cout << std::endl;
        std::cout << "\tEntry name: " << entry.name << std::endl;
        std::cout << "\tTorrent name: " << entry.torrent_name << std::endl;
        std::cout << "\tDownload URL: " << entry.download_url << std::endl;
        std::cout << "\tCaption: " << entry.caption << std::endl;
        std::cout << "\tPath: " << entry.path << std::endl;
        std::cout << "\tSeeded seconds: " << entry.seeded_seconds << std::endl;
        std::cout << "\tDownloaded seconds: " << entry.downloaded_seconds << std::endl;
        std::cout << "\tBlock size: " << entry.blocksize << std::endl;
        std::cout << "\tBytes downloaded: " << entry.bytes_downloaded << std::endl;
        std::cout << "\tBytes uploaded: " << entry.bytes_uploaded << std::endl;
        std::cout << "\tMetadata time: " << entry.metadata_time << std::endl;
        std::cout << "\tAdded time: " << entry.added_time << std::endl;
        std::cout << "\tCompleted time: " << entry.completed_time << std::endl;
        std::cout << "\tLast seen complete time: " << entry.last_seen_complete_time << std::endl;

        std::cout << "\tPeers: " << std::endl;
        for (const auto& [ip, port] : entry.peers)
            std::cout << "\t\t" << ip << ":" << port << std::endl;
    }
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Main function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
main (int argc, char **argv)
{
  mobius::core::application app;
  mobius::core::set_logfile_path ("mobius.log");

  app.start ();

  std::cerr << app.get_name () << " v" << app.get_version () << std::endl;
  std::cerr << app.get_copyright () << std::endl;
  std::cerr << "µTorrent resume.dat viewer v1.0" << std::endl;
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
      std::cerr << "Error: you must enter at least one path to resume.dat file" << std::endl;
      usage ();
      exit (EXIT_FAILURE);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Show hive info
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  while (optind < argc)
    {
      try
        {
          show_resume_dat(argv[optind]);
        }
      catch (const std::exception& e)
        {
          std::cerr <<  "Error: " << e.what () << std::endl;
          exit (EXIT_FAILURE);
        }

      optind++;
    }

  app.stop ();

  return EXIT_SUCCESS;
}
