// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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
#include "ctag.hpp"
#include <mobius/core/log.h>
#include <mobius/string_functions.h>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Tag types
// @see srchybrid/opcodes.h
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
constexpr std::uint8_t TAGTYPE_HASH = 0x01;
constexpr std::uint8_t TAGTYPE_STRING = 0x02;
constexpr std::uint8_t TAGTYPE_UINT32 = 0x03;
constexpr std::uint8_t TAGTYPE_FLOAT32 = 0x04;
constexpr std::uint8_t TAGTYPE_BOOL = 0x05;
constexpr std::uint8_t TAGTYPE_BOOLARRAY = 0x06;
constexpr std::uint8_t TAGTYPE_BLOB = 0x07;
constexpr std::uint8_t TAGTYPE_UINT16 = 0x08;
constexpr std::uint8_t TAGTYPE_UINT8 = 0x09;
constexpr std::uint8_t TAGTYPE_BSOB = 0x0A;
constexpr std::uint8_t TAGTYPE_UINT64 = 0x0B;
constexpr std::uint8_t TAGTYPE_STR1 = 0x11;
constexpr std::uint8_t TAGTYPE_STR2 = 0x12;
constexpr std::uint8_t TAGTYPE_STR3 = 0x13;
constexpr std::uint8_t TAGTYPE_STR4 = 0x14;
constexpr std::uint8_t TAGTYPE_STR5 = 0x15;
constexpr std::uint8_t TAGTYPE_STR6 = 0x16;
constexpr std::uint8_t TAGTYPE_STR7 = 0x17;
constexpr std::uint8_t TAGTYPE_STR8 = 0x18;
constexpr std::uint8_t TAGTYPE_STR9 = 0x19;
constexpr std::uint8_t TAGTYPE_STR10 = 0x1A;
constexpr std::uint8_t TAGTYPE_STR11 = 0x1B;
constexpr std::uint8_t TAGTYPE_STR12 = 0x1C;
constexpr std::uint8_t TAGTYPE_STR13 = 0x1D;
constexpr std::uint8_t TAGTYPE_STR14 = 0x1E;
constexpr std::uint8_t TAGTYPE_STR15 = 0x1F;
constexpr std::uint8_t TAGTYPE_STR16 = 0x20;

} // namespace

namespace mobius::extension::app::emule
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param decoder Decoder object
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ctag::ctag (mobius::decoder::data_decoder& decoder)
{
  mobius::core::log log (__FILE__, __FUNCTION__);

log.debug (__LINE__, "POS=" + std::to_string (decoder.tell ()));
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Read tag id and name
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  type_ = decoder.get_uint8 ();

  if (type_ & 0x80)
    {
      type_ &= 0x7f;
      id_ = decoder.get_uint8 ();
    }

  else
    {
      auto length = decoder.get_uint16_le ();

      if (length == 1)
        id_ = decoder.get_uint8 ();
        
      else
        name_ = decoder.get_string_by_size (length);
    }

log.debug (__LINE__, "TYPE=" + std::to_string (type_));
log.debug (__LINE__, "ID=" + std::to_string (id_));
log.debug (__LINE__, "NAME=" + name_);

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Read tag value
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  switch (type_)
    {
      case TAGTYPE_HASH:
            value_ = decoder.get_hex_string_by_size (16);
            break;

      case TAGTYPE_STRING:
        {
            auto length = decoder.get_uint16_le ();
            value_ = decoder.get_string_by_size (length, "utf-8");
            break;
        }

      case TAGTYPE_UINT32:
            value_ = static_cast <std::int64_t> (decoder.get_uint32_le ());
            break;

      case TAGTYPE_FLOAT32:
            log.development (__LINE__, "TAGTYPE_FLOAT32 not implemented");
            break;

      case TAGTYPE_BOOL:
            value_ = decoder.get_uint8 () == 1;
            break;

      case TAGTYPE_BOOLARRAY:
        {
            auto length = decoder.get_uint16_le ();
            decoder.skip (length / 8 + 1);
            // @todo 07-Apr-2004: eMule versions prior to 0.42e.29 used the formula "(len+7)/8"!
            break;
        }

      case TAGTYPE_BLOB:
        {
            // @todo 07-Apr-2004: eMule versions prior to 0.42e.29 handled the "len" as int16!
            auto size = decoder.get_uint32_le ();
            value_ = decoder.get_bytearray_by_size (size);
            break;
        }

      case TAGTYPE_UINT16:
            value_ = decoder.get_uint16_le ();
            break;

      case TAGTYPE_UINT8:
            value_ = decoder.get_uint8 ();
            break;

      case TAGTYPE_BSOB:
            // @todo implement
            log.development (__LINE__, "TAGTYPE_BSOB not implemented");
            break;

      case TAGTYPE_UINT64:
            value_ = static_cast <std::int64_t> (decoder.get_uint64_le ());
            break;
            
      case TAGTYPE_STR1: [[fallthrough]]
      case TAGTYPE_STR2: [[fallthrough]]
      case TAGTYPE_STR3: [[fallthrough]]
      case TAGTYPE_STR4: [[fallthrough]]
      case TAGTYPE_STR5: [[fallthrough]]
      case TAGTYPE_STR6: [[fallthrough]]
      case TAGTYPE_STR7: [[fallthrough]]
      case TAGTYPE_STR8: [[fallthrough]]
      case TAGTYPE_STR9: [[fallthrough]]
      case TAGTYPE_STR10: [[fallthrough]]
      case TAGTYPE_STR11: [[fallthrough]]
      case TAGTYPE_STR12: [[fallthrough]]
      case TAGTYPE_STR13: [[fallthrough]]
      case TAGTYPE_STR14: [[fallthrough]]
      case TAGTYPE_STR15: [[fallthrough]]
      case TAGTYPE_STR16:
        {
            std::uint32_t length = type_ - TAGTYPE_STR1 + 1;
log.debug(__LINE__, "LENGTH=" + std::to_string (length));          
            value_ = decoder.get_string_by_size (length);
            type_ = TAGTYPE_STRING;
            break;
        }
        
      default:
            log.development (__LINE__, "Unknown tag type: 0x" + mobius::string::to_hex (type_, 2));
    };
}

} // namespace mobius::extension::app::emule
