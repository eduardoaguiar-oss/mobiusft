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
#include <mobius/io/entry.h>
#include <mobius/io/file.h>
#include <mobius/io/folder.h>
#include <mobius/io/stream.h>
#include <mobius/core/vfs/vfs.hpp>
#include <iostream>
#include <unistd.h>

void show_entry (const mobius::io::entry&, const std::string&);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Shows streams
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_streams (const std::vector <mobius::io::stream>& streams)
{
  int i = 1;
  std::cout << "     streams:" << std::endl;

  for (const auto& stream : streams)
    {
      std::cout << "              stream " << i << ": " << stream.get_type ()
                << ", size: " << stream.get_size () << ", name: " << stream.get_name ()
                << std::endl;

      try
        {
          auto reader = stream.new_reader ();
          auto data = reader.read (16);

          std::cout << "                        ";
          for (std::size_t j = 0; j < data.size (); j++)
            printf ("%02x ", data[j]);

          std::cout << std::endl;
        }
      catch (const std::runtime_error& e)
        {
          std::cerr << "warning: " << e.what () << std::endl;
        }

      ++i;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Show file information
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_file (const mobius::io::file& f, const std::string& parent_path)
{
  // metadata
  std::cout << std::endl;
  std::cout << "   [" << f.get_inode () << "] ";

  if (f.is_deleted ())
    std::cout << "<DEL> ";

  std::cout << parent_path << '/' << f.get_name () << std::endl;

  std::cout << "        size: " << f.get_size () << std::endl;
  std::cout << "        type: " << static_cast <int> (f.get_type ()) << std::endl;
  std::cout << "        name: " << f.get_name () << std::endl;
  std::cout << "       atime: " << f.get_access_time () << std::endl;
  std::cout << "       mtime: " << f.get_modification_time () << std::endl;
  std::cout << "       ctime: " << f.get_metadata_time () << std::endl;
  std::cout << "      crtime: " << f.get_creation_time () << std::endl;
  std::cout << "       dtime: " << f.get_deletion_time () << std::endl;
  std::cout << "     bkptime: " << f.get_backup_time () << std::endl;

  // parent
  auto parent = f.get_parent ();

  if (parent)
    {
      std::cout << " parent name: " << parent.get_name () << std::endl;
      std::cout << " parent path: " << parent_path << std::endl;
    }

  // streams
  show_streams (f.get_streams ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Show folder information
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_folder (const mobius::io::folder& f, const std::string& parent_path)
{
  // metadata
  std::cout << std::endl;
  std::cout << " + [" << f.get_inode () << "] ";

  if (f.is_deleted ())
    std::cout << "<DEL> ";

  std::string path = parent_path;
  if (!f.get_name ().empty ())
    path += '/' + f.get_name ();

  std::cout << path << std::endl;

  std::cout << "        size: " << f.get_size () << std::endl;
  std::cout << "        name: " << f.get_name () << std::endl;
  std::cout << "       atime: " << f.get_access_time () << std::endl;
  std::cout << "       mtime: " << f.get_modification_time () << std::endl;
  std::cout << "       ctime: " << f.get_metadata_time () << std::endl;
  std::cout << "      crtime: " << f.get_creation_time () << std::endl;
  std::cout << "       dtime: " << f.get_deletion_time () << std::endl;

  // parent
  auto parent = f.get_parent ();

  if (parent)
    {
      std::cout << " parent name: " << parent.get_name () << std::endl;
      std::cout << " parent path: " << parent_path << std::endl;
    }

  // streams
  show_streams (f.get_streams ());

  // children
  for (const auto& child : f.get_children ())
    show_entry (child, path);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Show entry information
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_entry (const mobius::io::entry& entry, const std::string& parent_path)
{
  try
    {
      if (entry.is_file ())
        show_file (entry.get_file (), parent_path);

      else
        show_folder (entry.get_folder (), parent_path);
    }
  catch (const std::exception& e)
    {
      std::cerr << "Warning: " << e.what () << std::endl;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
  std::cerr << std::endl;
  std::cerr << "Use: dirfs [OPTIONS] <URL>" << std::endl;
  std::cerr << "e.g: dirfs -t raw file://disk.raw" << std::endl;
  std::cerr << "     dirfs file://disk.ewf" << std::endl;
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
//! \brief Main function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
main (int argc, char **argv)
{
  mobius::core::application app;
  app.start ();

  std::cerr << app.get_name () << " v" << app.get_version () << std::endl;
  std::cerr << app.get_copyright () << std::endl;
  std::cerr << "DirFS v1.4" << std::endl;
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
      std::cerr << "Error: You must enter a valid URL to an imagefile" << std::endl;
      usage ();
      exit (EXIT_FAILURE);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Create VFS and check if it is available
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  mobius::core::vfs::vfs vfs;
  vfs.add_disk (mobius::core::vfs::new_disk_by_url (argv[optind], type));

  if (!vfs.is_available ())
    {
      std::cerr << std::endl;
      std::cerr << "Error: Image file is not available" << std::endl;
      usage ();
      exit (EXIT_FAILURE);
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Show entries
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  for (const auto& entry : vfs.get_root_entries ())
    {
      std::cout << std::endl;
      show_entry (entry, {});
    }

  app.stop ();
  return EXIT_SUCCESS;
}
