// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008-2026 Eduardo Aguiar
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
#include <iostream>
#include <mobius/core/application.hpp>
#include <mobius/core/crypt/hash.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/vfs/vfs.hpp>
#include <unistd.h>

void process_file (const mobius::core::io::file &, const std::string &,
                   const std::string &);
void process_folder (const mobius::core::io::folder &, const std::string &,
                     const std::string &);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: hashfs [OPTIONS] <URL>" << std::endl;
    std::cerr << "e.g: hashfs -t md5 file://disk.raw" << std::endl;
    std::cerr << "     hashfs file://disk.ewf" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options are:" << std::endl;
    std::cerr << "  -t hash type\t\t" << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Process entry
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
process_entry (const mobius::core::io::entry &entry,
               const std::string &hash_type, const std::string &path = "")
{
    try
    {
        if (entry.is_file ())
        {
            auto fchild = entry.get_file ();
            process_file (fchild, hash_type, path + '/' + fchild.get_name ());
        }

        else
        {
            auto fchild = entry.get_folder ();
            process_folder (fchild, hash_type, path + '/' + fchild.get_name ());
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Warning: " << e.what () << std::endl;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Process file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
process_file (const mobius::core::io::file &f, const std::string &hash_type,
              const std::string &path)
{
    try
    {
        if (f.is_reallocated ())
            return;

        auto reader = f.new_reader ();

        if (!reader)
            return;

        mobius::core::crypt::hash h (hash_type);

        constexpr int BLOCK_SIZE = 65536;
        auto data = reader.read (BLOCK_SIZE);

        while (data)
        {
            h.update (data);
            data = reader.read (BLOCK_SIZE);
        }

        std::cout << h.get_hex_digest () << '\t' << path << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Warning: " << e.what () << std::endl;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Process folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
process_folder (const mobius::core::io::folder &folder,
                const std::string &hash_type, const std::string &path = "")
{
    if (folder.is_reallocated ())
        return;

    try
    {
        for (const auto &entry : folder.get_children ())
            process_entry (entry, hash_type, path);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Warning: " << e.what () << std::endl;
    }
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
    std::cerr << "HashFS v1.3" << std::endl;
    std::cerr << "by Eduardo Aguiar" << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // parse command line
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    int opt;
    std::string hash_type = "md5";

    while ((opt = getopt (argc, argv, "ht:")) != EOF)
    {
        switch (opt)
        {
        case 'h':
            usage ();
            exit (EXIT_SUCCESS);
            break;

        case 't':
            hash_type = optarg;
            break;

        default:
            usage ();
            exit (EXIT_FAILURE);
        }
    }

    if (optind >= argc)
    {
        std::cerr << std::endl;
        std::cerr << "Error: You must enter a valid URL to an imagefile"
                  << std::endl;
        usage ();
        exit (EXIT_FAILURE);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Create VFS and check if it is available
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::vfs::vfs vfs;
    vfs.add_disk (mobius::core::vfs::new_disk_by_url (argv[optind]));

    if (!vfs.is_available ())
    {
        std::cerr << std::endl;
        std::cerr << "Error: Image file is not available" << std::endl;
        usage ();
        exit (EXIT_FAILURE);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // calculate hashes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &entry : vfs.get_root_entries ())
        process_entry (entry, hash_type);

    app.stop ();
    return EXIT_SUCCESS;
}
