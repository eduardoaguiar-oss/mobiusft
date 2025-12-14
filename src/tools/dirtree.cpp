// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
// Eduardo Aguiar
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
#include <iomanip>
#include <iostream>
#include <mobius/core/application.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/folder.hpp>
#include <mobius/core/string_functions.hpp>
#include <unistd.h>

#ifdef HAVE_SMB_SUPPORT
#include <mobius/core/io/smb/init.hpp>
#endif

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Show entry metadata
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
template <typename F>
void
show_metadata (int level, F f)
{
    const std::string indent (level * 2, ' ');

    std::cout << indent << "  Name: " << f.get_name () << std::endl;
    std::cout << indent << "  Short name: " << f.get_short_name () << std::endl;
    std::cout << indent << "  Inode: " << f.get_inode () << std::endl;
    std::cout << indent << "  Size: " << f.get_size () << " bytes" << std::endl;
    std::cout << indent << "  Is deleted: " << std::boolalpha << f.is_deleted ()
              << std::endl;
    std::cout << indent << "  Is reallocated: " << std::boolalpha
              << f.is_reallocated () << std::endl;
    std::cout << indent << "  Is hidden: " << std::boolalpha << f.is_hidden ()
              << std::endl;
    std::cout << indent << "  User: " << f.get_user_name () << " ("
              << f.get_user_id () << ')' << std::endl;
    std::cout << indent << "  Group: " << f.get_group_name () << " ("
              << f.get_group_id () << ')' << std::endl;
    std::cout << indent << "  Permissions: " << std::oct << f.get_permissions ()
              << std::dec << std::endl;
    std::cout << indent
              << "  Last access time (atime): " << f.get_access_time ()
              << std::endl;
    std::cout << indent << "  Last modification time (mtime): "
              << f.get_modification_time () << std::endl;
    std::cout << indent
              << "  Last metadata time (ctime): " << f.get_metadata_time ()
              << std::endl;
    std::cout << indent << "  Creation time: " << f.get_creation_time ()
              << std::endl;
    std::cout << indent << "  Deletion time: " << f.get_deletion_time ()
              << std::endl;
    std::cout << indent << "  Backup time: " << f.get_backup_time ()
              << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Show file info
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_file (const mobius::core::io::file &f, const std::string &url,
           int level = 0)
{
    std::cout << std::endl;
    const std::string indent (level * 2, ' ');

    if (!f || !f.exists ())
    {
        std::cout << indent << "? " << url << std::endl;
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // show metadata
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << indent << ". " << url << std::endl;
    show_metadata (level, f);
    std::cout << indent << "  Is regular file: " << std::boolalpha
              << f.is_regular_file () << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // show some bytes...
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    if (f.is_regular_file ())
    {
        try
        {
            auto reader = f.new_reader ();
            auto data = reader.read (16);

            std::cout << indent << "  First 16 bytes:";
            for (auto b : data)
                std::cout << ' ' << mobius::core::string::to_hex (b, 2);
            std::cout << std::endl;
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << "Warning: " << e.what () << std::endl;
        }
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Show folder info
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_folder (const mobius::core::io::folder &folder,
             const std::string &url = "/", int level = 0)
{
    std::cout << std::endl;
    const std::string indent (level * 2, ' ');

    if (!folder || !folder.exists ())
    {
        std::cout << indent << "? " << url << std::endl;
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // show metadata
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << indent << "+ " << url << std::endl;
    show_metadata (level, folder);

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // handle entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    try
    {
        for (const auto &entry : folder.get_children ())
        {
            if (entry.is_folder ())
            {
                auto cfolder = entry.get_folder ();

                const std::string curl = (url == "/")
                                             ? url + cfolder.get_name ()
                                             : url + '/' + cfolder.get_name ();

                show_folder (cfolder, curl, level + 1);
            }

            else if (entry.is_file ())
            {
                auto cfile = entry.get_file ();

                const std::string curl = (url == "/")
                                             ? url + cfile.get_name ()
                                             : url + '/' + cfile.get_name ();

                show_file (cfile, curl, level + 1);
            }
        }
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Warning: " << e.what () << std::endl;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: dirtree [OPTIONS] <url1> [url2] ..." << std::endl;
    std::cerr << "e.g: dirtree file:///etc" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options are:" << std::endl;
    std::cerr << "  -u user\tSMB user name" << std::endl;
    std::cerr << "  -p password\tSMB user password" << std::endl;
    std::cerr << std::endl;
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
    std::cerr << "DirTree v1.2" << std::endl;
    std::cerr << "by Eduardo Aguiar" << std::endl;
    std::cerr << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Process command line
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    int opt;
    std::string smb_user;
    std::string smb_password;

    while ((opt = getopt (argc, argv, "u:p:")) != EOF)
    {
        switch (opt)
        {
        case 'h':
            usage ();
            exit (EXIT_SUCCESS);
            break;

        case 'u':
            smb_user = optarg;
            break;

        case 'p':
            smb_password = optarg;
            break;

        default:
            std::cerr << "Error: Invalid option '-" << char (opt) << "'"
                      << std::endl;
            usage ();
            exit (EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        std::cerr << "Error: You must pass a valid url" << std::endl;
        usage ();
        exit (EXIT_FAILURE);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifdef HAVE_SMB_SUPPORT
    if (!smb_user.empty () || !smb_password.empty ())
        mobius::core::io::smb::init (smb_user, smb_password);
#endif

    while (optind < argc)
    {
        const std::string url = argv[optind];
        auto folder = mobius::core::io::new_folder_by_url (url);

        try
        {
            show_folder (folder, url);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what () << std::endl;
            exit (EXIT_FAILURE);
        }

        optind++;
    }

    return EXIT_SUCCESS;
}
