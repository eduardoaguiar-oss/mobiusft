// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025
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
#include <iostream>
#include <mobius/core/application.hpp>
#include <mobius/core/io/entry.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/io/folder.hpp>
#include <mobius/core/io/stream.hpp>
#include <mobius/core/resource.hpp>
#include <mobius/core/vfs/vfs.hpp>
#include <unistd.h>

void show_entry (const mobius::core::io::entry &, const std::string &, bool);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Shows streams
// @param streams Vector of streams
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_streams (const std::vector<mobius::core::io::stream> &streams)
{
    int i = 1;
    std::cout << "     streams:" << '\n';

    for (const auto &stream : streams)
    {
        std::cout << "              stream " << i << ": " << stream.get_type ()
                  << ", size: " << stream.get_size ()
                  << ", name: " << stream.get_name () << '\n';

        try
        {
            auto reader = stream.new_reader ();
            auto data = reader.read (16);

            std::cout << "                        ";
            for (std::size_t j = 0; j < data.size (); j++)
                printf ("%02x ", data[j]);

            std::cout << '\n';
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << "warning: " << e.what () << '\n';
        }

        ++i;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show file information
// @param f File object
// @param parent_path Parent path
// @param show_data Only listing flag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_file (const mobius::core::io::file &f, const std::string &parent_path,
           bool show_data)
{
    // metadata
    std::cout << '\n';
    std::cout << "   [" << f.get_inode () << "] ";

    if (f.is_deleted ())
        std::cout << "<DEL> ";

    std::cout << parent_path << '/' << f.get_name () << '\n';

    std::cout << "        size: " << f.get_size () << '\n';
    std::cout << "        type: " << static_cast<int> (f.get_type ()) << '\n';
    std::cout << "        name: " << f.get_name () << '\n';
    std::cout << "       atime: " << f.get_access_time () << '\n';
    std::cout << "       mtime: " << f.get_modification_time () << '\n';
    std::cout << "       ctime: " << f.get_metadata_time () << '\n';
    std::cout << "      crtime: " << f.get_creation_time () << '\n';
    std::cout << "       dtime: " << f.get_deletion_time () << '\n';
    std::cout << "     bkptime: " << f.get_backup_time () << '\n';

    // parent
    auto parent = f.get_parent ();

    if (parent)
    {
        std::cout << " parent name: " << parent.get_name () << '\n';
        std::cout << " parent path: " << parent_path << '\n';
    }

    // streams
    if (show_data)
        show_streams (f.get_streams ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show folder information
// @param f Folder object
// @param parent_path Parent path
// @param show_data Only listing flag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_folder (const mobius::core::io::folder &f, const std::string &parent_path,
             bool show_data)
{
    // metadata
    std::cout << '\n';
    std::cout << " + [" << f.get_inode () << "] ";

    if (f.is_deleted ())
        std::cout << "<DEL> ";

    std::string path = parent_path;
    if (!f.get_name ().empty ())
        path += '/' + f.get_name ();

    std::cout << path << '\n';

    std::cout << "        size: " << f.get_size () << '\n';
    std::cout << "        name: " << f.get_name () << '\n';
    std::cout << "       atime: " << f.get_access_time () << '\n';
    std::cout << "       mtime: " << f.get_modification_time () << '\n';
    std::cout << "       ctime: " << f.get_metadata_time () << '\n';
    std::cout << "      crtime: " << f.get_creation_time () << '\n';
    std::cout << "       dtime: " << f.get_deletion_time () << '\n';

    // parent
    auto parent = f.get_parent ();

    if (parent)
    {
        std::cout << " parent name: " << parent.get_name () << '\n';
        std::cout << " parent path: " << parent_path << '\n';
    }

    // streams
    if (show_data)
        show_streams (f.get_streams ());

    // children
    for (const auto &child : f.get_children ())
        show_entry (child, path, show_data);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show entry information
// @param entry Entry object
// @param parent_path Parent path
// @param show_data Only listing flag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_entry (const mobius::core::io::entry &entry,
            const std::string &parent_path, bool show_data)
{
    try
    {
        if (entry.is_file ())
            show_file (entry.get_file (), parent_path, show_data);

        else
            show_folder (entry.get_folder (), parent_path, show_data);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Warning: " << e.what () << '\n';
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << '\n';
    std::cerr << "Use: dirfs [OPTIONS] <URL>" << '\n';
    std::cerr << "e.g: dirfs -t raw file://disk.raw" << '\n';
    std::cerr << "     dirfs file://disk.ewf" << '\n';
    std::cerr << '\n';
    std::cerr << "Options are:" << '\n';
    std::cerr << "  -t imagefile type\t\t" << '\n';
    std::cerr << '\n';
    std::cerr << "     Image file type can be:" << '\n';
    std::cerr << "       autodetect\tTry to autodetect imagefile type (default)"
              << '\n';

    for (const auto &r : mobius::core::get_resources ("vfs.imagefile"))
        std::cerr << "       " << r.get_id () << "\t\t" << r.get_description ()
                  << '\n';

    std::cerr << '\n';
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief Main function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
main (int argc, char **argv)
{
    mobius::core::application app;
    app.start ();

    std::cerr << app.get_name () << " v" << app.get_version () << '\n';
    std::cerr << app.get_copyright () << '\n';
    std::cerr << "DirFS v1.5" << '\n';
    std::cerr << "by Eduardo Aguiar" << '\n';

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // parse command line
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    int opt;
    std::string type = "autodetect";
    bool show_data = false;

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

        case 'd':
            show_data = true;
            break;

        default:
            usage ();
            exit (EXIT_FAILURE);
        }
    }

    if (optind >= argc)
    {
        std::cerr << '\n';
        std::cerr << "Error: You must enter a valid URL to an imagefile"
                  << '\n';
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
        std::cerr << '\n';
        std::cerr << "Error: Image file is not available" << '\n';
        usage ();
        exit (EXIT_FAILURE);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    for (const auto &entry : vfs.get_root_entries ())
    {
        std::cout << '\n';
        show_entry (entry, {}, show_data);
    }

    app.stop ();
    return EXIT_SUCCESS;
}
