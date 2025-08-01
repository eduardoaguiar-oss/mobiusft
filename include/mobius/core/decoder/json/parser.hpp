#ifndef MOBIUS_CORE_DECODER_JSON_PARSER_HPP
#define MOBIUS_CORE_DECODER_JSON_PARSER_HPP

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
#include <mobius/core/bytearray.hpp>
#include <mobius/core/decoder/json/tokenizer.hpp>
#include <mobius/core/io/reader.hpp>
#include <mobius/core/pod/data.hpp>
#include <mobius/core/pod/map.hpp>

namespace mobius::core::decoder::json
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief JSON parser
// @author Eduardo Aguiar
// @see https://www.json.org/json-en.html
// Input must be UTF-8 stream
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class parser
{
  public:
    explicit parser (const mobius::core::io::reader &);
    explicit parser (const mobius::core::bytearray &);
    mobius::core::pod::data parse ();

  private:
    // @brief Tokenizer object
    mobius::core::decoder::json::tokenizer tokenizer_;

    // Helper functions
    mobius::core::pod::map _decode_map ();
    mobius::core::pod::data _decode_array ();
    mobius::core::pod::data
    _get_token_data (tokenizer::token_type, const std::string &);
};

} // namespace mobius::core::decoder::json

#endif
