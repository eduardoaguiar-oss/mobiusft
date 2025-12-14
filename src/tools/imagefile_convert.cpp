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
#include <cstdint>
#include <iostream>
#include <mobius/core/application.hpp>
#include <mobius/core/io/uri.hpp>
#include <mobius/core/resource.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/core/vfs/imagefile.hpp>
#include <unistd.h>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "Use: imagefile_convert [OPTIONS] <INPUT-URL> [OUTPUT-URL]"
              << std::endl;
    std::cerr << std::endl;
    std::cerr << "e.g: imagefile_convert -s 2GB file://disk.raw file://disk.001"
              << std::endl;
    std::cerr << "     imagefile_convert -f ewf -t raw file://disk.raw"
              << std::endl;
    std::cerr << "     imagefile_convert -t raw file://disk.ewf" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options are:" << std::endl;
    std::cerr << "  -f type\t\tInput imagefile type (default: autodetect)"
              << std::endl;
    std::cerr << "     Image file type can be:" << std::endl;
    std::cerr << "       autodetect\tTry to autodetect imagefile type (default)"
              << std::endl;

    for (const auto &r : mobius::core::get_resources ("vfs.imagefile"))
        std::cerr << "       " << r.get_id () << "\t\t" << r.get_description ()
                  << std::endl;

    std::cerr << std::endl;
    std::cerr << "  -t type\t\toutput imagefile type (default: autodetect)"
              << std::endl;
    std::cerr << "     Output types are:" << std::endl;
    std::cerr << "       autodetect\tTry to autodetect imagefile type (default)"
              << std::endl;

    for (const auto &r : mobius::core::get_resources ("vfs.imagefile"))
    {
        auto img_resource =
            r.get_value<mobius::core::vfs::imagefile_resource_type> ();

        if (img_resource.is_writeable)
            std::cerr << "       " << r.get_id () << "\t\t"
                      << r.get_description () << std::endl;
    }

    std::cerr << std::endl;
    std::cerr
        << "  -s size\t\tsegment size (suffixes: KB,MB,GB,TB) (default: 4GB)"
        << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get size from size string
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
//! \brief get type from URL
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_type_from_url (const std::string &url)
{
    std::string type;

    mobius::core::io::uri uri (url);
    std::string extension = uri.get_extension ();

    if (extension == "001")
        type = "split";

    else if (extension == "E01")
        type = "ewf";

    else
        type = "raw";

    return type;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! \brief get output URL from input URL and type
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
get_url_from_type (const std::string &input_url, const std::string &type)
{
    mobius::core::io::uri uri (input_url);
    std::string url = input_url.substr (0, input_url.length () -
                                               uri.get_extension ().length ());

    if (type == "raw")
        url += "raw";

    else if (type == "split")
        url += "001";

    return url;
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
    std::cerr << "Imagefile Convert v1.1" << std::endl;
    std::cerr << "by Eduardo Aguiar" << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // parse command line
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    int opt;
    std::string input_type_arg = "autodetect";
    std::string output_type_arg = "autodetect";
    std::string segment_size_arg = "4GB";
    std::string input_url;
    std::string output_url;

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
        input_url = argv[optind];
        output_url = argv[optind + 1];

        if (output_type_arg == "autodetect")
            output_type_arg = get_type_from_url (output_url);
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

        input_url = argv[optind];
        output_url = get_url_from_type (input_url, output_type_arg);
    }

    // either no URL or more than two URL's
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
    auto image_in =
        mobius::core::vfs::new_imagefile_by_url (input_url, input_type_arg);

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
    auto image_out =
        mobius::core::vfs::new_imagefile_by_url (output_url, output_type_arg);

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
    std::cout << "  from " << input_url << std::endl;
    std::cout << "  to " << output_url << std::endl;

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
