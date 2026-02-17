#ifndef MOBIUS_EXTENSION_APP_SKYPE_PARSE_MESSAGE_HPP
#define MOBIUS_EXTENSION_APP_SKYPE_PARSE_MESSAGE_HPP

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
#include <mobius/core/pod/map.hpp>
#include <mobius/core/decoder/sgml/parser.hpp>
#include <mobius/core/richtext.hpp>
#include <string>
#include <vector>

namespace mobius::extension::app::skype
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Message parser class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class message_parser
{
  public:
    message_parser (const std::string &);
    void parse ();

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Add system element helper
    // @param text Text content
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    add_system_element (const std::string &text)
    {
        richtext_.add_system_text (text);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get richtext context
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::richtext
    get_richtext () const
    {
        return richtext_;
    }

  private:
    // @brief Richtext context
    mobius::core::richtext richtext_;

    // @brief SGML parser
    mobius::core::decoder::sgml::parser parser_;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Helper functions
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void _parse_start_tag (const std::string &tag);
    void _parse_end_tag (const std::string &tag);
    void _parse_empty_tag (const std::string &tag);
    void _parse_entity (const std::string &entity);

    void _parse_a ();
    void _parse_addmember ();
    void _parse_contacts ();
    void _parse_c_i ();
    void _parse_deletemember ();
    void _parse_files ();
    void _parse_flag ();
    void _parse_historydisclosedupdate ();
    void _parse_joiningenabledupdate ();
    void _parse_legacyquote ();
    void _parse_partlist ();
    void _parse_quote ();
    void _parse_sms ();
    void _parse_ss ();
    void _parse_topicupdate ();
    void _parse_uriobject ();
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Function prototypes
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
mobius::core::richtext parse_message (const std::string &);
mobius::core::richtext parse_notice (const std::string &);
mobius::core::richtext parse_popcard (const std::string &);

} // namespace mobius::extension::app::skype

#endif