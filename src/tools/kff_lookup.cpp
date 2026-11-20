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
    std::cerr << "use: kff_lookup hash_type hash_value" << std::endl;
    std::cerr << "e.g: kff_lookup md5 1234567890abcdef" << std::endl;

    // Hash types
    std::cerr << std::endl;
    std::cout << "Hash types:" << std::endl;

    for (std::uint16_t i = 1; i < 32; i++)
    {
        auto desc = mobius::core::kff::get_hash_type_string (i);
        if (desc.empty ())
            break;
        else
            std::cout << '\t' << mobius::core::string::rpad (desc, 20) << std::endl;
    }

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
    std::cerr << "KFF Lookup v1.0" << std::endl;
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

    if (optind + 1 >= argc)
    {
        std::cerr << std::endl;
        std::cerr << "Error: You must enter a hash type and hash value" << std::endl;
        usage ();
        exit (EXIT_FAILURE);
    }

    auto hash_type = mobius::core::kff::get_hash_type (argv[optind]);
    auto hash_value = argv[optind + 1];

    if (hash_type == 0)
    {
        std::cerr << std::endl;
        std::cerr << "Error: Invalid hash type" << std::endl;
        usage ();
        exit (EXIT_FAILURE);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Lookup
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    try
    {
        std::cout << std::endl;

        auto f = mobius::core::kff::lookup (hash_type, hash_value);

        if (f)
        {
            std::cout << "File ID: " << f.get_id () << std::endl;
            std::cout << "File size: " << f.get_size () << std::endl;
            std::cout << "Is alert: " << (f.is_alert () ? "Yes" : "No") << std::endl;
            std::cout << "Password: " << f.get_password () << std::endl;

            std::cout << "File hashes: " << std::endl;

            auto hashes = f.get_hashes ();
            std::vector<std::pair<std::string, std::string>> sorted_hashes;
            std::transform (
                hashes.begin (),
                hashes.end (),
                std::back_inserter (sorted_hashes),
                [] (const auto &p)
                { return std::make_pair (mobius::core::kff::get_hash_type_string (p.first), p.second); }
            );
            std::sort (sorted_hashes.begin (), sorted_hashes.end ());

            for (const auto &[type, hash] : sorted_hashes)
                std::cout << "\t" << mobius::core::string::rpad (type, 12) << hash << std::endl;
        }
        else
        {
            std::cout << "Hash not found" << std::endl;
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
