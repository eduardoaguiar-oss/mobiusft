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
#include <mobius/core/application.hpp>
#include <mobius/core/io/file.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <unistd.h>
#include <iostream>
#include "../file_web_data.hpp"

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: chromium_web_data [OPTIONS] <path>" << std::endl;
    std::cerr << "e.g: chromium_web_data 'Web Data'" << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show Chromium Web Data info
// @param path Web Data path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_web_data (const std::string &path)
{
    std::cout << std::endl;
    std::cout << ">> " << path << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Try to decode file
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    auto f = mobius::core::io::new_file_by_path (path);
    auto reader = f.new_reader ();

    mobius::extension::app::chromium::file_web_data dat (reader);
    if (!dat)
    {
        std::cerr << "\tFile is not an instance of Web Data" << std::endl;
        return;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show autofill entries
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << "Autofill entries:" << std::endl;

    for (const auto &entry : dat.get_autofill_entries ())
    {
        std::cout << std::endl;
        std::cout << "\tIndex: " << entry.idx << std::endl;
        std::cout << "\tName: " << entry.name << std::endl;

        if (entry.is_encrypted)
            std::cout << "\tValue: (encrypted)" << std::endl
                      << entry.value.dump (8) << std::endl;
        else
            std::cout << "\tValue: " << entry.value.to_string () << std::endl;

        std::cout << "\tDate created: " << to_string (entry.date_created)
                  << std::endl;
        std::cout << "\tDate last used: " << to_string (entry.date_last_used)
                  << std::endl;
        std::cout << "\tCount: " << entry.count << std::endl;
        std::cout << "\tIs encrypted: " << (entry.is_encrypted ? "yes" : "no")
                  << std::endl;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show autofill profiles
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << std::endl;
    std::cout << "Autofill profiles:" << std::endl;
    for (const auto &profile : dat.get_autofill_profiles ())
    {
        std::cout << std::endl;
        std::cout << "\tIndex: " << profile.idx << std::endl;
        std::cout << "\tGUID: " << profile.guid << std::endl;
        std::cout << "\tCompany name: " << profile.company_name << std::endl;
        std::cout << "\tDate modified: " << profile.date_modified << std::endl;
        std::cout << "\tOrigin: " << profile.origin << std::endl;
        std::cout << "\tLanguage code: " << profile.language_code << std::endl;
        std::cout << "\tUse count: " << profile.use_count << std::endl;
        std::cout << "\tDate last used: " << profile.date_last_used
                  << std::endl;

        // show names
        for (const auto &name : profile.names)
        {
            std::cout << "\tName:" << std::endl;
            std::cout << "\t\tFirst name: " << name.first_name << std::endl;
            std::cout << "\t\tMiddle name: " << name.middle_name << std::endl;
            std::cout << "\t\tLast name: " << name.last_name << std::endl;
            std::cout << "\t\tFull name: " << name.full_name << std::endl;
            if (!name.honorific_prefix.empty ())
                std::cout << "\t\tHonorific prefix: " << name.honorific_prefix
                          << std::endl;
            if (!name.first_last_name.empty ())
                std::cout << "\t\tFirst last name: " << name.first_last_name
                          << std::endl;
            if (!name.conjunction_last_name.empty ())
                std::cout << "\t\tConjunction last name: "
                          << name.conjunction_last_name << std::endl;
            if (!name.second_last_name.empty ())
                std::cout << "\t\tSecond last name: " << name.second_last_name
                          << std::endl;
            if (!name.full_name_with_honorific_prefix.empty ())
                std::cout << "\t\tFull name with honorific prefix: "
                          << name.full_name_with_honorific_prefix << std::endl;
        }

        // show addresses
        for (const auto &address : profile.addresses)
        {
            std::cout << "\tAddress:" << std::endl;
            std::cout << "\t\tAddress line 1: " << address.address_line_1
                      << std::endl;
            std::cout << "\t\tAddress line 2: " << address.address_line_2
                      << std::endl;
            std::cout << "\t\tStreet address: " << address.street_address
                      << std::endl;
            std::cout << "\t\tStreet name: " << address.street_name
                      << std::endl;
            std::cout << "\t\tDependent street name: "
                      << address.dependent_street_name << std::endl;
            std::cout << "\t\tHouse number: " << address.house_number
                      << std::endl;
            std::cout << "\t\tSubpremise: " << address.subpremise << std::endl;
            std::cout << "\t\tPremise name: " << address.premise_name
                      << std::endl;
            std::cout << "\t\tDependent locality: "
                      << address.dependent_locality << std::endl;
            std::cout << "\t\tCity: " << address.city << std::endl;
            std::cout << "\t\tState: " << address.state << std::endl;
            std::cout << "\t\tZip code: " << address.zip_code << std::endl;
            std::cout << "\t\tCountry code: " << address.country_code
                      << std::endl;
            std::cout << "\t\tApartment number: " << address.apartment_number
                      << std::endl;
            std::cout << "\t\tFloor: " << address.floor << std::endl;
            std::cout << "\t\tCountry: " << address.country << std::endl;
        }

        // show emails
        std::cout << "\tEmails:" << std::endl;
        for (const auto &email : profile.emails)
        {
            std::cout << "\t\t" << email << std::endl;
        }

        // show phones
        std::cout << "\tPhones:" << std::endl;
        for (const auto &phone : profile.phones)
        {
            std::cout << "\t\tType: " << phone.type << std::endl;
            std::cout << "\t\tNumber: " << phone.number << std::endl;
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show credit cards
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::cout << std::endl;
    std::cout << "Credit cards:" << std::endl;

    for (const auto &card : dat.get_credit_cards ())
    {
        std::cout << std::endl;
        std::cout << "\tIndex: " << card.idx << std::endl;
        std::cout << "\tGUID: " << card.guid << std::endl;
        std::cout << "\tBank name: " << card.bank_name << std::endl;
        std::cout << "\tBilling address ID: " << card.billing_address_id
                  << std::endl;
        std::cout << "\tCard art URL: " << card.card_art_url << std::endl;
        std::cout << "\tCard info retrieval enrollment state: "
                  << card.card_info_retrieval_enrollment_state << std::endl;
        std::cout << "\tCard issuer: " << card.card_issuer << std::endl;
        std::cout << "\tCard issuer ID: " << card.card_issuer_id << std::endl;
        std::cout << "\tDate modified: " << card.date_modified << std::endl;
        std::cout << "\tExpiration month: " << card.expiration_month
                  << std::endl;
        std::cout << "\tExpiration year: " << card.expiration_year << std::endl;
        std::cout << "\tID: " << card.id << std::endl;
        std::cout << "\tInstrument ID: " << card.instrument_id << std::endl;
        std::cout << "\tLast four: " << card.last_four << std::endl;
        std::cout << "\tName on card: " << card.name_on_card << std::endl;
        std::cout << "\tNetwork: " << card.network << std::endl;
        std::cout << "\tNickname: " << card.nickname << std::endl;
        std::cout << "\tOrigin: " << card.origin << std::endl;
        std::cout << "\tProduct description: " << card.product_description
                  << std::endl;
        std::cout << "\tProduct terms URL: " << card.product_terms_url
                  << std::endl;
        std::cout << "\tStatus: " << card.status << std::endl;
        std::cout << "\tType: " << card.type << std::endl;
        std::cout << "\tVirtual card enrollment state: "
                  << card.virtual_card_enrollment_state << std::endl;
        std::cout << "\tVirtual card enrollment type: "
                  << card.virtual_card_enrollment_type << std::endl;
        std::cout << "\tUse count: " << card.use_count << std::endl;
        std::cout << "\tUse date: " << card.use_date << std::endl;
        std::cout << "\tUnmask date: " << card.unmask_date << std::endl;
        std::cout << "\tCard number: " << card.card_number << std::endl;
        std::cout << "\tCard number encrypted: "
                  << card.card_number_encrypted.dump () << std::endl;

        // show tags
        std::cout << "\tTags:" << std::endl;
        for (const auto &tag : card.tags)
        {
            std::cout << "\t\tDate modified: " << tag.date_modified
                      << std::endl;
            std::cout << "\t\tTag: " << tag.tag << std::endl;
            std::cout << "\t\tEncrypted tag: " << tag.tag_encrypted.dump ()
                      << std::endl;
        }
    }
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Main function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
main (int argc, char **argv)
{
    mobius::core::application app;
    mobius::core::set_logfile_path ("mobius.log");

    app.start ();

    std::cerr << app.get_name () << " v" << app.get_version () << std::endl;
    std::cerr << app.get_copyright () << std::endl;
    std::cerr << "Chromium Web Data viewer v1.0" << std::endl;
    std::cerr << "by Eduardo Aguiar" << std::endl;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Parse command line
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
        std::cerr << "Error: you must enter at least one path to Web Data file"
                  << std::endl;
        usage ();
        exit (EXIT_FAILURE);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Show info
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    while (optind < argc)
    {
        try
        {
            show_web_data (argv[optind]);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what () << std::endl;
            exit (EXIT_FAILURE);
        }

        optind++;
    }

    app.stop ();

    return EXIT_SUCCESS;
}
