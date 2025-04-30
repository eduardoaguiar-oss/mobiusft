#ifndef MOBIUS_CORE_ENCODER_DATA_ENCODER_HPP
#define MOBIUS_CORE_ENCODER_DATA_ENCODER_HPP

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
#include <mobius/core/io/writer.hpp>
#include <mobius/core/bytearray.hpp>
#include <cstdint>

namespace mobius::core::encoder
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Data encoder class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class data_encoder
{
public:
  explicit data_encoder (const mobius::core::io::writer&);
  explicit data_encoder (mobius::core::bytearray&);
  void encode_uint8 (std::uint8_t);
  void encode_uint16_le (std::uint16_t);
  void encode_uint32_le (std::uint32_t);
  void encode_uint64_le (std::uint64_t);

  void encode_int64_le (std::int64_t);
  void encode_string_by_size (const std::string&, std::size_t);
  void encode_bytearray (const mobius::core::bytearray&);
  void fill (std::size_t, std::uint8_t);

private:
  mobius::core::io::writer writer_;
};

} // namespace mobius::core::encoder

#endif




