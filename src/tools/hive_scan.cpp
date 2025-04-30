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
#include <mobius/io/file.h>
#include <mobius/core/os/win/registry/hive_file.hpp>
#include <iostream>
#include <unistd.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
  std::cerr << std::endl;
  std::cerr << "use: hive_scan [OPTIONS] <URL>" << std::endl;
  std::cerr << "e.g: hive_scan file://SAM" << std::endl;
  std::cerr << "     hive_scan file://NTUSER.dat" << std::endl;
  std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief print hive key
//! \param key hive key
//! \param indent indentation level
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
print_hive_key (const mobius::core::os::win::registry::hive_key& key, const std::string path = "")
{
  const std::string key_path = (path.empty ()) ? key.get_name () : path + '/' + key.get_name ();
  const std::string s_indent = "   ";

  // metadata
  std::cout << std::endl;
  std::cout << s_indent << "* " << key_path << std::endl;
  std::cout << s_indent << "              Name: " << key.get_name () << std::endl;
  std::cout << s_indent << "            Offset: " << key.get_offset () << std::endl;
  std::cout << s_indent << "        Class name: " << key.get_classname () << std::endl;
  std::cout << s_indent << "         Signature: " << key.get_signature () << std::endl;
  std::cout << s_indent << "             Flags: " << key.get_flags () << std::endl;
  std::cout << s_indent << "  Last modif. time: " << key.get_last_modification_time () << std::endl;
  std::cout << s_indent << "       Access bits: " << key.get_access_bits () << std::endl;
  std::cout << s_indent << "           Subkeys: " << key.get_subkeys_count () << std::endl;
  std::cout << s_indent << "  Volatile subkeys: " << key.get_volatile_subkeys_count () << std::endl;
  std::cout << s_indent << "            Values: " << key.get_values_count () << std::endl;
  std::cout << s_indent << "       Is root key: " << key.is_root_key () << std::endl;
  std::cout << s_indent << "      Is read only: " << key.is_readonly () << std::endl;
  std::cout << s_indent << "        Is symlink: " << key.is_symlink () << std::endl;

  // values
  std::cout << s_indent << "            Values:" << std::endl;

  for (const auto& value : key.get_values ())
    {
      const std::string name = (value.get_name ().empty ()) ? "(default)" : value.get_name ();

      std::cout << std::endl;
      std::cout << s_indent << "                 Name: " << name << std::endl;
      std::cout << s_indent << "               Offset: " << value.get_offset () << std::endl;
      std::cout << s_indent << "            Signature: " << value.get_signature () << std::endl;
      std::cout << s_indent << "                Flags: " << value.get_flags () << std::endl;

      auto data = value.get_data ();
      std::cout << s_indent << "            Data size: " << data.get_size () << std::endl;
      std::cout << s_indent << "            Data type: " << static_cast <int> (data.get_type ()) << std::endl;

      std::string str_data;
      auto d = data.get_data ();

      switch (data.get_type ())
        {
        default:
          if (d.size () > 16)
            d.resize (16);

          str_data = d.to_hexstring ();
          break;
        }

      std::cout << s_indent << "                 Data: " << str_data << std::endl;
    }

  for (const auto& child : key)
    print_hive_key (child, key_path);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief print hive file info
//! \param URL hive file URL
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
print_hivefile (const std::string& url)
{
  std::cout << std::endl;
  std::cout << ">> " << url << std::endl;

  auto f = mobius::io::new_file_by_url (url);
  auto reader = f.new_reader ();
  mobius::core::os::win::registry::hive_file hf (reader);

  auto root_key = hf.get_root_key ();
  print_hive_key (root_key);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief main function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
main (int argc, char **argv)
{
  mobius::core::application app;
  std::cerr << app.get_name () << " v" << app.get_version () << std::endl;
  std::cerr << app.get_copyright () << std::endl;
  std::cerr << "Hive Scan v1.0" << std::endl;
  std::cerr << "by Eduardo Aguiar" << std::endl;

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
      std::cerr << std::endl;
      std::cerr << "Error: you must enter a valid URL to a hive file" << std::endl;
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
          print_hivefile (argv[optind]);
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
