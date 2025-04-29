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
#include <mobius/core/application.hpp>
#include <mobius/core/resource.hpp>
#include <mobius/string_functions.h>
#include <mobius/core/vfs/vfs.hpp>
#include <iostream>
#include <unistd.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
  std::cerr << std::endl;
  std::cerr << "Use: filesystem_scan [OPTIONS] <URL>" << std::endl;
  std::cerr << "e.g: filesystem_scan -t raw file://disk.raw" << std::endl;
  std::cerr << "     filesystem_scan file://disk.ewf" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Options are:" << std::endl;
  std::cerr << "  -t imagefile type\t\t" << std::endl;
  std::cerr << std::endl;
  std::cerr << "     Image file type can be:" << std::endl;
  std::cerr << "       autodetect\tTry to autodetect imagefile type (default)" << std::endl;

  for (const auto& r : mobius::core::get_resources ("vfs.imagefile"))
    std::cerr << "       " << r.get_id () << "\t\t" << r.get_description () << std::endl;

  std::cerr << std::endl;
  std::cerr << "Supported filesystems:" << std::endl;

  for (const auto& r : mobius::core::get_resources ("vfs.filesystem"))
    std::cerr << "       " << r.get_id () << "\t\t" << r.get_description () << std::endl;

  std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief main function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
main (int argc, char **argv)
{
  mobius::core::application app;
  app.start ();

  std::cerr << app.get_name () << " v" << app.get_version () << std::endl;
  std::cerr << app.get_copyright () << std::endl;
  std::cerr << "Filesystem Scan v1.3" << std::endl;
  std::cerr << "by Eduardo Aguiar" << std::endl;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // parse command line
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  int opt;
  std::string type = "autodetect";

  while ((opt = getopt (argc, argv, "ht:")) != EOF)
    {
      switch (opt)
        {
        case 'h':
          usage ();
          exit (EXIT_SUCCESS);
          break;

        case 't':
          type = optarg;
          break;

        default:
          usage ();
          exit (EXIT_FAILURE);
        }
    }

  if (optind >= argc)
    {
      std::cerr << std::endl;
      std::cerr << "Error: you must enter a valid URL to an imagefile" << std::endl;
      usage ();
      exit (EXIT_FAILURE);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create VFS
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::core::vfs::vfs vfs;
  vfs.add_disk (mobius::core::vfs::new_disk_by_url (argv[optind], type));

  if (!vfs.is_available ())
    {
      std::cerr << std::endl;
      std::cerr << "Error: VFS is not available" << std::endl;
      usage ();
      exit (EXIT_FAILURE);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Show filesystems
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& block : vfs.get_blocks ())
    {
      if (block.get_type () == "filesystem")
        {
          std::cout << std::endl;

          for (const auto& [name, value] : block.get_attributes ())
            {
              std::string key = mobius::string::replace (mobius::string::capitalize (name), "_", " ");
              std::cout << "   " << key << ": " << value.to_string () << std::endl;
            }
        }
    }

  app.stop ();
}
