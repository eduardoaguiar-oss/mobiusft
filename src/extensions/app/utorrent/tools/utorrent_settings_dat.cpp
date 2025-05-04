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
#include "../file_settings_dat.hpp"
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
  std::cerr << "use: utorrent_file_settings_dat [OPTIONS] <path>" << std::endl;
  std::cerr << "e.g: utorrent_file_settings_dat settings.dat" << std::endl;
  std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show uTorrent settings.dat info
// @param path settings.dat path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_settings_dat (const std::string& path)
{
  std::cout << std::endl;
  std::cout << ">> " << path << std::endl;

  auto f = mobius::core::io::new_file_by_path (path);
  auto reader = f.new_reader ();
  
  mobius::extension::app::utorrent::file_settings_dat settings (reader);
  if (!settings)
    {
      std::cerr << "\tFile is not an instance of settings.dat" << std::endl;
      return;
    }

  std::cout << "\tTotal downloaded bytes: " << settings.get_total_bytes_downloaded() << std::endl;
  std::cout << "\tTotal uploaded bytes: " << settings.get_total_bytes_uploaded() << std::endl;
  std::cout << "\tAutostart flag: " << (settings.get_autostart() ? "Enabled" : "Disabled") << std::endl;
  std::cout << "\tComputer ID: " << settings.get_computer_id() << std::endl;
  std::cout << "\tInstallation time: " << settings.get_installation_time() << std::endl;
  std::cout << "\tLast used time: " << settings.get_last_used_time() << std::endl;
  std::cout << "\tLast bin change time: " << settings.get_last_bin_change_time() << std::endl;
  std::cout << "\tExecution count: " << settings.get_execution_count() << std::endl;
  std::cout << "\tVersion: " << settings.get_version() << std::endl;
  std::cout << "\tInstallation version: " << settings.get_installation_version() << std::endl;
  std::cout << "\tLanguage: " << settings.get_language() << std::endl;
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
  std::cerr << "Emule Known.met viewer v1.0" << std::endl;
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
      std::cerr << "Error: you must enter at least one path to Known.met file" << std::endl;
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
          show_settings_dat(argv[optind]);
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
