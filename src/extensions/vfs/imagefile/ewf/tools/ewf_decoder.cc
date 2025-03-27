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
#include "segment_decoder.h"
#include <mobius/core/application.h>
#include <mobius/io/file.h>
#include <mobius/string_functions.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <unistd.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
  std::cerr << std::endl;
  std::cerr << "use: ewf_decoder [OPTIONS] <URL>" << std::endl;
  std::cerr << "e.g: ewf_decoder -v file://disk.E01" << std::endl;
  std::cerr << "     ewf_decoder file://disk.ewf" << std::endl;
  std::cerr << std::endl;
  std::cerr << "options are:" << std::endl;
  std::cerr << "  -v show section's metadata" << std::endl;
  std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief show uint32_t as hexstring
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
to_hexstring (std::uint32_t value)
{
  std::ostringstream stream;
  stream << "0x" << std::hex << std::setfill ('0') << std::setw (8) << value << std::dec << std::setfill (' ');
  return stream.str ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief show uint64_t as hexstring
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
to_hex64 (std::uint64_t value)
{
  std::ostringstream stream;
  stream << "0x" << std::hex << std::setfill ('0') << std::setw (16) << value << std::dec << std::setfill (' ');
  return stream.str ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief main function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
main (int argc, char **argv)
{
  mobius::core::application app;
  std::cerr << app.get_name () << " v" << app.get_version () << std::endl;
  std::cerr << app.get_copyright () << std::endl;
  std::cerr << "EWF decoder v1.0" << std::endl;
  std::cerr << "by Eduardo Aguiar" << std::endl;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // parse command line
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  int opt;
  int verbose_level = 0;

  while ((opt = getopt (argc, argv, "hv")) != EOF)
    {
      switch (opt)
        {
        case 'h':
          usage ();
          exit (EXIT_SUCCESS);
          break;

        case 'v':
          verbose_level++;
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
  // show EWF file information
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  auto f = mobius::io::new_file_by_url (argv[optind]);
  auto reader = f.new_reader ();

  segment_decoder decoder (reader);
  std::cout << std::endl;
  std::cout << "segment number: " << decoder.get_segment_number () << std::endl;
  std::cout << std::endl;
  std::cout << std::setw (12) << "offset" << "\t" << std::setw (12) << "next offset" << "\tname\t\t" << std::setw (12) << "size" << "\theader Adler32" << std::endl;
  std::cout << ' ' << std::string (78, '=') << std::endl;

  for (const auto& section : decoder)
    {
      if (verbose_level > 0)
        std::cout << std::endl;

      bool is_valid = section.get_adler32 () == section.get_calculated_adler32 ();

      std::cout << std::setw (12) << section.get_offset () << '\t'
                << std::setw (12) << section.get_next_offset () << '\t'
                << std::setw (12) << std::left << section.get_name () << std::right << '\t'
                << std::setw (12) << section.get_size () << '\t'
                << to_hexstring (section.get_adler32 ()) << ' '
                << (is_valid ? "OK" : "ERR") << std::endl;

      if (verbose_level > 0)
        std::cout << std::endl;

      // show details
      if (verbose_level == 0)
        ;

      else if (section.get_name () == "hash")
        {
          auto hash_section = decoder.decode_hash_section (section);
          std::cout << "\t\tMD5 hash            : " << hash_section.get_md5_hash () << std::endl;
          std::cout << "\t\tAdler-32 CRC        : " << to_hexstring (hash_section.get_section_adler32 ()) << std::endl;
        }

      else if (section.get_name () == "volume" || section.get_name () == "disk" || section.get_name () == "data")
        {
          auto volume_section = decoder.decode_volume_section (section);
          std::cout << "\t\tmedia type          : " << volume_section.get_media_type () << std::endl;
          std::cout << "\t\tchunks              : " << volume_section.get_chunk_count () << std::endl;
          std::cout << "\t\tsectors per chunk   : " << volume_section.get_chunk_sectors () << std::endl;
          std::cout << "\t\tsector size         : " << volume_section.get_sector_size () << " bytes" << std::endl;
          std::cout << "\t\tsectors             : " << volume_section.get_sectors () << std::endl;
          std::cout << "\t\tmedia flags         : " << volume_section.get_media_flags () << std::endl;
          std::cout << "\t\tcompression level   : " << volume_section.get_compression_level () << std::endl;
          std::cout << "\t\tGUID                : " << volume_section.get_guid () << std::endl;
          std::cout << "\t\tAdler-32 CRC        : " << to_hexstring (volume_section.get_section_adler32 ()) << std::endl;
        }

      else if (section.get_name () == "header" || section.get_name () == "header2")
        {
          auto header_section = decoder.decode_header_section (section);
          auto text = mobius::string::replace (header_section.get_text (), "\n", "\n\t\t\t");
          std::cout << "\t\tdrive model         : " << header_section.get_drive_model () << std::endl;
          std::cout << "\t\tdrive serial number : " << header_section.get_drive_serial_number () << std::endl;
          std::cout << "\t\tacquisition user    : " << header_section.get_acquisition_user () << std::endl;
          std::cout << "\t\tacquisition time    : " << header_section.get_acquisition_time () << std::endl;
          std::cout << "\t\tacquisition tool    : " << header_section.get_acquisition_tool () << std::endl;
          std::cout << "\t\tacquisition platform: " << header_section.get_acquisition_platform () << std::endl;

          if (verbose_level > 1)
            {
              std::cout << "\t\ttext" << std::endl;
              std::cout << "\t\t\t" << text << std::endl;
            }
        }

      else if (section.get_name () == "table" || section.get_name () == "table2")
        {
          auto table_section = decoder.decode_table_section (section);
          std::cout << "\t\tchunks              : " << table_section.get_chunk_count () << std::endl;
          std::cout << "\t\tbase offset         : " << table_section.get_base_offset () << std::endl;
          std::cout << "\t\tmetadata adler-32   : " << to_hexstring (table_section.get_section_adler32 ()) << std::endl;
          std::cout << "\t\toffset list adler-32: " << to_hexstring (table_section.get_table_adler32 ()) << std::endl;

          if (verbose_level > 1)
            {
              std::cout << "\t\toffsets" << std::endl;

              for (auto offset : table_section.get_chunk_offset_list ())
                {
                  bool compressed = offset & 0x8000000000000000;
                  offset = offset & 0x7fffffffffffffff;
                  std::cout << "\t\t\t" << to_hex64 (offset) << (compressed ? " (compressed)" : "") << std::endl;
                }
            }
        }
    }
}


