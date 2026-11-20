#include <mobius/core/application.hpp>
#include <mobius/core/kff/kff.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <iostream>
#include <string>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: kff_status" << std::endl;
    std::cerr << std::endl;
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
    std::cerr << "KFF Status v1.0" << std::endl;
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

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show KFF status
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    try
    {
        std::cout << std::endl;
        std::cout << "Summary: " << std::endl;
        std::cout << "\tTotal files: " << mobius::core::kff::get_file_count () << std::endl;
        std::cout << "\tTotal alert files: " << mobius::core::kff::get_alert_count () << std::endl;
        std::cout << "\tTotal hashes: " << mobius::core::kff::get_hash_count () << std::endl;
        std::cout << "\tTotal thumbnails: " << mobius::core::kff::get_thumbnail_count () << std::endl;

        // File types
        std::cout << std::endl;
        std::cout << "File types:" << std::endl;

        for (std::uint16_t i = 1; i < 32; i++)
        {
            auto desc = mobius::core::kff::get_file_type_string (i);
            if (desc.empty ())
                break;
            else
                std::cout << '\t' << i << '\t' << mobius::core::string::rpad (desc, 20) << std::endl;
        }

        // Hash types
        std::cout << std::endl;
        std::cout << "Hash types:" << std::endl;

        for (std::uint16_t i = 1; i < 32; i++)
        {
            auto desc = mobius::core::kff::get_hash_type_string (i);
            if (desc.empty ())
                break;
            else
                std::cout << '\t' << i << '\t' << mobius::core::string::rpad (desc, 20) << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what () << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown exception occurred" << std::endl;
    }

    app.stop ();
    return EXIT_SUCCESS;
}
