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
#include "../file_cancelled_met.hpp"
#include <mobius/core/application.h>
#include <mobius/core/log.h>
#include <mobius/io/file.h>
#include <mobius/string_functions.h>
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
  std::cerr << "use: emule_cancelled_met [OPTIONS] <path>" << std::endl;
  std::cerr << "e.g: emule_cancelled_met Cancelled.met" << std::endl;
  std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show CCancelledFile data
// @param f CCancelledFile data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
show_file (const mobius::extension::app::emule::file_cancelled_met::CCancelledFile& f)
{
  std::cerr << std::endl;
  std::cerr << "\t\t>> File" << std::endl;
  std::cerr << "\t\tHash ED2K: " << f.hash_ed2k << std::endl;
  std::cerr << "\t\t\tTags: " << f.tags.size () << std::endl;

  for (const auto& tag : f.tags)
    {
        std::cerr   << "\t\t\t\t"
                    << int (tag.get_id ()) << '\t'
                    << mobius::string::to_hex (tag.get_type (), 2) << '\t'
                    << tag.get_name () << '\t'
                    << tag.get_value ().to_string () << std::endl;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show Emule Cancelled.met info
// @param path Cancelled.met path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_cancelled_met (const std::string& path)
{
  std::cout << std::endl;
  std::cout << ">> " << path << std::endl;

  auto f = mobius::io::new_file_by_path (path);
  auto reader = f.new_reader ();
  
  mobius::extension::app::emule::file_cancelled_met met (reader);
  if (!met)
    {
      std::cerr << "\tFile is not an instance of Cancelled.met" << std::endl;
      return;
    }

  for (const auto& f : met.get_cancelled_files ())
    show_file (f);
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
  std::cerr << "Emule Cancelled.met viewer v1.0" << std::endl;
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
      std::cerr << "Error: you must enter at least one path to Cancelled.met file" << std::endl;
      usage ();
      exit (EXIT_FAILURE);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Show info
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  while (optind < argc)
    {
      try
        {
          show_cancelled_met (argv[optind]);
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
