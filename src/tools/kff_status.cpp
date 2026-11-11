#include <mobius/core/application.hpp>
#include <iostream>

#include <mobius/core/kff/kff.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <string>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Main function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
main (int, char **)
{
    mobius::core::set_logfile_path ("mobius.log");

    try
    {
        // Summary
        std::cout << "Summary: " << std::endl;
        std::cout << "\tTotal files: " << mobius::core::kff::get_file_count () << std::endl;
        std::cout << "\tTotal alert files: " << mobius::core::kff::get_alert_count () << std::endl;
        std::cout << "\tTotal hashes: " << mobius::core::kff::get_hash_count () << std::endl;
        std::cout << "\tTotal thumbnails: " << mobius::core::kff::get_thumbnail_count () << std::endl;

        // File types
        std::cout << std::endl;
        std::cout << "File types:" << std::endl;

        for (std::uint16_t i = 0; i < 32; i++)
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

    return 0;
}
