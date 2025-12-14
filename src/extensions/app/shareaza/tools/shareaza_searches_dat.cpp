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
#include "../CWndSearch.hpp"
#include <iostream>
#include <mobius/core/application.hpp>
#include <mobius/core/io/file.hpp>
#include <unistd.h>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief show usage text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
usage ()
{
    std::cerr << std::endl;
    std::cerr << "use: shareaza_searches_dat [OPTIONS] <path>" << std::endl;
    std::cerr << "e.g: shareaza_searches_dat Searches.dat" << std::endl;
    std::cerr << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show CXMLElement data
// @param indent Number of indentation tabs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
show_cxmlelement (const mobius::extension::app::shareaza::CXMLElement &e,
                  int indent)
{
    const std::string name = e.get_name ();

    if (name.empty ())
        return;

    std::cout << std::string (indent, '\t') << '<' << name;

    for (const auto &[k, v] : e.get_attributes ())
        std::cout << ' ' << k << "=\"" << v << '"';

    std::cout << '>' << e.get_value () << std::endl;

    for (const auto &child : e)
        show_cxmlelement (child, indent + 1);

    std::cout << std::string (indent, '\t') << "</" << name << '>' << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show CQueryHit data
// @param indent Number of indentation tabs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
show_cqueryhit (const mobius::extension::app::shareaza::CQueryHit &qh,
                int indent)
{
    const std::string indent_text = std::string (indent, '\t');

    std::cerr << std::endl;
    std::cerr << indent_text << "Name: " << qh.get_name () << std::endl;
    std::cerr << indent_text << "Size: " << qh.get_size () << std::endl;
    std::cerr << indent_text << "URL: " << qh.get_url () << std::endl;
    std::cerr << indent_text << "Protocol ID: " << qh.get_protocol_id ()
              << std::endl;
    std::cerr << indent_text << "Protocol name: " << qh.get_protocol_name ()
              << std::endl;
    std::cerr << indent_text << "Search ID: " << qh.get_search_id ()
              << std::endl;
    std::cerr << indent_text << "Client ID: " << qh.get_client_id ()
              << std::endl;
    std::cerr << indent_text << "IP: " << qh.get_ip () << std::endl;
    std::cerr << indent_text << "IPv6: " << qh.get_ipv6 () << std::endl;
    std::cerr << indent_text << "Port: " << qh.get_port () << std::endl;
    std::cerr << indent_text << "Nick: " << qh.get_nick () << std::endl;
    std::cerr << indent_text << "Speed: " << qh.get_speed () << std::endl;
    std::cerr << indent_text << "Str. speed: " << qh.get_str_speed ()
              << std::endl;
    std::cerr << indent_text << "Vendor code: " << qh.get_vendor_code ()
              << std::endl;
    std::cerr << indent_text << "Vendor name: " << qh.get_vendor_name ()
              << std::endl;
    std::cerr << indent_text << "Tri busy: " << qh.get_tri_busy () << std::endl;
    std::cerr << indent_text << "Tri push: " << qh.get_tri_push () << std::endl;
    std::cerr << indent_text << "Tri stable: " << qh.get_tri_stable ()
              << std::endl;
    std::cerr << indent_text << "Tri measured: " << qh.get_tri_measured ()
              << std::endl;
    std::cerr << indent_text << "Up slots: " << qh.get_up_slots () << std::endl;
    std::cerr << indent_text << "Up queue: " << qh.get_up_queue () << std::endl;
    std::cerr << indent_text << "Flag chat: " << qh.get_b_chat () << std::endl;
    std::cerr << indent_text << "Flag browse host: " << qh.get_b_browse_host ()
              << std::endl;
    std::cerr << indent_text << "Flag matched: " << qh.get_b_matched ()
              << std::endl;
    std::cerr << indent_text << "Flag size: " << qh.get_b_size () << std::endl;
    std::cerr << indent_text << "Flag preview: " << qh.get_b_preview ()
              << std::endl;
    std::cerr << indent_text << "Flag collection: " << qh.get_b_collection ()
              << std::endl;
    std::cerr << indent_text << "Flag bogus: " << qh.get_b_bogus ()
              << std::endl;
    std::cerr << indent_text << "Flag download: " << qh.get_b_download ()
              << std::endl;
    std::cerr << indent_text << "Flag exact match: " << qh.get_b_exact_match ()
              << std::endl;
    std::cerr << indent_text << "Hash SHA-1: " << qh.get_hash_sha1 ()
              << std::endl;
    std::cerr << indent_text << "Hash Tiger: " << qh.get_hash_tiger ()
              << std::endl;
    std::cerr << indent_text << "Hash ED2K: " << qh.get_hash_ed2k ()
              << std::endl;
    std::cerr << indent_text << "Hash BTH: " << qh.get_hash_bth () << std::endl;
    std::cerr << indent_text << "Hash MD5: " << qh.get_hash_md5 () << std::endl;
    std::cerr << indent_text << "Index: " << qh.get_index () << std::endl;
    std::cerr << indent_text << "Hit sources: " << qh.get_hit_sources ()
              << std::endl;
    std::cerr << indent_text << "Partial: " << qh.get_partial () << std::endl;
    std::cerr << indent_text << "Preview: " << qh.get_preview () << std::endl;
    std::cerr << indent_text << "Schema URI: " << qh.get_schema_uri ()
              << std::endl;
    std::cerr << indent_text << "Schema Plural: " << qh.get_schema_plural ()
              << std::endl;
    std::cerr << indent_text << "Rating: " << qh.get_rating () << std::endl;
    std::cerr << indent_text << "Comments: " << qh.get_comments () << std::endl;

    for (const auto &[k, v] : qh.get_xml_metadata ())
        std::cerr << indent_text << k << ": " << v << std::endl;

    std::cerr << indent_text << "Pxml:" << std::endl;
    show_cxmlelement (qh.get_pxml (), indent + 1);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show CQuerySearch data
// @param indent Number of indentation tabs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
show_cquerysearch (const mobius::extension::app::shareaza::CQuerySearch &qs,
                   int indent)
{
    const std::string indent_text = std::string (indent, '\t');

    std::cerr << indent_text << "Version: " << qs.get_version () << std::endl;
    std::cerr << indent_text << "GUID: " << qs.get_guid () << std::endl;
    std::cerr << indent_text << "Text: " << qs.get_text () << std::endl;
    std::cerr << indent_text << "Hash SHA-1: " << qs.get_hash_sha1 ()
              << std::endl;
    std::cerr << indent_text << "Hash Tiger: " << qs.get_hash_tiger ()
              << std::endl;
    std::cerr << indent_text << "Hash ED2K: " << qs.get_hash_ed2k ()
              << std::endl;
    std::cerr << indent_text << "Hash BTH: " << qs.get_hash_bth () << std::endl;
    std::cerr << indent_text << "Hash MD5: " << qs.get_hash_md5 () << std::endl;
    std::cerr << indent_text << "URI: " << qs.get_uri () << std::endl;
    std::cerr << indent_text << "PXML: " << std::endl;
    show_cxmlelement (qs.get_pxml (), indent + 1);
    std::cerr << indent_text << "Flag want URL: " << qs.get_flag_want_url ()
              << std::endl;
    std::cerr << indent_text << "Flag want DN: " << qs.get_flag_want_dn ()
              << std::endl;
    std::cerr << indent_text << "Flag want XML: " << qs.get_flag_want_xml ()
              << std::endl;
    std::cerr << indent_text << "Flag want COM: " << qs.get_flag_want_com ()
              << std::endl;
    std::cerr << indent_text << "Flag want PFS: " << qs.get_flag_want_pfs ()
              << std::endl;
    std::cerr << indent_text << "Min. size: " << qs.get_min_size ()
              << std::endl;
    std::cerr << indent_text << "Max. size: " << qs.get_max_size ()
              << std::endl;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show CManagedSearch data
// @param indent Number of indentation tabs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
show_cmanagedsearch (const mobius::extension::app::shareaza::CManagedSearch &ms,
                     int indent)
{
    const std::string indent_text = std::string (indent, '\t');

    std::cerr << std::endl;
    std::cerr << indent_text << "Version: " << ms.get_version () << std::endl;
    std::cerr << indent_text << "Priority: " << ms.get_priority () << std::endl;
    std::cerr << indent_text << "Flag active: " << ms.get_flag_active ()
              << std::endl;
    std::cerr << indent_text << "Flag receive: " << ms.get_flag_receive ()
              << std::endl;
    std::cerr << indent_text << "Allow G1: " << ms.get_allow_g1 () << std::endl;
    std::cerr << indent_text << "Allow G2: " << ms.get_allow_g2 () << std::endl;
    std::cerr << indent_text << "Allow ED2K: " << ms.get_allow_ed2k ()
              << std::endl;
    std::cerr << indent_text << "Allow DC: " << ms.get_allow_dc () << std::endl;

    std::cerr << indent_text << "Query Search: " << std::endl;
    show_cquerysearch (ms.get_query_search (), 3);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show CMatchFile data
// @param indent Number of indentation tabs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
show_cmatchfile (const mobius::extension::app::shareaza::CMatchFile &mf)
{
    std::cerr << std::endl;
    std::cerr << "\t\t\tSize: " << mf.get_size () << std::endl;
    std::cerr << "\t\t\tHash SHA-1: " << mf.get_hash_sha1 () << std::endl;
    std::cerr << "\t\t\tHash Tiger: " << mf.get_hash_tiger () << std::endl;
    std::cerr << "\t\t\tHash ED2K: " << mf.get_hash_ed2k () << std::endl;
    std::cerr << "\t\t\tHash BTH: " << mf.get_hash_bth () << std::endl;
    std::cerr << "\t\t\tHash MD5: " << mf.get_hash_md5 () << std::endl;
    std::cerr << "\t\t\tTri busy: " << mf.get_tri_busy () << std::endl;
    std::cerr << "\t\t\tTri push: " << mf.get_tri_push () << std::endl;
    std::cerr << "\t\t\tTri stable: " << mf.get_tri_stable () << std::endl;
    std::cerr << "\t\t\tSpeed: " << mf.get_speed () << std::endl;
    std::cerr << "\t\t\tStr. speed: " << mf.get_str_speed () << std::endl;
    std::cerr << "\t\t\tFlag expanded: " << mf.get_flag_expanded ()
              << std::endl;
    std::cerr << "\t\t\tFlag existing: " << mf.get_flag_existing ()
              << std::endl;
    std::cerr << "\t\t\tFlag download: " << mf.get_flag_download ()
              << std::endl;
    std::cerr << "\t\t\tFlag one valid: " << mf.get_flag_one_valid ()
              << std::endl;
    std::cerr << "\t\t\tFound time: " << mf.get_found_time () << std::endl;

    std::cerr << "\t\t\tPreview:" << std::endl;
    std::cerr << mf.get_preview ().dump (24) << std::endl;

    for (const auto &qh : mf.get_query_hits ())
        show_cqueryhit (qh, 4);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show CMatchList data
// @param indent Number of indentation tabs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
show_cmatchlist (const mobius::extension::app::shareaza::CMatchList &ml)
{
    std::cerr << std::endl;
    std::cerr << "\t\tVersion: " << ml.get_version () << std::endl;
    std::cerr << "\t\tFilter name: " << ml.get_filter_name () << std::endl;
    std::cerr << "\t\tFilter busy: " << ml.get_filter_busy () << std::endl;
    std::cerr << "\t\tFilter push: " << ml.get_filter_push () << std::endl;
    std::cerr << "\t\tFilter unstable: " << ml.get_filter_unstable ()
              << std::endl;
    std::cerr << "\t\tFilter reject: " << ml.get_filter_reject () << std::endl;
    std::cerr << "\t\tFilter local: " << ml.get_filter_local () << std::endl;
    std::cerr << "\t\tFilter bogus: " << ml.get_filter_bogus () << std::endl;
    std::cerr << "\t\tFilter drm: " << ml.get_filter_drm () << std::endl;
    std::cerr << "\t\tFilter adult: " << ml.get_filter_adult () << std::endl;
    std::cerr << "\t\tFilter suspicious: " << ml.get_filter_suspicious ()
              << std::endl;
    std::cerr << "\t\tRegexp: " << ml.get_regexp () << std::endl;
    std::cerr << "\t\tFilter min. size: " << ml.get_filter_min_size ()
              << std::endl;
    std::cerr << "\t\tFilter max. size: " << ml.get_filter_max_size ()
              << std::endl;
    std::cerr << "\t\tFilter sources: " << ml.get_filter_sources ()
              << std::endl;
    std::cerr << "\t\tSort column: " << ml.get_sort_column () << std::endl;
    std::cerr << "\t\tSort dir: " << ml.get_sort_dir () << std::endl;

    for (const auto &mf : ml.get_match_files ())
        show_cmatchfile (mf);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show CWndSearch data
// @param indent Number of indentation tabs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void
show_cwndsearch (const mobius::extension::app::shareaza::CWndSearch &ws)
{
    std::cerr << ">> CWndSearch" << std::endl;
    std::cerr << "\tVersion: " << ws.get_version () << std::endl;
    std::cerr << "\tSchema: " << ws.get_schema () << std::endl;
    std::cerr << "\tSearches:" << std::endl;

    for (const auto &ms : ws.get_searches ())
        show_cmanagedsearch (ms, 2);

    std::cerr << std::endl;
    std::cerr << "\tMatch List:" << std::endl;
    show_cmatchlist (ws.get_match_list ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Show Shareaza Searches.dat info
// @param path Searches.dat path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
show_searches_dat (const std::string &path)
{
    std::cout << std::endl;
    std::cout << ">> " << path << std::endl;

    auto f = mobius::core::io::new_file_by_path (path);
    auto reader = f.new_reader ();
    auto decoder = mobius::core::decoder::mfc (reader);

    std::uint32_t count = decoder.get_count ();

    while (count == 1)
    {
        mobius::extension::app::shareaza::CWndSearch cwndsearch;
        cwndsearch.decode (decoder);

        show_cwndsearch (cwndsearch);

        count = decoder.get_count ();
    }
}

} // namespace

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief main function
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int
main (int argc, char **argv)
{
    mobius::core::application app;
    std::cerr << app.get_name () << " v" << app.get_version () << std::endl;
    std::cerr << app.get_copyright () << std::endl;
    std::cerr << "Shareaza Searches.dat viewer v1.0" << std::endl;
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

    if (optind >= argc)
    {
        std::cerr << std::endl;
        std::cerr
            << "Error: you must enter at least one path to Searches.dat file"
            << std::endl;
        usage ();
        exit (EXIT_FAILURE);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // show hive info
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    while (optind < argc)
    {
        try
        {
            show_searches_dat (argv[optind]);
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
