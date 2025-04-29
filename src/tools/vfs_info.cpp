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
#include <mobius/core/log.hpp>
#include <mobius/core/resource.hpp>
#include <mobius/core/vfs/vfs.hpp>
#include <iostream>
#include <unistd.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
  std::cerr << std::endl;
  std::cerr << "Use: vfs_info [OPTIONS] <URL>" << std::endl;
  std::cerr << "e.g: vfs_info -t raw file://disk.raw" << std::endl;
  std::cerr << "     vfs_info file://disk.ewf" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Options are:" << std::endl;
  std::cerr << "  -t imagefile type\t\t" << std::endl;
  std::cerr << std::endl;
  std::cerr << "     Image file type can be:" << std::endl;
  std::cerr << "       autodetect\tTry to autodetect imagefile type (default)" << std::endl;

  for (const auto& r : mobius::core::get_resources ("vfs.imagefile"))
    std::cerr << "       " << r.get_id () << "\t\t" << r.get_description () << std::endl;

  std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Show VFS block
//! \param block Block object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_block (const mobius::core::vfs::block& block)
{
  std::cout << std::endl;
  std::cout << "  UID: " << block.get_uid () << std::endl;
  std::cout << "  Is handled: " << block.is_handled () << std::endl;
  std::cout << "  Is complete: " << block.is_complete () << std::endl;
  std::cout << "  Is available: " << block.is_available () << std::endl;
  std::cout << "  Type: " << block.get_type () << std::endl;

  std::cout << "  Parents: ";
  for (const auto& p : block.get_parents ())
    std::cout << p.get_uid () << ' ';
  std::cout << std::endl;

  std::cout << "  Children: ";
  for (const auto& c : block.get_children ())
    std::cout << c.get_uid () << ' ';
  std::cout << std::endl;

  for (const auto& c : block.get_children ())
    std::cout << "    " << c.get_uid () << ' ' << c.get_type () << ' ' << c.get_attributes () << std::endl;

  std::cout << "  Attributes: " << std::endl;

  for (const auto& [name, value] : block.get_attributes ())
    {
      std::cout << "    " << name << '\t' << value.to_string () << std::endl;
    }

  std::cout << "  State: " << block.get_state () << std::endl;

  std::cout << std::endl;
  std::cout << "  Data (first 512 bytes):" << std::endl;

  auto reader = block.new_reader ();
  std::cout << reader.read (512).dump (4) << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Show VFS
//! \param vfs VFS object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_vfs (const mobius::core::vfs::vfs& vfs)
{
  std::cout << std::endl;
  std::cout << "Is available: " << vfs.is_available () << std::endl;

  std::cout << std::endl;
  std::cout << "Disks:" << std::endl;

  for (const auto& d : vfs.get_disks ())
    {
      std::cout << d.get_type () << std::endl;

      for (const auto& p : d.get_attributes ())
        std::cout << "    " << p.first << ": " << p.second << std::endl;

      std::cout << "    " << d.get_path () << std::endl;
      std::cout << "    " << d.get_state () << std::endl;

      auto reader = d.new_reader ();
      std::cout << std::endl;
      std::cout << reader.read (512).dump (4) << std::endl;
    }

  std::cout << std::endl;
  std::cout << "Blocks:" << std::endl;

  for (const auto& b: vfs.get_blocks ())
    {
      show_block (b);
    }

  std::cout << std::endl;
  std::cout << "State:" << std::endl;
  std::cout << vfs.get_state () << std::endl;
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
  std::cerr << "VFS info v1.0" << std::endl;
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
  // Show VFS info
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  show_vfs (vfs);

  app.stop ();
}
