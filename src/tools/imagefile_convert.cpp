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
#include <cstdint>
#include <iostream>
#include <mobius/core/application.hpp>
#include <mobius/core/io/path.hpp>
#include <mobius/core/resource.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/core/vfs/imagefile.hpp>
#include <unistd.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "Use: imagefile_convert [OPTIONS] <INPUT-PATH> [OUTPUT-PATH]"
              << std::endl;
    std::cerr << std::endl;
    std::cerr << "e.g: imagefile_convert -s 2GB disk.raw disk.001"
              << std::endl;
    std::cerr << "     imagefile_convert -f ewf -t raw disk.raw"
              << std::endl;
    std::cerr << "     imagefile_convert -t raw disk.ewf" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options are:" << std::endl;
    std::cerr << "  -f type\t\tInput imagefile type (default: autodetect)"
              << std::endl;
    std::cerr << "     Image file type can be:" << std::endl;
    std::cerr << "       autodetect\tTry to autodetect imagefile type (default)"
              << std::endl;

    for (const auto &type : mobius::core::vfs::get_imagefile_types ())
        std::cerr << "       " << type.id << "\t\t" << type.description << std::endl;

    std::cerr << std::endl;
    std::cerr << "  -t type\t\toutput imagefile type (default: autodetect)"
              << std::endl;
    std::cerr << "     Output types are:" << std::endl;
    std::cerr << "       autodetect\tTry to autodetect imagefile type (default)"
              << std::endl;

    for (const auto &type : mobius::core::vfs::get_imagefile_types ())
    {
        if (type.is_writeable)
            std::cerr << "       " << type.id << "\t\t" << type.description << std::endl;
    }

    std::cerr << std::endl;
    std::cerr
        << "  -s size\t\tsegment size (suffixes: KB,MB,GB,TB) (default: 4GB)"
        << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get size from size string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint64_t
get_size (const std::string &text)
{
    std::uint64_t size = stoll (text);

    if (mobius::core::string::endswith (text, "KB"))
        size *= 1024;

    else if (mobius::core::string::endswith (text, "MB"))
        size *= 1024 * 1024;

    else if (mobius::core::string::endswith (text, "GB"))
        size *= 1024L * 1024L * 1024L;

    else if (mobius::core::string::endswith (text, "TB"))
        size *= 1024L * 1024L * 1024L * 1024L;

    return size;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get type from path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_type_from_path (const std::string &path)
{
    std::string type;

    mobius::core::io::path p (path);
    std::string extension = p.get_extension ();

    if (extension == "001")
        type = "split";

    else if (extension == "E01")
        type = "ewf";

    else
        type = "raw";

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get output path from input path and type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_path_from_type (const std::string &input_path, const std::string &type)
{
    mobius::core::io::path p (input_path);
    std::string path = p.get_prefix ();

    if (type == "raw")
        path += ".raw";

    else if (type == "split")
        path += ".001";

    else if (type == "ewf")
        path += ".ewf";

    return path;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Main function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
main (int argc, char **argv)
{
    mobius::core::application app;
    app.start ();

    std::cerr << app.get_name () << " v" << app.get_version () << std::endl;
    std::cerr << app.get_copyright () << std::endl;
    std::cerr << "Imagefile Convert v1.1" << std::endl;
    std::cerr << "by Eduardo Aguiar" << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // parse command line
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    int opt;
    std::string input_type_arg = "autodetect";
    std::string output_type_arg = "autodetect";
    std::string segment_size_arg = "4GB";
    std::string input_path;
    std::string output_path;

    while ((opt = getopt (argc, argv, "hf:s:t:")) != EOF)
    {
        switch (opt)
        {
        case 'h':
            usage ();
            exit (EXIT_SUCCESS);
            break;

        case 'f':
            input_type_arg = optarg;
            break;

        case 's':
            segment_size_arg = optarg;
            break;

        case 't':
            output_type_arg = optarg;
            break;

        default:
            usage ();
            exit (EXIT_FAILURE);
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // evaluate arguments
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

    // two URL's given
    if (optind < argc - 1)
    {
        input_path = argv[optind];
        output_path = argv[optind + 1];

        if (output_type_arg == "autodetect")
            output_type_arg = get_type_from_path (output_path);
    }

    // one URL. output_type must be given
    else if (optind < argc)
    {
        if (output_type_arg == "autodetect")
        {
            std::cerr << std::endl;
            std::cerr << "Error: invalid command line" << std::endl;
            usage ();
            exit (EXIT_FAILURE);
        }

        input_path = argv[optind];
        output_path = get_path_from_type (input_path, output_type_arg);
    }

    // either no path or more than two paths
    else
    {
        std::cerr << std::endl;
        std::cerr << "Error: invalid command line" << std::endl;
        usage ();
        exit (EXIT_FAILURE);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // check if input imagefile is available
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto image_in = mobius::core::vfs::new_imagefile_by_path (input_path, input_type_arg);

    if (!image_in.is_available ())
    {
        std::cerr << std::endl;
        std::cerr << "Error: imagefile is not available." << std::endl;
        usage ();
        exit (EXIT_FAILURE);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // create output imagefile
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto image_out = mobius::core::vfs::new_imagefile_by_path (output_path, output_type_arg);

    if (image_out.get_type () == "ewf")
    {
        image_out.set_attribute ("segment_size", get_size (segment_size_arg));
        image_out.set_attribute ("compression_level", 1);
    }

    else if (image_out.get_type () == "split")
    {
        image_out.set_attribute ("segment_size", get_size (segment_size_arg));
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // copy imagefile
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto reader = image_in.new_reader ();
    auto writer = image_out.new_writer ();
    auto block_size = reader.get_block_size ();

    std::cout << std::endl;
    std::cout << "About to copy " << reader.get_size () << " bytes"
              << std::endl;
    std::cout << "  from " << input_path << std::endl;
    std::cout << "  to " << output_path << std::endl;

    auto data = reader.read (block_size);
    auto size = reader.get_size ();
    decltype (size) copied = 0;

    while (!data.empty ())
    {
        writer.write (data);
        copied += data.size ();
        printf ("Copied %lu bytes\r", copied);
        data = reader.read (block_size);
    }

    std::cout << std::endl;

    app.stop ();
}
